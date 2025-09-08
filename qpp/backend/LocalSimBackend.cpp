#include "qpp/backend/LocalSimBackend.hpp"

#include <algorithm>
#include <cctype>
#include <numeric>
#include <sstream>

namespace qpp {

LocalSimBackend::LocalSimBackend(int max_qubits, bool use_fftw, unsigned seed)
    : maxQubits_(max_qubits), useFFTW_(use_fftw), rng_(seed) {}

void LocalSimBackend::loadCircuit(const std::string& circuit) {
    ops_.clear();
    measuredQubits_.clear();
    shots_ = 0;

    // Normalize string: split by ';' or newlines
    int max_qubit = -1;
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
        std::string lower;
        lower.resize(gate.size());
        std::transform(gate.begin(), gate.end(), lower.begin(),
                       [](unsigned char ch) { return std::tolower(ch); });
        if (lower == "shots") {
            ls >> shots_;
            return;
        }
        std::vector<int> args;
        int v;
        while (ls >> v)
            args.push_back(v);
        if (lower == "measure") {
            for (int q : args) {
                measuredQubits_.push_back(q);
                max_qubit = std::max(max_qubit, q);
            }
            return;
        }
        for (int q : args)
            max_qubit = std::max(max_qubit, q);
        ops_.push_back({lower, args});
    };

    std::string line;
    std::stringstream ss(circuit);
    while (std::getline(ss, line, ';'))
        process_line(line);

    int required_qubits = max_qubit + 1;
    if (required_qubits < 0)
        required_qubits = 0;
    psi_ = StateVector(required_qubits);

    for (const auto& op : ops_) {
        if (op.gate == "h" && !op.args.empty())
            psi_.apply_h(op.args[0]);
        else if (op.gate == "x" && !op.args.empty())
            psi_.apply_x(op.args[0]);
        else if (op.gate == "y" && !op.args.empty())
            psi_.apply_y(op.args[0]);
        else if (op.gate == "z" && !op.args.empty())
            psi_.apply_z(op.args[0]);
        else if (op.gate == "cx" && op.args.size() >= 2)
            psi_.apply_cx(op.args[0], op.args[1]);
        else if (op.gate == "ccx" && op.args.size() >= 3)
            psi_.apply_ccx(op.args[0], op.args[1], op.args[2]);
    }
}

RunResult LocalSimBackend::run() {
    RunResult result;
    const auto& amp = psi_.data().amplitude;
    std::size_t dim = amp.size();

    std::vector<int> meas = measuredQubits_;
    if (meas.empty()) {
        int n = psi_.data().qubits();
        meas.resize(n);
        std::iota(meas.begin(), meas.end(), 0);
    }

    if (shots_ == 0) {
        for (std::size_t i = 0; i < dim; ++i) {
            double p = std::norm(amp[i]);
            if (p == 0.0)
                continue;
            std::string bits;
            for (int q : meas)
                bits.push_back(((i >> q) & 1) ? '1' : '0');
            result.probabilities[bits] += p;
        }
    } else {
        std::vector<double> probs(dim);
        for (std::size_t i = 0; i < dim; ++i)
            probs[i] = std::norm(amp[i]);
        std::discrete_distribution<std::size_t> dist(probs.begin(), probs.end());
        for (unsigned s = 0; s < shots_; ++s) {
            std::size_t idx = dist(rng_);
            std::string bits;
            for (int q : meas)
                bits.push_back(((idx >> q) & 1) ? '1' : '0');
            result.counts[bits]++;
        }
    }
    return result;
}

} // namespace qpp
