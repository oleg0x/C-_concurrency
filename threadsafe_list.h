#include <memory>
#include <mutex>

using namespace std;



template <typename T>
struct Node
{
	shared_ptr<T> data;
	unique_ptr<Node> next;
	mutex m;					// For fine-grained locks

	Node() : next() {}			// To construct head of the list
	Node(const T& value) : data(make_shared<T>(value)) {}
};



template <typename T>
class TreadsafeList
{
public:
	TreadsafeList() {}
	~TreadsafeList() { removeIf([](const Node<T>&){ return true; }); }

	TreadsafeList(const TreadsafeList&) = delete;
	TreadsafeList& operator=(const TreadsafeList&) = delete;

	void pushFront(const T& value)
	{
		unique_ptr<Node<T>> new_node(new Node<T>(value));
		lock_guard<mutex> lk(head_.m);
		new_node->next = move(head_.next);
		head_.next = move(new_node);
	}

	template <typename Function>
	void forEach(Function func)
	{
		Node<T>* curr = &head_;
		unique_lock<mutex> lk(head_.m);
		while ( Node<T>* const next = curr->next.get() )
		{
			unique_lock<mutex> next_lk(next->m);
			lk.unlock();
			func(*next->data);
			curr = next;
			lk = move(next_lk);
		}
	}

	template <typename Predicate>
	shared_ptr<T> findFirstIf(Predicate pred)
	{
		Node<T>* curr = &head_;
		unique_lock<mutex> lk(head_.m);
		while ( Node<T>* const next = curr->next.get() )
		{
			unique_lock<mutex> next_lk(next->m);
			lk.unlock();
			if ( pred(*next->data) )
				return next->data;
			curr = next;
			lk = move(next_lk);
		}
		return shared_ptr<T>();
	}

	template <typename Predicate>
	void removeIf(Predicate pred)
	{
		Node<T>* curr = &head_;
		unique_lock<mutex> lk(head_.m);
		while ( Node<T>* const next = curr->next.get() )
		{
			unique_lock<mutex> next_lk(next->m);
			if ( pred(*next->data) )
			{
				unique_ptr<Node<T>> old_next = move(curr->next);
				curr->next = move(next->next);
				next_lk.unlock();
			}
			else
			{
				lk.unlock();
				curr = next;
				lk = move(next_lk);
			}
		}
	}

private:
	Node<T> head_;
};
