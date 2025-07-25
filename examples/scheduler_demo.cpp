#include <iostream>
#include "qpp/scheduler.hpp"
#include "qpp/qregister.hpp"
#include "qpp/hardware_stub.hpp"

int main() {
    qpp::Scheduler sched;
    qpp::HardwareStub hw;
    qpp::qregister qr(1);

    sched.add([&]{
        hw.emit("H 0");
        qr.data().apply_h(0);
    }, 1);

    sched.add([&]{
        hw.emit("X 0");
        qr.data().apply_x(0);
    }, 0);

    sched.run();
    std::cout << hw.result();
    return 0;
}
