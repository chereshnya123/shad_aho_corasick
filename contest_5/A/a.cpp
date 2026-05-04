#include <array>
#include <cassert>
#include <format>
#include <iostream>
#include <limits>
#include <optional>
#include <ranges>

struct TrustEdge {
    int tail;
    int head;
    int capacity;
};

class Graph {
public:
    using Capacity = int;

    static constexpr auto kNoEdgeValue = -1;
    static constexpr auto kMaxNodesCount = 100;

    Graph(int size) : size_{size} {}

    void SetBarsCount(int node_idx, int bars_count) {
        bars_count_[node_idx] = bars_count;
        max_bar_ = std::max(max_bar_, bars_count);
        bars_sum_ += bars_count;
    }

    int GetMaxBarsCount() const { return max_bar_; }

    int GetBarsSum() const { return bars_sum_; }

    int GetSize() const { return size_; }

    int GetNodeBars(int idx) { return bars_count_[idx]; }

private:
    using From = int;
    using To = int;

    std::array<int, kMaxNodesCount> bars_count_{};
    int size_;
    int max_bar_ = 0;
    int bars_sum_ = 0;
};

class FlowGraph : public Graph {
public:
    static constexpr auto kSourceIdx = 100;
    static constexpr auto kTargetIdx = 101;

    static constexpr auto kFromVertex = 0;
    static constexpr auto kToVertex = 1;

    FlowGraph(int size) : Graph{size} {}

    void Init() {
        heights_.fill(0);
        heights_[kSourceIdx] = GetSize() + 2;

        for (auto& node_flow : flow_) {
            node_flow.fill(0);
        }
        for (int node_idx = 0; node_idx < GetSize(); ++node_idx) {
            flow_[kSourceIdx][node_idx] = GetNodeBars(node_idx);
            flow_[node_idx][kSourceIdx] = -GetNodeBars(node_idx);
        }

        for (auto node_idx = 0; node_idx < GetSize(); ++node_idx) {
            excesses_[node_idx] = GetNodeBars(node_idx);
        }
        excesses_[kSourceIdx] = 0;
        excesses_[kTargetIdx] = 0;
    }

    void SetTrustEdge(int tail, int head) {
        capacities_[tail][head] = std::numeric_limits<int>::max();
    }

    void SetBarsCount(int node_idx, int bars_count) {
        capacities_[kSourceIdx][node_idx] = bars_count;
        Graph::SetBarsCount(node_idx, bars_count);
    }

    void SetTargetEdgesCap(int target_edges_cap) {
        for (auto node_idx = 0; node_idx < GetSize(); ++node_idx) {
            capacities_[node_idx][kTargetIdx] = target_edges_cap;
        }
    }

    std::optional<int> GetVertexWithExcess() {
        for (int node_idx = 0; node_idx < GetSize(); ++node_idx) {
            if (excesses_[node_idx] > 0) {
                return node_idx;
            }
        }

        return std::nullopt;
    }

    std::optional<int> GetNeighborToPush(int from) {
        for (auto neighbor_idx = 0; neighbor_idx < GetSize(); ++neighbor_idx) {
            if (capacities_[from][neighbor_idx] - flow_[from][neighbor_idx] >
                    0 &&
                heights_[from] == heights_[neighbor_idx] + 1) {
                return neighbor_idx;
            }
        }

        for (auto neighbor : {kSourceIdx, kTargetIdx}) {
            if (capacities_[from][neighbor] - flow_[from][neighbor] > 0 &&
                heights_[from] == heights_[neighbor] + 1) {
                return neighbor;
            }
        }

        return std::nullopt;
    }

    void Push(int from, int to) {
        auto residual_cap = capacities_[from][to] - flow_[from][to];
        auto has_excess = excesses_[from] > 0;
        auto has_residual_cap = residual_cap > 0;
        auto is_height_correct = heights_[from] == heights_[to] + 1;
        assert(has_excess && has_residual_cap && is_height_correct);

        auto push_volume = std::min(residual_cap, excesses_[from]);
        flow_[from][to] += push_volume;
        flow_[to][from] -= push_volume;
        // Log(std::format("Push: {} --> {}. Amount = {}", from, to,
        // push_volume));

        excesses_[from] -= push_volume;
        excesses_[to] += push_volume;
    }

    void Relabel(int node) {
        assert(excesses_[node] > 0);
        auto min_height = heights_[node];
        for (auto node_idx = 0; node_idx < GetSize(); ++node_idx) {
            if (capacities_[node][node_idx] - flow_[node][node_idx] > 0) {
                min_height = std::min(heights_[node_idx], min_height);
            }
        }

        for (auto node_idx : {kSourceIdx, kTargetIdx}) {
            if (capacities_[node][node_idx] - flow_[node][node_idx] > 0) {
                min_height = std::min(heights_[node_idx], min_height);
            }
        }
        // Log(std::format("Relabel {}. New height = {}", node, min_height +
        // 1));

        heights_[node] = min_height + 1;
    }

    int GetFlowVal() {
        auto flow_val = 0;
        for (auto node_idx = 0; node_idx < GetSize(); ++node_idx) {
            flow_val += flow_[node_idx][kTargetIdx];
        }

        return flow_val;
    }

    void BuildMaxFlow() {
        Init();
        // Log("Inited");
        while (true) {
            auto node = GetVertexWithExcess();
            if (!node.has_value()) {
                // Log("Can not find vertex with excess");
                break;
            }

            auto neighbor = GetNeighborToPush(*node);
            if (neighbor.has_value()) {
                Push(*node, *neighbor);
            } else {
                Relabel(*node);
            }
        }
    }

    bool IsFlowMaximizible() {
        BuildMaxFlow();
        // Log(std::format("Build max flow. Val = {}, bars_sum = {}",
        // GetFlowVal(), GetBarsSum()));
        return GetFlowVal() == GetBarsSum();
    }

private:
    using EdgeFlow = int;
    using NodeHeight = int;

    std::array<std::array<EdgeFlow, kMaxNodesCount + 2>, kMaxNodesCount + 2>
        flow_{};  // Matrix
    std::array<std::array<int, kMaxNodesCount + 2>, kMaxNodesCount + 2>
        capacities_{};
    std::array<NodeHeight, kMaxNodesCount + 2> heights_{};
    std::array<int, kMaxNodesCount + 2> excesses_{};
};

FlowGraph ReadFlowGraph() {
    int vertex_count;
    int edges_count;
    std::cin >> vertex_count >> edges_count;

    FlowGraph graph{vertex_count};

    int bars;
    for (auto man_idx : std::views::iota(0, vertex_count)) {
        std::cin >> bars;
        graph.SetBarsCount(man_idx, bars);
    }

    int tail;
    int head;
    for ([[maybe_unused]] auto edge_num : std::views::iota(0, edges_count)) {
        std::cin >> tail >> head;
        graph.SetTrustEdge(tail - 1, head - 1);
    }

    return graph;
}

int CalculateLeastMaxLoad(FlowGraph& graph) {
    int left = graph.GetBarsSum() / graph.GetSize();
    int right = graph.GetMaxBarsCount();

    // Log(std::format("bars sum = {}, size = {}", graph.GetBarsSum(),
    // graph.GetSize())); Log(std::format("Get answer interval: [{}, {}]", left,
    // right));
    while (left < right) {
        auto mid = (left + right) / 2;
        graph.SetTargetEdgesCap(mid);
        // Log(std::format("Set target edges cap: {}", mid));
        if (graph.IsFlowMaximizible()) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }

    return (left + right) / 2;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
    // Log("Input data");
    auto graph = ReadFlowGraph();
    auto minmax_bars = CalculateLeastMaxLoad(graph);

    std::cout << minmax_bars << std::endl;
}