// g++ lock_free_stack.cpp -std=c++20 -Wall -Wextra -pthread -latomic -o zzz

#include <atomic>
#include <cassert>
#include <iostream>
#include <memory>

using namespace std;



template <typename T>
struct Node
{
	shared_ptr<T> data;
	Node* next;
	Node(const T& d) : data {make_shared<T>(d)} {}
};



template <typename T>
class LockFreeStack
{
public:
	void push(const T& d);
	shared_ptr<T> pop();

private:
	static void deleteNodes(Node<T>* nodes);
	void tryReclaim(Node<T>* old_head);
	void chainPendingNodes(Node<T>* nodes);
	void chainPendingNodes(Node<T>* first, Node<T>* last);
	void chainPendingNode(Node<T>* n);

	atomic<Node<T>*> head_;
	atomic<uint32_t> threads_in_pop_;
	atomic<Node<T>*> to_be_deleted_;
};



template <typename T>
void LockFreeStack<T>::push(const T& d)
{
	Node<T>* const new_node = new Node(d);
	new_node->next = head_.load();
	while ( !head_.compare_exchange_weak(new_node->next, new_node) ) ;
}



template <typename T>
shared_ptr<T> LockFreeStack<T>::pop()
{
	++threads_in_pop_;
	Node<T>* old_head = head_.load();
	while ( old_head &&
		!head_.compare_exchange_weak(old_head, old_head->next) ) ;
	shared_ptr<T> res;
	if ( old_head )  res.swap(old_head->data);
	tryReclaim(old_head);
	return res;
}



template <typename T>
void LockFreeStack<T>::deleteNodes(Node<T>* nodes)
{
	while ( nodes )
	{
		Node<T>* next = nodes->next;
		delete nodes;
		nodes = next;
	}
}



template <typename T>
void LockFreeStack<T>::tryReclaim(Node<T>* old_head)
{
	if ( threads_in_pop_ == 1 )
	{
		Node<T>* nodes_to_delete = to_be_deleted_.exchange(nullptr);
		if ( --threads_in_pop_ == 0 )
			deleteNodes(nodes_to_delete);
		else if ( nodes_to_delete )
			chainPendingNodes(nodes_to_delete);
		delete old_head;
	}
	else  // At least two threads in pop()
	{
		chainPendingNodes(old_head, old_head);
		--threads_in_pop_;
	}
}



template <typename T>
void LockFreeStack<T>::chainPendingNodes(Node<T>* nodes)
{
	Node<T>* last = nodes;
	while ( Node<T>* const next = last->next )  last = next;
	chainPendingNodes(nodes, last);
}



template <typename T>
void LockFreeStack<T>::chainPendingNodes(Node<T>* first, Node<T>* last)
{
	last->next = to_be_deleted_;
	while ( !to_be_deleted_.compare_exchange_weak(last->next, first) ) ;
}



int main()
{
	auto p = make_shared<int>(42);
	cout << boolalpha << "atomic<shared_ptr> is lock free? "
		 << atomic_is_lock_free(&p) << ' ' << *p << '\n';

	LockFreeStack<int> lfs;
	lfs.push(100);
	lfs.push(200);
	lfs.push(300);
	cout << *(lfs.pop()) << ' '
		 << *(lfs.pop()) << ' '
		 << *(lfs.pop()) << '\n';
}
