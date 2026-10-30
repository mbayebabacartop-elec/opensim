#include "lab_controller.hpp"

#include <QCoreApplication>
#include <QEventLoop>
#include <QTimer>

#include <filesystem>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

namespace {
int fail(const char* message) { std::cerr << message << '\n'; return 1; }
}

int main(int argc, char* argv[]) {
    QCoreApplication application(argc, argv);
    openlab::app::LabController controller;
    controller.runOrCancel();

    QEventLoop loop;
    bool timedOut = false;
    QObject::connect(&controller, &openlab::app::LabController::runningChanged, &loop, [&] {
        if (!controller.running()) loop.quit();
    });
    QTimer::singleShot(10000, &loop, [&] { timedOut = true; loop.quit(); });
    loop.exec();
    if (timedOut || controller.running()) return fail("Controller simulation did not complete in time");
    if (controller.currentSamples().size() < 2 || controller.sampleCount() != 101 || std::abs(controller.timeConstantSeconds() - 0.01) > 1e-14 || std::abs(controller.finalCurrentAmps() - 0.496197755001063) > 1e-9) return fail("Controller did not publish the computed RL result");

    const auto base = std::filesystem::temp_directory_path();
    const auto projectPath = base / "openlab-controller-smoke.olsim";
    const auto csvPath = base / "openlab-controller-smoke.csv";
    const auto projectUrl = QUrl::fromLocalFile(QString::fromStdWString(projectPath.wstring()));
    const auto csvUrl = QUrl::fromLocalFile(QString::fromStdWString(csvPath.wstring()));
    controller.saveProject(projectUrl);
    if (controller.statusText().contains(QStringLiteral("Échec"))) return fail("Controller could not save the RL project");

    openlab::app::LabController restored;
    restored.loadProject(projectUrl);
    if (restored.statusText().contains(QStringLiteral("Échec")) || std::abs(restored.resistanceOhms() - 10.0) > 1e-12 ||
        std::abs(restored.inductanceHenries() - 0.1) > 1e-12 || restored.currentSamples().size() != 0) return fail("Controller could not restore the saved setup");
    controller.exportCsv(csvUrl);
    std::ifstream csv(csvPath);
    std::string header;
    std::getline(csv, header);
    if (header != "time_s,current_A") return fail("Controller CSV export has an unexpected header");
    std::size_t rows = 0;
    std::string row;
    while (std::getline(csv, row)) ++rows;
    if (rows != 101) return fail("CSV export did not preserve every raw simulation sample");

    timedOut = false;
    controller.runMotorSimulation();
    loop.exec();
    if (timedOut || controller.running() || controller.motorSampleCount() != 1001 ||
        controller.motorSamples().size() < 2 || controller.motorCurrentAmps() <= 0.0 ||
        controller.motorAngularVelocity() <= 0.0 || controller.motorTorqueNewtonMetres() <= 0.0 ||
        controller.motorElectricalPowerWatts() <= 0.0 || controller.motorMechanicalPowerWatts() <= 0.0 ||
        controller.motorEfficiencyPercent() <= 0.0 || controller.motorBackEmfVolts() <= 0.0)
        return fail("Controller did not publish the coupled DC motor simulation");

    controller.setMotorPowered(false);
    controller.setMotorLoadTorque(0.0);
    timedOut = false;
    controller.runMotorSimulation();
    loop.exec();
    if (timedOut || controller.running() || controller.motorSampleCount() != 1001 ||
        std::abs(controller.motorCurrentAmps()) > 1e-9 || std::abs(controller.motorAngularVelocity()) > 1e-9)
        return fail("Controller did not handle unpowered DC motor state correctly");
    controller.setMotorPowered(true);
    controller.setMotorLoadTorque(0.03);

    timedOut = false;
    controller.runMnaSimulation();
    loop.exec();
    if (timedOut || controller.running() || controller.mnaSamples().size() != 101 ||
        controller.mnaSampleCount() != 101 || controller.mnaFinalVoltage() < 4.99 || controller.mnaFinalVoltage() > 5.01)
        return fail("Controller did not publish the computed MNA circuit result");

    std::error_code ec;
    std::filesystem::remove(projectPath, ec);
    std::filesystem::remove(csvPath, ec);
    return 0;
}
