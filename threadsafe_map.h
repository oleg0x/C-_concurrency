#include <algorithm>
#include <list>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <vector>

using namespace std;



template <typename Key, typename Value, typename Hash = hash<Key>>
class Bucket
{
public:
	Value getValue(const Key& key, const Value& default_value) const
	{
		shared_lock<shared_mutex> lock(mutex_);
		BucketConstIterator found_entry = findEntryFor(key);
		return (found_entry == data_.end() ? default_value : found_entry->second);
	}

	void addOrUpdate(const Key& key, const Value& value)
	{
		unique_lock<shared_mutex> lock(mutex_);
		const BucketIterator found_entry = findEntryFor(key);
		if ( found_entry == data_.end() )
			data_.push_back(BucketValue(key, value));
		else
			found_entry->second = value;
	}

	void remove(const Key& key)
	{
		unique_lock<shared_mutex> lock(mutex_);
		const BucketIterator found_entry = findEntryFor(key);
		if ( found_entry != data_.end() )
			data_.erase(found_entry);
	}

private:
	using BucketValue = pair<Key, Value>;
	using BucketData = list<BucketValue>;
	using BucketIterator = typename BucketData::iterator;
	using BucketConstIterator = typename BucketData::const_iterator;

	BucketIterator findEntryFor(const Key& key)
	{
		return std::find_if(data_.begin(), data_.end(),
			[&](BucketValue& item){ return item.first == key; });
	}

	BucketConstIterator findEntryFor(const Key& key) const
	{
		return std::find_if(data_.begin(), data_.end(),
			[&](const BucketValue& item){ return item.first == key; });
	}

	BucketData data_;
	mutable shared_mutex mutex_;
};



template <typename Key, typename Value, typename Hash = hash<Key>>
class ThreadsafeMap
{
public:
	using key_type = Key;
	using value_type = Value;
	using hash_type = Hash;

	ThreadsafeMap(uint32_t num_buckets = 19, const Hash& hasher = Hash())
		: buckets_(num_buckets)
		, hasher_ {hasher}
	{
		for ( uint32_t i = 0; i < num_buckets; ++i )
			buckets_[i].reset(new Bucket<Key, Value>);
	}

	ThreadsafeMap(const ThreadsafeMap& other) = delete;
	ThreadsafeMap& operator=(const ThreadsafeMap& other) = delete;

	Value getValue(const Key& key, const Value& default_value) const
	{
		return getBucket(key).getValue(key, default_value);
	}

	void addOrUpdate(const Key& key, const Value& value)
	{
		getBucket(key).addOrUpdate(key, value);
	}

	void remove(const Key& key)
	{
		getBucket(key).remove(key);
	}

private:
	Bucket<Key, Value>& getBucket(const Key& key) const
	{
		const size_t bucket_index = hasher_(key) % buckets_.size();
		return *buckets_[bucket_index];
	}

	vector<unique_ptr<Bucket<Key, Value>>> buckets_;
	Hash hasher_;
};
