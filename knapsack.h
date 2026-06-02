#ifndef KNAPSACK_H
#define KNAPSACK_H

#include "common.h"
#include <algorithm>

class KnapsackSolver {
public:
    struct Item {
        long long weight, value;
        double density;
        int original_id;
    };

    struct DPResult {
        long long optimal_value, operations;
        std::vector<int> selected;
    };

    // 轻量 DP: 一维滚动数组, O(n*C)
    static DPResult solve_dp_light(const std::vector<long long>& w,
                                   const std::vector<long long>& v,
                                   long long C) {
        int n = (int)w.size();
        DPResult res; res.operations = 0;
        std::vector<long long> dp(C + 1, 0);
        for (int i = 0; i < n; i++)
            for (long long c = C; c >= w[i]; c--) {
                res.operations++;
                dp[c] = std::max(dp[c], dp[c - w[i]] + v[i]);
            }
        res.optimal_value = dp[C];
        return res;
    }

    struct BTResult {
        long long optimal_value, nodes_visited;
        std::vector<int> selected;
    };

    // 回溯法: 价值密度排序 + 分数背包上界剪枝 + 节点限制
    static BTResult solve_backtrack(const std::vector<long long>& weights,
                                    const std::vector<long long>& values,
                                    long long capacity,
                                    long long max_nodes = 100000000) {
        int n = (int)weights.size();
        std::vector<Item> items(n);
        for (int i = 0; i < n; i++) {
            items[i].weight = weights[i];
            items[i].value = values[i];
            items[i].density = (items[i].weight > 0) ?
                (double)items[i].value / items[i].weight : 0.0;
            items[i].original_id = i;
        }
        std::sort(items.begin(), items.end(), [](const Item& a, const Item& b) {
            if (std::abs(a.density - b.density) > 1e-12) return a.density > b.density;
            return a.weight < b.weight;
        });

        BTResult res; res.optimal_value = 0; res.nodes_visited = 0;
        std::vector<int> cur, best;
        dfs(items, capacity, 0, 0LL, 0LL, cur, best, res, max_nodes);

        res.selected.clear();
        for (int idx : best) res.selected.push_back(items[idx].original_id);
        std::sort(res.selected.begin(), res.selected.end());
        return res;
    }

private:
    static void dfs(const std::vector<Item>& items, long long C,
                    int idx, long long cw, long long cv,
                    std::vector<int>& cur, std::vector<int>& best,
                    BTResult& res, long long max_nodes) {
        res.nodes_visited++;
        if (res.nodes_visited > max_nodes) return;
        if (cv > res.optimal_value) { res.optimal_value = cv; best = cur; }
        if (idx >= (int)items.size()) return;

        // 分数背包上界
        long long rem = C - cw, bound = cv, tmp = 0;
        for (int i = idx; i < (int)items.size() && tmp < rem; i++) {
            if (items[i].weight <= rem - tmp) {
                bound += items[i].value; tmp += items[i].weight;
            } else { bound += (long long)(items[i].density * (rem - tmp)); break; }
        }
        if (bound <= res.optimal_value) return;

        // 左分支: 选
        if (cw + items[idx].weight <= C) {
            cur.push_back(idx);
            dfs(items, C, idx + 1, cw + items[idx].weight,
                cv + items[idx].value, cur, best, res, max_nodes);
            cur.pop_back();
        }
        // 右分支: 不选
        dfs(items, C, idx + 1, cw, cv, cur, best, res, max_nodes);
    }
};

#endif
