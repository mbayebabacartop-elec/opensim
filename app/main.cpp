#include "logging.hpp"
#include "main_window.hpp"

#include "openlab/core/version.hpp"

#include <QApplication>
#include <QMessageBox>
#include <QLoggingCategory>

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("OpenLab"));
    QCoreApplication::setApplicationName(QStringLiteral("OpenLab Simulator"));
    QCoreApplication::setApplicationVersion(
        QString::fromUtf8(openlab::core::version().data(), static_cast<qsizetype>(openlab::core::version().size())));

    QString loggingError;
    if (!openlab::app::initializeLogging(&loggingError)) {
        QMessageBox::critical(nullptr, QStringLiteral("Erreur de journalisation"), loggingError);
        return 1;
    }

    qInfo() << "Démarrage d’OpenLab Simulator" << QCoreApplication::applicationVersion();
    openlab::app::MainWindow window;
    window.show();
    const int result = application.exec();
    qInfo() << "Arrêt d’OpenLab Simulator" << result;
    return result;
}
