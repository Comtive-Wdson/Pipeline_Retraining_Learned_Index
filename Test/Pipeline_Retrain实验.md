## Pipeline_Retrain实验

### 1. 方法概述

#### 1.1 数据结构

​	每个结点由一个record_t数组和一个linear_model组成。每个record_t包含key, value, composite_flag和leaf指针。leaf指针构成一个leaf，储存插入在该record_t后面的数据

```C++
class Node{
    struct record_t{
        struct AtomicVal{
            val_t val;
            std::vector<record_t> *leaf;
            long long composite_flag;
        };
        key_t key;
        struct AtomicVal val;
    }
    
    std::vector<record_t> data;
    LinearRegressionModel<key_t> lr_model;
}
```

#### 1.2 prefetch pipeline

​	训练的过程在于遍历每个数据以及每个数据的叶子数据。对于单一的数据结点来说，包括两个步骤：一个是计算该数据，另一个是对leaf pointer进行dereference，顺序计算叶子内的数据。

​	分析可知，第一个步骤的计算任务极其简单，而第二个步骤是顺序访问，内存开销较低。核心需要的内存访问瓶颈在于对leaf pointer dereference。因此prefetch的主要内容在于预取后续数据节点的叶子结点。

​	利用prefetch方法构成二阶段流水线，有以下三种方法。

##### 1.2.1 方法一：固定距离预取叶子

```C++
for(size_t i = 0; i < data.size(); i++)
{
    if(i + distance < data.size() && data[i+distance].val.has_leaf())
        _mm_prefetch((char *)data[i+distance].val.leaf, _MM_HINT_T0);
    //计算该数据
    //若有叶子，计算该数据的叶子内数据
}
```

##### 1.2.2 方法二：只有有叶子才尝试预取

​	这个方法的出发点在于，单一数据节点的计算任务很小，只有有叶子的数据节点，其计算任务才密集。这个时候分配预取指令才更有可能做到计算与预取的overlap。

```C++
for(size_t i = 0; i < data.size(); i++)
{
    if(data[i].val.has_leaf())
    	if(i + distance < data.size() && data[i+distance].val.has_leaf())
        	_mm_prefetch((char *)data[i+distance].val.leaf, _MM_HINT_T0);
    //计算该数据
    //若有叶子，计算该数据的叶子内数据
}
```

##### 1.2.3 方法三：只有有叶子的才尝试预取，且尽量预取到

​	这个方法的出发点在于，方法二努力让计算密集的任务与prefetch进行overlap，但是无法保证一定能做到overlap。因为固定距离外的数据节点不一定有叶子，因此不一定能预取成功。该方法尽量保证有叶子的数据节点执行预取且尽量预取到。

```C++
for(size_t i = 0; i < data.size(); i++)
{
    if(data[i].val.has_leaf())
            {
                int flag = 0;
                for(size_t j = i + 1; j< distance + step && j < data.size(); j++)
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
    //计算该数据
    //若有叶子，计算该数据的叶子内数据
}
```

### 2. Experiment

#### 2.1 Dataset and Machine

##### 2.1.1 Dataset

200M 股票数据集，每个记录有8 byte integer key与 8 byte integer value.

同时设置两个参数

p: 表示每个数据后有插入数据的概率。表示一个数据节点有叶子结点的概率

m: 表示每个数据后最大的插入数据的个数。表示一个数据节点的叶子结点的长度。

写脚本，通过输入p与m，划分200M股票数据集，分别生成初始化数据集与插入数据集。 

##### 2.2.2 Machine

| **CPU主频** | 2.9GHz |
| ----------- | ------ |
| L1d cache   | 256KB  |
| L2 cache    | 2MB    |
| L3 cache    | 16MB   |

#### 2.2 Tuning parameters

可调的参数主要包括一下四个参数。

**way**:  prefetch的三种方式，如1.2中所述。

**distance**:  prefetch的距离，即预取距离多远的数据。

**p, m**: insert pattern，衡量了叶子结点的个数与长度，如2.1.1中所述。

**组数**：分成1000组，每组原始数据加插入数据共200000个数据。

#### 2.3 Experiment results

##### 2.3.1 insert pattern实验

改变m与p，其中way与distance选取使得pipeline_retrain延时最小的相关参数。

**理论分析**：应该是p越接近1，m越小时prefetch提升效果最佳，因为此时数据节点有叶子的比例较高，且叶子长度较短，因此叶子结点指针访问构成内存访问的主要方式，内存访问构成瓶颈。预取可以有效缓解。

###### 2.3.1.1 m = 2

| p                | 0.00     | 0.25  | 0.50  | 0.75  | 1.00  |
| ---------------- | -------- | ----- | ----- | ----- | ----- |
| normal retrain   | 内存不够 | 766ms | 768ms | 793ms | 609ms |
| pipeline retrain | 内存不够 | 758ms | 759ms | 757ms | 624ms |

###### 2.3.1.2 m = 500

| p                | 0.00     | 0.25  | 0.50  | 0.75  | 1.00  |
| ---------------- | -------- | ----- | ----- | ----- | ----- |
| normal retrain   | 内存不够 | 383ms | 383ms | 382ms | 381ms |
| pipeline retrain | 内存不够 | 377ms | 376ms | 374ms | 382ms |

###### 2.3.1.3 m = 2000

| p                | 0.00     | 0.25  | 0.50  | 0.75  | 1.00  |
| ---------------- | -------- | ----- | ----- | ----- | ----- |
| normal retrain   | 内存不够 | 332ms | 334ms | 334ms | 332ms |
| pipeline retrain | 内存不够 | 333ms | 336ms | 336ms | 333ms |

**实验结果**：实验结果表明，insert pattern 基本上p越高，m越小，prefetch效果越好，但不是p最高最好。

**实验结果分析**：基本上p越高越好，因为p越高代表叶子数目越多，叶子指针内存访问越多，prefetch能发挥作用						但是p到1时效果反而差。因为p=1代表每个数据节点都有叶子，这时访问模式固定，编译器会自						动进行硬件预取优化。m的值较小时，有较好的效果。因为m越小，代表叶子的长度越小，顺序访   						问的占比越小，prefetch可优化的空间越大。

##### 2.3.2 prefetch方法实验

###### 2.3.1 p=1.0, m=2

对理论上的最佳情况做prefetch参数调整实验

normal_retrain: 629ms

| way\distance | 1     | 3     | 5     | 7     | 9     | 11    | 13    | 17    | 19    | 21    |
| ------------ | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- |
| 1            | 642ms | 704ms | 741ms | 731ms | 727ms | 725ms | 726ms | 724ms | 725ms | 724ms |
| 2            | 636ms | 634ms | 703ms | 715ms | 718ms | 724ms | 725ms | 733ms | 739ms | 746ms |
| 3            | 648ms | 631ms | 624ms | 627ms | 633ms | 637ms | 644ms | 654ms | 662ms | 673ms |

###### 2.3.2 p=0.75, m=500

对实际上的整体最佳情况做prefetch参数调整实验

normal_retrain:382ms

| way\distance | 1     | 3     | 5     | 7     | 9     | 11    | 13    | 17    | 19    | 21    |
| ------------ | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- |
| 1            | 379ms | 377ms | 375ms | 375ms | 377ms | 379ms | 374ms | 376ms | 379ms | 374ms |
| 2            | 376ms | 378ms | 376ms | 376ms | 376ms | 374ms | 377ms | 379ms | 380ms | 375ms |
| 3            | 376ms | 378ms | 375ms | 374ms | 377ms | 377ms | 377ms | 377ms | 379ms | 375ms |

###### 2.3.3 p=0.75, m=2

对实际上的绝对最佳情况做prefetch参数调整实验

normal_retrain:793ms

| way\distance | 1     | 3     | 5     | 7     | 9     | 11    | 13    | 17    | 19    | 21    |
| ------------ | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- |
| 1            | 816ms | 812ms | 853ms | 899ms | 905ms | 899ms | 893ms | 889ms | 888ms | 885ms |
| 2            | 806ms | 824ms | 856ms | 866ms | 866ms | 868ms | 867ms | 867ms | 870ms | 876ms |
| 3            | 797ms | 789ms | 774ms | 766ms | 760ms | 757ms | 757ms | 760ms | 758ms | 761ms |

**实验结果**：三种方法中，第三种方法整体效果最好。distance是一个实验性质的参数，没有绝对定论。

**实验结果分析**：第三种方法确保了对于计算密集型的拥有叶子的数据节点，尽量的做预取操作；而没有叶子的结点						不是计算密集型结点，不需要进行预取操作，这样可以最大化overlap，效果最好。distance是一个						实验性质的参数，没有定论。distance太小，计算任务时间太短，无法覆盖prefetch时间；distance						太大，导致提前预取的会被逐出cache，再次访问时需要重新访问，prefetch丧失意义。方法三的实						验数据也支撑了这一分析。但是方法一和方法二的实验数据却与之不符*（疑问点）*

### 3. 深度探索

虽然实验结果表明，通过prefetch实现pipeline_retrain确实可以取得更好的效果，但是这个效果并不高。

对此进行更深入的研究分析。

理论上叶子指针访问内存占内存访问占比越高，内存瓶颈越高，prefetch效果越好。即p=1,m=2的访问模式

对不带leaf insert pattern的retrain，p=1，m=2 insert pattern的normal_retrain与prefetch_retrain程序分别使**用perf工具进行分析，计量平均IPC**

IPC测量结果如下

|                          | -O0  | -O3  |
| ------------------------ | ---- | ---- |
| 不带leaf的retrain        | 2.37 | 2.11 |
| 带leaf的normal_retrain   | 1.9  | 1.28 |
| 带leaf的prefetch_retrain | 1.88 | 1.20 |
| 跳跃访问数组             | 0.54 | 0.54 |

实验结果表明，这种带有leaf的结构相比没有leaf的纯顺序训练相比，IPC较低，内存访问开销占比更多。

但是即使是p=1, m=2内存开销最大的insert pattern，IPC依然高达1.2，大于1，属于计算密集型任务，而非内存密集型任务。

查看相关的论文，使用prefetch的典型场景包括hash table，binary search等，都是内存访问高度不可预测的场景，属于内存密集型任务。

所以，导致prefetch pipeline_retrain有效果但效果较小的根本原因在于该任务依然不是内存密集型任务。