#include "threadsafe_map.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <thread>

using namespace std;



void simpleTest()
{
	cout << "\n---------- " << __func__ << " ----------\n";
	ThreadsafeMap<string, int> tm;
	tm.addOrUpdate("five", 5);
	tm.addOrUpdate("ten", 10);
	tm.addOrUpdate("one", 1);
	tm.addOrUpdate("three", 3);
	cout << tm.getValue("one", 0) << ' '
		 << tm.getValue("three", 0) << ' '
		 << tm.getValue("five", 0) << ' '
		 << tm.getValue("ten", 0) << '\n';
	assert(tm.getValue("one", 0) == 1);
	assert(tm.getValue("two", 0) == 0);
	assert(tm.getValue("three", 0) == 3);
	assert(tm.getValue("five", 0) == 5);
	assert(tm.getValue("ten", 0) == 10);
	tm.remove("three");
	tm.remove("ten");
	cout << tm.getValue("one", 0) << ' '
		 << tm.getValue("three", 0) << ' '
		 << tm.getValue("five", 0) << ' '
		 << tm.getValue("ten", 0) << ' ';
	assert(tm.getValue("one", 0) == 1);
	assert(tm.getValue("two", 0) == 0);
	assert(tm.getValue("three", 0) == 0);
	assert(tm.getValue("five", 0) == 5);
	assert(tm.getValue("ten", 0) == 0);
}



const uint32_t SIZE = 1'000'000;
ThreadsafeMap<int, string> tmap(SIZE);

void Foo(uint32_t n)
{
	cout << "Function Foo\n";
	for ( uint32_t i = 0; i < n; ++i )
		tmap.addOrUpdate(i, "foo");
}

void Bar(uint32_t n1, uint32_t n2)
{
	cout << "Function Bar\n";
	vector<string> res;
	for ( uint32_t i = n1; i < n2; ++i )
		res.push_back(tmap.getValue(i, "bar"));
	cout << res.size() << '\n';
}

void Baz(uint32_t n1, uint32_t n2)
{
	cout << "Function Baz\n";
	for ( uint32_t i = n1; i < n2; ++i )
		tmap.remove(i);
}

void multithreadingTest()
{
	cout << "\n---------- " << __func__ << " ----------\n";
	using namespace std::chrono;

	Foo(SIZE);
	auto t = steady_clock::now();
	Bar(0, SIZE/2);
	Baz(SIZE/2, SIZE);
	auto dur = steady_clock::now() - t;
	cout << "1 thread: " << duration_cast<milliseconds>(dur).count() << " ms\n";

	Foo(SIZE);
	t = steady_clock::now();
	thread th1(Bar, 0, SIZE/2);
	thread th2(Baz, SIZE/2, SIZE);
	th1.join();
	th2.join();
	dur = steady_clock::now() - t;
	cout << "2 threads: " << duration_cast<milliseconds>(dur).count() << " ms\n";
}
