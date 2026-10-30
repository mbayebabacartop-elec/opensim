#pragma once

#include "openlab/core/result.hpp"
#include "openlab/electrical/rl_engine.hpp"
#include "openlab/electrical/dc_motor_engine.hpp"
#include "openlab/electrical/linear_circuit_engine.hpp"

#include <QObject>
#include <QUrl>
#include <QVariantList>
#include <atomic>
#include <filesystem>
#include <thread>

namespace openlab::app {

class LabController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(double resistanceOhms READ resistanceOhms WRITE setResistanceOhms NOTIFY inputsChanged)
    Q_PROPERTY(double inductanceHenries READ inductanceHenries WRITE setInductanceHenries NOTIFY inputsChanged)
    Q_PROPERTY(double sourceVolts READ sourceVolts WRITE setSourceVolts NOTIFY inputsChanged)
    Q_PROPERTY(double initialCurrentAmps READ initialCurrentAmps WRITE setInitialCurrentAmps NOTIFY inputsChanged)
    Q_PROPERTY(double durationSeconds READ durationSeconds WRITE setDurationSeconds NOTIFY inputsChanged)
    Q_PROPERTY(double stepSeconds READ stepSeconds WRITE setStepSeconds NOTIFY inputsChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
    Q_PROPERTY(QVariantList currentSamples READ currentSamples NOTIFY resultsChanged)
    Q_PROPERTY(int sampleCount READ sampleCount NOTIFY resultsChanged)
    Q_PROPERTY(double timeConstantSeconds READ timeConstantSeconds NOTIFY resultsChanged)
    Q_PROPERTY(double finalCurrentAmps READ finalCurrentAmps NOTIFY resultsChanged)
    Q_PROPERTY(bool motorPowered READ motorPowered WRITE setMotorPowered NOTIFY motorInputsChanged)
    Q_PROPERTY(double motorVoltage READ motorVoltage WRITE setMotorVoltage NOTIFY motorInputsChanged)
    Q_PROPERTY(double motorLoadTorque READ motorLoadTorque WRITE setMotorLoadTorque NOTIFY motorInputsChanged)
    Q_PROPERTY(QVariantList motorSamples READ motorSamples NOTIFY motorResultsChanged)
    Q_PROPERTY(int motorSampleCount READ motorSampleCount NOTIFY motorResultsChanged)
    Q_PROPERTY(double motorCurrentAmps READ motorCurrentAmps NOTIFY motorResultsChanged)
    Q_PROPERTY(double motorAngularVelocity READ motorAngularVelocity NOTIFY motorResultsChanged)
    Q_PROPERTY(double motorRpm READ motorRpm NOTIFY motorResultsChanged)
    Q_PROPERTY(double motorTorqueNewtonMetres READ motorTorqueNewtonMetres NOTIFY motorResultsChanged)
    Q_PROPERTY(double motorElectricalPowerWatts READ motorElectricalPowerWatts NOTIFY motorResultsChanged)
    Q_PROPERTY(double motorMechanicalPowerWatts READ motorMechanicalPowerWatts NOTIFY motorResultsChanged)
    Q_PROPERTY(double motorEfficiencyPercent READ motorEfficiencyPercent NOTIFY motorResultsChanged)
    Q_PROPERTY(double motorBackEmfVolts READ motorBackEmfVolts NOTIFY motorResultsChanged)
    Q_PROPERTY(double mnaResistanceOhms READ mnaResistanceOhms WRITE setMnaResistanceOhms NOTIFY mnaInputsChanged)
    Q_PROPERTY(double mnaCapacitanceFarads READ mnaCapacitanceFarads WRITE setMnaCapacitanceFarads NOTIFY mnaInputsChanged)
    Q_PROPERTY(double mnaVoltage READ mnaVoltage WRITE setMnaVoltage NOTIFY mnaInputsChanged)
    Q_PROPERTY(QVariantList mnaSamples READ mnaSamples NOTIFY mnaResultsChanged)
    Q_PROPERTY(int mnaSampleCount READ mnaSampleCount NOTIFY mnaResultsChanged)
    Q_PROPERTY(double mnaFinalVoltage READ mnaFinalVoltage NOTIFY mnaResultsChanged)

public:
    explicit LabController(QObject* parent = nullptr);
    ~LabController() override;

    double resistanceOhms() const noexcept { return resistance_; }
    double inductanceHenries() const noexcept { return inductance_; }
    double sourceVolts() const noexcept { return source_; }
    double initialCurrentAmps() const noexcept { return initialCurrent_; }
    double durationSeconds() const noexcept { return duration_; }
    double stepSeconds() const noexcept { return step_; }
    bool running() const noexcept { return running_; }
    QString statusText() const { return status_; }
    QVariantList currentSamples() const { return samples_; }
    int sampleCount() const noexcept { return static_cast<int>(rawSamples_.size()); }
    double timeConstantSeconds() const noexcept { return tau_; }
    double finalCurrentAmps() const noexcept { return finalCurrent_; }
    bool motorPowered() const noexcept { return motorPowered_; }
    double motorVoltage() const noexcept { return motorVoltage_; }
    double motorLoadTorque() const noexcept { return motorLoadTorque_; }
    QVariantList motorSamples() const { return motorSamples_; }
    int motorSampleCount() const noexcept { return static_cast<int>(motorRawSamples_.size()); }
    double motorCurrentAmps() const noexcept { return motorCurrent_; }
    double motorAngularVelocity() const noexcept { return motorAngularVelocity_; }
    double motorRpm() const noexcept { return motorAngularVelocity_ * 60.0 / (2.0 * 3.14159265358979323846); }
    double motorTorqueNewtonMetres() const noexcept { return motorTorque_; }
    double motorElectricalPowerWatts() const noexcept { return (motorPowered_ ? motorVoltage_ : 0.0) * motorCurrent_; }
    double motorMechanicalPowerWatts() const noexcept { return motorTorque_ * motorAngularVelocity_; }
    double motorEfficiencyPercent() const noexcept {
        const double pElec = motorElectricalPowerWatts();
        if (pElec <= 1e-4) return 0.0;
        const double eff = (motorMechanicalPowerWatts() / pElec) * 100.0;
        return eff < 0.0 ? 0.0 : (eff > 100.0 ? 100.0 : eff);
    }
    double motorBackEmfVolts() const noexcept { return 0.05 * motorAngularVelocity_; }
    double mnaResistanceOhms() const noexcept { return mnaResistance_; }
    double mnaCapacitanceFarads() const noexcept { return mnaCapacitance_; }
    double mnaVoltage() const noexcept { return mnaVoltage_; }
    QVariantList mnaSamples() const { return mnaSamples_; }
    int mnaSampleCount() const noexcept { return static_cast<int>(mnaSamples_.size()); }
    double mnaFinalVoltage() const noexcept { return mnaFinalVoltage_; }

    void setResistanceOhms(double value);
    void setInductanceHenries(double value);
    void setSourceVolts(double value);
    void setInitialCurrentAmps(double value);
    void setDurationSeconds(double value);
    void setStepSeconds(double value);
    void setMotorPowered(bool value);
    void setMotorVoltage(double value);
    void setMotorLoadTorque(double value);
    void setMnaResistanceOhms(double value);
    void setMnaCapacitanceFarads(double value);
    void setMnaVoltage(double value);

    Q_INVOKABLE void runOrCancel();
    Q_INVOKABLE void runMotorSimulation();
    Q_INVOKABLE void runMnaSimulation();
    Q_INVOKABLE void saveProject(const QUrl& fileUrl);
    Q_INVOKABLE void loadProject(const QUrl& fileUrl);
    Q_INVOKABLE void saveMotorProject(const QUrl& fileUrl);
    Q_INVOKABLE void loadMotorProject(const QUrl& fileUrl);
    Q_INVOKABLE void saveMnaProject(const QUrl& fileUrl);
    Q_INVOKABLE void loadMnaProject(const QUrl& fileUrl);
    Q_INVOKABLE void exportCsv(const QUrl& fileUrl);

signals:
    void inputsChanged();
    void runningChanged();
    void statusChanged();
    void resultsChanged();
    void motorInputsChanged();
    void motorResultsChanged();
    void mnaInputsChanged();
    void mnaResultsChanged();

private:
    void setStatus(QString text);
    void setRunning(bool value);
    void finishSimulation(openlab::core::Result<openlab::electrical::RlResult> result);
    void finishMotorSimulation(openlab::core::Result<openlab::electrical::DcMotorResult> result);
    void finishMnaSimulation(openlab::core::Result<openlab::electrical::LinearTransientResult> result);
    void joinWorker();

    double resistance_{10.0};
    double inductance_{0.1};
    double source_{5.0};
    double initialCurrent_{};
    double duration_{0.05};
    double step_{0.0005};
    bool running_{};
    QString status_{QStringLiteral("Prêt — configurez le banc RL puis lancez la simulation.")};
    QVariantList samples_;
    std::vector<openlab::electrical::RlSample> rawSamples_;
    double tau_{};
    double finalCurrent_{};
    double motorVoltage_{12.0};
    double motorLoadTorque_{0.03};
    bool motorPowered_{true};
    QVariantList motorSamples_;
    std::vector<openlab::electrical::DcMotorSample> motorRawSamples_;
    double motorCurrent_{};
    double motorAngularVelocity_{};
    double motorTorque_{};
    double mnaResistance_{1000.0};
    double mnaCapacitance_{1.0e-6};
    double mnaVoltage_{5.0};
    QVariantList mnaSamples_;
    double mnaFinalVoltage_{};
    std::atomic_bool cancelRequested_{false};
    std::thread worker_;
};

} // namespace openlab::app
