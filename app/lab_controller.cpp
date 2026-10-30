#include "lab_controller.hpp"

#include "openlab/core/project_model.hpp"
#include "openlab/core/project_store.hpp"

#include <QFile>
#include <QLocale>
#include <QMetaObject>
#include <QSaveFile>
#include <QTextStream>
#include <QUrl>

#include <algorithm>
#include <cmath>

namespace openlab::app {
namespace {
using namespace openlab::core;
constexpr auto experimentId = "org.openlab.experiment.series-rl.step.v1";

PortDefinition electricalPort(std::string id, std::string name) {
    return {std::move(id), std::move(name), ScientificDomain::Electrical, PortKind::Conserving,
            SignalDirection::Bidirectional, {}, si::voltage, si::electricCurrent};
}

ComponentDefinition sourceDefinition() {
    return {{"openlab.electrical.dc_step_source", "1.0"}, "Source de tension DC idéale", ScientificDomain::Electrical,
        {electricalPort("positive", "Positif"), electricalPort("negative", "Négatif")},
        {{"voltage", "Tension après échelon", si::voltage, 5.0, {}, {}},
         {"initial_current", "Courant initial", si::electricCurrent, 0.0, {}, {}},
         {"duration", "Durée", si::time, 0.05, 1e-9, {}},
         {"time_step", "Pas maximal", si::time, 0.0005, 1e-9, {}}}};
}
ComponentDefinition resistorDefinition() {
    return {{"openlab.electrical.resistor", "1.0"}, "Résistance idéale", ScientificDomain::Electrical,
        {electricalPort("a", "Borne A"), electricalPort("b", "Borne B")},
        {{"resistance", "Résistance", si::resistance, 10.0, 1e-12, {}}}};
}
ComponentDefinition inductorDefinition() {
    return {{"openlab.electrical.inductor", "1.0"}, "Inductance idéale", ScientificDomain::Electrical,
        {electricalPort("a", "Borne A"), electricalPort("b", "Borne B")},
        {{"inductance", "Inductance", si::inductance, 0.1, 1e-15, {}}}};
}

Result<ProjectModel> makeProject(double r, double l, double u, double i0, double duration, double step) {
    ProjectModel project(experimentId, "Banc d'essai RL série");
    const auto source = sourceDefinition(); const auto resistor = resistorDefinition(); const auto inductor = inductorDefinition();
    for (const auto* definition : {&source, &resistor, &inductor}) {
        const auto added = project.addDefinition(*definition);
        if (!added) return Result<ProjectModel>::failure(added.error());
    }
    for (auto component : {ComponentInstance{"source", source.ref, "Source DC", {
        {"voltage", {u, si::voltage}}, {"initial_current", {i0, si::electricCurrent}},
        {"duration", {duration, si::time}}, {"time_step", {step, si::time}}}},
        ComponentInstance{"resistor", resistor.ref, "R1", {{"resistance", {r, si::resistance}}}},
        ComponentInstance{"inductor", inductor.ref, "L1", {{"inductance", {l, si::inductance}}}}}) {
        const auto added = project.addComponent(std::move(component));
        if (!added) return Result<ProjectModel>::failure(added.error());
    }
    for (const auto& pair : {std::pair{PortRef{"source", "positive"}, PortRef{"resistor", "a"}},
                             std::pair{PortRef{"resistor", "b"}, PortRef{"inductor", "a"}},
                             std::pair{PortRef{"inductor", "b"}, PortRef{"source", "negative"}}}) {
        const auto connected = project.connect(pair.first, pair.second);
        if (!connected) return Result<ProjectModel>::failure(connected.error());
    }
    return Result<ProjectModel>::success(std::move(project));
}

bool parameter(const ProjectModel& project, const char* componentId, const char* id, Dimension dimension, double& output) {
    const auto component = project.components().find(componentId);
    if (component == project.components().end()) return false;
    const auto value = component->second.parameters.find(id);
    if (value == component->second.parameters.end() || !value->second.hasDimension(dimension) || !value->second.isFinite()) return false;
    output = value->second.si_value;
    return true;
}
} // namespace

LabController::LabController(QObject* parent) : QObject(parent) {}

LabController::~LabController() {
    cancelRequested_.store(true, std::memory_order_relaxed);
    joinWorker();
}

void LabController::joinWorker() {
    if (worker_.joinable()) worker_.join();
}

void LabController::setResistanceOhms(double value) { if (std::isfinite(value) && resistance_ != value) { resistance_ = value; samples_.clear(); rawSamples_.clear(); tau_ = 0; finalCurrent_ = 0; emit inputsChanged(); emit resultsChanged(); setStatus(QStringLiteral("Paramètres modifiés — relancez le calcul.")); } }
void LabController::setInductanceHenries(double value) { if (std::isfinite(value) && inductance_ != value) { inductance_ = value; samples_.clear(); rawSamples_.clear(); tau_ = 0; finalCurrent_ = 0; emit inputsChanged(); emit resultsChanged(); setStatus(QStringLiteral("Paramètres modifiés — relancez le calcul.")); } }
void LabController::setSourceVolts(double value) { if (std::isfinite(value) && source_ != value) { source_ = value; samples_.clear(); rawSamples_.clear(); tau_ = 0; finalCurrent_ = 0; emit inputsChanged(); emit resultsChanged(); setStatus(QStringLiteral("Paramètres modifiés — relancez le calcul.")); } }
void LabController::setInitialCurrentAmps(double value) { if (std::isfinite(value) && initialCurrent_ != value) { initialCurrent_ = value; samples_.clear(); rawSamples_.clear(); tau_ = 0; finalCurrent_ = 0; emit inputsChanged(); emit resultsChanged(); setStatus(QStringLiteral("Paramètres modifiés — relancez le calcul.")); } }
void LabController::setDurationSeconds(double value) { if (std::isfinite(value) && duration_ != value) { duration_ = value; samples_.clear(); rawSamples_.clear(); tau_ = 0; finalCurrent_ = 0; emit inputsChanged(); emit resultsChanged(); setStatus(QStringLiteral("Paramètres modifiés — relancez le calcul.")); } }
void LabController::setStepSeconds(double value) { if (std::isfinite(value) && step_ != value) { step_ = value; samples_.clear(); rawSamples_.clear(); tau_ = 0; finalCurrent_ = 0; emit inputsChanged(); emit resultsChanged(); setStatus(QStringLiteral("Paramètres modifiés — relancez le calcul.")); } }

void LabController::setStatus(QString text) {
    if (status_ == text) return;
    status_ = std::move(text);
    emit statusChanged();
}
void LabController::setRunning(bool value) {
    if (running_ == value) return;
    running_ = value;
    emit runningChanged();
}

void LabController::runOrCancel() {
    if (running_) {
        cancelRequested_.store(true, std::memory_order_relaxed);
        setStatus(QStringLiteral("Annulation de la simulation…"));
        return;
    }
    joinWorker();
    cancelRequested_.store(false, std::memory_order_relaxed);
    samples_.clear(); rawSamples_.clear(); emit resultsChanged();
    setRunning(true);
    setStatus(QStringLiteral("Calcul en cours…"));
    const electrical::SeriesRlStepCase testCase{
        {resistance_, core::si::resistance}, {inductance_, core::si::inductance},
        {source_, core::si::voltage}, {initialCurrent_, core::si::electricCurrent}, duration_, step_};
    worker_ = std::thread([this, testCase] {
        auto result = electrical::simulateSeriesRlStep(testCase, &cancelRequested_);
        QMetaObject::invokeMethod(this, [this, result = std::move(result)]() mutable {
            finishSimulation(std::move(result));
        }, Qt::QueuedConnection);
    });
}

void LabController::finishSimulation(core::Result<electrical::RlResult> result) {
    joinWorker();
    setRunning(false);
    if (!result) {
        if (result.error().code == core::ErrorCode::Conflict && cancelRequested_.load(std::memory_order_relaxed)) {
            setStatus(QStringLiteral("Simulation annulée."));
        } else {
            setStatus(QStringLiteral("Erreur : %1").arg(QString::fromStdString(result.error().message)));
        }
        return;
    }
    rawSamples_ = std::move(result.value().samples);
    constexpr std::size_t maximumPlotPoints = 5000;
    const std::size_t pointCount = std::min(rawSamples_.size(), maximumPlotPoints);
    samples_.reserve(static_cast<qsizetype>(pointCount));
    for (std::size_t j = 0; j < pointCount; ++j) {
        const std::size_t index = pointCount < 2 ? 0 : j * (rawSamples_.size() - 1) / (pointCount - 1);
        const auto& sample = rawSamples_[index];
        samples_.push_back(QVariantMap{{QStringLiteral("time"), sample.timeSeconds},
                                      {QStringLiteral("current"), sample.current.si_value}});
    }
    tau_ = result.value().timeConstantSeconds;
    finalCurrent_ = rawSamples_.back().current.si_value;
    emit resultsChanged();
    setStatus(QStringLiteral("Terminé — Euler implicite, %1 points calculés.").arg(samples_.size()));
}

void LabController::saveProject(const QUrl& fileUrl) {
    const auto path = fileUrl.toLocalFile();
    if (path.isEmpty()) { setStatus(QStringLiteral("Enregistrement annulé.")); return; }
    auto project = makeProject(resistance_, inductance_, source_, initialCurrent_, duration_, step_);
    if (!project) { setStatus(QStringLiteral("Paramètres invalides : %1").arg(QString::fromStdString(project.error().message))); return; }
    const auto saved = core::ProjectStore::save(project.value(), std::filesystem::path(path.toStdWString()));
    setStatus(saved ? QStringLiteral("Projet RL enregistré.") : QStringLiteral("Échec d'enregistrement : %1").arg(QString::fromStdString(saved.error().message)));
}

void LabController::loadProject(const QUrl& fileUrl) {
    const auto path = fileUrl.toLocalFile();
    if (path.isEmpty()) { setStatus(QStringLiteral("Ouverture annulée.")); return; }
    auto loaded = core::ProjectStore::load(std::filesystem::path(path.toStdWString()));
    if (!loaded) { setStatus(QStringLiteral("Échec d'ouverture : %1").arg(QString::fromStdString(loaded.error().message))); return; }
    double r{}, l{}, u{}, i0{}, duration{}, step{};
    const auto& project = loaded.value();
    const auto source = project.components().find("source");
    const auto resistor = project.components().find("resistor");
    const auto inductor = project.components().find("inductor");
    if (project.id() != experimentId || project.components().size() != 3 || project.connections().size() != 3 ||
        source == project.components().end() || resistor == project.components().end() || inductor == project.components().end() ||
        source->second.definition.id != "openlab.electrical.dc_step_source" ||
        resistor->second.definition.id != "openlab.electrical.resistor" || inductor->second.definition.id != "openlab.electrical.inductor" ||
        !parameter(project, "source", "voltage", core::si::voltage, u) ||
        !parameter(project, "source", "initial_current", core::si::electricCurrent, i0) ||
        !parameter(project, "source", "duration", core::si::time, duration) ||
        !parameter(project, "source", "time_step", core::si::time, step) ||
        !parameter(project, "resistor", "resistance", core::si::resistance, r) ||
        !parameter(project, "inductor", "inductance", core::si::inductance, l) ||
        r <= 0.0 || l <= 0.0 || duration <= 0.0 || step <= 0.0 || duration / step > 1'000'000.0) {
        setStatus(QStringLiteral("Ce fichier n'est pas un banc RL compatible.")); return;
    }
    resistance_ = r; inductance_ = l; source_ = u; initialCurrent_ = i0; duration_ = duration; step_ = step;
    samples_.clear(); rawSamples_.clear(); tau_ = 0.0; finalCurrent_ = 0.0;
    emit inputsChanged(); emit resultsChanged();
    setStatus(QStringLiteral("Banc RL chargé ; lancez la simulation pour recalculer les résultats."));
}

void LabController::exportCsv(const QUrl& fileUrl) {
    const auto path = fileUrl.toLocalFile();
    if (path.isEmpty()) { setStatus(QStringLiteral("Export annulé.")); return; }
    if (samples_.isEmpty()) { setStatus(QStringLiteral("Aucun résultat à exporter.")); return; }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) { setStatus(QStringLiteral("Impossible d'écrire le fichier CSV.")); return; }
    QTextStream stream(&file);
    stream.setLocale(QLocale::c());
    stream << "time_s,current_A\n";
    for (const auto& sample : rawSamples_) {
        stream << sample.timeSeconds << ',' << sample.current.si_value << '\n';
    }
    if (!file.commit()) { setStatus(QStringLiteral("Échec lors de la finalisation du CSV.")); return; }
    setStatus(QStringLiteral("Mesures exportées en CSV."));
}

} // namespace openlab::app
