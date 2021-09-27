#pragma once

#include <mutex>

template<class T>
class LockStack
{
public:
	LockStack() = default;

	LockStack(LockStack&&)noexcept = default;
	LockStack& operator=(LockStack&&)noexcept = default;

	void push(const T& value) noexcept
	{
		scoped_lock lock{ _mutex };
		_stack.push(std::move(value));
		_cv.notify_one();
	}

	bool try_pop(T& value)
	{
		scoped_lock lock{ _mutex };
		if (_stack.empty())
			return false;

		value = _stack.top();
		_stack.pop();
		return true;
	}

	void wait_pop(T& value)
	{
		unique_lock lck{ _mutex };
		_cv.wait(lck, [this] {return false == _stack.empty(); });
		value = std::move(_stack.top());
		_stack.pop();
	}

private:
	stack<T> _stack;
	mutex _mutex;
	condition_variable _cv;
};




template<class T>
class LockFreeStack
{
	struct Node
	{
		Node(const T& value) :data{ make_shared<T>(value) } {};
		shared_ptr<T> data;
		shared_ptr<Node> next;
	};


public:

	void push(const T& value)
	{
		shared_ptr<Node> node = make_shared<Node>(value);
		while (false == _head.compare_exchange_weak(node->next, node))
			;;;
	}

	shared_ptr<T> try_pop()
	{
		shared_ptr<Node> old_head = _head;
		if (nullptr == old_head)return nullptr;

		while (old_head && false == _head.compare_exchange_weak(old_head, old_head->next))
			;;;

		return old_head->data;
	}


private:

	atomic<shared_ptr<Node>> _head;

};