#pragma once
#ifndef __LR_MODEL1_H__
#define __LR_MODEL1_H__

#include <array>
#include <vector>
#include <assert.h>
#include <algorithm>
#include <cmath>
#include <ctime>

template <class key_t>
class LinearRegressionModel1 {


public:
    inline LinearRegressionModel1();
    inline LinearRegressionModel1(double w, double b);
    ~LinearRegressionModel1();
    void train1(const typename std::vector<key_t>::const_iterator& it, size_t size);
    void train1(std::vector<key_t>& keys,
        std::vector<size_t>& positions);
    void train1(std::vector<key_t>& keys);
    void dprint_weights1() const;
    size_t predict1(const key_t& key) const;
    std::vector<size_t> predict1(const std::vector<key_t>& keys) const;
    size_t max_error1(const typename std::vector<key_t>::const_iterator& keys_begin,
        uint32_t size);
    size_t max_error1(const std::vector<key_t>& keys,
        const std::vector<size_t>& positions);
    inline double get_weight01() { return weights1[0]; }
    inline double get_weight11() { return weights1[1]; }
    inline void set_weight01(double w)   {weights1[0] = w;}
    inline void set_weight11(double w)   {weights1[1] = w;}
    inline size_t get_maxErr1() { return maxErr1; }

    uint64_t predict_count = 0;
    uint64_t search_count = 0;
    long long count_sum = 0;
    int search(std::vector<key_t>& keys, key_t key);
    double hit_ratio_calculate(std::vector<key_t>& keys);
    long search_time_calculate(std::vector<key_t>& keys);
    long random_search_time_calculate(std::vector<key_t>& keys);
    long train_time_calculate(std::vector<key_t>& keys);
    size_t error_calculate(std::vector<key_t>& keys);
    long throughput(std::vector<key_t>& keys);
    double random_throughput(std::vector<key_t>& keys);
    void test(std::vector<key_t>& keys);

private:
    size_t maxErr1 = 0;
    std::array<double, 2> weights1;

};



#endif
