#include "logging.hpp"
#include "lab_controller.hpp"
#include "main_window.hpp"

#include "openlab/core/version.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QLoggingCategory>

int main(int argc, char* argv[]) {
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
    QGuiApplication application(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("OpenLab"));
    QCoreApplication::setApplicationName(QStringLiteral("OpenLab Simulator"));
    QCoreApplication::setApplicationVersion(
        QString::fromUtf8(openlab::core::version().data(), static_cast<qsizetype>(openlab::core::version().size())));

    QString loggingError;
    if (!openlab::app::initializeLogging(&loggingError)) {
        qCritical().noquote() << loggingError;
        return 1;
    }

    qInfo() << "Démarrage d’OpenLab Simulator" << QCoreApplication::applicationVersion();
    openlab::app::LabController labController;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("lab"), &labController);
    QString loadError;
    if (!openlab::app::loadMainWindow(engine, &loadError)) {
        qCritical().noquote() << loadError;
        return 2;
    }

    const int result = application.exec();
    qInfo() << "Arrêt d’OpenLab Simulator" << result;
    return result;
}
