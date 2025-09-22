#include "gtest/gtest.h"

#include "qpp/quantum_worlds.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <string>
#include <vector>

using namespace qpp::quantum;

TEST(FactorRegistryTest, AssignsUniquePrimes) {
    FactorRegistry registry;

    auto f_with_prime = registry.register_factor("f_with");
    auto g_relation_prime = registry.register_factor("g_relation");
    auto ancillary_prime = registry.register_factor("ancilla");

    EXPECT_EQ(static_cast<FactorRegistry::value_type>(2), f_with_prime);
    EXPECT_NE(f_with_prime, g_relation_prime);
    EXPECT_NE(g_relation_prime, ancillary_prime);
    EXPECT_EQ(f_with_prime, registry.register_factor("f_with"));
    EXPECT_TRUE(registry.contains("g_relation"));
    EXPECT_EQ(static_cast<std::size_t>(3), registry.size());
}

TEST(SignatureParsingTest, HandlesFWithAndGRelationConstraints) {
    WorldSignature expected;
    expected.add_factor("g_relation", 0.75);
    expected.add_factor("f_with", 1.25);
    expected.sort();

    auto serialized = signature_to_string(expected);
    EXPECT_EQ(std::string("f_with:1.25,g_relation:0.75"), serialized);

    auto parsed = signature_from_string(" g_relation:0.75 , f_with : 1.25 ");
    EXPECT_EQ(expected.size(), parsed.size());
    EXPECT_EQ(std::string("f_with"), parsed.factors[0].first);
    EXPECT_NEAR(1.25, parsed.factors[0].second, 1e-9);
    EXPECT_EQ(std::string("g_relation"), parsed.factors[1].first);
    EXPECT_NEAR(0.75, parsed.factors[1].second, 1e-9);
}

TEST(SpectrumTest, MaintainsSpectralOrdering) {
    FactorRegistry registry;
    WorldSignature signature({{"g_relation", 0.4}, {"ancilla", 0.7}, {"f_with", 1.1}});
    signature.sort();

    auto primes = assign_primes(registry, signature);
    EXPECT_EQ(static_cast<std::size_t>(3), primes.size());

    EXPECT_EQ(std::string("ancilla"), signature.factors[0].first);
    EXPECT_EQ(std::string("f_with"), signature.factors[1].first);
    EXPECT_EQ(std::string("g_relation"), signature.factors[2].first);

    EXPECT_EQ(static_cast<FactorRegistry::value_type>(2), primes[0]);
    EXPECT_EQ(static_cast<FactorRegistry::value_type>(3), primes[1]);
    EXPECT_EQ(static_cast<FactorRegistry::value_type>(5), primes[2]);

    auto spectrum = generate_spectrum(primes, signature);
    EXPECT_EQ(primes.size(), spectrum.size());
    EXPECT_NEAR(1.4, spectrum[0], 1e-9); // 2 * 0.7
    EXPECT_NEAR(3.3, spectrum[1], 1e-9); // 3 * 1.1
    EXPECT_NEAR(2.0, spectrum[2], 1e-9); // 5 * 0.4

    WorldSignature permuted({{"f_with", 1.1}, {"g_relation", 0.4}, {"ancilla", 0.7}});
    permuted.sort();
    auto primes_permuted = assign_primes(registry, permuted);
    auto spectrum_permuted = generate_spectrum(primes_permuted, permuted);

    EXPECT_EQ(primes.size(), primes_permuted.size());
    for (std::size_t i = 0; i < primes.size(); ++i)
        EXPECT_EQ(primes[i], primes_permuted[i]);

    EXPECT_EQ(spectrum.size(), spectrum_permuted.size());
    for (std::size_t i = 0; i < spectrum.size(); ++i)
        EXPECT_NEAR(spectrum[i], spectrum_permuted[i], 1e-9);
}

TEST(GaussianOverlapTest, DistinguishesAlignedSpectra) {
    std::vector<double> reference{1.4, 3.3, 5.0};
    std::vector<double> displaced{10.0, 12.0, 14.0};

    double sigma = 0.75;
    double identical = gaussian_overlap(reference, reference, sigma);
    double offset = gaussian_overlap(reference, displaced, sigma);
    double symmetric = gaussian_overlap(displaced, reference, sigma);

    EXPECT_GT(identical, offset);
    EXPECT_NEAR(offset, symmetric, 1e-9);
    EXPECT_NEAR(identical, gaussian_overlap(reference, reference, sigma), 1e-12);
}

TEST(SoftmaxTest, NormalizesAndRespectsTemperature) {
    std::vector<double> weights{0.2, 1.0, -0.5, 0.7};
    auto base = softmax(weights);

    double sum = std::accumulate(base.begin(), base.end(), 0.0);
    EXPECT_NEAR(1.0, sum, 1e-9);

    auto max_pos = std::max_element(weights.begin(), weights.end()) - weights.begin();
    auto max_prob = std::max_element(base.begin(), base.end()) - base.begin();
    EXPECT_EQ(max_pos, max_prob);

    auto cooled = softmax(weights, 0.5);
    auto warmed = softmax(weights, 2.0);

    EXPECT_NEAR(1.0, std::accumulate(cooled.begin(), cooled.end(), 0.0), 1e-9);
    EXPECT_NEAR(1.0, std::accumulate(warmed.begin(), warmed.end(), 0.0), 1e-9);

    EXPECT_GT(*std::max_element(cooled.begin(), cooled.end()),
              *std::max_element(base.begin(), base.end()));
    EXPECT_LT(*std::max_element(warmed.begin(), warmed.end()),
              *std::max_element(base.begin(), base.end()));
}

TEST(SampleWorldsTest, CpuQpuParity) {
    std::vector<double> weights{1.0, 2.5, 0.75, -0.5};

    auto expected = softmax(weights);

    auto cpu_distribution = sample_worlds(weights, BackendKind::CPU, 0, 2024);
    EXPECT_EQ(expected.size(), cpu_distribution.size());
    for (std::size_t i = 0; i < expected.size(); ++i)
        EXPECT_NEAR(expected[i], cpu_distribution[i], 1e-9);

    auto cpu_counts = sample_worlds(weights, BackendKind::CPU, 60000, 2024);
    for (std::size_t i = 0; i < expected.size(); ++i)
        EXPECT_NEAR(expected[i], cpu_counts[i], 5e-3);

    auto qpu_payload = sample_worlds(weights, BackendKind::QPU_SIM, 0, 2024);
    EXPECT_EQ(expected.size(), qpu_payload.size());
    for (std::size_t i = 0; i < expected.size(); ++i)
        EXPECT_NEAR(expected[i], qpu_payload[i] * qpu_payload[i], 1e-9);
}

TEST(QuantumFrontTest, BuildsConsistentWorld) {
    FactorRegistry registry;
    WorldSignature signature({{"f_with", 1.2}, {"g_relation", 0.6}, {"context", 0.9}});

    auto front = make_quantum_front(registry, signature, BackendKind::CPU, 10000, 7);

    EXPECT_EQ(BackendKind::CPU, front.backend);
    EXPECT_EQ(signature.size(), front.signature.size());
    EXPECT_EQ(std::string("context"), front.signature.factors[0].first);
    EXPECT_NEAR(front.signature.factors[0].second, 0.9, 1e-9);

    EXPECT_EQ(front.signature.size(), front.primes.size());
    EXPECT_EQ(front.primes.size(), front.spectrum.size());

    auto expected_probs = sample_worlds(front.spectrum, BackendKind::CPU, 0, 7);
    EXPECT_EQ(expected_probs.size(), front.payload.size());
    for (std::size_t i = 0; i < expected_probs.size(); ++i)
        EXPECT_NEAR(expected_probs[i], front.payload[i], 1e-2);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

