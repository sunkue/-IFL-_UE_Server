#include "pch.h"
#include "CorePch.h"

#include <windows.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <future>

using namespace std;


int32 x = 0;
int32 y = 0;
int32 r2 = 0;
int32 r1 = 0;

volatile bool ready;

void f1() {
	while (!ready)
		;
	y = 1;
	r1 = x;
}

void f2() {
	while (!ready)
		;
	x = 1;
	r2 = y;
}

int main()
{
	int32 count = 0;
	for (;; count++) {
		ready = false;

		x = y = r1 = r2 = 0;

		thread t1{ f1 };
		thread t2{ f2 };
		
		ready = true;

		t1.join();
		t2.join();

		if (r1 == 0 && r2 == 0)
			break;
	}
	cout << count << endl;
}