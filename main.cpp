void testThreadsafeMap();
void testThreadsafeMapMultithread();
void testTreadsafeList();



int main()
{
	testThreadsafeMap();
	testThreadsafeMapMultithread();
	testTreadsafeList();
}

// g++ threadsafe_map_test.cpp threadsafe_list_test.cpp main.cpp -std=c++20 -Wall -Wextra -pthread -o zzz
