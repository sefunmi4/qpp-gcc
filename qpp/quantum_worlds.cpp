#include "qpp/quantum_worlds.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <random>
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
    throw std::invalid_argument("unknown backend: " + std::string{name});
}

FactorRegistry::FactorRegistry() : next_candidate_(2), mapping_() {}

void FactorRegistry::clear() {
    mapping_.clear();
    next_candidate_ = 2;
}

FactorRegistry::value_type FactorRegistry::register_factor(const std::string &name) {
    auto it = mapping_.find(name);
    if (it != mapping_.end())
        return it->second;

    value_type prime = next_prime(next_candidate_);
    mapping_.emplace(name, prime);
    next_candidate_ = prime + 1;
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

std::size_t FactorRegistry::size() const noexcept { return mapping_.size(); }

bool FactorRegistry::is_prime(value_type candidate) {
    if (candidate < 2)
        return false;
    if (candidate == 2)
        return true;
    if (candidate % 2 == 0)
        return false;
    for (value_type i = 3; i * i <= candidate; i += 2) {
        if (candidate % i == 0)
            return false;
    }
    return true;
}

FactorRegistry::value_type FactorRegistry::next_prime(value_type candidate) {
    if (candidate <= 2)
        return 2;
    if (candidate % 2 == 0)
        ++candidate;
    while (!is_prime(candidate))
        candidate += 2;
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
    std::vector<FactorRegistry::value_type> primes;
    primes.reserve(signature.factors.size());
    for (const auto &entry : signature.factors)
        primes.push_back(registry.register_factor(entry.first));
    return primes;
}

std::vector<double> generate_spectrum(const std::vector<FactorRegistry::value_type> &primes,
                                      const WorldSignature &signature) {
    if (primes.size() != signature.factors.size())
        throw std::invalid_argument("prime list size mismatch with signature");
    std::vector<double> spectrum(primes.size());
    for (std::size_t i = 0; i < primes.size(); ++i)
        spectrum[i] = static_cast<double>(primes[i]) * signature.factors[i].second;
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
                                  std::size_t shots, unsigned seed) {
    auto probabilities = softmax(weights);
    if (backend == BackendKind::CPU) {
        if (probabilities.empty())
            return {};
        if (shots == 0)
            return probabilities;
        std::vector<double> samples(probabilities.size(), 0.0);
        std::mt19937 rng(seed);
        std::discrete_distribution<std::size_t> dist(probabilities.begin(), probabilities.end());
        for (std::size_t i = 0; i < shots; ++i)
            samples[dist(rng)] += 1.0;
        for (double &v : samples)
            v /= static_cast<double>(shots);
        return samples;
    }

    std::vector<double> amplitudes(probabilities.size(), 0.0);
    double norm = 0.0;
    for (std::size_t i = 0; i < probabilities.size(); ++i) {
        amplitudes[i] = std::sqrt(probabilities[i]);
        norm += amplitudes[i] * amplitudes[i];
    }
    if (norm > 0.0) {
        double inv = 1.0 / std::sqrt(norm);
        for (double &v : amplitudes)
            v *= inv;
    }
    return amplitudes;
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

