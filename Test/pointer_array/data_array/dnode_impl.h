#pragma once
#ifndef __DNode_IMPL_H__
#define __DNode_IMPL_H__

#include "dnode.h"
#include "../function.h"


template<class key_t, class val_t>
inline DNode<key_t, val_t>::DNode(std::vector<key_t> keys, std::vector<val_t> vals)
{
    for(size_t i = 0; i < keys.size(); i++)
    {
        record_t record(keys[i], vals[i]);
        data_array.push_back(record);
    }
    dtrain();
}

template<class key_t, class val_t>
inline DNode<key_t, val_t>::DNode(std::vector<key_t> keys, std::vector<val_t> vals, size_t start, size_t end)
{
    for(size_t i = start; i < end; i++)
    {
        record_t record(keys[i], vals[i]);
        data_array.push_back(record);
    }
    dtrain();
}


template<class key_t, class val_t>
int DNode<key_t, val_t>::dupper_less(key_t key)
{
    size_t pos = dlr_model.predict1(key);
    size_t maxErr = dlr_model.get_maxErr1();
    int start = (int)(pos - maxErr) < 0 ? 0 : pos - maxErr;
    if(start >= data_array.size())
        start = data_array.size() - 1;
    size_t end = pos + maxErr > data_array.size() - 1 ? data_array.size() - 1 : pos + maxErr;
    int get_pos = -1;
    
    while(start <= end)
    {
        size_t mid = (start + end) / 2;
        if(data_array[mid].key <= key){
            get_pos = mid;
            start = mid + 1;
        }
        else{
            end = mid - 1;
        }
    }
    if(get_pos == -1){
        size_t left = start;
        while(left >= 0 && data_array[left].key > key)
            left--;
        get_pos = left;
    }
    else{
        size_t right = get_pos;
        while(right+1 < data_array.size() && data_array[right+1].key <= key)
            right++;
        get_pos = right;
    }
    return get_pos;
}




template<class key_t, class val_t>
bool DNode<key_t, val_t>::dget(key_t key)
{
    int get_pos = dupper_less(key);
    if(get_pos == -1){
        std::cout << "get first wrong" << std::endl;
        return false;
    }

    //std::cout << "get_pos: " << get_pos << std::endl;
    if(data_array[get_pos].key == key)
        return true;
    else
        return false;  
}


template<class key_t, class val_t>
bool DNode<key_t, val_t>::dput(key_t key, val_t val)
{
    int get_pos = dupper_less(key);
    if(get_pos == -1)
        return false;

    if(data_array[get_pos].key == key)
    {
        data_array[get_pos].val = val;
        return true;
    }
    else
        return false;  
    
}

template<class key_t, class val_t>
bool DNode<key_t, val_t>::dinsert(key_t key, val_t val, int &lookup_time, int &move_time)
{   
    uint64_t t1 = perf_counter();
	int insert_pos = dupper_less(key);
	uint64_t t2 = perf_counter();
	lookup_time = (t2 - t1) * 1e6 / 2904002;
   
    if(insert_pos == -1)
        return false;

    record_t record_pointer(key, val);
    
    uint64_t t3 = perf_counter();
	data_array.insert(data_array.begin() + insert_pos + 1, record_pointer);
	uint64_t t4 = perf_counter();
	move_time = (t4 - t3) * 1e6 / 2904002;
    
    return true;
}



template<class key_t, class val_t>
bool DNode<key_t, val_t>::dcheck()
{
    key_t next = -1;
    for(size_t i = 0; i < data_array.size(); i++)
    {
        key_t prev = next;
        key_t next = data_array[i].key;
        if(prev >= next)
            return false;
    }
    return true;
}



template<class key_t, class val_t>
void DNode<key_t, val_t>::dtrain()
{
    std::vector<key_t> keys;
    for(size_t i = 0; i < data_array.size(); i++)
    {
        keys.push_back(data_array[i].key);
    }
    dlr_model.train1(keys);
}


template<class key_t, class val_t>
void DNode<key_t, val_t>::dnormal_retrain()
{
    double x_expected = 0, y_expected = 0, xy_expected = 0,
        x_square_expected = 0;
    int position = -1;
    for(size_t i = 0; i < data_array.size(); i++)
    {
        double key = data_array[i].key;
        position++;
        x_expected += key;
        y_expected += position;
        x_square_expected += key * key;
        xy_expected += key * position;
    }
    position++;
    x_expected /= position;
    y_expected /= position;
    x_square_expected /= position;
    xy_expected /= position;

    double weight0 = (xy_expected - x_expected * y_expected) /
        (x_square_expected - x_expected * x_expected);
    double weight1 = (x_square_expected * y_expected - x_expected * xy_expected) /
        (x_square_expected - x_expected * x_expected);
    dlr_model.set_weight01(weight0);
    dlr_model.set_weight11(weight1);
    //dlr_model.dprint_weights1();
    //return position;
}


#endif
