#ifndef QPP_HARDWARE_HPP
#define QPP_HARDWARE_HPP

#include <cstdlib>

namespace qpp {
/// Check for availability of quantum hardware backend.
/// Returns true when the environment variable QPP_HW_AVAILABLE is set to "1".
inline bool hardware_available() {
    const char* env = std::getenv("QPP_HW_AVAILABLE");
    return env && env[0] == '1';
}
} // namespace qpp

#endif // QPP_HARDWARE_HPP
