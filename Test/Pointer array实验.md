## Pointer array实验

### 1. Dataset

200M 股票数据集，每个记录有8 byte integer key与 8 byte integer value.

### 2. Competitors

- Pointer_array:  每个节点是一个指向一个record的指针。每个节点4 byte
- Data_array: 每个节点是一个record。每个节点24 byte（8B key + 8B value + 8B flag）
- leaf: 之前的方法，每个节点是一个record，同时有一个leaf，存储插入的数据。每个节点28byte

理论上，pointer_array每个节点内存占用小，move的延时小，理论上insert性能优越。且对于pointer array的重训练是一个内存密集型任务，可以使用prefetch方法

### 3. Experiments

#### 3.1 基于Pointer array的prefetch retrain

第一行表示数据量（number）

|                                | 1M     | 10M  | 100M  |
| ------------------------------ | ------ | ---- | ----- |
| pointer_array normal retrain   | 2124us | 21ms | 207ms |
| pointer_array prefetch retrain | 1676us | 17ms | 176ms |
| data_array normal retrain      | 1479us | 15ms | 154ms |

**实验结论**：pointer_array 的prefetch确实有效果，但是整体效果不如data_array

**实验分析**：pointer_array的retrain是内存访问较为频繁的，此时prefetch可以起到效果。但是归根到底不如不用					指针的data_array效果好，因为data_array是顺序访问，内存访问不构成瓶颈。

#### 3.2 基于Pointer array的insert

insert包括search与move两个部分，分别进行测量

##### 3.2.1 10000 build keys + 5000 insert keys

|               | insert中的search | insert中的move | insert总时间 |
| ------------- | ---------------- | -------------- | ------------ |
| pointer_array | 366ns            | 597ns          | 964ns        |
| data_array    | 246ns            | 2118ns         | 2364ns       |

##### 3.2.2 100000 build keys + 50000 insert keys

|               | insert中的search | insert中的move | insert总时间 |
| ------------- | ---------------- | -------------- | ------------ |
| pointer_array | 2175ns           | 10501ns        | 12676ns      |
| data_array    | 1135ns           | 31005ns        | 32143ns      |

**实验结论**：insert中move的开销占比较大，pointer_array对move的性能有较好的提升，insert延时小

**实验分析**：pointer_array是对4 byte的指针进行move，而data array是对24 byte的数据进行move，move受数				据的字节数影响大，pointer_array的move效果显著好于data_array的move。又move是insert中开销占				比较大的操作，因此pointer_array的insert效果显著好于data_array	                      

#### 3.3 pointer_array 与 leaf两种方法实验对比

数据量为10M，对不同的插入比例和插入模式进行测试

##### 3.3.1 insert latency

| insert_ratio  | 0.25  | 0.5   | skewd  |
| ------------- | ----- | ----- | ------ |
| pointer_array | 572ns | 679ns | 30us   |
| leaf          | 418ns | 564ns | 1217ns |

##### 3.3.2 retrain latency

| insert_ratio           | 0.25 | 0.5   | skewd |
| ---------------------- | ---- | ----- | ----- |
| pointer_array          | 57ms | 147ms | 372ms |
| pointer_array_prefetch | 49ms | 108ms | 197ms |
| leaf                   | 91ms | 333ms | 898ms |

**实验结论**：pointer_array的insert效果不如leaf方法，retrain比leaf好，且prefetch有效

**实验分析**：pointer_array中的insert涉及到整个数组大量的move操作，开销较大；而leaf的insert中的move是在					局部的叶子数组内部的move，开销较小。另一方面，pointer_array每个节点是4byte，且是指针，所					以retrain过程的顺序访问cache miss次数少，prefetch也有效。而leaf方法每个节点是28byte，顺序					访问过程cache miss次数多，所以retrain不如pointer_array。且规律性显著，编译器会进行自动的预					测优化，prefetch效果不大。
