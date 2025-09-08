#ifndef QPP_SIM_CHARACTERS_HPP
#define QPP_SIM_CHARACTERS_HPP

#include <cstddef>
#include <vector>

namespace qpp::sim {

/// Check whether two integers are congruent modulo `mod`.
inline bool proj_mod_eq(std::size_t a, std::size_t b, std::size_t mod) {
    return mod == 0 ? a == b : (a % mod) == (b % mod);
}

/// Simple sieve of Eratosthenes producing primes up to `limit`.
inline std::vector<int> prime_sieve_frequency(int limit) {
    if (limit < 2)
        return {};
    std::vector<bool> sieve(limit + 1, true);
    sieve[0] = sieve[1] = false;
    for (int p = 2; p * p <= limit; ++p) {
        if (sieve[p]) {
            for (int i = p * p; i <= limit; ++i)
                if (proj_mod_eq(i, 0, p))
                    sieve[i] = false;
        }
    }
    std::vector<int> primes;
    for (int i = 2; i <= limit; ++i)
        if (sieve[i])
            primes.push_back(i);
    return primes;
}

} // namespace qpp::sim

#endif // QPP_SIM_CHARACTERS_HPP
