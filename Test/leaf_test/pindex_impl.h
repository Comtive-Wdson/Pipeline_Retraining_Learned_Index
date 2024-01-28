#pragma once
#ifndef __PINDEX_IMPL_H__
#define __PINDEX_IMPL_H__

#include "pindex.h"

template<class key_t, class val_t>
void pindex<key_t, val_t>::build(std::vector<key_t> keys, std::vector<val_t> vals, size_t num)
{
    size_t chunck_size = keys.size() / num;
    for(int i = 0; i < num; i++)
    {
        size_t start = i * chunck_size;
        size_t end = i == num-1 ? keys.size() : start + chunck_size;

        pivots.push_back(keys[start]);
        Node<key_t, val_t> node(keys, vals, start, end);
        nodes.push_back(node);
    }
    lr_model_root.train(pivots);
}


template<class key_t, class val_t>
int pindex<key_t, val_t>::get_pos(key_t key)
{
    size_t pos = lr_model_root.predict(key);
    size_t maxErr = lr_model_root.get_maxErr();
    int start = (int)(pos - maxErr) < 0 ? 0 : pos - maxErr;
    
    if(start < 0)
        start = 0;
    if(start >= pivots.size())
        start = pivots.size() - 1;
    size_t end = pos + maxErr > pivots.size() - 1 ? pivots.size() - 1 : pos + maxErr;
    int position = -1;
    
    while(start <= end)
    {
        size_t mid = (start + end) / 2;
        if(pivots[mid] <= key){
            position = mid;
            start = mid + 1;
        }
        else{
            end = mid - 1;
        }
    }
    if(position == -1){
        size_t left = (int)(pos - maxErr) < 0 ? 0 : pos - maxErr;
        while(left >= 0 && pivots[left] > key)
            left--;
        position = left;
        if(position == -1)
        std::cout << "root insert position wrong" << std::endl;
    }
    else{
        size_t right = position;
        while(right+1 < pivots.size() && pivots[right+1] <= key)
            right++;
        position = right;
    }
    return position;
}


template<class key_t, class val_t>
bool pindex<key_t, val_t>::get(key_t key)
{
    size_t position = get_pos(key);
    return nodes[position].get(key);
}

template<class key_t, class val_t>
bool pindex<key_t, val_t>::put(key_t key, val_t val)
{
    size_t position = get_pos(key);
    return nodes[position].put(key, val);
}

template<class key_t, class val_t>
bool pindex<key_t, val_t>::insert(key_t key, val_t val, int &lookup_time, int &move_time)
{
    size_t position = get_pos(key);
    return nodes[position].insert(key, val, lookup_time, move_time);
}

template<class key_t, class val_t>
bool pindex<key_t, val_t>::insert(key_t key, val_t val)
{
    size_t position = get_pos(key);
    return nodes[position].insert(key, val);
}


template<class key_t, class val_t>
void pindex<key_t, val_t>::normal_retrain()
{
    //int p = 0;
    for(int i = 0; i < pivots.size(); i++)
        nodes[i].normal_retrain();
}

template<class key_t, class val_t>
void pindex<key_t, val_t>::pipeline_retrain(size_t distance, int way)
{
    for(int i = 0; i < pivots.size(); i++)
        nodes[i].pipeline_retrain(distance, way);
}


template<class key_t, class val_t>
bool pindex<key_t, val_t>::check()
{
    for(int i = 0; i < pivots.size(); i++)
        if(!nodes[i].check())
            return false;
    return true;
}

template<class key_t, class val_t>
void pindex<key_t, val_t>::insert_time(std::vector<key_t> keys)
{   
    int lookup_time = 0;
    int move_time = 0;
    int insert_time = 0;
    for(size_t i = 0; i < keys.size(); i++)
    {
        int t1 = 0;
        int t2 = 0;
        bool f = insert(keys[i], 1, t1, t2);
        lookup_time += t1;
        move_time += t2;
        insert_time = insert_time + t1 + t2;
    }

    std::cout << "leaf array lookup time: " << lookup_time  / keys.size() << std::endl;
    std::cout << "leaf array move time: " << move_time  / keys.size() << std::endl;
    std::cout << "leaf array insert time: " << insert_time  / keys.size() << std::endl;
}


#endif
