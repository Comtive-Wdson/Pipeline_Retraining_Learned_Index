#pragma once
#ifndef __DINDEX_IMPL_H__
#define __DINDEX_IMPL_H__

#include "dindex.h"
#include "../function.h"

template<class key_t, class val_t>
void dindex<key_t, val_t>::dbuild(std::vector<key_t> keys, std::vector<val_t> vals, size_t num)
{
    size_t chunck_size = keys.size() / num;
    for(int i = 0; i < num; i++)
    {
        size_t start = i * chunck_size;
        size_t end = i == num-1 ? keys.size() : start + chunck_size;

        dpivots.push_back(keys[start]);
        DNode<key_t, val_t> node(keys, vals, start, end);
        dnodes.push_back(node);
    }
    dlr_model_root.train1(dpivots);
}


template<class key_t, class val_t>
int dindex<key_t, val_t>::dget_pos(key_t key)
{
    size_t pos = dlr_model_root.predict1(key);
    size_t maxErr = dlr_model_root.get_maxErr1();
    int start = (int)(pos - maxErr) < 0 ? 0 : pos - maxErr;
    if(start >= dpivots.size())
        start = dpivots.size() - 1;
    size_t end = pos + maxErr > dpivots.size() - 1 ? dpivots.size() - 1 : pos + maxErr;
    int position = -1;
    
    while(start <= end)
    {
        size_t mid = (start + end) / 2;
        if(dpivots[mid] <= key){
            position = mid;
            start = mid + 1;
        }
        else{
            end = mid - 1;
        }
    }
    if(position == -1){
        size_t left = (int)(pos - maxErr) < 0 ? 0 : pos - maxErr;
        while(left >= 0 && dpivots[left] > key)
            left--;
        position = left;
        if(position == -1)
        std::cout << "root insert position wrong" << std::endl;
    }
    else{
        size_t right = position;
        while(right+1 < dpivots.size() && dpivots[right+1] <= key)
            right++;
        position = right;
    }
    return position;
}


template<class key_t, class val_t>
bool dindex<key_t, val_t>::dget(key_t key)
{
    int position = dget_pos(key);
    if(position == -1)
        return false;
    else
        return dnodes[position].dget(key);
}

template<class key_t, class val_t>
bool dindex<key_t, val_t>::dput(key_t key, val_t val)
{
    int position = dget_pos(key);
    if(position == -1)
        return false;
    else
        return dnodes[position].dput(key, val);
}

template<class key_t, class val_t>
bool dindex<key_t, val_t>::dinsert(key_t key, val_t val, int &lookup_time, int &move_time)
{
    int position = dget_pos(key);
    if(position == -1)
        return false;
    else
        return dnodes[position].dinsert(key, val, lookup_time, move_time);
}


template<class key_t, class val_t>
void dindex<key_t, val_t>::dnormal_retrain()
{
    for(int i = 0; i < dpivots.size(); i++)
        dnodes[i].dnormal_retrain();
}

template<class key_t, class val_t>
bool dindex<key_t, val_t>::dcheck()
{
    for(int i = 0; i < dpivots.size(); i++)
        if(!dnodes[i].dcheck())
            return false;
    return true;
}


template<class key_t, class val_t>
void dindex<key_t, val_t>::dinsert_time(std::vector<key_t> keys)
{   
    int lookup_time = 0;
    int move_time = 0;
    int insert_time = 0;
    for(size_t i = 0; i < keys.size(); i++)
    {
        int t1 = 0;
        int t2 = 0;
        bool f = dinsert(keys[i], 1, t1, t2);
        lookup_time += t1;
        move_time += t2;
        insert_time = insert_time + t1 + t2;
    }

    std::cout << "data array lookup time: " << lookup_time  / keys.size() << std::endl;
    std::cout << "data array move time: " << move_time  / keys.size() << std::endl;
    std::cout << "data array insert time: " << insert_time  / keys.size() << std::endl;
}

#endif