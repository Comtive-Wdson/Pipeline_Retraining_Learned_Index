#pragma once
#ifndef __NODE_IMPL_H__
#define __NODE_IMPL_H__

#include "node.h"


template<class key_t, class val_t>
inline Node<key_t, val_t>::Node(std::vector<key_t> keys, std::vector<val_t> vals)
{
    for(size_t i = 0; i < keys.size(); i++)
    {
        record_t* record_pointer = new record_t(keys[i], vals[i]);
        pointer_array.push_back(record_pointer);
        //std::cout << (*pointer_array[0]).key << std::endl;
    }
    lr_model.train(keys);
}

template<class key_t, class val_t>
inline Node<key_t, val_t>::Node(std::vector<key_t> keys, std::vector<val_t> vals, size_t start, size_t end)
{
    std::vector<key_t> keyss;
    for(size_t i = start; i < end; i++)
    {
        record_t* record_pointer = new record_t(keys[i], vals[i]);
        pointer_array.push_back(record_pointer);
        keyss.push_back(keys[i]);
        
    }
    lr_model.train(keyss);
}


template<class key_t, class val_t>
int Node<key_t, val_t>::upper_less(key_t key)
{
    size_t pos = lr_model.predict(key);
    size_t maxErr = lr_model.get_maxErr();
    int start = (int)(pos - maxErr) < 0 ? 0 : pos - maxErr;
    //std::cout << "start: " << start << std::endl;
    if(start < 0)
        start = 0;
    if(start >= pointer_array.size())
        start = pointer_array.size() - 1;
    size_t end = pos + maxErr > pointer_array.size() - 1 ? pointer_array.size() - 1 : pos + maxErr;
    //std::cout << "end: " << end << std::endl;
    int get_pos = -1;
    
    while(start <= end)
    {
        size_t mid = (start + end) / 2;
        //std::cout << "mid: " << (*pointer_array[mid]).key << std::endl;
        if((*pointer_array[mid]).key <= key){
            get_pos = mid;
            start = mid + 1;
        }
        else{
            end = mid - 1;
        }
    }
    if(get_pos == -1){
        size_t left = start;
        while(left >= 0 && (*pointer_array[left]).key > key)
            left--;
        get_pos = left;
    }
    else{
        size_t right = get_pos;
        while(right+1 < pointer_array.size() && (*pointer_array[right+1]).key <= key)
            right++;
        get_pos = right;
    }
    return get_pos;
}




template<class key_t, class val_t>
bool Node<key_t, val_t>::get(key_t key)
{
    int get_pos = upper_less(key);
    if(get_pos == -1){
        std::cout << "get first wrong" << std::endl;
        return false;
    }

    //std::cout << (*pointer_array[get_pos]).key << std::endl;
    //std::cout << key << std::endl;
    if((*pointer_array[get_pos]).key == key)
        return true;
    else
        return false;  
}


template<class key_t, class val_t>
bool Node<key_t, val_t>::put(key_t key, val_t val)
{
    int get_pos = upper_less(key);
    if(get_pos == -1)
        return false;

    if((*pointer_array[get_pos]).key == key)
    {
        (*pointer_array[get_pos]).val = val;
        return true;
    }
    else
        return false;  
    
}

template<class key_t, class val_t>
bool Node<key_t, val_t>::insert(key_t key, val_t val, int &lookup_time, int &move_time)
{
    uint64_t t1 = perf_counter();
	int insert_pos = upper_less(key);
	uint64_t t2 = perf_counter();
	lookup_time = (t2 - t1) * 1e6 / 2904002;
   
    if(insert_pos == -1)
        return false;

    uint64_t t3 = perf_counter();
	
    record_t* record_pointer = new record_t(key, val);
    pointer_array.insert(pointer_array.begin() + insert_pos + 1, record_pointer);

	uint64_t t4 = perf_counter();
	move_time = (t4 - t3) * 1e6 / 2904002;

    return true;
}

template<class key_t, class val_t>
bool Node<key_t, val_t>::insert(key_t key, val_t val)
{
	int insert_pos = upper_less(key);
    if(insert_pos == -1)
        return false;

    record_t* record_pointer = new record_t(key, val);
    pointer_array.insert(pointer_array.begin() + insert_pos + 1, record_pointer);
    return true;
}



template<class key_t, class val_t>
bool Node<key_t, val_t>::check()
{
    key_t next = -1;
    for(size_t i = 0; i < pointer_array.size(); i++)
    {
        key_t prev = next;
        key_t next = (*pointer_array[i]).key;
        if(prev >= next)
            return false;
    }
    return true;
}



template<class key_t, class val_t>
void Node<key_t, val_t>::train()
{
    std::vector<key_t> keys;
    for(size_t i = 0; i < pointer_array.size(); i++)
    {
        keys.push_back((*pointer_array[i]).key);
    }
    lr_model.train(keys);
}


template<class key_t, class val_t>
void Node<key_t, val_t>::normal_retrain()
{
    double x_expected = 0, y_expected = 0, xy_expected = 0,
        x_square_expected = 0;
    int position = -1;
    for(size_t i = 0; i < pointer_array.size(); i++)
    {
        double key = (*pointer_array[i]).key;
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
    lr_model.set_weight0(weight0);
    lr_model.set_weight1(weight1);
    //lr_model.print_weights();
    //return position;
}


template<class key_t, class val_t>
void Node<key_t, val_t>::pipeline_retrain(size_t distance)
{
    double x_expected = 0, y_expected = 0, xy_expected = 0,
        x_square_expected = 0;
    int position = -1;
    for(size_t i = 0; i < pointer_array.size(); i++)
    {   
        if(i + distance < pointer_array.size())
            _mm_prefetch((char *)pointer_array[i + distance], _MM_HINT_T0);

        double key = (*pointer_array[i]).key;
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
    lr_model.set_weight0(weight0);
    lr_model.set_weight1(weight1);
    //lr_model.print_weights();
}

#endif
