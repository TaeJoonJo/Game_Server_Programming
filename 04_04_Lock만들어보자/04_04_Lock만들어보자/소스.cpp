#include <iostream>
#include <Windows.h>
#include <thread>
#include <atomic>

using namespace std;

atomic_int sum = 0;

atomic_int victim;
atomic_bool flag[2] = { false, false };

void worker()
{
	for (int i = 0; i < 25000000; ++i)
		sum += 2;
}

int main()
{
	thread c{ worker };
	thread p{ worker };

	c.join();
	p.join();

	cout << "Sum : " << sum << "\n";

	system("pause");

	return 1;
}