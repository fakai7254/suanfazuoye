#ifndef MATRIXCHAIN_H
#define MATRIXCHAIN_H

#include "common.h"
#include <climits>

class MatrixChainSolver {
public:
    struct DPResult {
        long long optimal_cost;
        long long operations;
        std::string parenthesization;
    };

    static DPResult solve_dp(const std::vector<int>& dims) {
        int n = (int)dims.size() - 1;
        DPResult res;
        res.operations = 0;

        std::vector<std::vector<long long>> dp(n + 2,
            std::vector<long long>(n + 2, 0));
        std::vector<std::vector<int>> split(n + 2,
            std::vector<int>(n + 2, 0));

        for (int len = 2; len <= n; len++) {
            for (int i = 1; i <= n - len + 1; i++) {
                int j = i + len - 1;
                dp[i][j] = LLONG_MAX;
                for (int k = i; k < j; k++) {
                    res.operations++;
                    long long cost = dp[i][k] + dp[k + 1][j] +
                        (long long)dims[i - 1] * dims[k] * dims[j];
                    if (cost < dp[i][j]) {
                        dp[i][j] = cost;
                        split[i][j] = k;
                    }
                }
            }
        }

        res.optimal_cost = (n >= 1) ? dp[1][n] : 0;
        if (n >= 1) {
            res.parenthesization = build_paren(split, 1, n);
        }
        return res;
    }

    struct BTResult {
        long long optimal_cost;
        long long nodes_visited;
        std::string parenthesization;
    };

    static BTResult solve_backtrack(const std::vector<int>& dims) {
        int n = (int)dims.size() - 1;
        BTResult res;
        res.optimal_cost = LLONG_MAX;
        res.nodes_visited = 0;

        if (n == 0) { res.optimal_cost = 0; return res; }

        // n > 18 时 Catalan 数过大，回溯不可行，直接返回 DP 结果
        if (n > 18) {
            auto dp_r = solve_dp(dims);
            res.optimal_cost = dp_r.optimal_cost;
            res.parenthesization = dp_r.parenthesization;
            res.nodes_visited = -1;  // 标记为跳过
            return res;
        }

        std::vector<std::vector<long long>> lb(n + 2,
            std::vector<long long>(n + 2, 0));
        precompute_lower_bound(dims, lb);

        long long greedy_ub = greedy_upper_bound(dims, 1, n);
        long long nodes = 0;

        // 节点限制：最多 5 千万节点
        const long long MAX_NODES = 50000000;
        long long best = backtrack_search(dims, lb, 1, n, greedy_ub, nodes, MAX_NODES);

        res.optimal_cost = best;
        res.nodes_visited = nodes;

        if (n >= 1 && best < LLONG_MAX) {
            auto dp_check = solve_dp(dims);
            res.parenthesization = dp_check.parenthesization;
        }
        return res;
    }

private:
    static std::string build_paren(
            const std::vector<std::vector<int>>& split,
            int i, int j) {
        if (i == j) return "A" + std::to_string(i);
        int k = split[i][j];
        return "(" + build_paren(split, i, k)
               + build_paren(split, k + 1, j) + ")";
    }

    static void precompute_lower_bound(const std::vector<int>& dims,
                                        std::vector<std::vector<long long>>& lb) {
        int n = (int)dims.size() - 1;
        for (int i = 1; i <= n; i++) {
            for (int j = i; j <= n; j++) {
                if (i == j) { lb[i][j] = 0; continue; }
                std::vector<int> vals;
                for (int t = i - 1; t <= j; t++)
                    vals.push_back(dims[t]);
                std::sort(vals.begin(), vals.end());
                int len = j - i;
                lb[i][j] = len * (long long)vals[0] * vals[1] * vals[2];
            }
        }
    }

    static long long greedy_upper_bound(const std::vector<int>& dims,
                                        int i, int j) {
        if (i == j) return 0;
        int best_k = i;
        long long min_dim = dims[i];
        for (int k = i; k < j; k++) {
            if (dims[k] < min_dim) {
                min_dim = dims[k];
                best_k = k;
            }
        }
        long long merge = (long long)dims[i - 1] * dims[best_k] * dims[j];
        return greedy_upper_bound(dims, i, best_k) +
               greedy_upper_bound(dims, best_k + 1, j) + merge;
    }

    static long long backtrack_search(
            const std::vector<int>& dims,
            const std::vector<std::vector<long long>>& lb,
            int i, int j,
            long long upper_bound,
            long long& nodes,
            long long max_nodes) {

        nodes++;
        if (nodes > max_nodes) return upper_bound;  // 超时返回当前上界
        if (i == j) return 0;

        long long best_local = upper_bound;

        // 启发式：从中间向外尝试分割点
        int mid = (i + j) / 2;
        std::vector<int> order;
        for (int d = 0; d < j - i; d++) {
            int k = (d % 2 == 0) ? (mid + d / 2) : (mid - (d / 2 + 1));
            if (k >= i && k < j) order.push_back(k);
        }

        for (int k : order) {
            long long left_lb = lb[i][k];
            long long right_lb = lb[k + 1][j];
            long long merge_cost =
                (long long)dims[i - 1] * dims[k] * dims[j];

            // 下界剪枝
            if (left_lb + right_lb + merge_cost >= best_local) continue;

            // 传给左子问题的上界：best_local - right_lb - merge_cost
            long long left_cost = backtrack_search(
                dims, lb, i, k,
                best_local - right_lb - merge_cost, nodes, max_nodes);
            if (left_cost >= best_local) continue;

            // 传给右子问题的上界：best_local - left_cost - merge_cost
            long long right_cost = backtrack_search(
                dims, lb, k + 1, j,
                best_local - left_cost - merge_cost, nodes, max_nodes);

            long long total = left_cost + right_cost + merge_cost;
            if (total < best_local) {
                best_local = total;
            }
        }
        return best_local;
    }
};

#endif // MATRIXCHAIN_H
