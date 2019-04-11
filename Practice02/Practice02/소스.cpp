#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
using namespace std;
using namespace std::chrono;

volatile int sum = 0;

int threadnum = 16;

// DATA RACE를 막기위해
mutex mylock;

void _Threadfunc()
{
	//cout << "I'm " << id << " Thread\n";
	volatile int local_sum = 0;
	for (int i = 0; i < 50000000 / threadnum; ++i) {
		local_sum = local_sum + 2;
	}
	mylock.lock();
	sum = sum + local_sum;
	mylock.unlock();
	//while (true);
}

int main()
{
	/*for (int i = 0; i < 50000000; ++i) {
	sum += 2;
}*/

	vector<thread> mythreads;

	auto t = high_resolution_clock::now();

	for (int i = 0; i < threadnum; ++i)
		mythreads.emplace_back(thread{ _Threadfunc });

	for (auto &a : mythreads)
		a.join();

	auto d = high_resolution_clock::now() - t;

	cout << sum << "\n";

	cout << duration_cast<milliseconds>(d).count() << "mescs\n";

	system("pause");
}