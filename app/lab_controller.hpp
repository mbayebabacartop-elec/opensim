#pragma once

#include "openlab/core/result.hpp"
#include "openlab/electrical/rl_engine.hpp"

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

    void setResistanceOhms(double value);
    void setInductanceHenries(double value);
    void setSourceVolts(double value);
    void setInitialCurrentAmps(double value);
    void setDurationSeconds(double value);
    void setStepSeconds(double value);

    Q_INVOKABLE void runOrCancel();
    Q_INVOKABLE void saveProject(const QUrl& fileUrl);
    Q_INVOKABLE void loadProject(const QUrl& fileUrl);
    Q_INVOKABLE void exportCsv(const QUrl& fileUrl);

signals:
    void inputsChanged();
    void runningChanged();
    void statusChanged();
    void resultsChanged();

private:
    void setStatus(QString text);
    void setRunning(bool value);
    void finishSimulation(openlab::core::Result<openlab::electrical::RlResult> result);
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
    std::atomic_bool cancelRequested_{false};
    std::thread worker_;
};

} // namespace openlab::app
