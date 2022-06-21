#include "threadsafe_list.h"
#include <cassert>
#include <iostream>

using namespace std;



void someFunc(int& value)
{
	value *= 10;
	cout << value << ' ';
}

void testTreadsafeList()
{
	cout << "\n---------- " << __func__ << " ----------\n";
	TreadsafeList<int> tl;
	for ( int i : {10, 5, 1, 8, 4, 9, 2, 5} )
		tl.pushFront(i);
	tl.forEach(someFunc);
	cout << '\n';

	auto pred1 = [](int val){ return val % 4 == 0; };
	shared_ptr<int> res = tl.findFirstIf(pred1);
	cout << *res << '\n';
	assert(*res == 20);

	auto pred2 = [](int val){ return val > 50; };
	tl.removeIf(pred2);
	tl.forEach(someFunc);
	cout << '\n';
}
