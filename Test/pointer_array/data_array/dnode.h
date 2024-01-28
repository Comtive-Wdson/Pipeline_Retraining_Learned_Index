#pragma once
#ifndef __DNODE_H__
#define __DNODE_H__

#include <array>
#include <vector>
#include <assert.h>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <xmmintrin.h>
#include "lr_model1.h"
#include "lr_model_impl1.h"


template<class key_t, class val_t>
class DNode{

    struct record_t{

        struct AtomicVal{
        val_t val;
        long long composite_flag = 0;

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
    inline DNode()   {}
    inline DNode(std::vector<key_t> keys, std::vector<val_t> vals);
    inline DNode(std::vector<key_t> keys, std::vector<val_t> vals, size_t start, size_t end);
    void dtrain();
    void dnormal_retrain();
    void dpipeline_retrain(size_t distance);
    bool dget(key_t key);
    bool dput(key_t key, val_t val);
    bool dinsert(key_t key, val_t val, int &lookup_time, int &move_time);
    bool dcheck();

    //size_t middle_search(key_t key);
    int dupper_less(key_t key);
    //std::vector<record_t> data;


private:
    std::vector<record_t> data_array;
    LinearRegressionModel1<key_t> dlr_model;

};

#endif