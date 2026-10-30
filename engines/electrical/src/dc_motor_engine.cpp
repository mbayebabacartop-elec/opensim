#include "openlab/electrical/dc_motor_engine.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace openlab::electrical {
namespace {
using namespace openlab::core;
constexpr std::size_t maxSamples = 1'000'001;

Result<void> validate(const DcMotorCase& c) {
    if (!c.resistance.isFinite() || !c.resistance.hasDimension(si::resistance) || c.resistance.si_value <= 0.0 ||
        !c.inductance.isFinite() || !c.inductance.hasDimension(si::inductance) || c.inductance.si_value <= 0.0 ||
        !c.inertia.isFinite() || !c.inertia.hasDimension(si::momentOfInertia) || c.inertia.si_value <= 0.0)
        return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "DC motor R, L, and J must be finite, correctly dimensioned, and positive."));

    const auto emfDimension = si::voltage.multipliedBy(si::time);
    const auto torquePerAmpereDimension = si::torque.dividedBy(si::electricCurrent);
    if (!emfDimension || !torquePerAmpereDimension ||
        !c.backEmfConstant.isFinite() || !c.backEmfConstant.hasDimension(emfDimension.value()) || c.backEmfConstant.si_value < 0.0 ||
        !c.torqueConstant.isFinite() || !c.torqueConstant.hasDimension(torquePerAmpereDimension.value()) || c.torqueConstant.si_value < 0.0 ||
        !c.viscousFriction.isFinite() || !c.viscousFriction.hasDimension(si::rotationalFriction) || c.viscousFriction.si_value < 0.0 ||
        !c.loadTorque.isFinite() || !c.loadTorque.hasDimension(si::torque) ||
        !c.sourceVoltage.isFinite() || !c.sourceVoltage.hasDimension(si::voltage) ||
        !c.initialCurrent.isFinite() || !c.initialCurrent.hasDimension(si::electricCurrent) ||
        !c.initialAngularVelocity.isFinite() || !c.initialAngularVelocity.hasDimension(si::angularVelocity))
        return Result<void>::failure(makeError(ErrorCode::Incompatible, "DC motor parameters must be finite quantities with the documented SI dimensions."));

    if (!std::isfinite(c.endTimeSeconds) || !std::isfinite(c.maximumStepSeconds) || c.endTimeSeconds <= 0.0 || c.maximumStepSeconds <= 0.0)
        return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "DC motor duration and time step must be finite positive seconds."));
    const double ratio = c.endTimeSeconds / c.maximumStepSeconds;
    if (!std::isfinite(ratio) || ratio > static_cast<double>(maxSamples - 1))
        return Result<void>::failure(makeError(ErrorCode::ResourceLimit, "DC motor sample count exceeds the one-million-step limit."));
    return Result<void>::success();
}
} // namespace

openlab::core::Result<DcMotorResult> simulateDcMotor(const DcMotorCase& c, const std::atomic_bool* cancellationRequested) {
    using namespace openlab::core;
    auto valid = validate(c);
    if (!valid) return Result<DcMotorResult>::failure(valid.error());

    const double r = c.resistance.si_value, l = c.inductance.si_value;
    const double ke = c.backEmfConstant.si_value, kt = c.torqueConstant.si_value;
    const double inertia = c.inertia.si_value, friction = c.viscousFriction.si_value;
    const double load = c.loadTorque.si_value, voltage = c.sourceVoltage.si_value;
    const double ratio = c.endTimeSeconds / c.maximumStepSeconds;
    DcMotorResult result;
    result.samples.reserve(static_cast<std::size_t>(std::ceil(ratio)) + 1);

    double time = 0.0;
    double current = c.initialCurrent.si_value;
    double speed = c.initialAngularVelocity.si_value;
    result.samples.push_back({0.0, {current, si::electricCurrent}, {speed, si::angularVelocity}, {kt * current, si::torque}});
    while (time < c.endTimeSeconds) {
        if (cancellationRequested && cancellationRequested->load(std::memory_order_relaxed))
            return Result<DcMotorResult>::failure(makeError(ErrorCode::Conflict, "DC motor simulation cancelled by the user."));
        const double h = std::min(c.maximumStepSeconds, c.endTimeSeconds - time);
        const double a = r + l / h;
        const double b = ke;
        const double d = inertia / h + friction;
        const double determinant = a * d + b * kt;
        const double rhsI = voltage + (l / h) * current;
        const double rhsW = (inertia / h) * speed - load;
        const double nextCurrent = (rhsI * d - b * rhsW) / determinant;
        const double nextSpeed = (a * rhsW + kt * rhsI) / determinant;
        const double nextTime = time + h;
        const double nextTorque = kt * nextCurrent;
        if (!std::isfinite(determinant) || determinant <= 0.0 || !std::isfinite(nextCurrent) ||
            !std::isfinite(nextSpeed) || !std::isfinite(nextTorque) || !std::isfinite(nextTime) ||
            nextTime <= time || result.samples.size() >= maxSamples)
            return Result<DcMotorResult>::failure(makeError(ErrorCode::InvalidArgument, "DC motor integration exceeded numeric limits."));
        time = nextTime >= c.endTimeSeconds ? c.endTimeSeconds : nextTime;
        current = nextCurrent;
        speed = nextSpeed;
        result.samples.push_back({time, {current, si::electricCurrent}, {speed, si::angularVelocity}, {nextTorque, si::torque}});
    }
    return Result<DcMotorResult>::success(std::move(result));
}

} // namespace openlab::electrical
