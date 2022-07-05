// g++ atomic.cpp -std=c++20 -Wall -Wextra -pthread -latomic -o zzz

#include <atomic>
#include <cassert>
#include <iostream>
#include <string>

using namespace std;



struct Foo
{
	int i;
	string s;			// string is not a trivially copyable type
};

struct Bar
{
	char c;
	int i;
	double d;
	char s[10];
};

struct Baz
{
	int i1;
	int i2;
};



int main()
{
	cout << boolalpha;

	// Operations with atomic_flag
	atomic_flag af;				// Default ctor initializes to clear (false) state
	cout << af.test_and_set() << ' '	// Atomically changes the state to set (true) and returns the value it held before
		 << af.test_and_set() << ' '
		 << af.test_and_set() << '\n';
	af.clear();					// Atomically changes the state to clear
	cout << af.test() << ' ';
	assert(af.test() == false);
	af.test_and_set();
	assert(af.test() == true);
	cout << af.test() << "\n\n";
	assert(af.test() == true);

	// Operations with atomic<bool>
	atomic<bool> b(false);		// Is neither copyable nor movable
	cout << b << ' ';
	b.store(true);				// No return value
	b = true;					// Equivalent to store(value)
	cout << b << "\n\n";

	bool x;
	x = b.load(memory_order_acquire);	// Returns new value
	x = b;						// Equivalent to load(value)
	cout << b << " = " << x << "\n\n";
	assert(b == true && x == true);

	x = false;
	x = b.exchange(false, memory_order_acq_rel);	// Returns old value
	cout << b << " " << x << "\n\n";
	assert(b == false && x == true);

	// The cornerstone of programming with atomics: compare_exchange_*
	// compare_exchange_weak
	bool expected = false;
	while ( !b.compare_exchange_weak(expected, true) && !expected ) ;
	cout << b << "\n\n";
	assert(b == true);
	// compare_exchange_strong
	std::atomic<int> ai;
	int test_val = 4;
	int new_val = 5;
	bool exchanged = false;
	ai = 3;
	cout << "ai=" << ai << "  test_val=" << test_val
		 << "  new_val=" << new_val << "  exchanged=" << exchanged << "\n";
    // test_val != ai   ==>  test_val is modified
	exchanged = ai.compare_exchange_strong(test_val, new_val);
	cout << "ai=" << ai << "  test_val=" << test_val
		 << "  new_val=" << new_val << "  exchanged=" << exchanged << "\n";
	assert(ai == 3 && test_val == 3 && exchanged == false);
	// test_val == ai   ==>  ai is modified
	exchanged = ai.compare_exchange_strong(test_val, new_val);
	cout << "ai=" << ai << "  test_val=" << test_val
		 << "  new_val=" << new_val << "  exchanged=" << exchanged << "\n\n";
	assert(ai == 5 && test_val == 3 && exchanged == true);

	// Operations with atomic<T*>
	Foo arr[10] = { {10, "aaa"}, {20, "bbb"}, {30, "ccc"}};
	atomic<Foo*> p(arr);
	Foo* y = p.fetch_add(2);		// Returns old value
	cout << y->i << ' ' << y->s << ",  "
		 << p.load()->i << ' ' << p.load()->s << "\n";
	assert(y == arr);
	assert(p.load() == &arr[2]);
	y = (p -= 1);					// Returns new value
	cout << y->i << ' ' << y->s << ",  "
		 << p.load()->i << ' ' << p.load()->s << "\n\n";
	assert(y == &arr[1]);
	assert(p.load() == &arr[1]);

	// Operations with atomic<integral-type>
	atomic<int> i = 100;			// Is neither copyable nor movable
	int ii;
	ii = i.fetch_add(10);			// Returns old value
	cout << ii << ' ';
	assert(ii == 100);
	ii = i.fetch_sub(60);			// Returns old value
	assert(ii == 110);
	ii = (i += 10);					// Returns new value
	cout << ii << ' ';
	assert(ii == 60);
	i -= 18;						// Returns new value
	++i;							// Atomic pre-increment
	i--;							// Atomic post-decrement
	// Bitwise fetch_and, fetch_or, fetch_xor are also available
	cout << i << '\n';
	assert(i == 42);
	// Free functions
	ii = atomic_fetch_add(&i, 4);	// Returns old value
	atomic_fetch_add_explicit(&i, 4, memory_order_release);
	cout << i << ' ' << ii << '\n';
	assert(i == 50 && ii == 42);
	ii = atomic_fetch_sub(&i, 10);	// Returns old value
	atomic_fetch_sub_explicit(&i, 10, memory_order_acquire);
	cout << i << ' ' << ii << "\n\n";
	assert(i == 30 && ii == 50);

	atomic<char> ac('A');
	atomic<double> ad;
//	atomic<Foo> afoo;		// Compilation error: atomic requires a trivially copyable type
	cout << "'string' is trivially copyable? " << is_trivially_copyable<string>::value << '\n';
	assert(is_trivially_copyable<string>::value == false);
	atomic<Bar> abar;
	atomic<Baz> abaz;
	cout << "is_lock_free: "
		 << ac.is_lock_free() << ' ' << i.is_lock_free() << ' '
		 << ad.is_lock_free() << ' ' << p.is_lock_free() << ' '
		 << abar.is_lock_free() << ' ' << abaz.is_lock_free() << '\n';
}
