#ifndef SUBSETSUM_H
#define SUBSETSUM_H

#include "common.h"
#include <bitset>

// ============================================================
// 子集和数问题求解器
// 判断是否存在子集和为 target，并给出方案
// 同时提供动态规划和回溯法两种解法
// ============================================================
class SubsetSumSolver {
public:
    // ========================================
    // 动态规划解法
    // 使用 bitset 优化空间和时间（类似背包DP）
    // 时间复杂度 O(n * target / 64)，空间 O(target / 64)
    // ========================================
    struct DPResult {
        bool feasible;              // 是否存在解
        long long operations;       // 基本操作计数
        std::vector<int> selected;  // 选中元素的下标
    };

    static DPResult solve_dp(const std::vector<long long>& numbers,
                             long long target) {
        int n = (int)numbers.size();
        DPResult res;
        res.feasible = false;
        res.operations = 0;

        // corner case: target == 0 (空集)
        if (target == 0) {
            res.feasible = true;
            return res;
        }
        // 过大直接剪掉
        long long total = 0;
        for (long long x : numbers) total += x;
        if (total < target) {
            res.feasible = false;
            return res;
        }

        // 使用 vector<bool> 或 bitset 做 DP
        // 为了支持大 target，使用 vector<bool>（自动bit压缩）
        // 但同时需要记录 parent 来回溯方案
        // parent[j] = 用第几个元素达到和 j
        std::vector<int> parent(target + 1, -1);
        std::vector<bool> dp(target + 1, false);
        dp[0] = true;

        for (int i = 0; i < n; i++) {
            long long x = numbers[i];
            if (x > target) continue;  // 跳过大于target的数
            // 从大到小遍历（每个元素只能用一次）
            for (long long j = target; j >= x; j--) {
                res.operations++;
                if (!dp[j] && dp[j - x]) {
                    dp[j] = true;
                    parent[j] = i;
                }
            }
        }

        res.feasible = dp[target];

        // 回溯方案
        if (res.feasible) {
            long long cur = target;
            while (cur > 0 && parent[cur] != -1) {
                int idx = parent[cur];
                res.selected.push_back(idx);
                cur -= numbers[idx];
            }
            std::reverse(res.selected.begin(), res.selected.end());
        }

        return res;
    }

    // ========================================
    // 回溯法解法（DFS + 剪枝）
    // 排序后从大到小搜索，剪枝策略：
    //   1. cur_sum > target → 剪枝
    //   2. cur_sum + suffix_sum[idx] < target → 剪枝
    //   3. cur_sum == target → 直接返回
    // ========================================
    struct BTResult {
        bool feasible;
        long long nodes_visited;
        std::vector<int> selected;  // 选中元素的下标（排序后的）
    };

    static BTResult solve_backtrack(const std::vector<long long>& numbers,
                                    long long target) {
        int n = (int)numbers.size();
        BTResult res;
        res.feasible = false;
        res.nodes_visited = 0;

        // corner case
        if (target == 0) {
            res.feasible = true;
            return res;
        }
        long long total = 0;
        for (long long x : numbers) total += x;
        if (total < target) return res;

        // 构造带序号的结构体，按值从大到小排序
        std::vector<Elem> elems(n);
        for (int i = 0; i < n; i++) {
            elems[i].val = numbers[i];
            elems[i].original_id = i;
        }
        std::sort(elems.begin(), elems.end(),
                  [](const Elem& a, const Elem& b) {
                      return a.val > b.val;  // 降序：大数优先
                  });

        // 计算后缀和，用于剪枝
        std::vector<long long> suffix(n + 1, 0);
        for (int i = n - 1; i >= 0; i--) {
            suffix[i] = suffix[i + 1] + elems[i].val;
        }

        std::vector<int> cur_select;
        std::vector<int> best_select;
        bool found = false;

        dfs(elems, suffix, target, 0, 0LL,
            cur_select, best_select, found, res);

        // 映射回原始编号
        res.selected.clear();
        for (int idx : best_select) {
            res.selected.push_back(elems[idx].original_id);
        }
        std::sort(res.selected.begin(), res.selected.end());

        return res;
    }

private:
    // 内部结构体（用于排序）
    struct Elem {
        long long val;
        int original_id;
    };

    static void dfs(const std::vector<Elem>& elems,
                    const std::vector<long long>& suffix,
                    long long target,
                    int idx,
                    long long cur_sum,
                    std::vector<int>& cur_select,
                    std::vector<int>& best_select,
                    bool& found,
                    BTResult& res) {
        res.nodes_visited++;

        // 找到解
        if (cur_sum == target) {
            res.feasible = true;
            best_select = cur_select;
            found = true;
            return;
        }

        // 超出或越界
        if (cur_sum > target || idx >= (int)elems.size()) return;

        // 上界剪枝：即使全选也不够
        if (cur_sum + suffix[idx] < target) return;

        // 左分支：选当前元素
        if (cur_sum + elems[idx].val <= target) {
            cur_select.push_back(idx);
            dfs(elems, suffix, target, idx + 1,
                cur_sum + elems[idx].val,
                cur_select, best_select, found, res);
            cur_select.pop_back();
            if (found) return;  // 找到一个解就返回
        }

        // 右分支：不选当前元素
        dfs(elems, suffix, target, idx + 1,
            cur_sum, cur_select, best_select, found, res);
    }
};

#endif // SUBSETSUM_H
