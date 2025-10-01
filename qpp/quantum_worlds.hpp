#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "qpp/quantum/backend.hpp"

namespace qpp {
namespace quantum {

/** \brief Enumeration describing which backend is targeted. */
enum class BackendKind {
    CPU,
    QPU_SIM,
    OPENQASM_HTTP
};

/** \brief Convert a textual backend identifier into a BackendKind. */
BackendKind parse_backend(std::string_view name);

/** \brief Registry assigning unique prime numbers to named factors. */
class FactorRegistry {
  public:
    using value_type = std::uint64_t;

    FactorRegistry();

    /** \brief Clear all registered factors. */
    void clear();

    /** \brief Register a factor and return the associated prime number. */
    value_type register_factor(const std::string &name);

    /** \brief Retrieve the prime number associated with a factor.
     *  \throws std::out_of_range if the factor is unknown.
     */
    value_type prime_of(const std::string &name) const;

    /** \brief Query if a factor is already registered. */
    bool contains(const std::string &name) const;

    /** \brief Access the internal mapping for inspection. */
    const std::unordered_map<std::string, value_type> &mapping() const noexcept;

    /** \brief Access the ordered array of generated primes. */
    const std::vector<value_type> &primes() const noexcept;

    /** \brief Number of registered factors. */
    std::size_t size() const noexcept;

  private:
    bool is_prime(value_type candidate) const;
    value_type next_prime();

    value_type next_candidate_;
    std::unordered_map<std::string, value_type> mapping_;
    std::vector<value_type> primes_;
};

/** \brief Encodes a world signature with weighted named factors. */
struct WorldSignature {
    using Entry = std::pair<std::string, double>;

    std::vector<Entry> factors;

    WorldSignature() = default;
    explicit WorldSignature(std::vector<Entry> entries);
    WorldSignature(std::vector<std::string> labels, std::vector<double> weights);

    void add_factor(std::string label, double weight = 1.0);
    bool empty() const noexcept;
    std::size_t size() const noexcept;

    /** \brief Sort the factors deterministically by their labels. */
    void sort();
};

/** \brief Assign primes to all factors of the signature using the registry. */
std::vector<FactorRegistry::value_type> assign_primes(FactorRegistry &registry,
                                                      const WorldSignature &signature);

/** \brief Create a spectral representation for the signature using primes. */
std::vector<double> generate_spectrum(const std::vector<FactorRegistry::value_type> &primes,
                                      const WorldSignature &signature);

/** \brief Compute the Gaussian overlap of two spectra. */
double gaussian_overlap(const std::vector<double> &lhs, const std::vector<double> &rhs,
                        double sigma);

/** \brief Numerically stable softmax. */
std::vector<double> softmax(const std::vector<double> &values, double temperature = 1.0);

/** \brief Sample signatures for a backend.
 *
 *  For BackendKind::CPU the returned vector represents empirical probabilities
 *  from a deterministic RNG seeded with \p seed. For BackendKind::QPU_SIM the
 *  returned vector contains normalized amplitudes derived from the softmax
 *  distribution.
 */
std::vector<double> sample_worlds(const std::vector<double> &weights, BackendKind backend,
                                  std::size_t shots, BackendConfiguration config);

std::vector<double> sample_worlds(const std::vector<double> &weights, BackendKind backend,
                                  std::size_t shots, unsigned seed = 0);

/** \brief Composite container capturing the state of a quantum front. */
struct QuantumFront {
    WorldSignature signature;
    std::vector<FactorRegistry::value_type> primes;
    std::vector<double> spectrum;
    std::vector<double> payload;
    BackendKind backend;
};

/** \brief Build a quantum front using the provided backend. */
QuantumFront make_quantum_front(FactorRegistry &registry, const WorldSignature &signature,
                                BackendKind backend, std::size_t shots, unsigned seed = 0);

/** \brief Serialize a world signature into a textual representation. */
std::string signature_to_string(const WorldSignature &signature);

/** \brief Parse a signature from its textual representation. */
WorldSignature signature_from_string(const std::string &text);

} // namespace quantum
} // namespace qpp

