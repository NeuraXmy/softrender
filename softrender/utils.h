#ifndef UTILS_H
#define UTILS_H

#include <chrono>

inline static double get_time()
{
	return std::chrono::high_resolution_clock::now().time_since_epoch().count() * 1e-9;
}

#endif
