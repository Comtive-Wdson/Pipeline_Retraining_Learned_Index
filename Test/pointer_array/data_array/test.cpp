#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <fcntl.h>
#include <linux/perf_event.h>
#include <random>
#include <algorithm>
#include "dindex.h"
#include "dindex_impl.h"


int main(void)
{
	std::ifstream infile1;
	std::ifstream infile2;
	infile1.open("../../data/0.950000_1_build.txt", std::ios::in);
	if (!infile1.is_open())
	{
		std::cout << "open file failed" << std::endl;
		return 0;
	}
	infile2.open("../../data/0.950000_1_insert.txt", std::ios::in);
	if (!infile2.is_open())
	{
		std::cout << "open file failed" << std::endl;
		return 0;
	}
	std::string buf;
	std::vector<long long> build_keys;
	std::vector<long long> insert_keys;
	int l = 0;
	while (getline(infile1, buf) && l < 1e5)
	{   
        l++;
		long long x = std::stoll(buf);
		build_keys.push_back(x);
	}
	std::cout << build_keys.size() << "  build keys finish" << std::endl;
	std::vector<long long> build_values(build_keys.size(), 1);
	int l2 = 0;
	while (getline(infile2, buf) && l2 < 50000)
	{
		long long x = std::stoll(buf);
		insert_keys.push_back(x);
		l2++;
	}
	std::cout << insert_keys.size() << "  insert keys finish" << std::endl;
	infile1.close();
	infile2.close();

	dindex<long long, long long> d_index;
	d_index.dbuild(build_keys, build_values, 1);
	std::cout << "build finish" << std::endl;


    // //lookup bottleneck test
    // std::random_device rd1;
    // std::mt19937 gen1(rd1());
    // std::uniform_int_distribution<int> distribution1(0, l-2);
    // long time_b = 0;
    // for(int i = 0; i < l/2; i++)
    // {
    //     int index = distribution1(gen1);
    //     uint64_t tb1 = perf_counter();
    //     d_index.dget(build_keys[index]);
    //     uint64_t tb2 = perf_counter();
    //     time_b += (tb2 - tb1) * 1e6 / 2904002;
    // }
    // std::cout << "tiem: " << time_b / (l/2) << std::endl;

    
    // //shuffle insert_keys
	// std::random_device rd;
	// std::mt19937 g(rd());
	// std::shuffle(insert_keys.begin(), insert_keys.end(), g);
	
	// //insert
    // d_index.dinsert_time(insert_keys);


	// //check all lookup
	// for(size_t i = 0; i < build_keys.size(); i++)
	// {
	// 	if(d_index.dget(build_keys[i]) == false)
	// 		std::cout << "get build key fail!" << std::endl;
	// }
	// for(size_t i = 0; i < insert_keys.size(); i++)
	// {
	// 	if(d_index.dget(insert_keys[i]) == false)
	// 	{
	// 		//std::cout << "pos: " << i << "key: " << insert_keys[i] << std::endl;
	// 		std::cout << "get insert key fail!" << std::endl;
	// 	}
	// }
	// std::cout << "all lookup check right!" << std::endl;


	// // //check sorting
	// // std::cout << "check result: " << p_index.check() << std::endl;



	// std::cout << "start ipc test. input 1" << std::endl;
	// getchar();


	//retrain time
	wipe_cache();
	uint64_t t1 = perf_counter();
	d_index.dnormal_retrain();
	uint64_t t2 = perf_counter();
	long time1 = (t2 - t1) * 1e6 / 2904002;
	std::cout << "normal_train time: " << time1  << " ms" << std::endl << std::endl;

	// // wipe_cache();
	// // for(size_t distance = 1; distance < 15; distance+=1)
	// // {	
	// // 	// std::cout << "start ipc test. input 1";
	// // 	// getchar();
	// // 	uint64_t t3 = perf_counter();
	// // 	p_index.pipeline_retrain(distance);
	// // 	uint64_t t4 = perf_counter();
	// // 	long time2 = (t4 - t3) / 2904002;
	// // 	std::cout << "with distance " << distance << " pipeline retrain time: " << time2 << " ms" <<std::endl; 
	// // }
	// // std::cout << std::endl << std::endl;




	return 0;
}
