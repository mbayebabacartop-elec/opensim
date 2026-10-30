#pragma once

#include "project_model.hpp"

#include <filesystem>

namespace openlab::core {

class ProjectStore {
public:
    static constexpr unsigned formatVersion = 1;
    [[nodiscard]] static Result<void> save(const ProjectModel& project, const std::filesystem::path& filePath);
    [[nodiscard]] static Result<ProjectModel> load(const std::filesystem::path& filePath);
};

} // namespace openlab::core
