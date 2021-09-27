#include "pch.h"
#include "CorePch.h"

#include <windows.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <future>
#include "ConcurrentQueue.h"
#include "ConcurrentStack.h"
using namespace std;

LockQueue<int32_t> q;
LockFreeStack<int32_t> s;

void Push()
{
	for (;;)
	{
		int32_t value = rand() % 100;
		s.push(value);

		this_thread::sleep_for(10ms);
	}
}

void Pop()
{
	for (;;)
	{
		int32_t value;
		if (s.try_pop(OUT value))
			cout << value << endl;
	}
}

int main()
{
	thread t1{ Push };
	thread t2{ Pop };
	thread t3{ Pop };

	t1.join();
	t2.join();
	t3.join();

}