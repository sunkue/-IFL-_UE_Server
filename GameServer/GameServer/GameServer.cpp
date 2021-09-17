#include "pch.h"
#include "CorePch.h"

#include <windows.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <future>

using namespace std;



int64 f()
{
	int64 sum = 0;

	for (int32 i = 0; i < 100'000; i++)
	{
		sum += 2;
	}

	return sum;
}

void PromiseWorker(std::promise<string>&& promise)
{
	promise.set_value("Message");
}

void TaskWorker(std::packaged_task<int64()>&& task)
{
	task();
}

int main()
{
	// 동기(synhcronous) 실행
	// int64 sum = f();
	// cout << sum << endl;


	// future 
	// async
	{
		// opt 1) deferred			=> lazy evaluation
		// opt 2) async				=> thread
		// opt 3) deferred | async	=> whatever
		std::future<int64> future = std::async(std::launch::async, f);

		if (future.wait_for(1ns) != future_status::ready)
			cout << "not done yet" << endl;

		cout << future.get() << endl;

		class C {
		public:
			int64 f() { return 10; }
		} c;
		future = std::async(std::launch::async, &C::f, c);

		cout << future.get() << endl;
	}

	// promise
	{
		// 계약서..
		std::promise<string> promise;
		std::future<string> future = promise.get_future();

		thread t(PromiseWorker, std::move(promise)); //promise => empty

		cout << future.get() << endl;	//future => empty

		t.join();
	}

	// packaged_task
	{
		std::packaged_task task(f);
		future future = task.get_future();

		thread t(TaskWorker, move(task));

		cout << future.get() << endl;

		t.join();
	}

	// async /vs/ packaged_task
	// private_thread for task /vs/ task for any thread
	// 단발성 이벤트에서 유용하다

	// 결론
	// mutex, condition_variable 없이 간단한 task 처리.

	// 1) async
	// 함수를 비동기 실행
	// 2) promise
	// 결과를 future에 전달약속
	// 3) packaged_task
	// 결과를 future에 전달약속

	// 비동기 => lazy 여도 되고 thread 여도 되고 아무래도 상관없으니 언젠가 실행되어서 필요할때를 대비해 주시오.
}