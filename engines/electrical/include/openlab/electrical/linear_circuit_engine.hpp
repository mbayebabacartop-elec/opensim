#pragma once

#include "openlab/core/units.hpp"

#include <atomic>
#include <variant>
#include <vector>

namespace openlab::electrical {

using NodeId = unsigned int;
inline constexpr NodeId groundNode = 0;

struct Resistor { NodeId positive{}; NodeId negative{}; openlab::core::Quantity resistance; };
struct Capacitor {
    NodeId positive{}; NodeId negative{};
    openlab::core::Quantity capacitance;
    openlab::core::Quantity initialVoltage;
};
struct Inductor {
    NodeId positive{}; NodeId negative{};
    openlab::core::Quantity inductance;
    openlab::core::Quantity initialCurrent;
};
struct DcVoltageSource { NodeId positive{}; NodeId negative{}; openlab::core::Quantity voltage; };
using LinearElement = std::variant<Resistor, Capacitor, Inductor, DcVoltageSource>;

struct LinearTransientCase {
    unsigned int nodeCount{}; // Includes node 0, which is ground.
    std::vector<LinearElement> elements;
    double endTimeSeconds{};
    double maximumStepSeconds{};
};

struct LinearCircuitSample {
    double timeSeconds{};
    std::vector<openlab::core::Quantity> nodeVoltages; // Index by NodeId; node 0 is 0 V.
};

struct LinearTransientResult { std::vector<LinearCircuitSample> samples; };

// Linear resistor/capacitor/inductor/DC-source network using MNA and backward Euler.
[[nodiscard]] openlab::core::Result<LinearTransientResult> simulateLinearTransient(
    const LinearTransientCase& circuit, const std::atomic_bool* cancellationRequested = nullptr);

} // namespace openlab::electrical
