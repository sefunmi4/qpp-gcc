#ifndef QPP_BACKEND_FACTORY_HPP
#define QPP_BACKEND_FACTORY_HPP

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>

#include "Backend.hpp"
#include "LocalSimBackend.hpp"
#include "QPUBackend.hpp"

namespace qpp {

/// Factory that selects an appropriate backend implementation.
class BackendFactory {
public:
    /// Create a backend instance.
    ///
    /// Selection order:
    /// 1. If the environment variable \c QPP_BACKEND is set to
    ///    "qpu" or "sim", the corresponding backend is returned.
    /// 2. Otherwise if the configuration file specified by \p configPath
    ///    exists and contains the token "qpu" or "sim" in the first line,
    ///    that backend is forced.
    /// 3. If no selection is forced, a QPU backend is attempted and the
    ///    simulator is used as a fallback when no hardware is available.
    static std::unique_ptr<Backend> create(
        const std::string& configPath = "qpp_backend.conf") {
        std::string choice;
        if (const char* env = std::getenv("QPP_BACKEND"))
            choice = env;
        else {
            std::ifstream cfg(configPath);
            if (cfg)
                std::getline(cfg, choice);
        }
        auto toLower = [](std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            return s;
        };
        choice = toLower(choice);

        if (choice == "sim" || choice == "simulator")
            return std::make_unique<LocalSimBackend>();
        if (choice == "qpu" || choice == "hardware")
            return std::make_unique<QPUBackend>();

        auto qpu = std::make_unique<QPUBackend>();
        if (qpu->available())
            return qpu;
        return std::make_unique<LocalSimBackend>();
    }
};

} // namespace qpp

#endif // QPP_BACKEND_FACTORY_HPP
