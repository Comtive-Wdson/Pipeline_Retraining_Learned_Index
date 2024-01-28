#pragma once
#ifndef __LR_MODEL_IMPL_H__
#define __LR_MODEL_IMPL_H__
#include "lr_model.h"
#include<sys/time.h>
#include "function.h"
#include <fstream>
#include <xmmintrin.h>

template<class key_t>
inline LinearRegressionModel<key_t>::LinearRegressionModel() {}

template<class key_t>
inline LinearRegressionModel<key_t>::LinearRegressionModel(double w, double b)
{
    weights[0] = w;
    weights[1] = b;
}

template<class key_t>
LinearRegressionModel<key_t>::~LinearRegressionModel() {}


template<class key_t>
void LinearRegressionModel<key_t>::train(const typename std::vector<key_t>::const_iterator& it, size_t size)
{
    std::vector<key_t> trainkeys(size);
    std::vector<size_t> positions(size);
    for (size_t i = 0; i < size; i++) {
        trainkeys[i] = *(it + i);
        positions[i] = i;
    }
    train(trainkeys, positions);
}


template<class key_t>
void LinearRegressionModel<key_t>::train(std::vector<key_t>& keys,
    std::vector<size_t>& positions)
{
    assert(keys.size() == positions.size());
    if (keys.size() == 0) return;

    std::vector<double> model_keys(keys.size());
    std::vector<double*> key_ptrs(keys.size());
    for (size_t i = 0; i < keys.size(); i++) {
        model_keys[i] = keys[i];
    }

    if (positions.size() == 0) return;
    if (positions.size() == 1) {
        weights[0] = 0;
        weights[1] = positions[0];
        return;
    }
    // use multiple dimension LR when running tpc-c
    double x_expected = 0, y_expected = 0, xy_expected = 0,
        x_square_expected = 0;
    int step = 16;
    for (size_t key_i = 0; key_i < positions.size(); key_i = key_i + step) {
        if(key_i + step < positions.size())
        _mm_prefetch((char *)&model_keys[key_i + step], _MM_HINT_T0);
        // if(key_i + step * 2 < positions.size())
        // _mm_prefetch((char *)&model_keys[key_i + step * 2], _MM_HINT_T0);

        for(size_t j = key_i; j < key_i + step && j < positions.size(); j++)
        {
            double key = model_keys[j];
            x_expected += key;
            y_expected += j;
            x_square_expected += key * key;
            xy_expected += key * j;
        }

        // double key = model_keys[key_i];
        // x_expected += key;
        // y_expected += key_i;
        // x_square_expected += key * key;
        // xy_expected += key * key_i;
    }
    /*assert(x_expected > 0);
    assert(y_expected > 0);
    assert(x_square_expected > 0);
    assert(xy_expected > 0);*/

    x_expected /= positions.size();
    y_expected /= positions.size();
    x_square_expected /= positions.size();
    xy_expected /= positions.size();

    weights[0] = (xy_expected - x_expected * y_expected) /
        (x_square_expected - x_expected * x_expected);
    weights[1] = (x_square_expected * y_expected - x_expected * xy_expected) /
        (x_square_expected - x_expected * x_expected);
    maxErr = max_error(keys, positions);
}



template <class key_t>
void LinearRegressionModel<key_t>::train(std::vector<key_t>& keys)
{
    std::vector<size_t> positions(keys.size());
    for (size_t i = 0; i < keys.size(); i++)
    {
        positions[i] = i;
    }
    train(keys, positions);
}



template<class key_t>
void LinearRegressionModel<key_t>::print_weights() const {
    std::cout << "Weight[0]: " << weights[0] << " ,weight[1]: " << weights[1] << std::endl;
}

// ============ prediction ==============
template <class key_t>
size_t LinearRegressionModel<key_t>::predict(const key_t& key) const {
    double model_key = key;
    double res = weights[0] * model_key + weights[1];
    return res > 0 ? res : 0;
}

template <class key_t>
std::vector<size_t> LinearRegressionModel<key_t>::predict(const std::vector<key_t>& keys) const
{
    assert(keys.size() > 0);
    std::vector<size_t> pred(keys.size());
    for (int i = 0; i < keys.size(); i++)
    {
        pred[i] = predict(keys[i]);
    }
    return pred;
}

// =========== max__error ===========
template <class key_t>
size_t LinearRegressionModel<key_t>::max_error(
    const typename std::vector<key_t>::const_iterator& keys_begin,
    uint32_t size) {
    size_t max = 0;

    for (size_t key_i = 0; key_i < size; ++key_i) {
        long long int pos_actual = key_i;
        long long int pos_pred = predict(*(keys_begin + key_i));
        int error = std::abs(pos_actual - pos_pred);
        if (error > max) {
            max = error;
        }
    }
    return max;
}

template <class key_t>
size_t LinearRegressionModel<key_t>::max_error(const std::vector<key_t>& keys,
    const std::vector<size_t>& positions)
{
    size_t max = 0;

    for (size_t key_i = 0; key_i < keys.size(); ++key_i) {
        long long int pos_actual = positions[key_i];
        long long int pos_pred = predict(keys[key_i]);
        //std::cout << "actual: " << pos_actual << "pred: " << pos_pred << std::endl;
        int error = std::abs(pos_actual - pos_pred);
        if (error > max) {
            max = error;
        }
    }
    return max;
}


template<class key_t>
int LinearRegressionModel<key_t>::search(std::vector<key_t>& keys, const key_t key)
{
     //std::cout << std::endl << std::endl;
    size_t pos = predict(key);
    if (pos >= 0 && pos < keys.size() && keys[pos] == key)
    {
        return (int)pos;
    }
    else
    {
        int start = (int)(pos - maxErr) < 0 ? 0 : pos - maxErr;
        size_t end = pos + maxErr > keys.size() - 1 ? keys.size() - 1 : pos + maxErr;

        while (start <= end)
        {
            size_t mid = (start + end) / 2;
            //std::cout << mid <<std::endl;
            //std::cout << (uintptr_t)&keys[mid] / 64  << std::endl;
            
            if (keys[mid] == key)
            {
                return (int)mid;
            }
                
            else if (keys[mid] < key)
            {   
                start = mid + 1;
               // std::cout << "start: " << start << "end:" << end << std::endl << std::endl;
            }
                
            else
            {
                if((int)(mid - 1) < 0)
                    return -1;
                end = mid - 1;
                //std::cout << "start: " << start << "end:" << end << std::endl << std::endl;
            }
                
        }
        return -1;
    }
}
// template<class key_t>
// uint64_t LinearRegressionModel<key_t>::search(std::vector<key_t>& keys, const key_t key)
// {
//     //clean_buffer();
//     uint64_t t1 = perf_counter();
//     size_t pos = predict(key);
//     uint64_t t2 = perf_counter();
//     predict_count += t2 - t1;

//     //clean_buffer();
//     uint64_t t3;
//     t3 = perf_counter();
//     uint64_t tx = perf_counter();
//   // int count = 0;
//     if (pos >= 0 && pos < keys.size() && keys[pos] == key)
//     {
//         //count++;
//         //count_sum += count;
//         uint64_t t5 = perf_counter();
//         search_count += t5 - t3;
//         return (int)pos;
//     }
//     else
//     {
//         int start = (int)(pos - maxErr) < 0 ? 0 : pos - maxErr;
//         size_t end = pos + maxErr > keys.size() - 1 ? keys.size() - 1 : pos + maxErr;
//         while (start <= end)
//         {   
//             //t3 = perf_counter();
//             //count++;
//             size_t mid = (start + end) / 2;
//             if (keys[mid] == key)
//             {
//                 //count_sum += count;
//                 uint64_t t4 = perf_counter();
//                 //std::cout << "count:   " << count << std::endl;
//                 //std::cout << "timecount : " << t4 - t3 << std::endl;
//                //search_count += t4 - t3;
//                return t4 - tx;
//                  //return (int)mid;
//             }
                
//             else if (keys[mid] < key)
//             {
//                 //uint64_t t5 = perf_counter();
//                 //std::cout << t5 - t3 << " left search counts" << std::endl;
//                  start = mid + 1;
//             }
                
//             else
//             {
//                 //uint64_t t6 = perf_counter();
//                 //std::cout << t6 - t3 << " right search counts" << std::endl;
//                  end = mid - 1;
//             }
                
//         }
//         uint64_t t5 = perf_counter();
//         std::cout << t5 - t3 << " fail search counts" << std::endl;
//         return -1;
//     }
// }


template<class key_t>
double LinearRegressionModel<key_t>::hit_ratio_calculate(std::vector<key_t>& keys)
{
    int hit = 0;
    for (size_t i = 0; i < keys.size(); i++)
    {
        int pos = search(keys, keys[i]);
        if (pos != -1)
            hit++;
    }
    double hit_ratio = (double)hit / keys.size();
    return hit_ratio;
}

template<class key_t>
size_t LinearRegressionModel<key_t>::error_calculate(std::vector<key_t>& keys)
{
    size_t error_mean = 0;
    for (size_t i = 0; i < keys.size(); i++)
    {
        long long int pos_actual = i;
        long long int pos_pred = predict(keys[i]);
        int error = std::abs(pos_actual - pos_pred);
        error_mean += error;
    }
    error_mean = error_mean / keys.size();
    return error_mean;
}

template<class key_t>
long LinearRegressionModel<key_t>::train_time_calculate(std::vector<key_t>& keys)
{
        uint64_t t1 = perf_counter();
        train(keys);
        uint64_t t2 = perf_counter();

    	long endtime = (t2 - t1) * 1e6 / 2904002;
    	return endtime / 1e6;
}

template<class key_t>
long LinearRegressionModel<key_t>::search_time_calculate(std::vector<key_t>& keys)
{
	long endtime = 0;
	for (size_t i = 0; i < keys.size(); i++)
    {
      	//clean_buffer();
       	uint64_t t1 = perf_counter();
       	volatile int pos = search(keys, keys[i]);
       	uint64_t t2 = perf_counter();
       	endtime += (t2 - t1) * 1e6 / 2904002;
    }
	return endtime / keys.size();
}

template<class key_t>
long LinearRegressionModel<key_t>::random_search_time_calculate(std::vector<key_t>& keys)
{  
      long endtime_sum = 0;
	size_t end = 170000000;
	long endtime = 0;
	for (size_t i = 3; i < end+3; i+=1e7)
    {
      	//clean_buffer();
       	uint64_t t1 = perf_counter();
       	volatile int pos = search(keys, keys[i]);
       	uint64_t t2 = perf_counter();
       	endtime += (t2 - t1) * 1e6 / 2904002;
    }
	return endtime / 17;
}

template<class key_t>
long LinearRegressionModel<key_t>::throughput(std::vector<key_t>& keys)
{
    long endtime = 0;
        for (size_t i = 0; i < keys.size(); i++)
        {
            //clean_buffer();
            uint64_t t1 = perf_counter();
           int pos = search(keys, keys[i]);
            uint64_t t2 = perf_counter();
            endtime += (t2 - t1) * 1e6 / 2904002;
        }
    
    	return keys.size() * 1e3 / endtime;
}

template<class key_t>
double LinearRegressionModel<key_t>::random_throughput(std::vector<key_t>& keys)
{  
      long endtime_sum = 0;
	size_t end = 170000000;
	long endtime = 0;
	for (size_t i = 3; i < end+3; i+=1e7)
    {
      	//clean_buffer();
       	uint64_t t1 = perf_counter();
       	volatile int pos = search(keys, keys[i]);
       	uint64_t t2 = perf_counter();
       	endtime += (t2 - t1) * 1e6 / 2904002;
    }
	return (double)17 * 1e3 / endtime;
}

template<class key_t>
void LinearRegressionModel<key_t>::test(std::vector<key_t>& keys)
{
    std::cout << "predict: " << predict_count / 1e5<< std::endl;
    std::cout << "search:  " << search_count / 1e5<< std::endl;
    std::cout << "binary_count:  " << count_sum / 1e5<< std::endl;
    std::cout << std::endl;

}

#endif