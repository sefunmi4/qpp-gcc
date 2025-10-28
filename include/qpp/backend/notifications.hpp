#pragma once

#include <cstddef>
#include <functional>
#include <mutex>
#include <string_view>

namespace qpp::backend {

/// Kind of notification emitted by quantum-aware containers.
enum class NotificationKind { Clone, Measurement };

/// Detailed description of an emitted notification.
struct Notification {
    NotificationKind kind{NotificationKind::Clone};
    std::string_view source{};
    std::size_t size = 0;
    std::size_t index = 0;
    double probability = 0.0;
};

using NotificationSink = std::function<void(const Notification&)>;

/// Install a sink that receives backend notifications. Passing an empty
/// std::function disables notifications.
inline NotificationSink& sink_instance() {
    static NotificationSink sink;
    return sink;
}

inline std::mutex& sink_mutex() {
    static std::mutex mtx;
    return mtx;
}

inline void set_notification_sink(NotificationSink sink) {
    std::lock_guard<std::mutex> lock(sink_mutex());
    sink_instance() = std::move(sink);
}

/// Emit a notification to the currently installed sink, if any.
inline void notify(Notification notification) {
    NotificationSink current;
    {
        std::lock_guard<std::mutex> lock(sink_mutex());
        current = sink_instance();
    }
    if (current)
        current(notification);
}

} // namespace qpp::backend

