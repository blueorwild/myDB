#include <iostream>       // std::cout
#include <atomic>         // std::atomic, std::memory_order_relaxed
#include <thread>         // std::thread
#include <windows.h>
#include "test.h"

using namespace std;

atomic<bool> ready(false);  // 模拟多线程并发的开始信号
int a[1000] = { 0 };

void blue()
{
	while (!ready) Sleep(0);
	SequenceNumber* p = SequenceNumber::Initial();
	a[p->GetWriteSN()]++;
}

int main()
{
	ready = false;
	thread threads[1000];
	for (int i = 0; i < 1000; i++) threads[i] = thread(blue);
	ready = true;
	for (int i = 0; i < 1000; i++) threads[i].join();

	for (int i = 0; i < 1000; i++) cout << a[i] << ' ';

	system("pause");
	return 0;
}