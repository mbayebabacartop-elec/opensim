#include "logging.hpp"
#include "main_window.hpp"

#include "openlab/core/version.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QLoggingCategory>

int main(int argc, char* argv[]) {
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
    QQmlApplicationEngine engine;
    QString loadError;
    if (!openlab::app::loadMainWindow(engine, &loadError)) {
        qCritical().noquote() << loadError;
        return 2;
    }

    const int result = application.exec();
    qInfo() << "Arrêt d’OpenLab Simulator" << result;
    return result;
}
