#pragma once
#ifndef __FUNCTION_H__
#define __FUNCTION_H__
#include <iostream>
#include <cstdint>
#include <x86intrin.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <fstream>
#include <emmintrin.h>


__inline__ uint64_t perf_counter(void)
{
    unsigned int a;
    __asm__ __volatile__("" : : : "memory");
    uint64_t r =  __rdtscp(&a);
    __asm__ __volatile__("" : : : "memory");

    return r;
}

void clean_buffer()
{
    int fd = open("/proc/wbinvd", O_RDONLY);
	char buf[1];
	read(fd, buf, 1);
	close(fd);
}

static std::vector<uint64_t> memory(26214400 / 8 * 2);
static uint64_t random_sum;

static void wipe_cache(){
  // Make sure that all cache lines from large buffer are loaded
  for (uint64_t& iter : memory) {
    random_sum += iter;
  }
  _mm_mfence();
}

#endif