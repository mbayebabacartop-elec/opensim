#include "openlab/electrical/dc_motor_engine.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>

using namespace openlab;

namespace {
[[noreturn]] void fail(const char* message) {
    std::cerr << message << '\n';
    std::exit(EXIT_FAILURE);
}

electrical::DcMotorCase testCase(double step) {
    using core::Quantity;
    using namespace core::si;
    const auto backEmf = voltage.multipliedBy(core::si::time).value();
    const auto torquePerAmp = torque.dividedBy(electricCurrent).value();
    return {{2.0, resistance}, {0.1, inductance}, {0.1, backEmf}, {0.1, torquePerAmp},
            {0.01, momentOfInertia}, {0.01, rotationalFriction}, {0.0, torque}, {12.0, voltage},
            {0.0, electricCurrent}, {0.0, angularVelocity}, 10.0, step};
}
}

int main() {
    auto zero = testCase(0.001);
    zero.sourceVoltage.si_value = 0.0;
    auto zeroResult = electrical::simulateDcMotor(zero);
    if (!zeroResult || zeroResult.value().samples.back().current.si_value != 0.0 ||
        zeroResult.value().samples.back().angularVelocity.si_value != 0.0) fail("zero-input DC motor equilibrium changed");

    auto coarse = electrical::simulateDcMotor(testCase(0.02));
    auto medium = electrical::simulateDcMotor(testCase(0.01));
    auto fine = electrical::simulateDcMotor(testCase(0.0005));
    if (!coarse || !medium || !fine) fail("valid DC motor simulation failed");
    const auto& end = fine.value().samples.back();
    if (std::abs(end.current.si_value - 4.0) > 0.01 || std::abs(end.angularVelocity.si_value - 40.0) > 0.1)
        fail("DC motor did not reach its analytically predicted steady state");
    const auto error = [](const electrical::DcMotorSample& s) {
        return std::hypot(s.current.si_value - 4.0, (s.angularVelocity.si_value - 40.0) / 10.0);
    };
    if (!(error(medium.value().samples.back()) < error(coarse.value().samples.back())))
        fail("smaller DC motor step did not improve the final-state error");

    auto invalid = testCase(0.01);
    invalid.inertia.dimension = core::si::mass;
    if (electrical::simulateDcMotor(invalid)) fail("dimensionally invalid inertia was accepted");

    std::cout << "DC motor implicit integration, equilibrium and step refinement passed\n";
    return EXIT_SUCCESS;
}
