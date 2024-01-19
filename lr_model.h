#pragma once
#ifndef __LR_MODEL_H__
#define __LR_MODEL_H__

#include <array>
#include <vector>
#include <assert.h>
#include <algorithm>
#include <cmath>
#include <ctime>

template <class key_t>
class LinearRegressionModel {


public:
    inline LinearRegressionModel();
    inline LinearRegressionModel(double w, double b);
    ~LinearRegressionModel();
    void train(const typename std::vector<key_t>::const_iterator& it, size_t size);
    void train(std::vector<key_t>& keys,
        std::vector<size_t>& positions);
    void train(std::vector<key_t>& keys);
    void print_weights() const;
    size_t predict(const key_t& key) const;
    std::vector<size_t> predict(const std::vector<key_t>& keys) const;
    size_t max_error(const typename std::vector<key_t>::const_iterator& keys_begin,
        uint32_t size);
    size_t max_error(const std::vector<key_t>& keys,
        const std::vector<size_t>& positions);
    inline double get_weight0() { return weights[0]; }
    inline double get_weight1() { return weights[1]; }
    inline void set_weight0(double w)   {weights[0] = w;}
    inline void set_weight1(double w)   {weights[1] = w;}
    inline size_t get_maxErr() { return maxErr; }

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
    size_t maxErr = 0;
    std::array<double, 2> weights;

};



#endif
