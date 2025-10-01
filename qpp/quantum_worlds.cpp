#include "qpp/quantum_worlds.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace qpp {
namespace quantum {

namespace {
std::string to_lower(std::string_view text) {
    std::string out(text.begin(), text.end());
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return out;
}

std::string trim(std::string_view text) {
    std::size_t start = 0;
    std::size_t end = text.size();
    while (start < end && std::isspace(static_cast<unsigned char>(text[start])))
        ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1])))
        --end;
    return std::string{text.substr(start, end - start)};
}
} // namespace

BackendKind parse_backend(std::string_view name) {
    auto lowered = to_lower(name);
    if (lowered == "cpu")
        return BackendKind::CPU;
    if (lowered == "qpu_sim" || lowered == "qpu-sim" || lowered == "qpu")
        return BackendKind::QPU_SIM;
    if (lowered == "openqasm_http" || lowered == "openqasm-http" || lowered == "openqasm")
        return BackendKind::OPENQASM_HTTP;
    throw std::invalid_argument("unknown backend: " + std::string{name});
}

FactorRegistry::FactorRegistry() : next_candidate_(2), mapping_(), primes_() {}

void FactorRegistry::clear() {
    mapping_.clear();
    primes_.clear();
    next_candidate_ = 2;
}

FactorRegistry::value_type FactorRegistry::register_factor(const std::string &name) {
    auto [it, inserted] = mapping_.try_emplace(name, value_type{});
    if (!inserted)
        return it->second;

    value_type prime = next_prime();
    it->second = prime;
    return prime;
}

FactorRegistry::value_type FactorRegistry::prime_of(const std::string &name) const {
    return mapping_.at(name);
}

bool FactorRegistry::contains(const std::string &name) const {
    return mapping_.find(name) != mapping_.end();
}

const std::unordered_map<std::string, FactorRegistry::value_type> &
FactorRegistry::mapping() const noexcept {
    return mapping_;
}

const std::vector<FactorRegistry::value_type> &FactorRegistry::primes() const noexcept {
    return primes_;
}

std::size_t FactorRegistry::size() const noexcept { return mapping_.size(); }

bool FactorRegistry::is_prime(value_type candidate) const {
    if (candidate < 2)
        return false;
    if (candidate == 2)
        return true;
    if (candidate % 2 == 0)
        return false;

    const auto limit = static_cast<value_type>(
        std::sqrt(static_cast<long double>(candidate)));
    for (value_type prime : primes_) {
        if (prime < 2)
            continue;
        if (prime > limit)
            break;
        if (candidate % prime == 0)
            return false;
    }

    return true;
}

FactorRegistry::value_type FactorRegistry::next_prime() {
    value_type candidate = next_candidate_;
    if (candidate <= 2)
        candidate = 2;
    if (candidate % 2 == 0 && candidate != 2)
        ++candidate;

    while (!is_prime(candidate))
        candidate += 2;

    primes_.push_back(candidate);
    next_candidate_ = candidate + 1;
    return candidate;
}

WorldSignature::WorldSignature(std::vector<Entry> entries)
    : factors(std::move(entries)) {}

WorldSignature::WorldSignature(std::vector<std::string> labels,
                               std::vector<double> weights) {
    if (!weights.empty() && weights.size() != labels.size())
        throw std::invalid_argument("weights size does not match labels size");
    if (weights.empty())
        weights.assign(labels.size(), 1.0);
    factors.reserve(labels.size());
    for (std::size_t i = 0; i < labels.size(); ++i)
        factors.emplace_back(std::move(labels[i]), weights[i]);
}

void WorldSignature::add_factor(std::string label, double weight) {
    factors.emplace_back(std::move(label), weight);
}

bool WorldSignature::empty() const noexcept { return factors.empty(); }

std::size_t WorldSignature::size() const noexcept { return factors.size(); }

void WorldSignature::sort() {
    std::sort(factors.begin(), factors.end(),
              [](const Entry &a, const Entry &b) { return a.first < b.first; });
}

std::vector<FactorRegistry::value_type>
assign_primes(FactorRegistry &registry, const WorldSignature &signature) {
    std::vector<FactorRegistry::value_type> primes(signature.factors.size());
    std::transform(signature.factors.begin(), signature.factors.end(), primes.begin(),
                   [&registry](const WorldSignature::Entry &entry) {
                       return registry.register_factor(entry.first);
                   });
    return primes;
}

std::vector<double> generate_spectrum(const std::vector<FactorRegistry::value_type> &primes,
                                      const WorldSignature &signature) {
    if (primes.size() != signature.factors.size())
        throw std::invalid_argument("prime list size mismatch with signature");
    std::vector<double> spectrum(primes.size());
    std::transform(primes.begin(), primes.end(), signature.factors.begin(), spectrum.begin(),
                   [](FactorRegistry::value_type prime, const WorldSignature::Entry &entry) {
                       return static_cast<double>(prime) * entry.second;
                   });
    return spectrum;
}

double gaussian_overlap(const std::vector<double> &lhs, const std::vector<double> &rhs,
                        double sigma) {
    if (sigma <= 0.0)
        throw std::invalid_argument("sigma must be positive");
    if (lhs.empty() || rhs.empty())
        return 0.0;
    const double pi = std::acos(-1.0);
    const double norm = 1.0 / (sigma * std::sqrt(2.0 * pi));
    const double denom = 2.0 * sigma * sigma;

    double total = 0.0;
    for (double a : lhs) {
        for (double b : rhs) {
            double diff = a - b;
            total += std::exp(-(diff * diff) / denom);
        }
    }
    return norm * total;
}

std::vector<double> softmax(const std::vector<double> &values, double temperature) {
    if (temperature <= 0.0)
        throw std::invalid_argument("temperature must be positive");
    if (values.empty())
        return {};
    std::vector<double> scaled(values.size());
    std::transform(values.begin(), values.end(), scaled.begin(),
                   [temperature](double v) { return v / temperature; });
    double max_v = *std::max_element(scaled.begin(), scaled.end());
    double sum = 0.0;
    for (double &v : scaled) {
        v = std::exp(v - max_v);
        sum += v;
    }
    if (sum == 0.0)
        return std::vector<double>(values.size(), 1.0 / static_cast<double>(values.size()));
    for (double &v : scaled)
        v /= sum;
    return scaled;
}

std::vector<double> sample_worlds(const std::vector<double> &weights, BackendKind backend,
                                  std::size_t shots, BackendConfiguration config) {
    if (config.max_qubits == 0)
        config.max_qubits = weights.size();
    auto backend_impl = make_backend(backend, std::move(config));
    return backend_impl->run_distribution_experiment(weights, shots);
}

std::vector<double> sample_worlds(const std::vector<double> &weights, BackendKind backend,
                                  std::size_t shots, unsigned seed) {
    BackendConfiguration config;
    config.seed = seed;
    return sample_worlds(weights, backend, shots, std::move(config));
}

QuantumFront make_quantum_front(FactorRegistry &registry, const WorldSignature &signature,
                                BackendKind backend, std::size_t shots, unsigned seed) {
    QuantumFront front;
    front.signature = signature;
    front.signature.sort();
    front.backend = backend;
    front.primes = assign_primes(registry, front.signature);
    front.spectrum = generate_spectrum(front.primes, front.signature);
    front.payload = sample_worlds(front.spectrum, backend, shots, seed);
    return front;
}

std::string signature_to_string(const WorldSignature &signature) {
    std::ostringstream oss;
    bool first = true;
    for (const auto &entry : signature.factors) {
        if (!first)
            oss << ',';
        first = false;
        oss << entry.first << ':' << entry.second;
    }
    return oss.str();
}

WorldSignature signature_from_string(const std::string &text) {
    WorldSignature signature;
    if (text.empty())
        return signature;
    std::stringstream ss(text);
    std::string item;
    while (std::getline(ss, item, ',')) {
        auto trimmed = trim(item);
        if (trimmed.empty())
            continue;
        auto pos = trimmed.find(':');
        if (pos == std::string::npos) {
            signature.add_factor(trimmed, 1.0);
        } else {
            auto label = trim(trimmed.substr(0, pos));
            auto weight_str = trim(trimmed.substr(pos + 1));
            double weight = weight_str.empty() ? 1.0 : std::stod(weight_str);
            signature.add_factor(label, weight);
        }
    }
    signature.sort();
    return signature;
}

} // namespace quantum
} // namespace qpp

