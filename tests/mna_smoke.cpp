#include "openlab/electrical/linear_circuit_engine.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>

using namespace openlab;
namespace {
[[noreturn]] void fail(const char* message) { std::cerr << message << '\n'; std::exit(EXIT_FAILURE); }

electrical::LinearTransientCase rc(double step) {
    using namespace core::si;
    const auto capacitance = electricCurrent.multipliedBy(core::si::time).value().dividedBy(voltage).value();
    return {3, {electrical::DcVoltageSource{1,0,{5.0,voltage}}, electrical::Resistor{1,2,{1000.0,resistance}},
        electrical::Capacitor{2,0,{1e-6,capacitance},{0.0,voltage}}}, 0.003, step};
}
}

int main() {
    auto coarse = electrical::simulateLinearTransient(rc(0.001));
    auto fine = electrical::simulateLinearTransient(rc(0.0001));
    if (!coarse) { std::cerr << coarse.error().message << '\n'; fail("coarse RC MNA transient failed"); }
    if (!fine) { std::cerr << fine.error().message << '\n'; fail("fine RC MNA transient failed"); }
    if (coarse.value().samples.size()!=3 || fine.value().samples.size()!=30) fail("RC MNA sample count mismatch");
    const double coarseVoltage=coarse.value().samples.back().nodeVoltages[2].si_value;
    const double fineVoltage=fine.value().samples.back().nodeVoltages[2].si_value;
    const double analytic=5.0*(1.0-std::exp(-3.0));
    if (std::abs(coarseVoltage-4.375)>1e-12 || !(std::abs(fineVoltage-analytic)<std::abs(coarseVoltage-analytic)))
        fail("RC backward-Euler solution or refinement check failed");

    using namespace core::si;
    electrical::LinearTransientCase rl{3,{electrical::DcVoltageSource{1,0,{5.0,voltage}},
        electrical::Resistor{1,2,{5.0,resistance}}, electrical::Inductor{2,0,{0.5,inductance},{0.0,electricCurrent}}},0.5,0.001};
    auto rlResult=electrical::simulateLinearTransient(rl);
    if(!rlResult || rlResult.value().samples.size()!=500 || rlResult.value().samples.back().nodeVoltages[2].si_value<0.0 ||
       rlResult.value().samples.back().nodeVoltages[2].si_value>0.1) fail("MNA inductor branch integration failed");

    electrical::LinearTransientCase floating{3,{electrical::Resistor{1,2,{1000.0,core::si::resistance}}},0.1,0.01};
    auto singular=electrical::simulateLinearTransient(floating);
    if (singular || singular.error().code!=core::ErrorCode::Conflict) fail("floating MNA network was not rejected");

    auto invalid=rc(0.001);std::get<electrical::Resistor>(invalid.elements[1]).resistance.dimension=core::si::voltage;
    if (electrical::simulateLinearTransient(invalid)) fail("MNA element with incompatible dimensions was accepted");
    std::cout << "RC MNA transient, analytic response, step refinement and topology validation passed\n";
    return EXIT_SUCCESS;
}
