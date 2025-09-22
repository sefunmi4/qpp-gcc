#include "qpp/quantum_worlds.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {
using qpp::quantum::BackendKind;
using qpp::quantum::FactorRegistry;
using qpp::quantum::WorldSignature;

struct Options {
    BackendKind backend = BackendKind::CPU;
    std::size_t k = 128;
    double sigma = 1.0;
    double lambda = 0.5;
    double temperature = 1.0;
};

struct WorldInfo {
    std::string name;
    WorldSignature signature;
    std::vector<FactorRegistry::value_type> primes;
    std::vector<double> spectrum;
    double score = 0.0;
};

[[noreturn]] void usage(const char *program, const std::string &message) {
    std::cerr << "Error: " << message << "\n";
    std::cerr << "Usage: " << program
              << " [--backend cpu|qpu_sim] [--k <shots>] [--sigma <value>]"
                 " [--lambda <value>] [--temperature <value>]" << '\n';
    std::exit(EXIT_FAILURE);
}

std::string next_argument(int &i, int argc, char **argv, std::string option) {
    auto eq_pos = option.find('=');
    if (eq_pos != std::string::npos) {
        std::string value = option.substr(eq_pos + 1);
        option.resize(eq_pos);
        return value;
    }
    if (i + 1 >= argc)
        usage(argv[0], "missing value for option " + option);
    return argv[++i];
}

Options parse_arguments(int argc, char **argv) {
    Options opts;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0]
                      << " [--backend cpu|qpu_sim] [--k <shots>] [--sigma <value>]"
                         " [--lambda <value>] [--temperature <value>]" << '\n';
            std::exit(EXIT_SUCCESS);
        }
        std::string value;
        if (arg.rfind("--backend", 0) == 0) {
            value = next_argument(i, argc, argv, arg);
            try {
                opts.backend = qpp::quantum::parse_backend(value);
            } catch (const std::exception &e) {
                usage(argv[0], e.what());
            }
        } else if (arg.rfind("--k", 0) == 0) {
            value = next_argument(i, argc, argv, arg);
            try {
                opts.k = static_cast<std::size_t>(std::stoul(value));
            } catch (const std::exception &) {
                usage(argv[0], "invalid integer for --k");
            }
        } else if (arg.rfind("--sigma", 0) == 0) {
            value = next_argument(i, argc, argv, arg);
            try {
                opts.sigma = std::stod(value);
            } catch (const std::exception &) {
                usage(argv[0], "invalid real value for --sigma");
            }
        } else if (arg.rfind("--lambda", 0) == 0) {
            value = next_argument(i, argc, argv, arg);
            try {
                opts.lambda = std::stod(value);
            } catch (const std::exception &) {
                usage(argv[0], "invalid real value for --lambda");
            }
        } else if (arg.rfind("--temperature", 0) == 0) {
            value = next_argument(i, argc, argv, arg);
            try {
                opts.temperature = std::stod(value);
            } catch (const std::exception &) {
                usage(argv[0], "invalid real value for --temperature");
            }
        } else {
            usage(argv[0], "unknown option: " + arg);
        }
    }
    if (opts.sigma <= 0.0)
        usage(argv[0], "sigma must be positive");
    if (opts.temperature <= 0.0)
        usage(argv[0], "temperature must be positive");
    return opts;
}

std::vector<WorldInfo> build_sample_worlds() {
    std::vector<WorldInfo> worlds;
    worlds.push_back({"cat", WorldSignature({{"cat", 1.0}, {"feline", 0.8}, {"pet", 0.6}})});
    worlds.push_back({"hat", WorldSignature({{"hat", 1.0}, {"fashion", 0.7}, {"accessory", 0.9}})});
    worlds.push_back({"cat_hat_pair", WorldSignature({{"cat", 0.9}, {"hat", 0.9}, {"ensemble", 1.1}})});
    worlds.push_back({"cat_wearing_hat",
                      WorldSignature({{"cat", 1.0},
                                      {"hat", 1.0},
                                      {"wearing", 1.2},
                                      {"relationship", 0.8}})});
    worlds.push_back({"hat_on_cat",
                      WorldSignature({{"hat", 1.0},
                                      {"cat", 0.95},
                                      {"on_top", 1.3},
                                      {"balance", 0.6}})});
    worlds.push_back({"cat_hat_story",
                      WorldSignature({{"cat", 0.85},
                                      {"hat", 0.85},
                                      {"adventure", 1.1},
                                      {"narrative", 0.75}})});
    return worlds;
}

void prepare_worlds(std::vector<WorldInfo> &worlds, FactorRegistry &registry) {
    for (auto &world : worlds) {
        world.signature.sort();
        world.primes = qpp::quantum::assign_primes(registry, world.signature);
        world.spectrum =
            qpp::quantum::generate_spectrum(world.primes, world.signature);
    }
}

double compute_relational_score(const WorldInfo &target, const WorldInfo &other,
                                double sigma) {
    return qpp::quantum::gaussian_overlap(target.spectrum, other.spectrum, sigma);
}

void compute_scores(std::vector<WorldInfo> &worlds, double sigma, double lambda) {
    const std::size_t n = worlds.size();
    for (std::size_t i = 0; i < n; ++i) {
        double self_overlap = compute_relational_score(worlds[i], worlds[i], sigma);
        double relational = 0.0;
        for (std::size_t j = 0; j < n; ++j) {
            if (i == j)
                continue;
            relational += compute_relational_score(worlds[i], worlds[j], sigma);
        }
        worlds[i].score = self_overlap + lambda * relational;
    }
}

std::vector<double> adjust_for_temperature(const std::vector<double> &scores,
                                           double temperature) {
    if (temperature == 1.0)
        return scores;
    const double inv_temp = 1.0 / temperature;
    std::vector<double> adjusted(scores);
    for (double &v : adjusted)
        v *= inv_temp;
    return adjusted;
}

std::vector<double> amplitudes_to_probabilities(const std::vector<double> &amps) {
    std::vector<double> probs(amps.size(), 0.0);
    double total = 0.0;
    for (std::size_t i = 0; i < amps.size(); ++i) {
        probs[i] = amps[i] * amps[i];
        total += probs[i];
    }
    if (total > 0.0) {
        for (double &v : probs)
            v /= total;
    }
    return probs;
}

std::size_t sample_pick(const std::vector<double> &weights) {
    const unsigned seed = 1337u;
    auto probabilities = qpp::quantum::softmax(weights);
    std::mt19937 rng(seed);
    std::discrete_distribution<std::size_t> dist(probabilities.begin(), probabilities.end());
    return dist(rng);
}

std::string backend_name(BackendKind backend) {
    switch (backend) {
    case BackendKind::CPU:
        return "CPU";
    case BackendKind::QPU_SIM:
        return "QPU_SIM";
    }
    return "UNKNOWN";
}

} // namespace

int main(int argc, char **argv) {
    try {
        const Options opts = parse_arguments(argc, argv);

        auto worlds = build_sample_worlds();
        FactorRegistry registry;
        prepare_worlds(worlds, registry);
        compute_scores(worlds, opts.sigma, opts.lambda);

        std::vector<double> scores;
        scores.reserve(worlds.size());
        for (const auto &world : worlds)
            scores.push_back(world.score);

        const auto adjusted = adjust_for_temperature(scores, opts.temperature);

        constexpr unsigned base_seed = 424242u;
        auto cpu_distribution = qpp::quantum::sample_worlds(adjusted, BackendKind::CPU,
                                                            opts.k, base_seed);
        auto qpu_amplitudes = qpp::quantum::sample_worlds(adjusted, BackendKind::QPU_SIM,
                                                          0u, base_seed);
        auto qpu_distribution = amplitudes_to_probabilities(qpu_amplitudes);

        auto active_payload = qpp::quantum::sample_worlds(adjusted, opts.backend, opts.k,
                                                          base_seed);
        std::vector<double> active_probabilities;
        if (opts.backend == BackendKind::CPU)
            active_probabilities = active_payload;
        else
            active_probabilities = amplitudes_to_probabilities(active_payload);

        std::size_t sampled_index = sample_pick(adjusted);

        std::cout << "Quantum World Picker" << '\n';
        std::cout << "Backend: " << backend_name(opts.backend) << '\n';
        std::cout << "k (shots): " << opts.k << '\n';
        std::cout << "sigma: " << opts.sigma << '\n';
        std::cout << "lambda: " << opts.lambda << '\n';
        std::cout << "temperature: " << opts.temperature << '\n';
        std::cout << '\n';

        std::cout << std::fixed << std::setprecision(4);
        std::cout << "World" << std::setw(18) << "Score" << std::setw(16) << "CPU Prob"
                  << std::setw(16) << "QPU Prob" << "  Signature" << '\n';
        std::cout << std::string(80, '-') << '\n';
        for (std::size_t i = 0; i < worlds.size(); ++i) {
            std::cout << std::left << std::setw(16) << worlds[i].name;
            std::cout << std::right << std::setw(10) << worlds[i].score;
            double cpu_prob = i < cpu_distribution.size() ? cpu_distribution[i] : 0.0;
            double qpu_prob = i < qpu_distribution.size() ? qpu_distribution[i] : 0.0;
            std::cout << std::setw(16) << cpu_prob;
            std::cout << std::setw(16) << qpu_prob << "  ";
            std::cout << qpp::quantum::signature_to_string(worlds[i].signature) << '\n';
        }

        std::cout << '\n';
        if (sampled_index < worlds.size()) {
            std::cout << "Sampled world (deterministic RNG): " << worlds[sampled_index].name
                      << '\n';
        }

        if (!active_probabilities.empty()) {
            std::size_t best = std::distance(active_probabilities.begin(),
                                             std::max_element(active_probabilities.begin(),
                                                              active_probabilities.end()));
            std::cout << "Most probable for " << backend_name(opts.backend) << ": "
                      << worlds[best].name << " (" << active_probabilities[best]
                      << ")" << '\n';
        }

    } catch (const std::exception &e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

