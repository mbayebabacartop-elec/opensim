#include "openlab/core/simulation_time.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace openlab::core {

SimulationClock::SimulationClock(double startSeconds, double endSeconds, double preferredStepSeconds)
    : nowSeconds_(startSeconds), endSeconds_(endSeconds), preferredStepSeconds_(preferredStepSeconds) {}

Result<SimulationClock> SimulationClock::create(double startSeconds, double endSeconds, double preferredStepSeconds) {
    if (!std::isfinite(startSeconds) || !std::isfinite(endSeconds) || !std::isfinite(preferredStepSeconds) ||
        startSeconds < 0.0 || endSeconds <= startSeconds || preferredStepSeconds <= 0.0) {
        return Result<SimulationClock>::failure(makeError(ErrorCode::InvalidArgument, "Clock requires finite SI times, end > start, and a positive step."));
    }
    return Result<SimulationClock>::success(SimulationClock(startSeconds, endSeconds, preferredStepSeconds));
}

Result<double> SimulationClock::advance() {
    if (finished()) {
        return Result<double>::failure(makeError(ErrorCode::Conflict, "Simulation clock is already at its end time."));
    }
    const double next = std::min(endSeconds_, nowSeconds_ + preferredStepSeconds_);
    if (!std::isfinite(next) || next <= nowSeconds_) {
        return Result<double>::failure(makeError(ErrorCode::InvalidArgument, "Clock step is below floating-point resolution."));
    }
    nowSeconds_ = next;
    return Result<double>::success(nowSeconds_);
}

Result<void> SimulationClock::advanceTo(double nextTimeSeconds) {
    if (!std::isfinite(nextTimeSeconds) || nextTimeSeconds <= nowSeconds_ || nextTimeSeconds > endSeconds_) {
        return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "Next simulation time must be finite, increasing, and no later than the end."));
    }
    nowSeconds_ = nextTimeSeconds;
    return Result<void>::success();
}

bool EventQueue::LaterEvent::operator()(const QueuedEvent& lhs, const QueuedEvent& rhs) const noexcept {
    if (lhs.event.timeSeconds != rhs.event.timeSeconds) return lhs.event.timeSeconds > rhs.event.timeSeconds;
    return lhs.sequence > rhs.sequence;
}

Result<void> EventQueue::schedule(SimulationEvent event, double simulationNowSeconds) {
    if (!std::isfinite(simulationNowSeconds) || !std::isfinite(event.timeSeconds) || simulationNowSeconds < 0.0 ||
        event.timeSeconds < simulationNowSeconds || event.targetId.empty() || event.channelId.empty() || !event.value.isFinite()) {
        return Result<void>::failure(makeError(ErrorCode::InvalidArgument, "Event time, target, channel or value is invalid."));
    }
    if (event.timeSeconds < lastPopTimeSeconds_) {
        return Result<void>::failure(makeError(ErrorCode::Conflict, "Cannot schedule an event before the queue's dispatched time."));
    }
    events_.push(QueuedEvent{std::move(event), nextSequence_++});
    return Result<void>::success();
}

Result<std::vector<SimulationEvent>> EventQueue::popDue(double throughTimeSeconds) {
    if (!std::isfinite(throughTimeSeconds) || throughTimeSeconds < lastPopTimeSeconds_) {
        return Result<std::vector<SimulationEvent>>::failure(makeError(ErrorCode::InvalidArgument, "Event queue time must be finite and monotonic."));
    }
    lastPopTimeSeconds_ = throughTimeSeconds;
    std::vector<SimulationEvent> due;
    while (!events_.empty() && events_.top().event.timeSeconds <= throughTimeSeconds) {
        due.push_back(events_.top().event);
        events_.pop();
    }
    return Result<std::vector<SimulationEvent>>::success(std::move(due));
}

std::string_view eventKindName(EventKind kind) noexcept {
    switch (kind) {
    case EventKind::ParameterChange: return "parameter_change";
    case EventKind::InputUpdate: return "input_update";
    case EventKind::UserMarker: return "user_marker";
    }
    return "unknown";
}

Result<EventKind> parseEventKind(std::string_view value) {
    if (value == "parameter_change") return Result<EventKind>::success(EventKind::ParameterChange);
    if (value == "input_update") return Result<EventKind>::success(EventKind::InputUpdate);
    if (value == "user_marker") return Result<EventKind>::success(EventKind::UserMarker);
    return Result<EventKind>::failure(makeError(ErrorCode::CorruptData, "Unknown simulation event kind."));
}

} // namespace openlab::core
