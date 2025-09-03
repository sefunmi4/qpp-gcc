#ifndef QPP_INITIAL_MAPPER_HPP
#define QPP_INITIAL_MAPPER_HPP

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <functional>
#include <numeric>
#include <utility>
#include <vector>

namespace qpp {

struct CircuitInteractionGraph {
    int n_logical{};
    std::vector<std::pair<int, int>> edges;
};

struct DeviceGraph {
    int n_physical{};
    std::vector<std::pair<int, int>> edges;
    std::vector<double> edge_quality;
    std::vector<double> node_quality;
};

struct Placement {
    std::vector<int> logical_to_physical;
};

class AbstractInitialMapper {
public:
    virtual ~AbstractInitialMapper() = default;
    virtual Placement place(const CircuitInteractionGraph& cig,
                            const DeviceGraph& dg,
                            uint64_t seed = 0,
                            int time_budget_ms = 100) = 0;
};

class LineInitialMapper final : public AbstractInitialMapper {
public:
    Placement place(const CircuitInteractionGraph& cig,
                    const DeviceGraph& dg,
                    uint64_t /*seed*/ = 0,
                    int /*time_budget_ms*/ = 100) override {
        Placement P;
        P.logical_to_physical.assign(cig.n_logical, -1);
        if (cig.n_logical == 0 || dg.n_physical == 0)
            return P;

        std::vector<int> deg(cig.n_logical, 0);
        for (auto [u, v] : cig.edges) {
            ++deg[u];
            ++deg[v];
        }

        std::vector<int> order(cig.n_logical);
        std::iota(order.begin(), order.end(), 0);
        std::stable_sort(order.begin(), order.end(),
                         [&](int a, int b) { return deg[a] > deg[b]; });

        std::vector<int> device_line(dg.n_physical);
        std::iota(device_line.begin(), device_line.end(), 0);

        int anchor = dg.n_physical / 2;
        P.logical_to_physical[order[0]] = device_line[anchor];
        int left = anchor - 1;
        int right = anchor + 1;
        for (int i = 1; i < cig.n_logical && (left >= 0 || right < dg.n_physical);
             ++i) {
            int phys;
            if (right < dg.n_physical &&
                (left < 0 || right - anchor <= anchor - left))
                phys = device_line[right++];
            else
                phys = device_line[left--];
            P.logical_to_physical[order[i]] = phys;
        }
        return P;
    }
};

class VF2InitialMapper final : public AbstractInitialMapper {
public:
    Placement place(const CircuitInteractionGraph& cig,
                    const DeviceGraph& dg,
                    uint64_t /*seed*/ = 0,
                    int time_budget_ms = 100) override {
        auto start = std::chrono::steady_clock::now();
        Placement best;
        best.logical_to_physical.assign(cig.n_logical, -1);
        double best_score = -1e300;

        std::vector<std::vector<int>> C_adj(cig.n_logical);
        for (auto [u, v] : cig.edges) {
            C_adj[u].push_back(v);
            C_adj[v].push_back(u);
        }
        std::vector<std::vector<int>> D_adj(dg.n_physical);
        for (auto [u, v] : dg.edges) {
            D_adj[u].push_back(v);
            D_adj[v].push_back(u);
        }

        std::vector<int> map(cig.n_logical, -1);
        std::vector<char> used(dg.n_physical, 0);
        std::vector<int> order(cig.n_logical);
        std::iota(order.begin(), order.end(), 0);
        std::stable_sort(order.begin(), order.end(),
                         [&](int a, int b) {
                             return C_adj[a].size() > C_adj[b].size();
                         });

        std::function<void(int, double)> dfs = [&](int depth, double score) {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start)
                    .count() > time_budget_ms)
                return;
            if (depth == cig.n_logical) {
                if (score > best_score) {
                    best_score = score;
                    best.logical_to_physical = map;
                }
                return;
            }
            int u = order[depth];
            for (int p = 0; p < dg.n_physical; ++p) {
                if (used[p])
                    continue;
                bool ok = true;
                double local = 0;
                for (int v : C_adj[u]) {
                    if (map[v] != -1) {
                        auto it = std::find(D_adj[p].begin(), D_adj[p].end(), map[v]);
                        if (it == D_adj[p].end()) {
                            ok = false;
                            break;
                        }
                        local += 1.0;
                    }
                }
                if (!ok)
                    continue;
                map[u] = p;
                used[p] = 1;
                dfs(depth + 1, score + local);
                map[u] = -1;
                used[p] = 0;
            }
        };
        dfs(0, 0.0);

        if (best.logical_to_physical.empty() ||
            best.logical_to_physical[0] == -1) {
            LineInitialMapper line;
            return line.place(cig, dg);
        }
        return best;
    }
};

enum class MapperKind { LINE, VF2, AUTO };

struct DynamicPolicy {
    int vf2_max_logicals = 12;
    double vf2_min_density = 0.20;
};

class DynamicInitialMapper final : public AbstractInitialMapper {
    DynamicPolicy pol;

public:
    explicit DynamicInitialMapper(DynamicPolicy p = {}) : pol(p) {}

    Placement place(const CircuitInteractionGraph& cig,
                    const DeviceGraph& dg,
                    uint64_t seed = 0,
                    int time_budget_ms = 100) override {
        double density = 0.0;
        if (cig.n_logical > 1)
            density = 2.0 * cig.edges.size() /
                      (cig.n_logical * (cig.n_logical - 1));
        bool use_vf2 =
            (cig.n_logical <= pol.vf2_max_logicals) &&
            (density >= pol.vf2_min_density);
        if (use_vf2) {
            VF2InitialMapper vf2;
            return vf2.place(cig, dg, seed, time_budget_ms);
        } else {
            LineInitialMapper line;
            return line.place(cig, dg, seed, time_budget_ms);
        }
    }
};

} // namespace qpp

#endif // QPP_INITIAL_MAPPER_HPP
