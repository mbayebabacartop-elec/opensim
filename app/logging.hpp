#pragma once

#include <QString>

namespace openlab::app {

[[nodiscard]] bool initializeLogging(QString* errorMessage = nullptr);

} // namespace openlab::app
