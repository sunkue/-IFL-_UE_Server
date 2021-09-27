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
	struct Node;

	struct CountedNodePtr
	{
		int32_t external_count = { 0 };
		Node* ptr = { nullptr };
	};

	struct Node
	{
		Node(const T& value) :data{ make_shared<T>(value) } {};
		shared_ptr<T> data;
		atomic<int32_t> internal_count = { 0 };
		CountedNodePtr next;
	};


public:

	void push(const T& value)
	{
		CountedNodePtr node;
		node.ptr = new Node{ value };
		node.external_count = 1;
		while (false == _head.compare_exchange_weak(node.ptr->next, node))
			;;;
	}

	shared_ptr<T> try_pop()
	{
		CountedNodePtr old_head = _head;
		for (;;)
		{
			increase_head_count(old_head);
			Node* ret_ptr = old_head.ptr;

			if (nullptr == ret_ptr) { return shared_ptr<T>(); }

			if (_head.compare_exchange_strong(old_head, ret_ptr->next))
			{
				shared_ptr<T> ret;
				ret.swap(ret_ptr->data);

				const int32_t count_increase = { old_head.external_count - 2 };

				if ((-count_increase) == ret_ptr->internal_count.fetch_add(count_increase))
				{
					delete ret_ptr;
				}

				return ret;
			}
			else if (ret_ptr->internal_count.fetch_sub(1) == 1)
			{
				delete ret_ptr;
			}
		}
	}


private:

	void increase_head_count(CountedNodePtr& counted_ptr)
	{
		for (;;)
		{
			CountedNodePtr new_counted_ptr = counted_ptr;
			new_counted_ptr.external_count++;

			if (_head.compare_exchange_strong(counted_ptr, new_counted_ptr))
			{
				counted_ptr.external_count = new_counted_ptr.external_count;
				break;
			}
		}
	}

private:

	atomic<CountedNodePtr> _head;

};