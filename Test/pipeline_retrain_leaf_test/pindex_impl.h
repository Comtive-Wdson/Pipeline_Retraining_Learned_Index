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
        // if(i == 46)
        //     std::cout << "46 last; " << keys[end-1] << std::endl;
        // if(i == 47)
        //     std::cout << "47 first" << keys[start] << std::endl;
        // if(i == 46)
        Node<key_t, val_t> node(keys, vals, start, end);
        nodes.push_back(node);
    }
    lr_model_root.train(pivots);
}


template<class key_t, class val_t>
size_t pindex<key_t, val_t>::get_pos(key_t key)
{
    size_t pos = lr_model_root.predict(key);
    size_t maxErr = lr_model_root.get_maxErr();
    int start = (int)(pos - maxErr) < 0 ? 0 : pos - maxErr;
    size_t end = pos + maxErr > pivots.size() - 1 ? pivots.size() - 1 : pos + maxErr;
    size_t position = -1;
    
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
bool pindex<key_t, val_t>::insert(key_t key, val_t val)
{
    size_t position = get_pos(key);
    //std::cout << position << std::endl;
    return nodes[position].insert(key, val);
}


template<class key_t, class val_t>
void pindex<key_t, val_t>::normal_retrain()
{
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
void pindex<key_t, val_t>::count_leaf()
{
    int count_node = 0;
    int count_leaf = 0;
    for(int i = 0; i < nodes.size(); i++)
    {
        if(i >= 45 && i <= 48)
            std::cout << nodes[i].data.size() << std::endl;
        int countx = nodes[i].count_leaf();
        if(countx > 0)
        {
            count_node += 1;
            count_leaf += countx;
        }
    }
    std::cout << "count_leaf: " << count_leaf << std::endl;
    std::cout << "count_node: " << count_node << std::endl;

    std::cout << "check_leaf: " << nodes[46].data[71].val.has_leaf() << std::endl;
     std::cout << "check_leaf2: " << nodes[46].data[71].val.composite_flag << std::endl;
     std::cout << "check_leaf3: " << nodes[46].count_leaf() << std::endl;
}



#endif