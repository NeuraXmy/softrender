#ifndef UTILS_H
#define UTILS_H

#include <chrono>

inline static double get_time()
{
	return std::chrono::high_resolution_clock::now().time_since_epoch().count() * 1e-9;
}

inline static int get_batch_size(int total, int batch_count, int i)
{
	return total / batch_count + (i < total % batch_count ? 1 : 0);
}

#endif
