#include "pch.h"
#include "CorePch.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

#include <windows.h>

mutex m;
queue<int32> q;

// 유저오브젝트
condition_variable cv;

void Producer()
{
	for (;;)
	{
		{
			unique_lock lock{ m };
			q.push(10);
		}

		//lock을 풀고 전달해주어야함
		cv.notify_one();
	}
}

void Consumer()
{
	for (;;)
	{
		unique_lock lock{ m };
		cv.wait(lock, []() {return false == q.empty(); });
		// 조건 만족안하면 unlock, wait . unlock,lock 해야하므로 unique_lock

		auto data = q.front(); q.pop();
		cout << q.size() << endl;
	}
}

int main()
{
	thread t1{ Producer };
	thread t2{ Consumer };

	t1.join();
	t2.join();

	cout << "Job_Done" << endl;
}