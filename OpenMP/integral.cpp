// Int(4 / (1 + x^2)), x in [0, 1]. This integral equals to Pi.

#include "time_measurer.hpp"
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <omp.h>

using namespace std;
using namespace std::chrono;



constexpr uint64_t num_steps = 100'000'000;
constexpr double step = 1.0 / (double)num_steps;
constexpr uint32_t num_threads = 4;			// Requested number of threads



double integral_1()
{
	double x, sum = 0.0;

	for ( uint64_t i = 0; i < num_steps; ++i )
	{
		x = (i + 0.5) * step;				// In the middle of the rectangle
		sum += 4.0 / (1.0 + x*x);
	}

	return (step * sum);
}



double integral_2()
{
	double sum[num_threads];
	uint32_t nthreads;

	omp_set_num_threads(num_threads);
	#pragma omp parallel
	{
		double x;
		const uint32_t nthrds = omp_get_num_threads();
		const uint32_t id = omp_get_thread_num();
		if ( id == 0 )
			nthreads = nthrds;				// Actual number of threads
		sum[id] = 0;
		for ( uint64_t i = id; i < num_steps; i += nthrds )
		{
			x = (i + 0.5) * step;			// In the middle of the rectangle
			sum[id] += 4.0 / (1.0 + x*x);
		}
	}

	double pi = 0.0;
	for ( uint32_t i = 0; i < nthreads; ++i )
		pi += sum[i] * step;

	return pi;
}



double integral_3()
{
	constexpr uint32_t pad = 8;				// For 64 bytes L1 cache line
	double sum[num_threads][pad];
	uint32_t nthreads;

	omp_set_num_threads(num_threads);
	#pragma omp parallel
	{
		double x;
		const uint32_t nthrds = omp_get_num_threads();
		const uint32_t id = omp_get_thread_num();
		if ( id == 0 )
			nthreads = nthrds;				// Actual number of threads
		sum[id][0] = 0;
		for ( uint64_t i = id; i < num_steps; i += nthrds )
		{
			x = (i + 0.5) * step;			// In the middle of the rectangle
			sum[id][0] += 4.0 / (1.0 + x*x);
		}
	}

	double pi = 0.0;
	for ( uint32_t i = 0; i < nthreads; ++i )
		pi += sum[i][0] * step;

	return pi;
}



double integral_4()
{
	double pi = 0.0;

	omp_set_num_threads(num_threads);
	#pragma omp parallel
	{
		const uint32_t nthrds = omp_get_num_threads();
		const uint32_t id = omp_get_thread_num();
		double x, sum = 0.0;
		for ( uint64_t i = id; i < num_steps; i += nthrds )
		{
			x = (i + 0.5) * step;			// In the middle of the rectangle
			sum += 4.0 / (1.0 + x*x);
		}
		#pragma omp critical
			pi += sum * step;
	}
	return pi;
}



double integral_5()
{
	double pi = 0.0;

	omp_set_num_threads(num_threads);
	#pragma omp parallel
	{
		const uint32_t nthrds = omp_get_num_threads();
		const uint32_t id = omp_get_thread_num();
		double x, sum = 0.0;
		for ( uint64_t i = id; i < num_steps; i += nthrds )
		{
			x = (i + 0.5) * step;			// In the middle of the rectangle
			sum += 4.0 / (1.0 + x*x);
		}
		sum *= step;
		#pragma omp atomic
			pi += sum;
	}
	return pi;
}



double integral_6()			// Best version
{
	double sum = 0.0;

	#pragma omp parallel for reduction(+ : sum) schedule(static)
		for ( uint64_t i = 0; i < num_steps; ++i )
		{
			const double x = (i + 0.5) * step;		// In the middle of the rectangle
			sum += 4.0 / (1.0 + x*x);
		}

	return (step * sum);
}



int main()
{
	double d1, d2, d3, d4, d5, d6;

	START_TIMER("integral_1: ")
		d1 = integral_1();
	STOP_TIMER
	cout << "Intergal_1 is " << d1 << "\n";

	START_TIMER("integral_2: ")
		d2 = integral_2();
	STOP_TIMER
	cout << "Intergal_2 is " << d2 << "\n";

	START_TIMER("integral_3: ")
		d3 = integral_3();
	STOP_TIMER
	cout << "Intergal_1 is " << d3 << "\n";

	START_TIMER("integral_4: ")
		d4 = integral_4();
	STOP_TIMER
	cout << "Intergal_4 is " << d4 << "\n";

	START_TIMER("integral_5: ")
		d5 = integral_5();
	STOP_TIMER
	cout << "Intergal_5 is " << d5 << "\n";

	START_TIMER("integral_6: ")
		d6 = integral_6();
	STOP_TIMER
	cout << "Intergal_6 is " << d6 << "\n";
}

// g++ -std=c++20 -Wall -Wextra -fopenmp integral.cpp
