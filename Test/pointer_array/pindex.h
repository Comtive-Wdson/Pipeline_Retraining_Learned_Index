#pragma once
#ifndef __PINDEX_H__
#define __PINDEX_H__

#include "node.h"
#include "node_impl.h"

template<class key_t, class val_t>
class pindex{

public:
    pindex()    {}
    void build(std::vector<key_t> keys, std::vector<val_t> vals, size_t num);
    void normal_retrain();
    void pipeline_retrain(size_t distance);
    bool get(key_t key);
    bool put(key_t key, val_t val);
    bool insert(key_t key, val_t val, int &lookup_time, int &move_time);
    bool insert(key_t key, val_t val);
    bool check();
    void count_leaf();
    int get_pos(key_t key);
    void insert_time(std::vector<key_t> keys);

private:
    LinearRegressionModel<key_t> lr_model_root;
    std::vector<Node<key_t, val_t>> nodes;
    std::vector<key_t> pivots;
};


#endif