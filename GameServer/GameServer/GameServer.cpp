#include "pch.h"
#include "CorePch.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>

mutex m;
queue<int32> q;
HANDLE hEvent;

void Producer()
{
	for (;;)
	{
		{
			unique_lock lock{ m };
			q.push(10);
		}

		::SetEvent(hEvent);

		this_thread::sleep_for(10ms);
	}
}

void Consumer()
{
	for (;;)
	{
		::WaitForSingleObject(hEvent, INFINITE);
		
		unique_lock lock{ m };
		if (false == q.empty())
		{
			auto data = q.front(); q.pop();
			cout << data << endl;
		}
	}
}

int main()
{
	// 커널오브젝트. 프로세스도이고하 가능, 커널씨피유활용가능. 시스템콜이므로 자주하면 개느림
	hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	thread t1{ Producer };
	thread t2{ Consumer };

	t1.join();
	t2.join();

	cout << "Job_Done" << endl;

	if (hEvent)::CloseHandle(hEvent);
}