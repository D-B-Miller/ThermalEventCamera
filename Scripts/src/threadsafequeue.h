#include <queue>
#include <mutex>

// recipe from https://codetrips.com/2020/07/26/modern-c-writing-a-thread-safe-queue/
template<typename T>
class ThreadSafeQueue {
	private:
		std::queue<T> queue_; // default queue
		mutable std::mutex mutex_; // mutex to control access

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

		virtual ~ThreadSafeQueue() {}; // virtual deconstructor
		// const as there's no need to make the this pointer writable
		unsigned long size() const {
			std::lock_guard<std::mutex> lock(mutex_);
			return queue_.size();
		}

		// interface to check if empty
		bool empty() const{
			return queue_.empty();
		}
	
		// thread-safe pop function
		// updated passed ev refrence and returned a flag to indicate if it's successful
		bool pop(T& ev) {
			std::lock_guard<std::mutex> lock(mutex_);
			if (queue_.empty()){
				return false;
			}
			ev = queue_.front();
			queue_.pop();
			return true;
		}
		// thread-safe push operation
		void push(const T &item) {
			std::lock_guard<std::mutex> lock(mutex_);
			queue_.push(item);
		}
};
