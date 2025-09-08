#include "qpp/backend/QPUBackend.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>

namespace qpp {

namespace {
// Helper to normalise gate strings.
std::string to_lower(const std::string& s) {
    std::string r;
    r.resize(s.size());
    std::transform(s.begin(), s.end(), r.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return r;
}

} // namespace

QPUBackend::QPUBackend() {
    auto devices = discoverDevices();
    if (!devices.empty()) {
        const char* token = std::getenv("QPU_AUTH_TOKEN");
        connect(devices.front(), token ? token : "");
    }
}

std::vector<std::string> QPUBackend::discoverDevices() {
    std::vector<std::string> devices;
    if (const char* pci = std::getenv("QPU_PCI_DEVICE"))
        devices.emplace_back(pci);
    if (const char* usb = std::getenv("QPU_USB_DEVICE"))
        devices.emplace_back(usb);
    if (const char* net = std::getenv("QPU_NET_DEVICE"))
        devices.emplace_back(net);
    return devices;
}

void QPUBackend::connect(const std::string& device, const std::string& token) {
    device_ = device;
    authToken_ = token;
    connected_ = !device.empty();
}

std::string QPUBackend::mapGate(const std::string& gate) {
    // Mapping from generic gate names to vendor opcodes.
    if (gate == "h")
        return "VENDOR_H";
    if (gate == "x")
        return "VENDOR_X";
    if (gate == "y")
        return "VENDOR_Y";
    if (gate == "z")
        return "VENDOR_Z";
    if (gate == "cx")
        return "VENDOR_CX";
    if (gate == "ccx")
        return "VENDOR_CCX";
    return gate; // passthrough for unknown operations
}

void QPUBackend::loadCircuit(const std::string& circuit) {
    program_.clear();
    measuredQubits_.clear();
    shots_ = 0;

    auto process_line = [&](const std::string& line) {
        std::string cleaned;
        cleaned.reserve(line.size());
        for (char c : line) {
            if (std::isalnum(static_cast<unsigned char>(c)))
                cleaned.push_back(c);
            else
                cleaned.push_back(' ');
        }
        std::istringstream ls(cleaned);
        std::string gate;
        if (!(ls >> gate))
            return;
        std::string lower = to_lower(gate);
        if (lower == "shots") {
            ls >> shots_;
            return;
        }
        std::vector<int> args;
        int v;
        while (ls >> v)
            args.push_back(v);
        if (lower == "measure") {
            for (int q : args)
                measuredQubits_.push_back(q);
            return;
        }
        program_.push_back({mapGate(lower), args});
    };

    std::stringstream ss(circuit);
    std::string line;
    while (std::getline(ss, line, ';'))
        process_line(line);
}

RunResult QPUBackend::run() {
    RunResult result;
    result.success = connected_;
    if (!connected_)
        return result;

    // For demonstration we generate random results. A real implementation
    // would marshal 'program_' to the vendor SDK and process the response.
    std::size_t n = measuredQubits_.size();
    if (n == 0)
        return result;

    std::size_t dim = 1ULL << n;
    if (shots_ == 0) {
        double p = 1.0 / static_cast<double>(dim);
        for (std::size_t i = 0; i < dim; ++i) {
            std::string bits;
            for (std::size_t q = 0; q < n; ++q)
                bits.push_back(((i >> q) & 1) ? '1' : '0');
            result.probabilities[bits] = p;
        }
    } else {
        std::uniform_int_distribution<std::size_t> dist(0, dim - 1);
        for (unsigned s = 0; s < shots_; ++s) {
            std::size_t idx = dist(rng_);
            std::string bits;
            for (std::size_t q = 0; q < n; ++q)
                bits.push_back(((idx >> q) & 1) ? '1' : '0');
            result.counts[bits]++;
        }
    }

    if (noiseModel_)
        noiseModel_(result);

    return result;
}

} // namespace qpp

