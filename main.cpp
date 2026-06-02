/**
 * 算法实验 — 主程序
 * 实验 2.2：动态规划与回溯算法验证
 * 问题 4.1 (0/1背包) + 4.2 (子集和数) + 4.3 (矩阵乘法链)
 */

#include "common.h"
#include "knapsack.h"
#include "subsetsum.h"
#include "matrixchain.h"
#include <iomanip>
#include <cstring>
#include <set>

const std::string DATA_ROOT = "./datasets";

// ============================================================
// 实验 4.1：0/1 背包问题
// 两个数据源: FSU (分文件) + Unicauca (单文件)
// ============================================================
void run_knapsack_experiments(const std::string& base_path) {
    print_header("实验 4.1 — 0/1 背包问题");

    // --- 4.1 FSU: 分文件格式 ---
    std::string fsu_dir = base_path + "/4.1 Florida State University";
    if (std::filesystem::exists(fsu_dir)) {
        std::cout << "  [FSU 数据源]" << std::endl;

        // 收集所有 pXX 基础名
        std::set<std::string> bases;
        for (const auto& entry : std::filesystem::directory_iterator(fsu_dir)) {
            std::string name = entry.path().filename().string();
            if (name.size() >= 5 && name.substr(0, 1) == "p" && name.find("_c.txt") != std::string::npos) {
                bases.insert(name.substr(0, 3));  // "p01", "p02", ...
            }
        }

        std::cout << std::left
                  << std::setw(12) << "数据集"
                  << std::setw(8)  << "n"
                  << std::setw(10) << "容量"
                  << std::setw(10) << "DP(ms)"
                  << std::setw(10) << "回溯(ms)"
                  << std::setw(10) << "最优值"
                  << std::setw(10) << "标准答案"
                  << std::setw(6)  << "一致"
                  << std::endl;
        std::cout << std::string(76, '-') << std::endl;

        for (const auto& base : bases) {
            int n; long long cap;
            std::vector<long long> w, v;
            if (!DatasetReader::read_knapsack_fsu(fsu_dir, base, n, cap, w, v)) continue;

            ExperimentResult r;
            r.dataset_name = base;
            r.n = n; r.param = cap;

            Timer t;
            t.start();
            auto dp_r = KnapsackSolver::solve_dp_light(w, v, cap);
            t.stop();
            r.dp_time_ms = t.elapsed_ms();
            r.dp_ops = dp_r.operations;
            r.dp_optimal = dp_r.optimal_value;

            t.start();
            auto bt_r = KnapsackSolver::solve_backtrack(w, v, cap, 5000000);
            t.stop();
            r.bt_time_ms = t.elapsed_ms();
            r.bt_nodes = bt_r.nodes_visited;
            r.bt_optimal = bt_r.optimal_value;

            // 读取标准答案
            long long ground_truth = DatasetReader::compute_opt_from_solution(
                fsu_dir + "/" + base + "_s.txt", v);

            std::cout << std::left
                      << std::setw(12) << r.dataset_name
                      << std::setw(8)  << r.n
                      << std::setw(10) << r.param
                      << std::setw(10) << std::fixed << std::setprecision(3) << r.dp_time_ms
                      << std::setw(10) << std::fixed << std::setprecision(3) << r.bt_time_ms
                      << std::setw(10) << r.dp_optimal
                      << std::setw(10) << ground_truth
                      << std::setw(6)  << (r.dp_optimal == ground_truth ? "YES" : "NO")
                      << std::endl;
        }
        std::cout << std::endl;
    }

    // --- 4.1 Unicauca: 单文件格式 ---
    std::string uni_dir = base_path + "/4.1 Unicauca University/instances_01_KP";
    if (std::filesystem::exists(uni_dir)) {
        std::cout << "  [Unicauca 数据源]" << std::endl;
        auto all_files = DatasetReader::list_files(uni_dir, ".txt");
        // 也扫描子目录
        for (const auto& sub : {"low-dimensional", "large_scale"}) {
            auto sub_files = DatasetReader::list_files(uni_dir + "/" + sub);
            all_files.insert(all_files.end(), sub_files.begin(), sub_files.end());
        }

        std::cout << std::left
                  << std::setw(30) << "数据集"
                  << std::setw(8)  << "n"
                  << std::setw(10) << "容量"
                  << std::setw(10) << "DP(ms)"
                  << std::setw(10) << "回溯(ms)"
                  << std::setw(10) << "最优值"
                  << std::setw(10) << "标准答案"
                  << std::setw(6)  << "一致"
                  << std::endl;
        std::cout << std::string(94, '-') << std::endl;

        for (const auto& fp : all_files) {
            int n; long long cap;
            std::vector<long long> w, v;
            if (!DatasetReader::read_knapsack(fp, n, cap, w, v)) continue;

            ExperimentResult r;
            r.dataset_name = std::filesystem::path(fp).filename().string();
            r.n = n; r.param = cap;

            Timer t;
            t.start();
            auto dp_r = KnapsackSolver::solve_dp_light(w, v, cap);
            t.stop();
            r.dp_time_ms = t.elapsed_ms();
            r.dp_ops = dp_r.operations;
            r.dp_optimal = dp_r.optimal_value;

            t.start();
            auto bt_r = KnapsackSolver::solve_backtrack(w, v, cap, 5000000);
            t.stop();
            r.bt_time_ms = t.elapsed_ms();
            r.bt_nodes = bt_r.nodes_visited;
            r.bt_optimal = bt_r.optimal_value;

            // 读取标准答案
            long long ground_truth = -1;
            std::string dir = std::filesystem::path(fp).parent_path().string();
            std::string opt_dir = dir + "-optimum";
            if (std::filesystem::exists(opt_dir)) {
                std::string opt_path = opt_dir + "/" + r.dataset_name;
                ground_truth = DatasetReader::read_optimum(opt_path);
            }

            std::cout << std::left
                      << std::setw(30) << r.dataset_name
                      << std::setw(8)  << r.n
                      << std::setw(10) << r.param
                      << std::setw(10) << std::fixed << std::setprecision(3) << r.dp_time_ms
                      << std::setw(10) << std::fixed << std::setprecision(3) << r.bt_time_ms
                      << std::setw(10) << r.dp_optimal
                      << std::setw(10) << (ground_truth >= 0 ? std::to_string(ground_truth) : "-")
                      << std::setw(6)  << (ground_truth < 0 ? "-" : (r.dp_optimal == ground_truth ? "YES" : "NO"))
                      << std::endl;
        }
    }
    print_separator(90);
}

// ============================================================
// 实验 4.2：子集和数问题
// 数据源: FSU (分文件)
// ============================================================
void run_subsetsum_experiments(const std::string& base_path) {
    print_header("实验 4.2 — 子集和数问题");

    std::string fsu_dir = base_path + "/4.2 Florida State University";
    if (!std::filesystem::exists(fsu_dir)) {
        std::cout << "  未找到数据集目录" << std::endl;
        return;
    }

    // 收集 pXX 基础名
    std::set<std::string> bases;
    for (const auto& entry : std::filesystem::directory_iterator(fsu_dir)) {
        std::string name = entry.path().filename().string();
        if (name.size() >= 5 && name.substr(0, 1) == "p" && name.find("_c.txt") != std::string::npos)
            bases.insert(name.substr(0, 3));
    }

    std::cout << std::left
              << std::setw(12) << "数据集"
              << std::setw(8)  << "n"
              << std::setw(14) << "目标和"
              << std::setw(10) << "总和"
              << std::setw(10) << "DP(ms)"
              << std::setw(10) << "回溯(ms)"
              << std::setw(8)  << "可行?"
              << std::setw(6)  << "一致"
              << std::setw(6)  << "验证"
              << std::endl;
    std::cout << std::string(84, '-') << std::endl;

    for (const auto& base : bases) {
        int n; long long target;
        std::vector<long long> nums;
        if (!DatasetReader::read_subsetsum_fsu(fsu_dir, base, n, target, nums)) continue;

        long long total = 0;
        for (auto x : nums) total += x;

        ExperimentResult r;
        r.dataset_name = base;
        r.n = n; r.param = target;

        Timer t;
        t.start();
        auto dp_r = SubsetSumSolver::solve_dp(nums, target);
        t.stop();
        r.dp_time_ms = t.elapsed_ms();
        r.dp_ops = dp_r.operations;
        r.dp_optimal = dp_r.feasible ? 1 : 0;

        t.start();
        auto bt_r = SubsetSumSolver::solve_backtrack(nums, target);
        t.stop();
        r.bt_time_ms = t.elapsed_ms();
        r.bt_nodes = bt_r.nodes_visited;
        r.bt_optimal = bt_r.feasible ? 1 : 0;

        // 验证: 用DP找到的方案实际求和检验
        long long verify_sum = 0;
        for (int idx : dp_r.selected) verify_sum += nums[idx];
        bool verified = dp_r.feasible && (verify_sum == target);

        std::cout << std::left
                  << std::setw(12) << r.dataset_name
                  << std::setw(8)  << r.n
                  << std::setw(14) << r.param
                  << std::setw(10) << total
                  << std::setw(10) << std::fixed << std::setprecision(3) << r.dp_time_ms
                  << std::setw(10) << std::fixed << std::setprecision(3) << r.bt_time_ms
                  << std::setw(8)  << (dp_r.feasible ? "YES" : "NO")
                  << std::setw(6)  << (r.consistent() ? "YES" : "NO")
                  << std::setw(6)  << (dp_r.feasible ? (verified ? "OK" : "FAIL") : "-")
                  << std::endl;
    }
    print_separator(90);
}

// ============================================================
// 实验 4.3：矩阵乘法链问题
// ============================================================
void run_matrixchain_experiments(const std::string& base_path) {
    print_header("实验 4.3 — 矩阵乘法链问题");

    std::string mc_dir = base_path + "/4.3 Matrix Multiplication Chain";
    if (!std::filesystem::exists(mc_dir)) {
        std::cout << "  未找到数据集目录" << std::endl;
        return;
    }

    auto files = DatasetReader::list_files(mc_dir, ".txt");
    if (files.empty()) {
        std::cout << "  未找到数据集文件" << std::endl;
        return;
    }

    std::cout << std::left
              << std::setw(20) << "数据集"
              << std::setw(6)  << "n"
              << std::setw(16) << "维度范围"
              << std::setw(10) << "DP(ms)"
              << std::setw(10) << "回溯(ms)"
              << std::setw(10) << "回溯节点"
              << std::setw(16) << "最优值"
              << std::setw(6)  << "一致"
              << std::endl;
    std::cout << std::string(94, '-') << std::endl;

    for (const auto& fp : files) {
        int n; std::vector<int> dims;
        if (!DatasetReader::read_matrixchain(fp, n, dims)) continue;

        int dmin = *std::min_element(dims.begin(), dims.end());
        int dmax = *std::max_element(dims.begin(), dims.end());
        std::string range_str = "[" + std::to_string(dmin) + "," + std::to_string(dmax) + "]";

        ExperimentResult r;
        r.dataset_name = std::filesystem::path(fp).filename().string();
        r.n = n; r.param = dmax;

        Timer t;
        t.start();
        auto dp_r = MatrixChainSolver::solve_dp(dims);
        t.stop();
        r.dp_time_ms = t.elapsed_ms();
        r.dp_ops = dp_r.operations;
        r.dp_optimal = dp_r.optimal_cost;

        t.start();
        auto bt_r = MatrixChainSolver::solve_backtrack(dims);
        t.stop();
        r.bt_time_ms = t.elapsed_ms();
        r.bt_nodes = bt_r.nodes_visited;
        r.bt_optimal = bt_r.optimal_cost;

        std::cout << std::left
                  << std::setw(20) << r.dataset_name
                  << std::setw(6)  << r.n
                  << std::setw(16) << range_str
                  << std::setw(10) << std::fixed << std::setprecision(3) << r.dp_time_ms;
        if (bt_r.nodes_visited < 0)
            std::cout << std::setw(10) << "SKIP"
                      << std::setw(10) << "-";
        else
            std::cout << std::setw(10) << std::fixed << std::setprecision(3) << r.bt_time_ms
                      << std::setw(10) << r.bt_nodes;
        std::cout << std::setw(16) << r.dp_optimal
                  << std::setw(6)  << (r.consistent() ? "YES" : "NO")
                  << std::endl;
    }
    print_separator(90);
}

int main(int argc, char* argv[]) {
    std::string data_root = DATA_ROOT;
    if (argc > 1) data_root = argv[1];

    std::cout << "=====================================================" << std::endl;
    std::cout << "  Algorithm Experiment 2.2: DP vs Backtracking" << std::endl;
    std::cout << "  Problems: 4.1 Knapsack | 4.2 SubsetSum | 4.3 MatrixChain" << std::endl;
    std::cout << "=====================================================" << std::endl;
    std::cout << "  Data root: " << data_root << std::endl << std::endl;

    run_knapsack_experiments(data_root);
    std::cout << std::endl;
    run_subsetsum_experiments(data_root);
    std::cout << std::endl;
    run_matrixchain_experiments(data_root);
    std::cout << std::endl << "  All experiments completed!" << std::endl;
    return 0;
}
