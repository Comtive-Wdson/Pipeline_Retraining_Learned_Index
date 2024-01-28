#pragma once
#ifndef __NODE_H__
#define __NODE_H__

#include <array>
#include <vector>
#include <assert.h>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <xmmintrin.h>
#include "lr_model.h"
#include "lr_model_impl.h"


template<class key_t, class val_t>
class Node{
    // struct AtomicVal{
    //     //后续完善的地方，需要增加composite的信息，和结合体指针
    //     val_t val;
    //     //Node * leaf_node = NULL;
    //     std::vector<record_t> *leaf = NULL;
    //     long long composite_flag = 0;

    //     static const uint64_t leaf_mask = 0x1;
    //     static const uint64_t prefetch_mask = 0x2;
    //     bool has_leaf()
    //     {
    //         return composite_flag & leaf_mask;
    //     }
    //     bool has_prefetched()
    //     {
    //         return composite_flag & prefetch_mask;
    //     }
    //     void prefetched()
    //     {
    //         composite_flag |= prefetch_mask;
    //     }
    //     AtomicVal(val_t value){
    //         val = value;
    //     }
    // };

    struct record_t{

        struct AtomicVal{
        //后续完善的地方，需要增加composite的信息，和结合体指针
        val_t val;
        //Node * leaf_node = NULL;
        std::vector<record_t> *leaf = NULL;
        long long composite_flag = 0;

        static const uint64_t leaf_mask = 0x1;
        static const uint64_t prefetch_mask = 0x2;
        bool has_leaf()
        {
            return composite_flag & leaf_mask;
            // return false;
        }
        bool has_prefetched()
        {
            return composite_flag & prefetch_mask;
            // return true;
        }
        void prefetched()
        {
            composite_flag |= prefetch_mask;
        }
        AtomicVal(val_t value){
            val = value;
        }
    };

        key_t key;
        AtomicVal val;
        record_t(key_t k): key(k), val(1) {} 
        record_t(key_t k, val_t v): key(k), val(v) {}
    };



public:
    inline Node()   {}
    inline Node(std::vector<key_t> keys, std::vector<val_t> vals);
    inline Node(std::vector<key_t> keys, std::vector<val_t> vals, size_t start, size_t end);
    void train();
    void normal_retrain();
    void pipeline_retrain(size_t distance, int way);
    bool get(key_t key);
    bool put(key_t key, val_t val);
    bool insert(key_t key, val_t val, int &lookup_time, int &move_time);
    bool insert(key_t key, val_t val);
    bool check();
    size_t count_leaf();

    //size_t middle_search(key_t key);
    int upper_less(key_t key);
    //std::vector<record_t> data;


private:
    std::vector<record_t> data;
    LinearRegressionModel<key_t> lr_model;

};

#endif
