#pragma once
#ifndef __DINDEX_H__
#define __DINDEX_H__

#include "dnode.h"
#include "dnode_impl.h"

template<class key_t, class val_t>
class dindex{

public:
    dindex()    {}
    void dbuild(std::vector<key_t> keys, std::vector<val_t> vals, size_t num);
    void dnormal_retrain();
    bool dget(key_t key);
    bool dput(key_t key, val_t val);
    bool dinsert(key_t key, val_t val, int &lookup_time, int &move_time);
    bool dcheck();
    int dget_pos(key_t key);
    void dinsert_time(std::vector<key_t> keys);

private:
    LinearRegressionModel1<key_t> dlr_model_root;
    std::vector<DNode<key_t, val_t>> dnodes;
    std::vector<key_t> dpivots;
};


#endif