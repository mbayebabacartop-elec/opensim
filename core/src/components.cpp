#include "openlab/core/components.hpp"

#include <cmath>
#include <set>

namespace openlab::core {
namespace {
bool hasText(const std::string& value) {
    return !value.empty() && value.find_first_not_of(" \t\r\n") != std::string::npos;
}
} // namespace

Result<void> validateDefinition(const ComponentDefinition& definition) {
    if (!hasText(definition.ref.id) || !hasText(definition.ref.version) || !hasText(definition.name)) {
        return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "Component definition requires id, version and name."));
    }

    std::set<std::string, std::less<>> portIds;
    for (const auto& port : definition.ports) {
        if (!hasText(port.id) || !hasText(port.name) || !portIds.insert(port.id).second) {
            return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "Port ids must be non-empty and unique within a definition."));
        }
        if (definition.domain != ScientificDomain::Mixed && port.domain != definition.domain) {
            return Result<void>::failure(makeError(ErrorCode::Incompatible, "Port domain differs from its component domain."));
        }
    }

    std::set<std::string, std::less<>> parameterIds;
    for (const auto& parameter : definition.parameters) {
        if (!hasText(parameter.id) || !hasText(parameter.name) || !parameterIds.insert(parameter.id).second ||
            !std::isfinite(parameter.defaultSiValue)) {
            return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "Parameter ids must be unique and parameter values finite."));
        }
        if (parameter.minimumSiValue && (!std::isfinite(*parameter.minimumSiValue) || parameter.defaultSiValue < *parameter.minimumSiValue)) {
            return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "Parameter default is below its valid minimum."));
        }
        if (parameter.maximumSiValue && (!std::isfinite(*parameter.maximumSiValue) || parameter.defaultSiValue > *parameter.maximumSiValue)) {
            return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "Parameter default is above its valid maximum."));
        }
        if (parameter.minimumSiValue && parameter.maximumSiValue && *parameter.minimumSiValue > *parameter.maximumSiValue) {
            return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "Parameter minimum exceeds its maximum."));
        }
    }
    return Result<void>::success();
}

std::string_view domainName(ScientificDomain domain) noexcept {
    switch (domain) {
    case ScientificDomain::Electrical: return "electrical";
    case ScientificDomain::Mechanical: return "mechanical";
    case ScientificDomain::Thermal: return "thermal";
    case ScientificDomain::Control: return "control";
    case ScientificDomain::Digital: return "digital";
    case ScientificDomain::Fluid: return "fluid";
    case ScientificDomain::Mixed: return "mixed";
    }
    return "unknown";
}

std::string_view portKindName(PortKind kind) noexcept {
    switch (kind) {
    case PortKind::Conserving: return "conserving";
    case PortKind::Signal: return "signal";
    }
    return "unknown";
}

std::string_view signalDirectionName(SignalDirection direction) noexcept {
    switch (direction) {
    case SignalDirection::Input: return "input";
    case SignalDirection::Output: return "output";
    case SignalDirection::Bidirectional: return "bidirectional";
    }
    return "unknown";
}

Result<ScientificDomain> parseDomain(std::string_view value) {
    for (const auto domain : {ScientificDomain::Electrical, ScientificDomain::Mechanical, ScientificDomain::Thermal,
                              ScientificDomain::Control, ScientificDomain::Digital, ScientificDomain::Fluid, ScientificDomain::Mixed}) {
        if (domainName(domain) == value) return Result<ScientificDomain>::success(domain);
    }
    return Result<ScientificDomain>::failure(makeError(ErrorCode::CorruptData, "Unknown scientific domain."));
}

Result<PortKind> parsePortKind(std::string_view value) {
    if (value == "conserving") return Result<PortKind>::success(PortKind::Conserving);
    if (value == "signal") return Result<PortKind>::success(PortKind::Signal);
    return Result<PortKind>::failure(makeError(ErrorCode::CorruptData, "Unknown port kind."));
}

Result<SignalDirection> parseSignalDirection(std::string_view value) {
    if (value == "input") return Result<SignalDirection>::success(SignalDirection::Input);
    if (value == "output") return Result<SignalDirection>::success(SignalDirection::Output);
    if (value == "bidirectional") return Result<SignalDirection>::success(SignalDirection::Bidirectional);
    return Result<SignalDirection>::failure(makeError(ErrorCode::CorruptData, "Unknown signal direction."));
}

} // namespace openlab::core
