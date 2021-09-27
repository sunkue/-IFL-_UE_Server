#pragma once

#include <mutex>

template<class T>
class LockQueue
{
public:
	LockQueue() = default;

	LockQueue(LockQueue&&)noexcept = default;
	LockQueue& operator=(LockQueue&&)noexcept = default;

	void push(const T& value) noexcept {
		scoped_lock lock{ _mutex };
		_queue.push(std::move(value));
		_cv.notify_one();
	}

	bool try_pop(T& value) {
		scoped_lock lock{ _mutex };
		if (_queue.empty())
			return false;

		value = _queue.front();
		_queue.pop();
		return true;
	}

	void wait_pop(T& value) {
		unique_lock lck{ _mutex };
		_cv.wait(lck, [this] {return false == _queue.empty(); });
		value = std::move(_queue.top());
		_queue.pop();
	}

private:
	queue<T> _queue;
	mutex _mutex;
	condition_variable _cv;
};


