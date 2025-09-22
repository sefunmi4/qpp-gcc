#include "testing_framework.hpp"

#include "qpp/quantum_worlds.hpp"

#include <numeric>
#include <stdexcept>
#include <vector>

using qpp::quantum::BackendKind;
using qpp::quantum::FactorRegistry;
using qpp::quantum::WorldSignature;

QPP_TEST(ParseBackendRecognizesNames) {
    QPP_EXPECT_EQ(qpp::quantum::parse_backend("cpu"), BackendKind::CPU);
    QPP_EXPECT_EQ(qpp::quantum::parse_backend("QPU"), BackendKind::QPU_SIM);
    QPP_EXPECT_EQ(qpp::quantum::parse_backend("qpu_sim"), BackendKind::QPU_SIM);
    QPP_EXPECT_THROW(qpp::quantum::parse_backend("invalid"), std::invalid_argument);
}

QPP_TEST(FactorRegistryAssignsUniquePrimes) {
    FactorRegistry registry;
    auto cat = registry.register_factor("cat");
    auto hat = registry.register_factor("hat");
    QPP_EXPECT_NE(cat, hat);
    QPP_EXPECT_EQ(registry.prime_of("cat"), cat);
    QPP_EXPECT_TRUE(registry.contains("hat"));
    QPP_EXPECT_EQ(registry.size(), static_cast<std::size_t>(2));
    registry.clear();
    QPP_EXPECT_EQ(registry.size(), static_cast<std::size_t>(0));
}

QPP_TEST(WorldSignatureSorting) {
    WorldSignature signature;
    signature.add_factor("beta", 0.5);
    signature.add_factor("alpha", 1.0);
    signature.sort();
    QPP_EXPECT_EQ(signature.factors.front().first, "alpha");
    QPP_EXPECT_EQ(signature.factors.back().first, "beta");
}

QPP_TEST(SignatureSerializationRoundTrip) {
    WorldSignature signature({{"cat", 1.0}, {"hat", 0.5}});
    signature.sort();
    auto text = qpp::quantum::signature_to_string(signature);
    auto parsed = qpp::quantum::signature_from_string(text);
    QPP_EXPECT_EQ(parsed.size(), signature.size());
    QPP_EXPECT_EQ(parsed.factors.front().first, "cat");
    QPP_EXPECT_NEAR(parsed.factors.front().second, 1.0, 1e-12);
    QPP_EXPECT_EQ(parsed.factors.back().first, "hat");
    QPP_EXPECT_NEAR(parsed.factors.back().second, 0.5, 1e-12);
}

QPP_TEST(SoftmaxNormalizesProbabilities) {
    std::vector<double> values{1.0, 2.0, 3.0};
    auto probabilities = qpp::quantum::softmax(values);
    double sum = std::accumulate(probabilities.begin(), probabilities.end(), 0.0);
    QPP_EXPECT_NEAR(sum, 1.0, 1e-12);
    for (double probability : probabilities) {
        QPP_EXPECT_TRUE(probability >= 0.0);
    }
}

QPP_TEST(MakeQuantumFrontBuildsConsistentData) {
    FactorRegistry registry;
    WorldSignature signature({{"cat", 1.0}, {"hat", 2.0}, {"pet", 0.75}});
    auto front = qpp::quantum::make_quantum_front(registry, signature, BackendKind::CPU, 0, 1337u);
    QPP_EXPECT_EQ(front.signature.size(), signature.size());
    QPP_EXPECT_EQ(front.primes.size(), signature.size());
    QPP_EXPECT_EQ(front.spectrum.size(), signature.size());
    QPP_EXPECT_EQ(front.payload.size(), signature.size());
    double sum = std::accumulate(front.payload.begin(), front.payload.end(), 0.0);
    QPP_EXPECT_NEAR(sum, 1.0, 1e-9);
}

QPP_TEST(SampleWorldsQpuProducesNormalizedAmplitudes) {
    std::vector<double> weights{0.3, 0.7, 1.1};
    auto amplitudes = qpp::quantum::sample_worlds(weights, BackendKind::QPU_SIM, 0);
    double norm = 0.0;
    for (double amplitude : amplitudes)
        norm += amplitude * amplitude;
    QPP_EXPECT_NEAR(norm, 1.0, 1e-12);
}

QPP_TEST(SignatureFromStringHandlesWhitespaceAndDefaults) {
    auto signature = qpp::quantum::signature_from_string(" cat : 1.0 , hat , fancy :0.25 ");
    QPP_EXPECT_EQ(signature.size(), static_cast<std::size_t>(3));
    QPP_EXPECT_EQ(signature.factors[0].first, "cat");
    QPP_EXPECT_NEAR(signature.factors[0].second, 1.0, 1e-12);
    QPP_EXPECT_EQ(signature.factors[1].first, "fancy");
    QPP_EXPECT_NEAR(signature.factors[1].second, 0.25, 1e-12);
    QPP_EXPECT_EQ(signature.factors[2].first, "hat");
    QPP_EXPECT_NEAR(signature.factors[2].second, 1.0, 1e-12);
}

