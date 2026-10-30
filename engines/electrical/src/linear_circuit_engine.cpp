#include "openlab/electrical/linear_circuit_engine.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace openlab::electrical {
namespace {
using namespace openlab::core;
constexpr std::size_t maxUnknowns = 256;
constexpr std::size_t maxSamples = 1'000'001;

std::size_t branchCount(const LinearTransientCase& c) {
    return static_cast<std::size_t>(std::count_if(c.elements.begin(), c.elements.end(), [](const auto& element) {
        return std::holds_alternative<Inductor>(element) || std::holds_alternative<DcVoltageSource>(element);
    }));
}

Result<void> validate(const LinearTransientCase& c) {
    if (c.nodeCount < 2 || c.nodeCount > maxUnknowns || c.elements.empty())
        return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "MNA circuit needs ground, at least one node, and at least one element."));
    if (static_cast<std::size_t>(c.nodeCount - 1) + branchCount(c) > maxUnknowns)
        return Result<void>::failure(makeError(ErrorCode::ResourceLimit, "MNA circuit exceeds the 256-unknown limit."));
    if (!std::isfinite(c.endTimeSeconds) || !std::isfinite(c.maximumStepSeconds) || c.endTimeSeconds <= 0.0 || c.maximumStepSeconds <= 0.0)
        return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "MNA duration and maximum step must be positive finite SI seconds."));
    const double ratio = c.endTimeSeconds / c.maximumStepSeconds;
    if (!std::isfinite(ratio) || ratio > static_cast<double>(maxSamples - 1))
        return Result<void>::failure(makeError(ErrorCode::ResourceLimit, "MNA sample count exceeds the one-million-step limit."));

    const auto capacitanceDimension = si::electricCurrent.multipliedBy(si::time).value().dividedBy(si::voltage).value();
    for (const auto& element : c.elements) {
        const bool valid = std::visit([&](const auto& e) {
            using T = std::decay_t<decltype(e)>;
            if (e.positive >= c.nodeCount || e.negative >= c.nodeCount || e.positive == e.negative) return false;
            if constexpr (std::is_same_v<T, Resistor>)
                return e.resistance.isFinite() && e.resistance.hasDimension(si::resistance) && e.resistance.si_value > 0.0;
            else if constexpr (std::is_same_v<T, Capacitor>)
                return e.capacitance.isFinite() && e.capacitance.hasDimension(capacitanceDimension) && e.capacitance.si_value > 0.0 &&
                    e.initialVoltage.isFinite() && e.initialVoltage.hasDimension(si::voltage);
            else if constexpr (std::is_same_v<T, Inductor>)
                return e.inductance.isFinite() && e.inductance.hasDimension(si::inductance) && e.inductance.si_value > 0.0 &&
                    e.initialCurrent.isFinite() && e.initialCurrent.hasDimension(si::electricCurrent);
            else
                return e.voltage.isFinite() && e.voltage.hasDimension(si::voltage);
        }, element);
        if (!valid) return Result<void>::failure(makeError(ErrorCode::Incompatible, "MNA element has invalid terminals, parameters, or SI dimensions."));
    }
    return Result<void>::success();
}

bool solve(std::vector<double>& matrix, std::vector<double>& rhs, std::size_t n) {
    for (std::size_t col = 0; col < n; ++col) {
        std::size_t pivot = col;
        double magnitude = std::abs(matrix[col * n + col]);
        for (std::size_t row = col + 1; row < n; ++row) {
            const double candidate = std::abs(matrix[row * n + col]);
            if (candidate > magnitude) { magnitude = candidate; pivot = row; }
        }
        if (!std::isfinite(magnitude) || magnitude <= std::numeric_limits<double>::epsilon() * 64.0) return false;
        if (pivot != col) {
            for (std::size_t j = col; j < n; ++j) std::swap(matrix[pivot * n + j], matrix[col * n + j]);
            std::swap(rhs[pivot], rhs[col]);
        }
        const double diagonal = matrix[col * n + col];
        for (std::size_t row = col + 1; row < n; ++row) {
            const double factor = matrix[row * n + col] / diagonal;
            matrix[row * n + col] = 0.0;
            for (std::size_t j = col + 1; j < n; ++j) matrix[row * n + j] -= factor * matrix[col * n + j];
            rhs[row] -= factor * rhs[col];
        }
    }
    for (std::size_t row = n; row-- > 0;) {
        double value = rhs[row];
        for (std::size_t col = row + 1; col < n; ++col) value -= matrix[row * n + col] * rhs[col];
        rhs[row] = value / matrix[row * n + row];
        if (!std::isfinite(rhs[row])) return false;
    }
    return true;
}

void stampConductance(std::vector<double>& matrix, std::size_t n, NodeId p, NodeId q, double g) {
    if (p) matrix[(p - 1) * n + (p - 1)] += g;
    if (q) matrix[(q - 1) * n + (q - 1)] += g;
    if (p && q) { matrix[(p - 1) * n + (q - 1)] -= g; matrix[(q - 1) * n + (p - 1)] -= g; }
}
void stampCurrent(std::vector<double>& rhs, NodeId p, NodeId q, double fromPositiveToNegative) {
    if (p) rhs[p - 1] -= fromPositiveToNegative;
    if (q) rhs[q - 1] += fromPositiveToNegative;
}
} // namespace

openlab::core::Result<LinearTransientResult> simulateLinearTransient(
    const LinearTransientCase& c, const std::atomic_bool* cancellationRequested) {
    using namespace openlab::core;
    auto valid = validate(c);
    if (!valid) return Result<LinearTransientResult>::failure(valid.error());

    const std::size_t nodeUnknowns = c.nodeCount - 1;
    const std::size_t unknowns = nodeUnknowns + branchCount(c);
    const double ratio = c.endTimeSeconds / c.maximumStepSeconds;
    const double ratioTolerance = std::numeric_limits<double>::epsilon() * 16.0 * std::max(1.0, ratio);
    const auto integrationSteps = static_cast<std::size_t>(std::ceil(ratio - ratioTolerance));
    LinearTransientResult result;
    result.samples.reserve(integrationSteps);

    std::vector<double> capacitorVoltage(c.elements.size(), 0.0);
    std::vector<double> inductorCurrent(c.elements.size(), 0.0);
    for (std::size_t i = 0; i < c.elements.size(); ++i) {
        if (const auto* cap = std::get_if<Capacitor>(&c.elements[i])) capacitorVoltage[i] = cap->initialVoltage.si_value;
        if (const auto* ind = std::get_if<Inductor>(&c.elements[i])) inductorCurrent[i] = ind->initialCurrent.si_value;
    }

    double time = 0.0;
    for (std::size_t stepIndex = 0; stepIndex < integrationSteps; ++stepIndex) {
        if (cancellationRequested && cancellationRequested->load(std::memory_order_relaxed))
            return Result<LinearTransientResult>::failure(makeError(ErrorCode::Conflict, "MNA transient simulation cancelled by the user."));
        const double h = stepIndex + 1 == integrationSteps ? c.endTimeSeconds - time : c.maximumStepSeconds;
        std::vector<double> matrix(unknowns * unknowns, 0.0), rhs(unknowns, 0.0);
        std::size_t branch = nodeUnknowns;
        for (std::size_t eIndex = 0; eIndex < c.elements.size(); ++eIndex) {
            const auto stampResult = std::visit([&](const auto& e) -> bool {
                using T = std::decay_t<decltype(e)>;
                if constexpr (std::is_same_v<T, Resistor>) {
                    const double g = 1.0 / e.resistance.si_value;
                    if (!std::isfinite(g)) return false;
                    stampConductance(matrix, unknowns, e.positive, e.negative, g);
                } else if constexpr (std::is_same_v<T, Capacitor>) {
                    const double g = e.capacitance.si_value / h;
                    if (!std::isfinite(g)) return false;
                    stampConductance(matrix, unknowns, e.positive, e.negative, g);
                    stampCurrent(rhs, e.positive, e.negative, -g * capacitorVoltage[eIndex]);
                } else {
                    const double impedance = [&] {
                        if constexpr (std::is_same_v<T, Inductor>) return e.inductance.si_value / h;
                        else return 0.0;
                    }();
                    if constexpr (std::is_same_v<T, Inductor> || std::is_same_v<T, DcVoltageSource>) {
                        const std::size_t p = e.positive, q = e.negative;
                        if (p) { matrix[(p - 1) * unknowns + branch] += 1.0; matrix[branch * unknowns + (p - 1)] += 1.0; }
                        if (q) { matrix[(q - 1) * unknowns + branch] -= 1.0; matrix[branch * unknowns + (q - 1)] -= 1.0; }
                        if constexpr (std::is_same_v<T, Inductor>) {
                            if (!std::isfinite(impedance)) return false;
                            matrix[branch * unknowns + branch] -= impedance;
                            rhs[branch] = -impedance * inductorCurrent[eIndex];
                        } else {
                            rhs[branch] = e.voltage.si_value;
                        }
                        ++branch;
                    }
                }
                return true;
            }, c.elements[eIndex]);
            if (!stampResult) return Result<LinearTransientResult>::failure(makeError(ErrorCode::InvalidArgument, "MNA matrix stamp overflowed numeric limits."));
        }
        if (!solve(matrix, rhs, unknowns))
            return Result<LinearTransientResult>::failure(makeError(ErrorCode::Conflict, "MNA system is singular or ill-conditioned; check grounding and topology."));
        std::vector<Quantity> voltages(c.nodeCount, Quantity{0.0, si::voltage});
        for (NodeId node = 1; node < c.nodeCount; ++node) voltages[node] = {rhs[node - 1], si::voltage};
        for (std::size_t i = 0; i < c.elements.size(); ++i) {
            if (const auto* cap = std::get_if<Capacitor>(&c.elements[i])) capacitorVoltage[i] = voltages[cap->positive].si_value - voltages[cap->negative].si_value;
        }
        std::size_t updateBranch = nodeUnknowns;
        for (std::size_t i = 0; i < c.elements.size(); ++i) {
            if (std::holds_alternative<Inductor>(c.elements[i])) inductorCurrent[i] = rhs[updateBranch++];
            else if (std::holds_alternative<DcVoltageSource>(c.elements[i])) ++updateBranch;
        }
        const double nextTime = time + h;
        if (!std::isfinite(nextTime) || nextTime <= time || result.samples.size() >= maxSamples)
            return Result<LinearTransientResult>::failure(makeError(ErrorCode::InvalidArgument, "MNA integration exceeded numeric resolution or sample limits."));
        time = stepIndex + 1 == integrationSteps ? c.endTimeSeconds : nextTime;
        result.samples.push_back({time, std::move(voltages)});
    }
    return Result<LinearTransientResult>::success(std::move(result));
}

} // namespace openlab::electrical
