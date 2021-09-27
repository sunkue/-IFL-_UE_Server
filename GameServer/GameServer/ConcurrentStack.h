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
		Node(const T& value) :data{ value }
		{

		}

		T data;
		Node* next = nullptr;
	};

public:

	void push(const T& value)
	{
		Node* node = new Node{ value };
		node->next = _head;
		while (_head.compare_exchange_weak(node->next, node) == false)
			;;;
	}

	bool try_pop(T& value)
	{
		Node* old_head = _head;
		
		while(old_head && false == _head.compare_exchange_weak(old_head, old_head->next))
			;;;

		if (nullptr == old_head) {
			return false;
		}

		value = old_head->data;
		// delete old_head;
		return true;
	}

private:

	atomic<Node*> _head;

};