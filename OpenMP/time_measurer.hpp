#pragma once

#include <chrono>
#include <iostream>



class TimeMeasurer
{
public:
	TimeMeasurer(const char* mes) : mes_ {mes}
	{
		t1_ = std::chrono::steady_clock::now();
	}

	~TimeMeasurer()
	{
		auto t2_ = std::chrono::steady_clock::now() - t1_;
		std::cout << mes_
			<< std::chrono::duration_cast<std::chrono::milliseconds>(t2_).count()
			<< " ms\n";
	}

private:
	const char* mes_;
	std::chrono::time_point<std::chrono::steady_clock> t1_;
};



#define START_TIMER(mes) { TimeMeasurer tm(mes);
#define STOP_TIMER  }
