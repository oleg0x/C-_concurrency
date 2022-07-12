#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;



atomic<bool> x, y;
atomic<int> z;



void write_x()
{
	x.store(true, memory_order_seq_cst);
}

void write_y()
{
	y.store(true, memory_order_seq_cst);
}

void read_x_y()
{
	while ( !x.load(memory_order_seq_cst) ) ;
	if ( y.load(memory_order_seq_cst) )  ++z;
}

void read_y_x()
{
	while ( !y.load(memory_order_seq_cst) ) ;
	if ( x.load(memory_order_seq_cst) )  ++z;
}



void write_x_y_relaxed()
{
	x.store(true, memory_order_relaxed);
	y.store(true, memory_order_relaxed);
}

void read_y_x_relaxed()
{
	while ( !y.load(memory_order_relaxed) ) ;
	if ( x.load(memory_order_relaxed) )  ++z;
}



void write_x_y_3()
{
	x.store(true, memory_order_relaxed);		// x happened-before y, since they are in one thread
	y.store(true, memory_order_release);		// Release-acquire ordering
}

void read_y_x_3()
{
	while ( !y.load(memory_order_acquire) ) ;	// Release-acquire ordering
	if ( x.load(memory_order_relaxed) )  ++z;
}



void write_x_y_fence()
{
	x.store(true, memory_order_relaxed);
	atomic_thread_fence(memory_order_release);
	y.store(true, memory_order_relaxed);
}

void read_y_x_fence()
{
	while ( !y.load(memory_order_relaxed) ) ;
	atomic_thread_fence(memory_order_acquire);
	if ( x.load(memory_order_relaxed) )  ++z;
}



atomic<int> arr[4];
//atomic<bool> sync1(false), sync2(false);
atomic<int> sync0(0);

void func1()
{
	arr[0].store(10, memory_order_relaxed);
	arr[1].store(20, memory_order_relaxed);
	arr[2].store(30, memory_order_relaxed);
	arr[3].store(40, memory_order_relaxed);
//	sync1.store(true, memory_order_release);		// Release-acquire ordering by sync1
	sync0.store(1, memory_order_release);			// Release-acquire ordering by sync
}

void func2()
{
//	while ( !sync1.load(memory_order_acquire) ) ;	// Release-acquire ordering by sync1
//	sync2.store(true, memory_order_release);		// Release-acquire ordering by sync2
	int expected = 1;								// Release-acquire ordering by sync
	while ( !sync0.compare_exchange_strong(expected, 2, memory_order_acq_rel) )
		expected = 1;
}

void func3()
{
//	while ( !sync2.load(memory_order_acquire) ) ;		// Release-acquire ordering by sync2
	while ( sync0.load(memory_order_acquire) < 2 ) ;	// Release-acquire ordering by sync
	cout << arr[0] << ' ' << arr[1] << ' '
		 << arr[2] << ' ' << arr[3] << '\n';
	assert(arr[0] == 10 && arr[1] == 20 &&
		   arr[2] == 30 && arr[3] == 40);
}



struct X
{
	int i;
	string s;
};

atomic<X*> px;

void create_x()				// Producer (publisher)
{
	X* x = new X;
	x->i = 42;
	x->s = "Foo-bar";
	// Publisher makes a pointer through which the consumer accesses information
	px.store(x, memory_order_release);					// Release-consume ordering
}

void use_x()				// Consumer (subscriber)
{
	X* x;
	while ( !(x = px.load(memory_order_consume)) )		// Release-consume ordering
		this_thread::sleep_for(std::chrono::microseconds(1));
	cout << x->i << ' ' << x->s << '\n';
	assert(x->i == 42 && x->s == "Foo-bar");
}



vector<int> queue_data;
atomic<int> queue_count;

void populate_quiue()
{
	const unsigned number_of_items = 13;
	queue_data.clear();
	for ( unsigned i = 0; i < number_of_items; ++i )
		queue_data.push_back(i);
	queue_count.store(number_of_items, memory_order_release);
}

void consume_queue_items()
{
	while ( true )
	{
		int item_index = queue_count.fetch_sub(1, memory_order_acquire);
		if ( item_index <= 0 )
		{
			this_thread::sleep_for(std::chrono::milliseconds(5));
			break;
		}
		else
		{
//			process_data(queue_data[item_index-1]);
			cout << "Thread №" << this_thread::get_id() << ": "
				 << queue_data[item_index-1] << '\n';
		}
	}
}



int main()
{
	{
		x = y = false;
		z = 0;
		thread a(write_x);
		thread b(write_y);
		thread c(read_x_y);
		thread d(read_y_x);
		a.join();
		b.join();
		c.join();
		d.join();
		cout << "memory_order_seq_cst: " << z << '\n';
		assert(z.load() == 2 || z.load() == 1);
	}

	{
		x = y = false;
		z = 0;
		thread a(write_x_y_relaxed);
		thread b(read_y_x_relaxed);
		a.join();
		b.join();
		cout << "memory_order_relaxed: " << z << '\n';
		assert(z.load() == 1 || z.load() == 0);
	}

	{
		x = y = false;
		z = 0;
		thread a(write_x_y_3);
		thread b(read_y_x_3);
		a.join();
		b.join();
		cout << "memory_order_release, memory_order_acquire: " << z << '\n';
		assert(z.load() == 1);
	}

	{
		jthread a(func1);
		jthread b(func2);
		jthread c(func3);
	}

	{
		jthread a(create_x);
		jthread b(use_x);
	}

	{
		jthread a(populate_quiue);
		jthread b(consume_queue_items);
		jthread c(consume_queue_items);
	}

	{
		x = y = false;
		z = 0;
		thread a(write_x_y_fence);
		thread b(read_y_x_fence);
		a.join();
		b.join();
		cout << "fence: " << z << '\n';
		assert(z.load() == 1);
	}
}
