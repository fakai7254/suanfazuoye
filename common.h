#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <filesystem>

class Timer {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    void start() { _start = Clock::now(); }
    void stop()  { _end = Clock::now(); }
    double elapsed_ms() const {
        return std::chrono::duration<double, std::milli>(_end - _start).count();
    }
private:
    TimePoint _start, _end;
};

struct ExperimentResult {
    std::string dataset_name;
    int n = 0;
    long long param = 0;
    double dp_time_ms = 0, bt_time_ms = 0;
    long long dp_ops = 0, bt_nodes = 0;
    long long dp_optimal = 0, bt_optimal = 0;
    bool consistent() const { return dp_optimal == bt_optimal; }
};

class DatasetReader {
public:
    // --- 单文件格式 (Unicauca): 每行 (value, weight) ---
    static bool read_knapsack(const std::string& fp, int& n, long long& cap,
                              std::vector<long long>& w, std::vector<long long>& v) {
        std::ifstream fin(fp);
        if (!fin) return false;
        fin >> n >> cap;
        w.resize(n); v.resize(n);
        for (int i = 0; i < n; i++) {
            long long val, wei;
            if (!(fin >> val >> wei)) return false;
            v[i] = val;   // Unicauca: 先 value 后 weight
            w[i] = wei;
        }
        return true;
    }

    // --- FSU 分文件格式: base_c.txt / base_w.txt / base_p.txt ---
    static bool read_knapsack_fsu(const std::string& dir, const std::string& base,
                                   int& n, long long& cap,
                                   std::vector<long long>& w, std::vector<long long>& v) {
        std::ifstream fc(dir + "/" + base + "_c.txt");
        std::ifstream fw(dir + "/" + base + "_w.txt");
        std::ifstream fp(dir + "/" + base + "_p.txt");
        if (!fc || !fw || !fp) return false;
        fc >> cap;
        long long wi, vi;
        w.clear(); v.clear();
        while (fw >> wi && fp >> vi) { w.push_back(wi); v.push_back(vi); }
        n = (int)w.size();
        return n > 0;
    }

    // --- FSU 分文件格式: base_c.txt / base_w.txt ---
    static bool read_subsetsum_fsu(const std::string& dir, const std::string& base,
                                    int& n, long long& target,
                                    std::vector<long long>& nums) {
        std::ifstream fc(dir + "/" + base + "_c.txt");
        std::ifstream fw(dir + "/" + base + "_w.txt");
        if (!fc || !fw) return false;
        fc >> target;
        long long x;
        nums.clear();
        while (fw >> x) nums.push_back(x);
        n = (int)nums.size();
        return n > 0;
    }

    // --- 矩阵乘法链: 首行 n，后续 n 行每行 rows cols ---
    static bool read_matrixchain(const std::string& fp, int& n,
                                  std::vector<int>& dims) {
        std::ifstream fin(fp);
        if (!fin) return false;
        fin >> n;
        dims.resize(n + 1);
        int r, c;
        for (int i = 0; i < n; i++) {
            if (!(fin >> r >> c)) return false;
            if (i == 0) dims[0] = r;
            dims[i + 1] = c;
        }
        return true;
    }

    static std::vector<std::string> list_files(const std::string& dir,
                                                const std::string& ext = "") {
        std::vector<std::string> files;
        namespace fs = std::filesystem;
        if (!fs::exists(dir) || !fs::is_directory(dir)) return files;
        for (const auto& e : fs::directory_iterator(dir)) {
            if (e.is_regular_file()) {
                std::string p = e.path().string();
                if (ext.empty() || p.find(ext) != std::string::npos)
                    files.push_back(p);
            }
        }
        std::sort(files.begin(), files.end());
        return files;
    }
    // --- 读取 Unicauca 最优值文件 (单行数字) ---
    static long long read_optimum(const std::string& fp) {
        std::ifstream fin(fp);
        if (!fin) return -1;
        long long opt;
        fin >> opt;
        return opt;
    }

    // --- 从 FSU solution 文件计算最优值 (背包: 每行1/0表示选否) ---
    static long long compute_opt_from_solution(const std::string& sol_path,
                                                const std::vector<long long>& v) {
        std::ifstream fin(sol_path);
        if (!fin) return -1;
        long long total = 0;
        int sel;
        for (int i = 0; i < (int)v.size(); i++) {
            if (!(fin >> sel)) break;
            if (sel == 1) total += v[i];
        }
        return total;
    }
};

inline void print_separator(int w = 90) { std::cout << std::string(w, '=') << std::endl; }
inline void print_header(const std::string& t) { print_separator(); std::cout << "  " << t << std::endl; print_separator(); }

#endif
