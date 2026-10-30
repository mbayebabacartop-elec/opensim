#pragma once

#include "units.hpp"

#include <compare>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace openlab::core {

enum class ScientificDomain { Electrical, Mechanical, Thermal, Control, Digital, Fluid, Mixed };
enum class PortKind { Conserving, Signal };
enum class SignalDirection { Input, Output, Bidirectional };

struct PortDefinition {
    std::string id;
    std::string name;
    ScientificDomain domain{ScientificDomain::Electrical};
    PortKind kind{PortKind::Conserving};
    SignalDirection direction{SignalDirection::Bidirectional};
    Dimension signalDimension{};
    Dimension effortDimension{};
    Dimension flowDimension{};
};

struct ParameterDefinition {
    std::string id;
    std::string name;
    Dimension dimension{};
    double defaultSiValue{};
    std::optional<double> minimumSiValue;
    std::optional<double> maximumSiValue;
};

struct DefinitionRef {
    std::string id;
    std::string version;
    auto operator<=>(const DefinitionRef&) const = default;
};

struct ComponentDefinition {
    DefinitionRef ref;
    std::string name;
    ScientificDomain domain{ScientificDomain::Electrical};
    std::vector<PortDefinition> ports;
    std::vector<ParameterDefinition> parameters;
};

struct ComponentInstance {
    std::string id;
    DefinitionRef definition;
    std::string displayName;
    std::map<std::string, Quantity, std::less<>> parameters;
};

struct PortRef {
    std::string componentId;
    std::string portId;
    auto operator<=>(const PortRef&) const = default;
};

struct Connection {
    PortRef first;
    PortRef second;
    auto operator<=>(const Connection&) const = default;
};

[[nodiscard]] Result<void> validateDefinition(const ComponentDefinition& definition);
[[nodiscard]] std::string_view domainName(ScientificDomain domain) noexcept;
[[nodiscard]] std::string_view portKindName(PortKind kind) noexcept;
[[nodiscard]] std::string_view signalDirectionName(SignalDirection direction) noexcept;
[[nodiscard]] Result<ScientificDomain> parseDomain(std::string_view value);
[[nodiscard]] Result<PortKind> parsePortKind(std::string_view value);
[[nodiscard]] Result<SignalDirection> parseSignalDirection(std::string_view value);

} // namespace openlab::core
