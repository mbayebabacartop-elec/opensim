#pragma once

#include "components.hpp"

#include <cstdint>
#include <cstddef>
#include <queue>
#include <string>
#include <vector>

namespace openlab::core {

class SimulationClock {
public:
    [[nodiscard]] static Result<SimulationClock> create(double startSeconds, double endSeconds, double preferredStepSeconds);

    [[nodiscard]] double nowSeconds() const noexcept { return nowSeconds_; }
    [[nodiscard]] double endSeconds() const noexcept { return endSeconds_; }
    [[nodiscard]] double preferredStepSeconds() const noexcept { return preferredStepSeconds_; }
    [[nodiscard]] bool finished() const noexcept { return nowSeconds_ >= endSeconds_; }

    [[nodiscard]] Result<double> advance();
    [[nodiscard]] Result<void> advanceTo(double nextTimeSeconds);

private:
    SimulationClock(double startSeconds, double endSeconds, double preferredStepSeconds);
    double nowSeconds_{};
    double endSeconds_{};
    double preferredStepSeconds_{};
};

enum class EventKind { ParameterChange, InputUpdate, UserMarker };

struct SimulationEvent {
    double timeSeconds{};
    EventKind kind{EventKind::InputUpdate};
    std::string targetId;
    std::string channelId;
    Quantity value{};
};

class EventQueue {
public:
    [[nodiscard]] Result<void> schedule(SimulationEvent event, double simulationNowSeconds);
    [[nodiscard]] Result<std::vector<SimulationEvent>> popDue(double throughTimeSeconds);
    [[nodiscard]] std::size_t pendingCount() const noexcept { return events_.size(); }

private:
    struct QueuedEvent {
        SimulationEvent event;
        std::uint64_t sequence{};
    };
    struct LaterEvent {
        bool operator()(const QueuedEvent& lhs, const QueuedEvent& rhs) const noexcept;
    };

    std::priority_queue<QueuedEvent, std::vector<QueuedEvent>, LaterEvent> events_;
    std::uint64_t nextSequence_{};
    double lastPopTimeSeconds_{};
};

[[nodiscard]] std::string_view eventKindName(EventKind kind) noexcept;
[[nodiscard]] Result<EventKind> parseEventKind(std::string_view value);

} // namespace openlab::core
