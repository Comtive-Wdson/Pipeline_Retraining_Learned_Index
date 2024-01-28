#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <fcntl.h>
#include <linux/perf_event.h>
#include "pindex.h"
#include "pindex_impl.h"


int main(void)
{
	std::ifstream infile1;
	std::ifstream infile2;
	infile1.open("./data/0.250000_10000_build.txt", std::ios::in);
	if (!infile1.is_open())
	{
		std::cout << "open file failed" << std::endl;
		return 0;
	}
	infile2.open("./data/0.250000_10000_insert.txt", std::ios::in);
	if (!infile2.is_open())
	{
		std::cout << "open file failed" << std::endl;
		return 0;
	}
	std::string buf;
	std::vector<long long> build_keys;
	std::vector<long long> insert_keys;
	int i = 0;
	while (getline(infile1, buf))
	{
		long long x = std::stoll(buf);
		build_keys.push_back(x);
	}
	std::cout << build_keys.size() << "  build keys finish" << std::endl;
	std::vector<long long> build_values(build_keys.size(), 1);
	while (getline(infile2, buf))
	{
		long long x = std::stoll(buf);
		insert_keys.push_back(x);
	}
	std::cout << insert_keys.size() << "  insert keys finish" << std::endl;
	infile1.close();
	infile2.close();

	pindex<long long, long long> p_index;
	p_index.build(build_keys, build_values, 1000);
	std::cout << "build finish" << std::endl;

	
	//insert
	//p_index.insert(565202001020931,1);
	for(size_t i = 0; i < insert_keys.size(); i++)
	{
		if(p_index.insert(insert_keys[i], 1) == false)
			std::cout << "insert fail!" << std::endl;
	}


	// //check all lookup
	// for(size_t i = 0; i < build_keys.size(); i++)
	// {
	// 	if(p_index.get(build_keys[i]) == false)
	// 		std::cout << "get build key fail!" << std::endl;
	// }
	// for(size_t i = 0; i < 1000; i++)
	// {
	// 	if(p_index.get(insert_keys[i]) == false)
	// 	{
	// 		std::cout << "pos: " << i << "key: " << insert_keys[i] << std::endl;
	// 		std::cout << "get insert key fail!" << std::endl;
	// 	}
	// }
	// std::cout << "all lookup check right!" << std::endl;


	// //check sorting
	// std::cout << "check result: " << p_index.check() << std::endl;



	//retrain time
	uint64_t t1 = perf_counter();
	p_index.normal_retrain();
	uint64_t t2 = perf_counter();
	long time1 = (t2 - t1) / 2904002;
	std::cout << "normal_train time: " << time1  << " ms" << std::endl << std::endl;

	for(int way = 1; way <= 3; way++)
	{
		for(size_t distance = 12; distance < 50; distance+=5)
		{
			uint64_t t3 = perf_counter();
			p_index.pipeline_retrain(distance, way);
			uint64_t t4 = perf_counter();
			long time2 = (t2 - t1) / 2904002;
			std::cout << "way for " << way << ". with distance " << distance << " pipeline retrain time: " << time2 << " ms" <<std::endl; 
		}
		std::cout << std::endl << std::endl;
	}



	// LinearRegressionModel<long long> lrmodel;
	
	// uint64_t t1 = perf_counter();
	// lrmodel.train(keys);
	// uint64_t t2 = perf_counter();
	// long time = (t2 - t1) * 1e6 / 2904002;
	// std::cout << "train time: " << time / 1e6 << "ms" << std::endl;

	// for(size_t i = 0; i < 20; i++)
	// {
	// 	uint64_t t1 = perf_counter();
	// 	volatile long long k = keys[i];
	// 	uint64_t t2 = perf_counter();
	// 	//std::cout << keys[i] << std::endl;
	// 	std::cout << t2 - t1 << std::endl;
	// 	time1 += t2 - t1;
	// }

	// std::cout << "------------" << std::endl;
	// for(size_t i = 0; i < 20; i++)
	// {
	// 	clean_buffer();
	// 	uint64_t t1 = perf_counter();
	// 	volatile long long k = keys[i];
	// 	uint64_t t2 = perf_counter();
	// 	//std::cout << keys[i] << std::endl;
	// 	std::cout << t2 - t1 << std::endl;
	// 	time2 += t2 - t1;
	// }
	//  std::cout << time1 / 20 << std::endl;
	// std::cout << time2 / 20 << std::endl;


	// long long count1 = 0;
	// long long count2 = 0;
	// for(size_t i = 0; i < keys.size(); i++)
	// {
	// 	uint64_t t1 = perf_counter();
	// 	lrmodel.search(keys, keys[i]);
	// 	uint64_t t2 = perf_counter();
	// 	uint64_t t3 = perf_counter();
	// 	semodel.search(keys[i]);
	// 	uint64_t t4 = perf_counter();
	// 	if(t2 - t1 < t4 - t3)
	// 	{
	// 		std::cout << i << std::endl;
	// 		count1++;
	// 	}
	// 	else
	// 		count2++;
	// 	if(count1 > 10)
	// 	break;
	// }
	// std::cout << count1 << "   " << count2 << std::endl;

	// lrmodel.search(keys, keys[0]);
	// std::cout << "_________________-" << std::endl;
	// semodel.search(keys[0]);
	// std::cout << "Learned index" << std::endl;
	// getchar();
	// uint64_t a = 0, b = 0;
    // for(size_t i= 3; i < 7003; i+= 1000)
	// {
	// 	// std::cout << "Learned index" << std::endl;
	//  	// getchar();
	// 	//clean_buffer();
	// 	//size_t pos = lrmodel.predict(keys[i]);
	// 	uint64_t k = lrmodel.search(keys, keys[i]);
	// 	std::cout << std::endl << std::endl;
	// 	//long long k = semodel.search(keys[i]);
	// 	//std::cout << "k1:   " << k << std::endl;
	// 	//a += k;
	// }
    
	// // // std::cout << "Semantics Learned index" << std::endl;
	// // // getchar();
	
	// 	for(size_t i = 3; i < 1e8+3; i+= 1e6)
	// {
	// 	//clean_buffer();
	// 	//int k = 1;
	// 	//long long semantics1 = semodel.extract_semantics_1(keys[i]);
	// 	//volatile long long semantics2 = semodel.extract_semantics_2(keys[i]);
	// 	uint64_t k = semodel.search(keys[i]);
	// 	//std::cout << "k2:   " << k << std::endl;
	// 	b += k;
	// }
	// std::cout << a / 100 << std::endl;
	// std::cout << b / 100 << std::endl;
	//lrmodel.test(keys);
	//semodel.test(keys);
    
	// std::cout << "Semantics 3 Learned index" << std::endl;
	// getchar();
	// for(size_t i = 0; i < keys.size(); i++)
	// {
	// 	long long k = semodel_top.search(keys[i]);
	// }
 	
    //std::cout << t4 - t3 << " counts" << std::endl;

	// long long k = keys[14];
	// uint64_t t3 = perf_counter();
	// long long key2 = lrmodel.search(keys, k);
	// uint64_t t4 = perf_counter();
	// std::cout << t4 - t3 << " counts" << std::endl;
	// std::cout << std::endl;

	// uint64_t t1 = perf_counter();
    // long long key = semodel.search(k);
    // uint64_t t2 = perf_counter();
    // std::cout << t2 - t1 << " counts" << std::endl;

	

	// std::cout << "Traditional Learned Index:" << std::endl;
	// std::cout << "Hit_ratio:   " << lrmodel.hit_ratio_calculate(keys) << std::endl;
	// std::cout << "Max_error:   " << lrmodel.get_maxErr() << std::endl;
	// std::cout << "Mean_error:   " << lrmodel.error_calculate(keys) << std::endl;
	// std::cout << "Train_time:   " << lrmodel.train_time_calculate(keys) << "ms" << std::endl;
	// std::cout << "Lookup_time:   " << lrmodel.search_time_calculate(keys) << "ns (per key)" << std::endl;
	// std::cout << "Random_lookup_time:   " << lrmodel.random_search_time_calculate(keys) << "ns (per key)" << std::endl;
	// std::cout << "Throughput:   " << lrmodel.throughput(keys) << " (10^6 ops per second)" << std::endl;
	// std::cout << "Random_throughput:   " << lrmodel.random_throughput(keys) << " (10^6 ops per second)" << std::endl;


	// std::cout << std::endl << std::endl << std::endl << std::endl;
	// std::cout << "RMI Learned Index:" << std::endl;
	// std::cout << "Hit_ratio:   " << rmimodel.key_hit_ratio_calculate(keys) << std::endl;
	// std::cout << "Max_error:   " << rmimodel.max_error_calculate(keys) << std::endl;
	// std::cout << "Mean_error:   " << rmimodel.key_error_calculate(keys) << std::endl;
	// std::cout << "Train_time:   " << rmimodel.train_time_calculate(keys) << "ms" << std::endl;
	// std::cout << "Lookup_time:   " << rmimodel.search_time_calculate(keys) << "ns (per key)" << std::endl;
	// std::cout << "Random_lookup_time:   " << rmimodel.random_search_time_calculate(keys) << "ns (per key)" << std::endl;
	// std::cout << "Throughput:   " << rmimodel.throughput(keys) << " (10^6 ops per second)" << std::endl;
	// std::cout << "Random_throughput:   " << rmimodel.random_throughput(keys) << " (10^6 ops per second)" << std::endl;


	// std::cout << std::endl << std::endl << std::endl << std::endl;
	// //std::cout << "Semantic Learned Index:" << std::endl;
	// //std::cout << "Hit_ratio:   " << semodel.hit_ratio_calculate(keys) << std::endl;
	// //std::cout << "Max_error:   " << semodel.get_maxErr() << std::endl;
	// //std::cout << "Mean_error:   " << semodel.error_calculate(keys) << std::endl;
	// //std::cout << "Train_time:   " << semodel.train_time_calculate(keys) << std::endl;
	// //std::cout << "Search_time:   " << semodel.search_time_calculate(keys) << std::endl;

	// std::cout << "Semantic(2 stages) Learned Index:" << std::endl;
	// std::cout << "Class_Hit_ratio:   " << semodel.class_hit_ratio_calculate(keys) << std::endl;
	// std::cout << "Key_Hit_ratio:   " << semodel.key_hit_ratio_calculate(keys) << std::endl;
	// std::cout << "Max_error:   " << semodel.max_error_calculate() << std::endl;
	// std::cout << "Class_Mean_error:   " << semodel.class_error_calculate(keys) << std::endl;
	// std::cout << "Key_Mean_error:   " << semodel.key_error_calculate(keys) << std::endl;
	// std::cout << "Train_time:   " << semodel.train_time_calculate(keys) << "ms" << std::endl;
	// std::cout << "Lookup_time:   " << semodel.search_time_calculate(keys) << "ns (per key)" << std::endl;
	// std::cout << "Random_lookup_time:   " << semodel.random_search_time_calculate(keys) << "ns (per key)" << std::endl;
	// std::cout << "Throughput:   " << semodel.throughput(keys) << " (10^6 ops per second)" << std::endl;
	// std::cout << "Random_throughput:   " << semodel.random_throughput(keys) << " (10^6 ops per second)" << std::endl;


	// std::cout << std::endl << std::endl << std::endl << std::endl;
	// std::cout << "Semantic(3 stages) Learned Index:" << std::endl;
	// std::cout << "Key_Hit_ratio:   " << semodel_top.key_hit_ratio_calculate(keys) << std::endl;
	// std::cout << "Max_error:   " << semodel_top.max_error_calculate() << std::endl;
	// std::cout << "Key_Mean_error:   " << semodel_top.key_error_calculate(keys) << std::endl;
	// std::cout << "Train_time:   " << semodel_top.train_time_calculate(keys) << "ms" << std::endl;
	// std::cout << "Lookup_time:   " << semodel_top.search_time_calculate(keys) << "ns (per key)" << std::endl;
	// std::cout << "Random_lookup_time:   " << semodel_top.random_search_time_calculate(keys) << "ns (per key)" << std::endl;
	// std::cout << "Throughput:   " << semodel_top.throughput(keys) << " (10^6 ops per second)" << std::endl;
	// std::cout << "Random_throughput:   " << semodel_top.random_throughput(keys) << " (10^6 ops per second)" << std::endl;

	return 0;
}
