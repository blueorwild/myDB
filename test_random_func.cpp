#include <iostream>
#include <thread>
#include <atomic>
#include "random.h"

using namespace std;

atomic_bool ready = false;
int b[13] = { 0 };

Random *p = new Random(16807);
void blue()
{
	while (!ready) this_thread::yield();
	int a = p->RandomHeight(12);
	b[a]++;
}

int main()
{

	thread threads[1000];

	for (int i = 0; i < 1000; i++) {
		threads[i] = thread(blue);
	}

	ready = true;

	for (int i = 0; i < 1000; i++) {
		threads[i].join();
	}

	for (int i = 1; i <= 12; i++) {
		cout << b[i] << ' ';
	}
	cout << endl;
	system("pause");
	return 0;
}
