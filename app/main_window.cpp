#include "main_window.hpp"

#include <QQmlApplicationEngine>

namespace openlab::app {

bool loadMainWindow(QQmlApplicationEngine& engine, QString* errorMessage) {
    engine.loadFromModule(QStringLiteral("OpenLab"), QStringLiteral("Main"));
    if (!engine.rootObjects().isEmpty()) {
        return true;
    }

    if (errorMessage) {
        *errorMessage = QStringLiteral("La fenêtre QML OpenLab.Main n’a pas pu être chargée. Consultez le journal pour les détails.");
    }
    return false;
}

} // namespace openlab::app
