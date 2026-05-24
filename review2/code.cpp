#include <array>
#include <cassert>
#include <iostream>
#include <limits>
#include <optional>
#include <ranges>
#include <vector>

struct TrustEdge {
    int tail;
    int head;
    int capacity;
};

class Graph {
public:
    static constexpr auto kMaxNodesCount = 100;
    Graph() = delete;
    Graph(int size) : size_{size} {}

    void SetBarsCount(int node_idx, int bars_count) {
        bars_count_[node_idx] = bars_count;
        bars_sum_ += bars_count;
    }

    int GetBarsSum() const { return bars_sum_; }

    int GetSize() const { return size_; }

    int GetNodeBars(int idx) { return bars_count_[idx]; }

    void SetTrustEdge(int tail, int head) { edges_[tail].push_back(head); }

    const std::vector<int>& GetTrustedNeighbors(int node) {
        return edges_[node];
    }

private:
    std::array<std::vector<int>, kMaxNodesCount> edges_{};
    std::array<int, kMaxNodesCount> bars_count_{};
    int size_{};
    int bars_sum_ = 0;
};

class FlowNetwork {
public:
    static constexpr auto kSourceIdx = 100;
    static constexpr auto kTargetIdx = 101;

    FlowNetwork(Graph* graph) : size_{graph->GetSize()}, graph_{graph} {
        auto size = graph->GetSize();
        for (auto node = 0; node < size; ++node) {
            auto bars_count = graph->GetNodeBars(node);
            capacities_[kSourceIdx][node] = bars_count;
            auto neighbors = graph->GetTrustedNeighbors(node);
            if (neighbors.empty()) {
                continue;
            }

            for (auto neighbor : neighbors) {
                capacities_[node][neighbor] = std::numeric_limits<int>::max();
            }
        }
    }

    int GetFlowVal() {
        auto flow_val = 0;
        for (auto node_idx = 0; node_idx < size_; ++node_idx) {
            flow_val += flow_[node_idx][kTargetIdx];
        }

        return flow_val;
    }

    void BuildMaxFlow() {
        Init();
        while (true) {
            auto node = GetVertexWithExcess();
            if (!node.has_value()) {
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
        return GetFlowVal() == graph_->GetBarsSum();
    }

    void SetTargetEdgesCap(int target_edges_cap) {
        for (auto node_idx = 0; node_idx < size_; ++node_idx) {
            capacities_[node_idx][kTargetIdx] = target_edges_cap;
        }
    }

private:
    using EdgeFlow = int;
    using NodeHeight = int;

    void Init() {
        heights_.fill(0);
        heights_[kSourceIdx] = size_ + 2;
        for (auto& node_flow : flow_) {
            node_flow.fill(0);
        }
        for (int node_idx = 0; node_idx < size_; ++node_idx) {
            flow_[kSourceIdx][node_idx] = graph_->GetNodeBars(node_idx);
            flow_[node_idx][kSourceIdx] = -graph_->GetNodeBars(node_idx);
        }

        for (auto node_idx = 0; node_idx < size_; ++node_idx) {
            excesses_[node_idx] = graph_->GetNodeBars(node_idx);
        }
        excesses_[kSourceIdx] = 0;
        excesses_[kTargetIdx] = 0;
    }

    std::optional<int> GetVertexWithExcess() {
        for (int node_idx = 0; node_idx < size_; ++node_idx) {
            if (excesses_[node_idx] > 0) {
                return node_idx;
            }
        }

        return std::nullopt;
    }

    std::optional<int> GetNeighborToPush(int from) {
        for (auto neighbor_idx = 0; neighbor_idx < size_; ++neighbor_idx) {
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

        excesses_[from] -= push_volume;
        excesses_[to] += push_volume;
    }

    void Relabel(int node) {
        assert(excesses_[node] > 0);
        auto min_height = heights_[node];
        for (auto node_idx = 0; node_idx < size_; ++node_idx) {
            if (capacities_[node][node_idx] - flow_[node][node_idx] > 0) {
                min_height = std::min(heights_[node_idx], min_height);
            }
        }

        for (auto node_idx : {kSourceIdx, kTargetIdx}) {
            if (capacities_[node][node_idx] - flow_[node][node_idx] > 0) {
                min_height = std::min(heights_[node_idx], min_height);
            }
        }
        heights_[node] = min_height + 1;
    }

    int size_;
    Graph* graph_;
    std::array<std::array<EdgeFlow, Graph::kMaxNodesCount + 2>,
               Graph::kMaxNodesCount + 2>
        flow_{};  // Matrix
    std::array<std::array<int, Graph::kMaxNodesCount + 2>,
               Graph::kMaxNodesCount + 2>
        capacities_{};
    std::array<NodeHeight, Graph::kMaxNodesCount + 2> heights_{};
    std::array<int, Graph::kMaxNodesCount + 2> excesses_{};
};

std::tuple<Graph, int, int> ReadGraph() {
    int vertex_count;
    int edges_count;
    std::cin >> vertex_count >> edges_count;

    Graph graph{vertex_count};
    auto bars_sum = 0;
    auto max_bars = 0;
    int bars;
    for (auto man_idx : std::views::iota(0, vertex_count)) {
        std::cin >> bars;
        graph.SetBarsCount(man_idx, bars);
        bars_sum += bars;
        max_bars = std::max(max_bars, bars);
    }

    int tail;
    int head;
    for ([[maybe_unused]] auto edge_num : std::views::iota(0, edges_count)) {
        std::cin >> tail >> head;
        graph.SetTrustEdge(tail - 1, head - 1);
    }

    return {graph, bars_sum, max_bars};
}

int CalculateLeastMaxLoad(FlowNetwork& flow_network, int left, int right) {
    while (left < right) {
        auto mid = (left + right) / 2;
        flow_network.SetTargetEdgesCap(mid);
        if (flow_network.IsFlowMaximizible()) {
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

    auto [graph, bars_sum, max_bars] = ReadGraph();
    int left = bars_sum / graph.GetSize();
    int right = max_bars;

    FlowNetwork flow_network{&graph};
    auto minmax_bars = CalculateLeastMaxLoad(flow_network, left, right);

    std::cout << minmax_bars << std::endl;
}