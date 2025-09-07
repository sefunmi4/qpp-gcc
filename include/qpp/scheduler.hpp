#ifndef QPP_SCHEDULER_HPP
#define QPP_SCHEDULER_HPP

#include <functional>
#include <algorithm>
#include <queue>
#include <vector>
#include <cstddef>
#include <mutex>

namespace qpp {

/// Simple task representation
typedef std::function<void()> TaskFn;

struct Task {
    TaskFn fn;
    int priority{0};
    bool paused{false};
};

/// Priority-aware task scheduler with pause/resume
class Scheduler {
    std::vector<Task> tasks;
    std::mutex tasks_mutex;
public:
    /// Add a task with given priority
    void add(TaskFn fn, int priority = 0) {
        std::lock_guard<std::mutex> lock(tasks_mutex);
        tasks.push_back({fn, priority, false});
    }

    /// Remove a task by index
    void remove(std::size_t index) {
        std::lock_guard<std::mutex> lock(tasks_mutex);
        if (index < tasks.size())
            tasks.erase(tasks.begin() + index);
    }

    /// Change the priority of a task
    void set_priority(std::size_t index, int priority) {
        std::lock_guard<std::mutex> lock(tasks_mutex);
        if (index < tasks.size())
            tasks[index].priority = priority;
    }

    /// Remove all tasks
    void clear() {
        std::lock_guard<std::mutex> lock(tasks_mutex);
        tasks.clear();
    }

    /// Execute all tasks in priority order
    void run() {
        std::vector<Task> local;
        {
            std::lock_guard<std::mutex> lock(tasks_mutex);
            local = tasks;
        }
        std::sort(local.begin(), local.end(),
                  [](const Task& a, const Task& b) { return a.priority > b.priority; });
        for (auto& t : local) {
            if (!t.paused)
                t.fn();
        }
    }

    /// Pause a task by index
    void pause(std::size_t index) {
        std::lock_guard<std::mutex> lock(tasks_mutex);
        if (index < tasks.size())
            tasks[index].paused = true;
    }

    /// Resume a task by index
    void resume(std::size_t index) {
        std::lock_guard<std::mutex> lock(tasks_mutex);
        if (index < tasks.size())
            tasks[index].paused = false;
    }
};

} // namespace qpp

#endif // QPP_SCHEDULER_HPP
