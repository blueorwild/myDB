#include <iostream>       // std::cout
#include <atomic>         // std::atomic, std::memory_order_relaxed
#include <thread>         // std::thread
#include <random>
#include <windows.h>
#include "memory.h"

using namespace std;
atomic<bool> ready(false);  // 模拟多线程并发的开始信号
Memory *memory = NULL;
void blue1()
{
	char *a;
	while (!ready) { this_thread::yield(); }
	for (int i = 0; i < 100000; i++) {
		a = new char[rand() & 255];
		delete a;
	}
}

void blue2()
{
	char *a;
	while (!ready) { this_thread::yield(); }
	for (int i = 0; i < 100000; i++) {
		a = memory->Allocate(rand() & 255);
	}
}

int main()
{
	time_t start1, end1, start2, end2;
	for (int i = 0; i < 10; i++) {
		ready = false;
		thread T1(blue1);
		thread T2(blue1);

		start1 = clock();
		ready = true;
		T1.join();
		T2.join();
		end1 = clock();

		ready = false;
		thread T3(blue2);
		thread T4(blue2);

		start2 = clock();
		memory = new Memory;
		ready = true;
		T3.join();
		T4.join();
		delete memory;
		end2 = clock();


		double time1 = (double)(end1 - start1);
		double time2 = (double)(end2 - start2);
		printf("单次分配Use Time:%f\n", (time1 / CLOCKS_PER_SEC));
		printf("统一分配Use Time:%f\n", (time2 / CLOCKS_PER_SEC));
	}
	system("pause");
	return 0;
}