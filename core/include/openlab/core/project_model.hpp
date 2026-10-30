#pragma once

#include "components.hpp"

#include <map>
#include <string>
#include <vector>

namespace openlab::core {

class ProjectModel {
public:
    ProjectModel(std::string id, std::string name);

    [[nodiscard]] const std::string& id() const noexcept { return id_; }
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] const std::map<DefinitionRef, ComponentDefinition>& definitions() const noexcept { return definitions_; }
    [[nodiscard]] const std::map<std::string, ComponentInstance, std::less<>>& components() const noexcept { return components_; }
    [[nodiscard]] const std::vector<Connection>& connections() const noexcept { return connections_; }

    [[nodiscard]] Result<void> addDefinition(ComponentDefinition definition);
    [[nodiscard]] Result<void> addComponent(ComponentInstance component);
    [[nodiscard]] Result<void> setParameter(std::string_view componentId, std::string_view parameterId, Quantity value);
    [[nodiscard]] Result<void> connect(PortRef first, PortRef second);
    [[nodiscard]] Result<void> validate() const;

private:
    [[nodiscard]] const PortDefinition* findPort(const PortRef& ref) const noexcept;
    [[nodiscard]] const ParameterDefinition* findParameter(const ComponentInstance& instance, std::string_view parameterId) const noexcept;

    std::string id_;
    std::string name_;
    std::map<DefinitionRef, ComponentDefinition> definitions_;
    std::map<std::string, ComponentInstance, std::less<>> components_;
    std::vector<Connection> connections_;
};

} // namespace openlab::core
