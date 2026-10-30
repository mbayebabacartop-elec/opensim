#pragma once

#include "openlab/core/units.hpp"

#include <atomic>
#include <vector>

namespace openlab::electrical {

struct DcMotorCase {
    openlab::core::Quantity resistance;
    openlab::core::Quantity inductance;
    openlab::core::Quantity backEmfConstant;
    openlab::core::Quantity torqueConstant;
    openlab::core::Quantity inertia;
    openlab::core::Quantity viscousFriction;
    openlab::core::Quantity loadTorque;
    openlab::core::Quantity sourceVoltage;
    openlab::core::Quantity initialCurrent;
    openlab::core::Quantity initialAngularVelocity;
    double endTimeSeconds{};
    double maximumStepSeconds{};
};

struct DcMotorSample {
    double timeSeconds{};
    openlab::core::Quantity current;
    openlab::core::Quantity angularVelocity;
    openlab::core::Quantity electromagneticTorque;
};

struct DcMotorResult {
    std::vector<DcMotorSample> samples;
};

// Coupled ideal permanent-magnet DC motor using simultaneous backward Euler.
[[nodiscard]] openlab::core::Result<DcMotorResult> simulateDcMotor(
    const DcMotorCase& motor, const std::atomic_bool* cancellationRequested = nullptr);

} // namespace openlab::electrical
