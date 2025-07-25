#ifndef QPP_SCHEDULER_HPP
#define QPP_SCHEDULER_HPP

#include <functional>
#include <queue>
#include <vector>
#include <cstddef>

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
public:
    /// Add a task with given priority
    void add(TaskFn fn, int priority = 0) {
        tasks.push_back({fn, priority, false});
    }

    /// Execute all tasks in priority order
    void run() {
        std::sort(tasks.begin(), tasks.end(),
                  [](const Task& a, const Task& b) { return a.priority > b.priority; });
        for (auto& t : tasks) {
            if (!t.paused)
                t.fn();
        }
    }

    /// Pause a task by index
    void pause(std::size_t index) {
        if (index < tasks.size())
            tasks[index].paused = true;
    }

    /// Resume a task by index
    void resume(std::size_t index) {
        if (index < tasks.size())
            tasks[index].paused = false;
    }
};

} // namespace qpp

#endif // QPP_SCHEDULER_HPP
