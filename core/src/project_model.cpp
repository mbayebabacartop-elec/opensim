#include "openlab/core/project_model.hpp"

#include <algorithm>
#include <cmath>
#include <set>

namespace openlab::core {
namespace {
bool canSource(SignalDirection direction) {
    return direction == SignalDirection::Output || direction == SignalDirection::Bidirectional;
}
bool canSink(SignalDirection direction) {
    return direction == SignalDirection::Input || direction == SignalDirection::Bidirectional;
}
} // namespace

ProjectModel::ProjectModel(std::string id, std::string name)
    : id_(std::move(id)), name_(std::move(name)) {}

Result<void> ProjectModel::addDefinition(ComponentDefinition definition) {
    auto valid = validateDefinition(definition);
    if (!valid) return valid;
    if (definitions_.contains(definition.ref)) {
        return Result<void>::failure(makeError(ErrorCode::Conflict, "Component definition version already exists in this project."));
    }
    definitions_.emplace(definition.ref, std::move(definition));
    return Result<void>::success();
}

Result<void> ProjectModel::addComponent(ComponentInstance component) {
    if (component.id.empty()) {
        return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "Component instance id cannot be empty."));
    }
    if (components_.contains(component.id)) {
        return Result<void>::failure(makeError(ErrorCode::Conflict, "Component instance id already exists."));
    }
    const auto definitionIt = definitions_.find(component.definition);
    if (definitionIt == definitions_.end()) {
        return Result<void>::failure(makeError(ErrorCode::NotFound, "Component definition/version is missing from the project."));
    }

    const auto& definition = definitionIt->second;
    for (auto it = component.parameters.begin(); it != component.parameters.end();) {
        const auto* spec = findParameter(component, it->first);
        if (!spec) {
            return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "Instance contains an unknown parameter."));
        }
        if (!it->second.isFinite() || !it->second.hasDimension(spec->dimension)) {
            return Result<void>::failure(makeError(ErrorCode::Incompatible, "Parameter value is non-finite or has the wrong SI dimension."));
        }
        if ((spec->minimumSiValue && it->second.si_value < *spec->minimumSiValue) ||
            (spec->maximumSiValue && it->second.si_value > *spec->maximumSiValue)) {
            return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "Parameter value is outside its declared limits."));
        }
        ++it;
    }
    for (const auto& spec : definition.parameters) {
        if (!component.parameters.contains(spec.id)) {
            component.parameters.emplace(spec.id, Quantity{spec.defaultSiValue, spec.dimension});
        }
    }

    components_.emplace(component.id, std::move(component));
    return Result<void>::success();
}

const ParameterDefinition* ProjectModel::findParameter(const ComponentInstance& instance, std::string_view parameterId) const noexcept {
    const auto definitionIt = definitions_.find(instance.definition);
    if (definitionIt == definitions_.end()) return nullptr;
    const auto& parameters = definitionIt->second.parameters;
    const auto it = std::find_if(parameters.begin(), parameters.end(), [parameterId](const auto& parameter) {
        return parameter.id == parameterId;
    });
    return it == parameters.end() ? nullptr : &*it;
}

Result<void> ProjectModel::setParameter(std::string_view componentId, std::string_view parameterId, Quantity value) {
    const auto componentIt = components_.find(componentId);
    if (componentIt == components_.end()) {
        return Result<void>::failure(makeError(ErrorCode::NotFound, "Component instance does not exist."));
    }
    const auto* spec = findParameter(componentIt->second, parameterId);
    if (!spec) {
        return Result<void>::failure(makeError(ErrorCode::NotFound, "Parameter is not declared by the component definition."));
    }
    if (!value.isFinite() || !value.hasDimension(spec->dimension)) {
        return Result<void>::failure(makeError(ErrorCode::Incompatible, "Parameter value is non-finite or has the wrong SI dimension."));
    }
    if ((spec->minimumSiValue && value.si_value < *spec->minimumSiValue) ||
        (spec->maximumSiValue && value.si_value > *spec->maximumSiValue)) {
        return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "Parameter value is outside its declared limits."));
    }
    componentIt->second.parameters[std::string(parameterId)] = value;
    return Result<void>::success();
}

const PortDefinition* ProjectModel::findPort(const PortRef& ref) const noexcept {
    const auto componentIt = components_.find(ref.componentId);
    if (componentIt == components_.end()) return nullptr;
    const auto definitionIt = definitions_.find(componentIt->second.definition);
    if (definitionIt == definitions_.end()) return nullptr;
    const auto& ports = definitionIt->second.ports;
    const auto portIt = std::find_if(ports.begin(), ports.end(), [&ref](const auto& port) { return port.id == ref.portId; });
    return portIt == ports.end() ? nullptr : &*portIt;
}

Result<void> ProjectModel::connect(PortRef first, PortRef second) {
    if (first == second) {
        return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "A port cannot be connected to itself."));
    }
    const auto* firstPort = findPort(first);
    const auto* secondPort = findPort(second);
    if (!firstPort || !secondPort) {
        return Result<void>::failure(makeError(ErrorCode::NotFound, "Connection references an unknown component or port."));
    }
    if (firstPort->domain != secondPort->domain || firstPort->kind != secondPort->kind) {
        return Result<void>::failure(makeError(ErrorCode::Incompatible, "Connected ports must share a domain and connector kind."));
    }

    if (firstPort->kind == PortKind::Conserving) {
        if (firstPort->effortDimension != secondPort->effortDimension || firstPort->flowDimension != secondPort->flowDimension) {
            return Result<void>::failure(makeError(ErrorCode::Incompatible, "Conserving ports have incompatible SI dimensions."));
        }
        if (second < first) std::swap(first, second);
    } else {
        if (firstPort->signalDimension != secondPort->signalDimension) {
            return Result<void>::failure(makeError(ErrorCode::Incompatible, "Signal ports have incompatible SI dimensions."));
        }
        if (canSource(firstPort->direction) && canSink(secondPort->direction)) {
            // Keep source before sink for deterministic serialization.
        } else if (canSource(secondPort->direction) && canSink(firstPort->direction)) {
            std::swap(first, second);
            std::swap(firstPort, secondPort);
        } else {
            return Result<void>::failure(makeError(ErrorCode::Incompatible, "Signal connection requires an output and an input."));
        }
        if (secondPort->direction == SignalDirection::Input) {
            const bool alreadyDriven = std::any_of(connections_.begin(), connections_.end(), [&second](const auto& connection) {
                return connection.second == second;
            });
            if (alreadyDriven) {
                return Result<void>::failure(makeError(ErrorCode::Conflict, "Signal input already has a source."));
            }
        }
    }

    const Connection connection{first, second};
    const bool duplicate = std::any_of(connections_.begin(), connections_.end(), [&connection](const auto& existing) {
        return existing == connection || (existing.first == connection.second && existing.second == connection.first);
    });
    if (duplicate) {
        return Result<void>::failure(makeError(ErrorCode::Conflict, "Connection already exists."));
    }
    connections_.push_back(connection);
    return Result<void>::success();
}

Result<void> ProjectModel::validate() const {
    if (id_.empty() || name_.empty()) {
        return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "Project requires an id and a name."));
    }
    for (const auto& [ref, definition] : definitions_) {
        if (ref != definition.ref) {
            return Result<void>::failure(makeError(ErrorCode::CorruptData, "Definition map key differs from its definition reference."));
        }
        auto valid = validateDefinition(definition);
        if (!valid) return valid;
    }
    for (const auto& [id, instance] : components_) {
        if (id != instance.id || !definitions_.contains(instance.definition)) {
            return Result<void>::failure(makeError(ErrorCode::CorruptData, "Component instance has an inconsistent id or missing definition."));
        }
        for (const auto& [parameterId, value] : instance.parameters) {
            const auto* spec = findParameter(instance, parameterId);
            if (!spec || !value.isFinite() || !value.hasDimension(spec->dimension)) {
                return Result<void>::failure(makeError(ErrorCode::CorruptData, "Component instance has an invalid parameter."));
            }
        }
    }
    for (const auto& connection : connections_) {
        if (!findPort(connection.first) || !findPort(connection.second)) {
            return Result<void>::failure(makeError(ErrorCode::CorruptData, "Project connection references a missing port."));
        }
    }
    return Result<void>::success();
}

} // namespace openlab::core
