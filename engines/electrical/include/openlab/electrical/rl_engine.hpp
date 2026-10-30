#pragma once

#include "openlab/core/units.hpp"

#include <atomic>
#include <vector>

namespace openlab::electrical {

struct SeriesRlStepCase {
    openlab::core::Quantity resistance;      // Total series resistance, ohm.
    openlab::core::Quantity inductance;      // Series inductance, henry.
    openlab::core::Quantity sourceVoltage;   // Ideal voltage step applied at t = 0, volt.
    openlab::core::Quantity initialCurrent; // Inductor current immediately before t = 0, ampere.
    double endTimeSeconds{};
    double maximumStepSeconds{};
};

struct RlSample {
    double timeSeconds{};
    openlab::core::Quantity current;
    openlab::core::Quantity resistorVoltage;
    openlab::core::Quantity inductorVoltage;
};

struct RlResult {
    double timeConstantSeconds{};
    std::vector<RlSample> samples;
};

// First-order backward-Euler simulation of an ideal series RL circuit under a DC step.
[[nodiscard]] openlab::core::Result<RlResult> simulateSeriesRlStep(
    const SeriesRlStepCase& testCase, const std::atomic_bool* cancellationRequested = nullptr);

} // namespace openlab::electrical
