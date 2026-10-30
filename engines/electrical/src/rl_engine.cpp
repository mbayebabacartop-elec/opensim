#include "openlab/electrical/rl_engine.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace openlab::electrical {
namespace {
using namespace openlab::core;
constexpr std::size_t maxSamples = 1'000'001;

Result<void> validate(const SeriesRlStepCase& c) {
    if (!c.resistance.isFinite() || !c.resistance.hasDimension(si::resistance) || c.resistance.si_value <= 0.0)
        return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "RL resistance must be finite, positive, and expressed in ohms."));
    if (!c.inductance.isFinite() || !c.inductance.hasDimension(si::inductance) || c.inductance.si_value <= 0.0)
        return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "RL inductance must be finite, positive, and expressed in henries."));
    if (!c.sourceVoltage.isFinite() || !c.sourceVoltage.hasDimension(si::voltage))
        return Result<void>::failure(makeError(ErrorCode::Incompatible, "RL source must be a finite voltage quantity."));
    if (!c.initialCurrent.isFinite() || !c.initialCurrent.hasDimension(si::electricCurrent))
        return Result<void>::failure(makeError(ErrorCode::Incompatible, "RL initial state must be a finite current quantity."));
    if (!std::isfinite(c.endTimeSeconds) || !std::isfinite(c.maximumStepSeconds) || c.endTimeSeconds <= 0.0 || c.maximumStepSeconds <= 0.0)
        return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "RL duration and maximum step must be finite and positive SI seconds."));
    const double ratio = c.endTimeSeconds / c.maximumStepSeconds;
    if (!std::isfinite(ratio) || ratio > static_cast<double>(maxSamples - 1))
        return Result<void>::failure(makeError(ErrorCode::ResourceLimit, "RL sample count exceeds the one-million-step limit."));
    return Result<void>::success();
}
} // namespace

openlab::core::Result<RlResult> simulateSeriesRlStep(const SeriesRlStepCase& c, const std::atomic_bool* cancellationRequested) {
    using namespace openlab::core;
    auto valid = validate(c);
    if (!valid) return Result<RlResult>::failure(valid.error());

    const double r = c.resistance.si_value;
    const double l = c.inductance.si_value;
    const double u = c.sourceVoltage.si_value;
    const double tau = l / r;
    if (!std::isfinite(tau) || tau <= 0.0)
        return Result<RlResult>::failure(makeError(ErrorCode::InvalidArgument, "RL time constant is outside the representable range."));

    const double ratio = c.endTimeSeconds / c.maximumStepSeconds;
    const auto estimatedSteps = static_cast<std::size_t>(std::ceil(ratio));
    RlResult result;
    result.timeConstantSeconds = tau;
    result.samples.reserve(estimatedSteps + 1);

    double time = 0.0;
    double current = c.initialCurrent.si_value;
    auto addSample = [&](double t, double i, double h) -> bool {
        const double vR = r * i;
        const double vL = h > 0.0 ? l * (i - (result.samples.empty() ? c.initialCurrent.si_value : result.samples.back().current.si_value)) / h : u - vR;
        if (!std::isfinite(vR) || !std::isfinite(vL)) return false;
        result.samples.push_back({t, {i, si::electricCurrent}, {vR, si::voltage}, {vL, si::voltage}});
        return true;
    };
    // At t=0, initialize the derivative from the circuit equation and preserve the supplied current.
    const double initialVL = u - r * current;
    if (!std::isfinite(initialVL)) return Result<RlResult>::failure(makeError(ErrorCode::InvalidArgument, "RL initial voltages overflow the numeric range."));
    result.samples.push_back({0.0, {current, si::electricCurrent}, {r * current, si::voltage}, {initialVL, si::voltage}});

    while (time < c.endTimeSeconds) {
        if (cancellationRequested && cancellationRequested->load(std::memory_order_relaxed))
            return Result<RlResult>::failure(makeError(ErrorCode::Conflict, "RL simulation cancelled by the user."));
        const double h = std::min(c.maximumStepSeconds, c.endTimeSeconds - time);
        const double denominator = 1.0 + h * r / l;
        const double nextCurrent = (current + h * u / l) / denominator;
        const double nextTime = time + h;
        if (!std::isfinite(nextCurrent) || !std::isfinite(nextTime) || nextTime <= time || result.samples.size() >= maxSamples)
            return Result<RlResult>::failure(makeError(ErrorCode::InvalidArgument, "RL integration lost numeric resolution or exceeded its sample limit."));
        current = nextCurrent;
        time = nextTime >= c.endTimeSeconds ? c.endTimeSeconds : nextTime;
        if (!addSample(time, current, h))
            return Result<RlResult>::failure(makeError(ErrorCode::InvalidArgument, "RL sample contains a non-finite value."));
    }
    return Result<RlResult>::success(std::move(result));
}

} // namespace openlab::electrical
