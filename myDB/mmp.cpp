#include "db.h"
#include "bloom.h"

#include <Windows.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <atomic>

using namespace std;

atomic_bool ready = false;


void test()
{
	while (!ready) this_thread::yield(); // 控制多线程同时启动
	Bloom *bloom = new Bloom();
	if (bloom == NULL) cout << "hehe";
}


int main()
{
	_setmaxstdio(2048);  // 重要程度简直了（新版）
	thread t[500];

	for (int i = 0; i < 500; i++) {
		t[i] = thread(test);
	}
	ready = true;
	for (auto &i : t) i.join();

	system("pause");
	return 0;
}
