#ifndef QPP_API_SAMPLER_HPP
#define QPP_API_SAMPLER_HPP

#include <map>
#include <random>
#include <string>
#include <vector>

namespace qpp {
namespace api {

/// Utility for sampling measurement outcomes from probability distributions.
struct Sampler {
    template <typename RNG>
    static std::map<std::string, unsigned>
    sample(const std::map<std::string, double>& probs, unsigned shots, RNG& rng) {
        std::vector<std::string> outcomes;
        std::vector<double> weights;
        outcomes.reserve(probs.size());
        weights.reserve(probs.size());
        for (const auto& kv : probs) {
            outcomes.push_back(kv.first);
            weights.push_back(kv.second);
        }
        std::discrete_distribution<std::size_t> dist(weights.begin(), weights.end());
        std::map<std::string, unsigned> counts;
        for (unsigned i = 0; i < shots; ++i) {
            std::size_t idx = dist(rng);
            counts[outcomes[idx]]++;
        }
        return counts;
    }
};

} // namespace api
} // namespace qpp

#endif // QPP_API_SAMPLER_HPP
