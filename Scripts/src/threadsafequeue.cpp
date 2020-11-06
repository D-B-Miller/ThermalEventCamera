#include <iostream>
#include <queue>
#include <mutex>

// recipe from https://codetrips.com/2020/07/26/modern-c-writing-a-thread-safe-queue/
template<typename T>
class ThreadSafeQueue {
	std::queue<T> queue_;
	mutable std::mutex mutex_;
	// interface to check if empty
	bool empty() const{
		return queue_.empty();
	}

	public:
		// leave default constructor the same
		ThreadSafeQueue() = default;
		ThreadSafeQueue(const ThreadSafeQueue<T> &) = delete;
		// overriding assignment operator to delete operator
		ThreadSafeQueue& operator=(const ThreadSafeQueue<T> &) = delete;

		ThreadSafeQueue(ThreadSafeQueue<T>&& other){
			std::lock_guard<std::mutex> lock(mutex_);
			queue_ = std::move(other.queue);
		}

		virtual ~ThreadSafeQueue() {};
		// const as there's no need to make the this pointer
		// writable
		unsigned long size() const {
			std::lock_guard<std::mutex> lock(mutex_);
			return queue_.size();
		}

		// thread-safe pop function
		std::optional<T> pop() {
			std::lock_guard<std::mutex> lock(mutex_);
			if (queue_.empty()){
				return {};
			}
			T tmp = queue_.front();
			queue_.pop();
			return tmp;
		}
		// thread-safe push operation
		void push(const T &item) {
			std::lock_guard<std::mutex> lock(mutex_);
			queue_.push(item);
		}
};
