#include "compiler.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

namespace {

fs::path find_repo_root(fs::path start) {
    while (!start.empty()) {
        if (fs::exists(start / "include" / "qpp"))
            return start;
        auto parent = start.parent_path();
        if (parent == start)
            break;
        start = parent;
    }
    return {};
}

std::string quote(const fs::path& p) {
    return std::string("\"") + p.string() + "\"";
}

} // namespace

int compile_and_run(const std::string& source_path) {
    fs::path root = find_repo_root(fs::current_path());
    if (root.empty()) {
        std::cerr << "Could not locate repository root containing include/qpp\n";
        return 1;
    }

    fs::path include = root / "include";
    fs::path local_backend = root / "qpp" / "backend" / "LocalSimBackend.cpp";
    fs::path qpu_backend = root / "qpp" / "backend" / "QPUBackend.cpp";

    auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    fs::path out = fs::temp_directory_path() / ("qpp-" + std::to_string(now));

    std::ostringstream cmd;
    cmd << "g++ -std=c++17 -x c++ -I" << quote(include)
        << " -I" << quote(root)
        << " " << quote(fs::path(source_path))
        << " " << quote(local_backend)
        << " " << quote(qpu_backend)
        << " -o " << quote(out);

    if (std::system(cmd.str().c_str()) != 0) {
        std::cerr << "Compilation failed\n";
        return 1;
    }

    int ret = std::system(quote(out).c_str());
    fs::remove(out);
    return ret;
}

