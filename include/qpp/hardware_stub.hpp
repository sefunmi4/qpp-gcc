#ifndef QPP_HARDWARE_STUB_HPP
#define QPP_HARDWARE_STUB_HPP

#include <string>
#include <vector>
#include <sstream>

namespace qpp {

/// Minimal hardware backend emitting QIR strings
class HardwareStub {
public:
    std::vector<std::string> emitted;

    void emit(const std::string& qir) { emitted.push_back(qir); }

    std::string result() const {
        std::ostringstream oss;
        for (const auto& s : emitted)
            oss << s << "\n";
        return oss.str();
    }
};

} // namespace qpp

#endif // QPP_HARDWARE_STUB_HPP
