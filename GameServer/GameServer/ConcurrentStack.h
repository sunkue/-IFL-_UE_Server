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
		Node(const T& value) :data{ value } {};
		T data;
		Node* next = nullptr;
	};

	static void delete_nodes(Node* node)
	{
		while (node)
		{
			Node* temp = node->next;
			delete node;
			node = temp;
		}
	}

	void try_delete(Node* target)
	{
		if (_pop_count == 1)
		{
			Node* node = _pending_list.exchange(nullptr);

			if (0 == --_pop_count)
			{
				delete_nodes(node);
			}
			else if (node)
			{
				chain_pending_node_list(node);
			}
			delete target;
		}
		else
		{
			chain_pending_node(target);
			--_pop_count;
		}
	}

	void chain_pending_node_list(Node* first, Node* last)
	{
		while (false == _pending_list.compare_exchange_weak(last->next, first))
			;;;
	}

	void chain_pending_node_list(Node* node)
	{
		Node* last = node;
		while (last->next) { last = last->next; }
		chain_pending_node_list(node, last);
	}

	void chain_pending_node(Node* node)
	{
		chain_pending_node_list(node, node);
	}


public:

	void push(const T& value)
	{
		Node* node = new Node{ value };
		while (_head.compare_exchange_weak(node->next, node) == false)
			;;;
	}

	bool try_pop(T& value)
	{
		++_pop_count;

		Node* old_head = _head;
		while (old_head && false == _head.compare_exchange_weak(old_head, old_head->next))
			;;;

		if (nullptr == old_head)
		{
			--_pop_count;
			return false;
		}

		value = old_head->data;
		try_delete(old_head);
		return true;
	}


private:

	atomic<Node*> _head;

	atomic<uint32_t> _pop_count;
	atomic<Node*> _pending_list;
};