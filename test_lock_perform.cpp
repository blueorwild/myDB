#include <iostream>       // std::cout
#include <atomic>         // std::atomic, std::memory_order_relaxed
#include <thread>         // std::thread
#include <windows.h>
#include <mutex>
#include "mylock2.h"
#include "mylock3.h"

using namespace std;
mutex mux;

atomic<bool> ready(false);  // 模拟多线程并发的开始信号

int sum2 = 0;
int sum3 = 0;

void blue2()
{
	while (!ready) { this_thread::yield(); }
	for (int i = 0; i < 10000000; i++) {
		lock2();
		++sum2;
		unlock2();
	}
}

void blue3()
{
	while (!ready) { this_thread::yield(); }
	for (int i = 0; i < 10000000; i++) {
		lock3();
		++sum3;
		unlock3();
	}
}

int main()
{
	flag2.clear();
	time_t start1, end1, start2, end2;

	ready = false;
	thread T3(blue3);
	thread T4(blue3);

	start2 = clock();
	ready = true;
	T3.join();
	T4.join();
	end2 = clock();

	ready = false;
	thread T1(blue2);
	thread T2(blue2);

	start1 = clock();
	ready = true;
	T1.join();
	T2.join();
	end1 = clock();

	cout << sum2 << ' ' << sum3 << endl;
	double time1 = (double)(end1 - start1);
	double time2 = (double)(end2 - start2);
	printf("Use Time2:%f\n", (time1 / CLOCKS_PER_SEC));
	printf("Use Time3:%f\n", (time2 / CLOCKS_PER_SEC));

	system("pause");
	return 0;
}