#pragma once

#include <QString>

class QQmlApplicationEngine;

namespace openlab::app {

[[nodiscard]] bool loadMainWindow(QQmlApplicationEngine& engine, QString* errorMessage = nullptr);

} // namespace openlab::app
