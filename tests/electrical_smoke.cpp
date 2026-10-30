#include "openlab/electrical/rl_engine.hpp"

#include <cmath>
#include <iostream>

using namespace openlab::core;
using namespace openlab::electrical;
namespace { int fail(const char* message) { std::cerr << message << '\n'; return 1; } }

int main() {
    const SeriesRlStepCase base{{10.0, si::resistance}, {0.1, si::inductance},
        {5.0, si::voltage}, {0.0, si::electricCurrent}, 0.05, 0.001};
    auto coarse = simulateSeriesRlStep(base);
    if (!coarse) { std::cerr << coarse.error().message << '\n'; return fail("Coarse RL solve failed"); }
    auto fineCase = base;
    fineCase.maximumStepSeconds = 0.0005;
    auto fine = simulateSeriesRlStep(fineCase);
    if (!fine) return fail("Fine RL solve failed");

    const double exact = 0.5 * (1.0 - std::exp(-5.0));
    const double coarseError = std::abs(coarse.value().samples.back().current.si_value - exact);
    const double fineError = std::abs(fine.value().samples.back().current.si_value - exact);
    if (std::abs(coarse.value().timeConstantSeconds - 0.01) > 1e-14) return fail("RL time constant does not match L/R");
    if (coarse.value().samples.size() != 51 || fine.value().samples.size() != 101) return fail("RL sample count or endpoint is incorrect");
    if (!(fineError < coarseError && coarseError / fineError > 1.8 && coarseError / fineError < 2.2)) return fail("Backward Euler refinement did not show first-order convergence");
    if (fineError > 0.0005) return fail("Fine RL result exceeds analytical error tolerance");
    for (const auto& sample : fine.value().samples) {
        const double residual = sample.resistorVoltage.si_value + sample.inductorVoltage.si_value - 5.0;
        if (std::abs(residual) > 1e-10) return fail("KVL residual exceeds tolerance");
    }

    auto wrongUnits = base;
    wrongUnits.inductance = {0.1, si::resistance};
    const auto rejected = simulateSeriesRlStep(wrongUnits);
    if (rejected || rejected.error().code != ErrorCode::InvalidArgument) return fail("Incorrect inductance dimension was not rejected");

    auto invalidR = base;
    invalidR.resistance.si_value = 0.0;
    if (simulateSeriesRlStep(invalidR)) return fail("Non-positive resistance was not rejected");
    std::cout << "RL tau=" << coarse.value().timeConstantSeconds << " s; coarse error=" << coarseError
              << " A; fine error=" << fineError << " A; error ratio=" << coarseError / fineError << '\n';
    return 0;
}
