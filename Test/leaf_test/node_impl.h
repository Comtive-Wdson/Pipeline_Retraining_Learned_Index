#pragma once
#ifndef __NODE_IMPL_H__
#define __NODE_IMPL_H__

#include "node.h"


template<class key_t, class val_t>
inline Node<key_t, val_t>::Node(std::vector<key_t> keys, std::vector<val_t> vals)
{
    for(size_t i = 0; i < keys.size(); i++)
    {
        record_t record(keys[i], vals[i]);
        record.val.leaf = NULL;
        record.val.composite_flag = 0;
        data.push_back(record);
    }
    lr_model.train(keys);
}

template<class key_t, class val_t>
inline Node<key_t, val_t>::Node(std::vector<key_t> keys, std::vector<val_t> vals, size_t start, size_t end)
{   
    std::vector<key_t> keyss;
    for(size_t i = start; i < end; i++)
    {
        record_t record(keys[i], vals[i]);
        record.val.leaf = NULL;
        record.val.composite_flag = 0;
        data.push_back(record);
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
    if(start < 0)
        start = 0;
    if(start >= data.size())
        start = data.size() - 1;
    size_t end = pos + maxErr > data.size() - 1 ? data.size() - 1 : pos + maxErr;
    int get_pos = -1;

    
    while(start <= end)
    {
        size_t mid = (start + end) / 2;
        if(data[mid].key <= key){
            get_pos = mid;
            start = mid + 1;
        }
        else{
            end = mid - 1;
        }
    }
    if(get_pos == -1){
        size_t left = (int)(pos - maxErr) < 0 ? 0 : pos - maxErr;
        while(left >= 0 && data[left].key > key)
            left--;
        get_pos = left;
    }
    else{
        size_t right = get_pos;
        while(right+1 < data.size() && data[right+1].key <= key)
            right++;
        get_pos = right;
    }
    return get_pos;
}




template<class key_t, class val_t>
bool Node<key_t, val_t>::get(key_t key)
{
    size_t get_pos = upper_less(key);
    if(get_pos == -1){
        std::cout << "get first wrong" << std::endl;
        return false;
    }

    //std::cout << "get_pos: " << get_pos << std::endl;
    if(data[get_pos].key == key)
        return true;
    else if(!data[get_pos].val.has_leaf()){
        std::cout << "get second wrong" << std::endl;
        return false;
    }
        //return false;
    else{
        std::vector<record_t> &v =  *data[get_pos].val.leaf;
        int startx = 0;
        size_t endx = data[get_pos].val.leaf->size();
        //std::cout << "leaf first: " << v[startx].key << std::endl;
        while (startx <= endx)
        {
            size_t midx = (startx + endx) / 2;
            if (v[midx].key == key)
                return true;
            else if (v[midx].key < key)
                startx = midx + 1;
            else
            {
                if((int)(midx - 1) < 0)
                    return false;
                endx = midx - 1;
            }
                
        }
        return false;
    }   
}


template<class key_t, class val_t>
bool Node<key_t, val_t>::put(key_t key, val_t val)
{
    size_t get_pos = upper_less(key);
    if(get_pos == -1)
        return false;

    if(data[get_pos].key == key)
    {
        data[get_pos].val = val;
        return true;
    }
    else if(!data[get_pos].val.has_leaf())
        return false;
    else{
        std::vector<record_t> &v =  *data[get_pos].val.leaf;
        int startx = 0;
        size_t endx = data[get_pos].val.leaf->size();
        while (startx <= endx)
        {
            size_t midx = (startx + endx) / 2;
            if (v[midx].key == key)
            {
                v[midx].val = val;
                return true;
            }
            else if (v[midx].key < key)
                startx = midx + 1;
            else
            {
                if((int)(midx - 1) < 0)
                    return false;
                endx = midx - 1;
            }
                
        }
        return false;
    }  
    
}

template<class key_t, class val_t>
bool Node<key_t, val_t>::insert(key_t key, val_t val, int &lookup_time, int &move_time)
{
    uint64_t t1 = perf_counter();
	int insert_pos = upper_less(key);
    if(insert_pos == -1)
        return false;
    
    if(!data[insert_pos].val.has_leaf()){
        uint64_t t2 = perf_counter();
	    lookup_time = (t2 - t1) * 1e6 / 2904002;
         
        uint64_t t3 = perf_counter();

        data[insert_pos].val.leaf = new std::vector<record_t>;
        record_t rec = {key, val};
        data[insert_pos].val.leaf->push_back(rec);
        data[insert_pos].val.composite_flag = 1;
        
        uint64_t t4 = perf_counter();
        move_time = (t4 - t3) * 1e6 / 2904002;

        return true;
    }
    else{
        std::vector<record_t> &v =  *data[insert_pos].val.leaf;

        int startx = 0;
        int endx = v.size()-1;
        int posx = -1;
        while(startx <= endx)
        {
            int midx = (startx + endx) / 2;
            if(v[midx].key <= key){
                posx = midx;
                startx = midx + 1;
            }
            else{
                endx = midx - 1;
            }
        }


        uint64_t t5 = perf_counter();
        lookup_time = (t5 - t1) * 1e6 / 2904002;

        uint64_t t7 = perf_counter();

        record_t rec = {key, val};
        data[insert_pos].val.leaf->insert(data[insert_pos].val.leaf->begin() + posx + 1, rec);

        uint64_t t8 = perf_counter();
        move_time = (t8 - t7) * 1e6 / 2904002;
        return true;
    }
    return false;
}


template<class key_t, class val_t>
bool Node<key_t, val_t>::insert(key_t key, val_t val)
{
	int insert_pos = upper_less(key);
    if(insert_pos == -1)
        return false;
    
    if(!data[insert_pos].val.has_leaf()){
        data[insert_pos].val.leaf = new std::vector<record_t>;
        record_t rec = {key, val};
        data[insert_pos].val.leaf->push_back(rec);
        data[insert_pos].val.composite_flag = 1;
        return true;
    }
    else{
        std::vector<record_t> &v =  *data[insert_pos].val.leaf;
        int startx = 0;
        int endx = v.size()-1;
        int posx = -1;
        while(startx <= endx)
        {
            int midx = (startx + endx) / 2;
            if(v[midx].key <= key){
                posx = midx;
                startx = midx + 1;
            }
            else{
                endx = midx - 1;
            }
        }
        record_t rec = {key, val};
        data[insert_pos].val.leaf->insert(data[insert_pos].val.leaf->begin() + posx + 1, rec);
        return true;
    }
    return false;
}



template<class key_t, class val_t>
bool Node<key_t, val_t>::check()
{
    key_t next = -1;
    for(size_t i = 0; i < data.size(); i++)
    {
        key_t prev = next;
        key_t next = data[i].key;
        if(prev >= next)
            return false;
        if(data[i].val.has_leaf())
        {
            for(size_t j = 0; j < data[i].val.leaf->size(); j++)
            {
                key_t prev = next;
                key_t next = (*data[i].val.leaf)[j].key;
                if(prev >= next)
                    return false;
            }
        }  
    }
    return true;
}

template<class key_t, class val_t>
size_t Node<key_t, val_t>::count_leaf()
{
    size_t count = 0;
    for(size_t i = 0; i < data.size(); i++)
    {
        if(data[i].val.has_leaf())
            count++;
        if(i == 71)
        {
            std::cout << "check71: " << data[i].val.has_leaf() << std::endl;
        }
    }
        
    return count;
}


template<class key_t, class val_t>
void Node<key_t, val_t>::train()
{
    std::vector<key_t> keys;
    for(size_t i = 0; i < data.size(); i++)
    {
        keys.push_back(data[i].key);
    }
    lr_model.train(keys);
}

template<class key_t, class val_t>
void Node<key_t, val_t>::normal_retrain()
{
    double x_expected = 0, y_expected = 0, xy_expected = 0,
        x_square_expected = 0;
    int position = -1;
    for(size_t i = 0; i < data.size(); i++)
    {
        double key = data[i].key;
        position++;
        x_expected += key;
        y_expected += position;
        x_square_expected += key * key;
        xy_expected += key * position;

        if(data[i].val.has_leaf())
        {
            for(size_t j = 0; j < data[i].val.leaf->size(); j++)
            {
                double key = (*data[i].val.leaf)[j].key;
                position++;
                x_expected += key;
                y_expected += position;
                x_square_expected += key * key;
                xy_expected += key * position;
            }
        }  
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
void Node<key_t, val_t>::pipeline_retrain(size_t distance, int way)
{
    double x_expected = 0, y_expected = 0, xy_expected = 0,
        x_square_expected = 0;
    int position = -1;
    for(size_t i = 0; i < data.size(); i++)
    {   
        //方式一，固定距离取指针
        if(way == 1)
        {
            if(i + distance < data.size() &&  data[i + distance].val.has_leaf())
                _mm_prefetch((char *)data[i + distance].val.leaf, _MM_HINT_T0);
        }
        
        //方式二，只有有叶子的才预取
        else if(way == 2)
        {
            if(data[i].val.has_leaf())
                 if(i + distance < data.size() && data[i + distance].val.has_leaf())
                     _mm_prefetch((char *)data[i + distance].val.leaf, _MM_HINT_T0);
        }

        //方式三，只有有叶子的一定要预取到
        else{
            if(data[i].val.has_leaf())
            {
                int flag = 0;
                for(size_t j = i + 1; j< distance + 2 && j < data.size(); j++)
                {
                    if(data[j].val.has_leaf() && !data[j].val.has_prefetched()){
                        _mm_prefetch((char *)data[j].val.leaf, _MM_HINT_T0);
                        data[j].val.prefetched();
                        flag = 1;
                        break;
                    }
                }
                if(!flag)
                    if(i + distance < data.size())
                        _mm_prefetch((char *)data[i + distance].val.leaf, _MM_HINT_T0);
            }
        }

        //方式二，只有有叶子的才预取
        // if(data[i].val.has_leaf())
        //     if(data[i + distance].val.has_leaf())
        //         _mm_prefetch((char *)data[i + distance].val.leaf, _MM_HINT_T0);

        //方式三，只有有叶子的一定要预取到
        // if(data[i].val.has_leaf())
        // {
        //     int flag = 0;
        //     for(size_t j = i + 1; j< distance + 2; j++)
        //     {
        //         if(data[j].val.has_leaf() && !data[j].val.has_prefetched()){
        //             _mm_prefetch((char *)data[j].val.leaf, _MM_HINT_T0);
        //             data[j].val.prefetched();
        //             flag = 1;
        //             break;
        //         }
        //     }
        //     if(!flag)
        //         _mm_prefetch((char *)data[i + distance].val.leaf, _MM_HINT_T0);
        // }
        

        double key = data[i].key;
        position++;
        x_expected += key;
        y_expected += position;
        x_square_expected += key * key;
        xy_expected += key * position;

        if(data[i].val.has_leaf())
        {
            for(size_t j = 0; j < data[i].val.leaf->size(); j++)
            {
                double key = (*data[i].val.leaf)[j].key;
                position++;
                x_expected += key;
                y_expected += position;
                x_square_expected += key * key;
                xy_expected += key * position;
            }
        }  
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
}

#endif

