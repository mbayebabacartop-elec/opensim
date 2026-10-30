#include "logging.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QtGlobal>

namespace {
QFile logFile;
QMutex logMutex;

void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message) {
    QMutexLocker lock(&logMutex);
    if (!logFile.isOpen()) {
        return;
    }

    const char* severity = "INFO";
    switch (type) {
    case QtDebugMsg: severity = "DEBUG"; break;
    case QtInfoMsg: severity = "INFO"; break;
    case QtWarningMsg: severity = "WARNING"; break;
    case QtCriticalMsg: severity = "CRITICAL"; break;
    case QtFatalMsg: severity = "FATAL"; break;
    }

    const QString category = context.category ? QString::fromUtf8(context.category) : QStringLiteral("default");
    const QByteArray line = QStringLiteral("%1 [%2] %3: %4\n")
                                .arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs),
                                     QString::fromLatin1(severity), category, message)
                                .toUtf8();
    logFile.write(line);
    logFile.flush();
    if (type == QtFatalMsg) {
        std::abort();
    }
}
} // namespace

namespace openlab::app {

bool initializeLogging(QString* errorMessage) {
    const QString directory = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (directory.isEmpty() || !QDir().mkpath(directory)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Impossible de créer le dossier de journaux de l’application.");
        }
        return false;
    }

    logFile.setFileName(QDir(directory).filePath(QStringLiteral("openlab.log")));
    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Impossible d’ouvrir le journal : %1").arg(logFile.errorString());
        }
        return false;
    }

    qInstallMessageHandler(messageHandler);
    qInfo().noquote() << "Journalisation initialisée dans" << logFile.fileName();
    return true;
}

} // namespace openlab::app
