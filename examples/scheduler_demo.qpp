#include <iostream>
#include "qpp/scheduler.hpp"
#include "qpp/qregister.hpp"
#include "qpp/hardware_stub.hpp"

int main() {
    qpp::Scheduler sched;
    qpp::HardwareStub hw;
    qpp::qregister qr(1);

    // First task
    sched.add([&]{
        hw.emit("H 0");
        qr.data().apply_h(0);
    }, 1);

    // Second task
    sched.add([&]{
        hw.emit("X 0");
        qr.data().apply_x(0);
    }, 0);

    // Third task that will be removed
    sched.add([&]{
        hw.emit("Z 0");
        qr.data().apply_z(0);
    }, 0);

    // Increase priority of the second task and remove the third
    sched.set_priority(1, 2);
    sched.remove(2);

    sched.run();
    std::cout << hw.result();
    return 0;
}
