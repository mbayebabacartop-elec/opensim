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

    std::error_code ec;
    std::filesystem::remove(projectPath, ec);
    std::filesystem::remove(csvPath, ec);
    return 0;
}
