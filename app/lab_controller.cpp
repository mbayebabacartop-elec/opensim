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

PortDefinition mechanicalPort(std::string id, std::string name) {
    return {std::move(id), std::move(name), ScientificDomain::Mechanical, PortKind::Conserving,
            SignalDirection::Bidirectional, {}, si::torque, si::angularVelocity};
}

Result<ProjectModel> makeMotorProject(double voltageValue, double loadTorque) {
    using namespace si;
    const auto backEmfDimension = si::voltage.multipliedBy(time).value();
    const auto torquePerAmpereDimension = torque.dividedBy(electricCurrent).value();
    const auto durationDimension = time;
    const std::string id = "org.openlab.experiment.dc-motor.pm.v1";
    ProjectModel project(id, "Banc moteur CC a aimants permanents");
    ComponentDefinition supply{{"openlab.electrical.dc_voltage_source", "1.0"}, "Source de tension DC ideale", ScientificDomain::Electrical,
        {electricalPort("positive", "Positif"), electricalPort("negative", "Negatif")},
        {{"voltage", "Tension", si::voltage, 12.0, {}, {}}}};
    ComponentDefinition motor{{"openlab.electrical.dc_motor_pm", "1.0"}, "Moteur CC ideal couple", ScientificDomain::Mixed,
        {electricalPort("positive", "Induit +"), electricalPort("negative", "Induit -"), mechanicalPort("shaft", "Arbre")},
        {{"resistance", "Resistance d'induit", resistance, 2.0, 1e-12, {}},
         {"inductance", "Inductance d'induit", inductance, 0.1, 1e-15, {}},
         {"back_emf_constant", "Constante Ke", backEmfDimension, 0.05, 0.0, {}},
         {"torque_constant", "Constante Kt", torquePerAmpereDimension, 0.05, 0.0, {}},
         {"inertia", "Inertie totale", momentOfInertia, 0.002, 1e-15, {}},
         {"viscous_friction", "Frottement visqueux", rotationalFriction, 0.0005, 0.0, {}},
         {"initial_current", "Courant initial", electricCurrent, 0.0, {}, {}},
         {"initial_speed", "Vitesse initiale", angularVelocity, 0.0, {}, {}},
         {"duration", "Duree", durationDimension, 0.5, 1e-9, {}},
         {"time_step", "Pas maximal", durationDimension, 0.0005, 1e-9, {}}}};
    ComponentDefinition load{{"openlab.mechanical.rotational_load", "1.0"}, "Charge mecanique", ScientificDomain::Mechanical,
        {mechanicalPort("shaft", "Arbre")}, {{"resisting_torque", "Couple resistant", torque, 0.03, {}, {}}}};
    for (const auto* definition : {&supply, &motor, &load}) {
        auto added = project.addDefinition(*definition);
        if (!added) return Result<ProjectModel>::failure(added.error());
    }
    ComponentInstance cSupply{"source", supply.ref, "Alimentation DC", {{"voltage", Quantity{voltageValue, si::voltage}}}};
    ComponentInstance cMotor{"motor", motor.ref, "M1", {}};
    ComponentInstance cLoad{"load", load.ref, "Charge", {{"resisting_torque", Quantity{loadTorque, torque}}}};
    if (auto r = project.addComponent(std::move(cSupply)); !r) return Result<ProjectModel>::failure(r.error());
    if (auto r = project.addComponent(std::move(cMotor)); !r) return Result<ProjectModel>::failure(r.error());
    if (auto r = project.addComponent(std::move(cLoad)); !r) return Result<ProjectModel>::failure(r.error());

    for (const auto& pair : {std::pair{PortRef{"source", "positive"}, PortRef{"motor", "positive"}},
                             std::pair{PortRef{"motor", "negative"}, PortRef{"source", "negative"}},
                             std::pair{PortRef{"motor", "shaft"}, PortRef{"load", "shaft"}}}) {
        auto connected = project.connect(pair.first, pair.second);
        if (!connected) return Result<ProjectModel>::failure(connected.error());
    }
    return Result<ProjectModel>::success(std::move(project));
}

Result<ProjectModel> makeMnaProject(double voltageValue, double resistanceValue, double capacitanceValue) {
    using namespace si;
    const auto capacitanceDimension = electricCurrent.multipliedBy(time).value().dividedBy(si::voltage).value();
    const std::string id = "org.openlab.experiment.linear-rc.step.v1";
    ProjectModel project(id, "Circuit RC lineaire - echelon DC");
    ComponentDefinition supply{{"openlab.electrical.dc_voltage_source", "1.0"}, "Source de tension DC ideale", ScientificDomain::Electrical,
        {electricalPort("positive", "Positif"), electricalPort("negative", "Negatif")}, {{"voltage", "Tension", si::voltage, 5.0, {}, {}}}};
    ComponentDefinition resistor{{"openlab.electrical.resistor", "1.0"}, "Resistance ideale", ScientificDomain::Electrical,
        {electricalPort("a", "Borne A"), electricalPort("b", "Borne B")}, {{"resistance", "Resistance", si::resistance, 1000.0, 1e-12, {}}}};
    ComponentDefinition capacitor{{"openlab.electrical.capacitor", "1.0"}, "Condensateur ideal", ScientificDomain::Electrical,
        {electricalPort("a", "Borne A"), electricalPort("b", "Borne B")},
        {{"capacitance", "Capacite", capacitanceDimension, 1e-6, 1e-15, {}}, {"initial_voltage", "Tension initiale", si::voltage, 0.0, {}, {}}}};
    for (const auto* definition : {&supply, &resistor, &capacitor}) {
        auto added = project.addDefinition(*definition);
        if (!added) return Result<ProjectModel>::failure(added.error());
    }
    ComponentInstance cSupply{"source", supply.ref, "U1", {{"voltage", Quantity{voltageValue, si::voltage}}}};
    ComponentInstance cResistor{"resistor", resistor.ref, "R1", {{"resistance", Quantity{resistanceValue, si::resistance}}}};
    ComponentInstance cCapacitor{"capacitor", capacitor.ref, "C1", {{"capacitance", Quantity{capacitanceValue, capacitanceDimension}}, {"initial_voltage", Quantity{0.0, si::voltage}}}};
    if (auto r = project.addComponent(std::move(cSupply)); !r) return Result<ProjectModel>::failure(r.error());
    if (auto r = project.addComponent(std::move(cResistor)); !r) return Result<ProjectModel>::failure(r.error());
    if (auto r = project.addComponent(std::move(cCapacitor)); !r) return Result<ProjectModel>::failure(r.error());

    for (const auto& pair : {std::pair{PortRef{"source", "positive"}, PortRef{"resistor", "a"}},
                             std::pair{PortRef{"resistor", "b"}, PortRef{"capacitor", "a"}},
                             std::pair{PortRef{"capacitor", "b"}, PortRef{"source", "negative"}}}) {
        auto connected = project.connect(pair.first, pair.second);
        if (!connected) return Result<ProjectModel>::failure(connected.error());
    }
    return Result<ProjectModel>::success(std::move(project));
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
void LabController::setMotorPowered(bool value) {
    if (motorPowered_ != value) {
        motorPowered_ = value;
        motorSamples_.clear();
        motorRawSamples_.clear();
        motorCurrent_ = motorAngularVelocity_ = motorTorque_ = 0.0;
        emit motorInputsChanged();
        emit motorResultsChanged();
        setStatus(motorPowered_ ? QStringLiteral("Alimentation moteur connectée — relancez le calcul.")
                                : QStringLiteral("Interrupteur d'alimentation moteur ouvert (0 V)."));
    }
}
void LabController::setMotorVoltage(double value) { if (std::isfinite(value) && motorVoltage_ != value) { motorVoltage_ = value; motorSamples_.clear(); motorRawSamples_.clear(); motorCurrent_ = motorAngularVelocity_ = motorTorque_ = 0.0; emit motorInputsChanged(); emit motorResultsChanged(); setStatus(QStringLiteral("Paramètres moteur modifiés — relancez le calcul.")); } }
void LabController::setMotorLoadTorque(double value) { if (std::isfinite(value) && motorLoadTorque_ != value) { motorLoadTorque_ = value; motorSamples_.clear(); motorRawSamples_.clear(); motorCurrent_ = motorAngularVelocity_ = motorTorque_ = 0.0; emit motorInputsChanged(); emit motorResultsChanged(); setStatus(QStringLiteral("Charge moteur modifiée — relancez le calcul.")); } }
void LabController::setMnaResistanceOhms(double value) { if (std::isfinite(value) && value > 0.0 && mnaResistance_ != value) { mnaResistance_ = value; mnaSamples_.clear(); mnaFinalVoltage_ = 0.0; emit mnaInputsChanged(); emit mnaResultsChanged(); setStatus(QStringLiteral("Paramètres du circuit modifiés — relancez le calcul MNA.")); } }
void LabController::setMnaCapacitanceFarads(double value) { if (std::isfinite(value) && value > 0.0 && mnaCapacitance_ != value) { mnaCapacitance_ = value; mnaSamples_.clear(); mnaFinalVoltage_ = 0.0; emit mnaInputsChanged(); emit mnaResultsChanged(); setStatus(QStringLiteral("Paramètres du circuit modifiés — relancez le calcul MNA.")); } }
void LabController::setMnaVoltage(double value) { if (std::isfinite(value) && mnaVoltage_ != value) { mnaVoltage_ = value; mnaSamples_.clear(); mnaFinalVoltage_ = 0.0; emit mnaInputsChanged(); emit mnaResultsChanged(); setStatus(QStringLiteral("Paramètres du circuit modifiés — relancez le calcul MNA.")); } }

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

void LabController::runMotorSimulation() {
    if (running_) {
        cancelRequested_.store(true, std::memory_order_relaxed);
        setStatus(QStringLiteral("Annulation de la simulation moteur…"));
        return;
    }
    joinWorker();
    cancelRequested_.store(false, std::memory_order_relaxed);
    motorSamples_.clear(); motorRawSamples_.clear();
    emit motorResultsChanged();
    setRunning(true);
    setStatus(QStringLiteral("Calcul du moteur CC en cours…"));
    using namespace core;
    const auto backEmfDimension = si::voltage.multipliedBy(si::time).value();
    const auto torquePerAmpereDimension = si::torque.dividedBy(si::electricCurrent).value();
    const double effectiveVoltage = motorPowered_ ? motorVoltage_ : 0.0;
    const electrical::DcMotorCase motor{
        {2.0, si::resistance}, {0.1, si::inductance}, {0.05, backEmfDimension},
        {0.05, torquePerAmpereDimension}, {0.002, si::momentOfInertia}, {0.0005, si::rotationalFriction},
        {motorLoadTorque_, si::torque}, {effectiveVoltage, si::voltage}, {0.0, si::electricCurrent},
        {0.0, si::angularVelocity}, 0.5, 0.0005};
    worker_ = std::thread([this, motor] {
        auto result = electrical::simulateDcMotor(motor, &cancelRequested_);
        QMetaObject::invokeMethod(this, [this, result = std::move(result)]() mutable {
            finishMotorSimulation(std::move(result));
        }, Qt::QueuedConnection);
    });
}

void LabController::runMnaSimulation() {
    if (running_) {
        cancelRequested_.store(true, std::memory_order_relaxed);
        setStatus(QStringLiteral("Annulation de la simulation MNA…"));
        return;
    }
    joinWorker();
    cancelRequested_.store(false, std::memory_order_relaxed);
    mnaSamples_.clear();
    emit mnaResultsChanged();
    setRunning(true);
    setStatus(QStringLiteral("Calcul du réseau RC par MNA…"));
    using namespace core;
    const auto capacitanceDimension = si::electricCurrent.multipliedBy(si::time).value().dividedBy(si::voltage).value();
    const electrical::LinearTransientCase circuit{3,
        {electrical::DcVoltageSource{1,0,{mnaVoltage_,si::voltage}},
         electrical::Resistor{1,2,{mnaResistance_,si::resistance}},
         electrical::Capacitor{2,0,{mnaCapacitance_,capacitanceDimension},{0.0,si::voltage}}},
         0.01,0.0001};
    worker_ = std::thread([this, circuit] {
        auto result = electrical::simulateLinearTransient(circuit, &cancelRequested_);
        QMetaObject::invokeMethod(this, [this, result = std::move(result)]() mutable {
            finishMnaSimulation(std::move(result));
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

void LabController::finishMotorSimulation(core::Result<electrical::DcMotorResult> result) {
    joinWorker();
    setRunning(false);
    if (!result) {
        if (result.error().code == core::ErrorCode::Conflict && cancelRequested_.load(std::memory_order_relaxed))
            setStatus(QStringLiteral("Simulation du moteur annulée."));
        else
            setStatus(QStringLiteral("Erreur moteur : %1").arg(QString::fromStdString(result.error().message)));
        return;
    }
    motorRawSamples_ = std::move(result.value().samples);
    constexpr std::size_t maximumPlotPoints = 5000;
    const std::size_t pointCount = std::min(motorRawSamples_.size(), maximumPlotPoints);
    motorSamples_.reserve(static_cast<qsizetype>(pointCount));
    for (std::size_t j = 0; j < pointCount; ++j) {
        const std::size_t index = pointCount < 2 ? 0 : j * (motorRawSamples_.size() - 1) / (pointCount - 1);
        const auto& sample = motorRawSamples_[index];
        motorSamples_.push_back(QVariantMap{{QStringLiteral("time"), sample.timeSeconds},
            {QStringLiteral("current"), sample.current.si_value},
            {QStringLiteral("omega"), sample.angularVelocity.si_value},
            {QStringLiteral("torque"), sample.electromagneticTorque.si_value}});
    }
    const auto& last = motorRawSamples_.back();
    motorCurrent_ = last.current.si_value;
    motorAngularVelocity_ = last.angularVelocity.si_value;
    motorTorque_ = last.electromagneticTorque.si_value;
    emit motorResultsChanged();
    setStatus(QStringLiteral("Moteur CC calculé — Euler implicite, %1 points, paramètres illustratifs.").arg(motorSamples_.size()));
}

void LabController::finishMnaSimulation(core::Result<electrical::LinearTransientResult> result) {
    joinWorker();
    setRunning(false);
    if (!result) {
        if (result.error().code == core::ErrorCode::Conflict && cancelRequested_.load(std::memory_order_relaxed))
            setStatus(QStringLiteral("Simulation MNA annulée."));
        else
            setStatus(QStringLiteral("Erreur MNA : %1").arg(QString::fromStdString(result.error().message)));
        return;
    }
    mnaSamples_.reserve(static_cast<qsizetype>(result.value().samples.size()+1));
    mnaSamples_.push_back(QVariantMap{{QStringLiteral("time"),0.0},{QStringLiteral("voltage"),0.0}});
    for (const auto& sample : result.value().samples)
        mnaSamples_.push_back(QVariantMap{{QStringLiteral("time"),sample.timeSeconds},
            {QStringLiteral("voltage"),sample.nodeVoltages[2].si_value}});
    mnaFinalVoltage_ = result.value().samples.back().nodeVoltages[2].si_value;
    emit mnaResultsChanged();
    setStatus(QStringLiteral("Réseau RC calculé par MNA — Euler implicite, %1 points.").arg(mnaSamples_.size()));
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

void LabController::saveMotorProject(const QUrl& fileUrl) {
    const auto path = fileUrl.toLocalFile();
    if (path.isEmpty()) { setStatus(QStringLiteral("Enregistrement annulé.")); return; }
    auto project = makeMotorProject(motorVoltage_, motorLoadTorque_);
    if (!project) { setStatus(QStringLiteral("Paramètres invalides : %1").arg(QString::fromStdString(project.error().message))); return; }
    const auto saved = core::ProjectStore::save(project.value(), std::filesystem::path(path.toStdWString()));
    setStatus(saved ? QStringLiteral("Projet moteur CC enregistré.") : QStringLiteral("Échec d'enregistrement : %1").arg(QString::fromStdString(saved.error().message)));
}

void LabController::loadMotorProject(const QUrl& fileUrl) {
    const auto path = fileUrl.toLocalFile();
    if (path.isEmpty()) { setStatus(QStringLiteral("Ouverture annulée.")); return; }
    auto loaded = core::ProjectStore::load(std::filesystem::path(path.toStdWString()));
    if (!loaded) { setStatus(QStringLiteral("Échec d'ouverture : %1").arg(QString::fromStdString(loaded.error().message))); return; }
    double u{}, loadTorque{};
    const auto& project = loaded.value();
    if (project.id() != "org.openlab.experiment.dc-motor.pm.v1" ||
        !parameter(project, "source", "voltage", core::si::voltage, u) ||
        !parameter(project, "load", "resisting_torque", core::si::torque, loadTorque)) {
        setStatus(QStringLiteral("Ce fichier n'est pas un banc moteur CC compatible.")); return;
    }
    motorVoltage_ = u; motorLoadTorque_ = loadTorque;
    motorSamples_.clear(); motorRawSamples_.clear();
    motorCurrent_ = motorAngularVelocity_ = motorTorque_ = 0.0;
    emit motorInputsChanged(); emit motorResultsChanged();
    setStatus(QStringLiteral("Banc moteur CC chargé ; relancez le calcul."));
}

void LabController::saveMnaProject(const QUrl& fileUrl) {
    const auto path = fileUrl.toLocalFile();
    if (path.isEmpty()) { setStatus(QStringLiteral("Enregistrement annulé.")); return; }
    auto project = makeMnaProject(mnaVoltage_, mnaResistance_, mnaCapacitance_);
    if (!project) { setStatus(QStringLiteral("Paramètres invalides : %1").arg(QString::fromStdString(project.error().message))); return; }
    const auto saved = core::ProjectStore::save(project.value(), std::filesystem::path(path.toStdWString()));
    setStatus(saved ? QStringLiteral("Projet RC enregistré.") : QStringLiteral("Échec d'enregistrement : %1").arg(QString::fromStdString(saved.error().message)));
}

void LabController::loadMnaProject(const QUrl& fileUrl) {
    const auto path = fileUrl.toLocalFile();
    if (path.isEmpty()) { setStatus(QStringLiteral("Ouverture annulée.")); return; }
    auto loaded = core::ProjectStore::load(std::filesystem::path(path.toStdWString()));
    if (!loaded) { setStatus(QStringLiteral("Échec d'ouverture : %1").arg(QString::fromStdString(loaded.error().message))); return; }
    using namespace core::si;
    const auto capacitanceDimension = electricCurrent.multipliedBy(time).value().dividedBy(voltage).value();
    double u{}, r{}, c{};
    const auto& project = loaded.value();
    if (project.id() != "org.openlab.experiment.linear-rc.step.v1" ||
        !parameter(project, "source", "voltage", voltage, u) ||
        !parameter(project, "resistor", "resistance", resistance, r) ||
        !parameter(project, "capacitor", "capacitance", capacitanceDimension, c) ||
        r <= 0.0 || c <= 0.0) {
        setStatus(QStringLiteral("Ce fichier n'est pas un banc RC compatible.")); return;
    }
    mnaVoltage_ = u; mnaResistance_ = r; mnaCapacitance_ = c;
    mnaSamples_.clear(); mnaFinalVoltage_ = 0.0;
    emit mnaInputsChanged(); emit mnaResultsChanged();
    setStatus(QStringLiteral("Banc RC chargé ; relancez le calcul."));
}

} // namespace openlab::app
