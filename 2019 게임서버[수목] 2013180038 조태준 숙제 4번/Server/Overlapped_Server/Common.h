
#pragma once

#include <iostream>
#include <map>
#include <random>
#include <vector>
#include <thread>
#include <list>
#include <mutex>
using namespace std;

#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

static INT RandomINT()
{
	//< 1�ܰ�. �õ� ����
	random_device rn;
	mt19937_64 rnd(rn());

	//< 2�ܰ�. ���� ���� ( ���� )
	uniform_int_distribution<int> range(RAND_MAX, -RAND_MAX);

	//< 3�ܰ�. �� ����
	return range(rnd);
}

#define DEFAULTSIZE 50.f
#define BOARDSIZE 400.f
#define STARTX -175.f
#define STARTY -175.f



constexpr int WORLD_HEIGHT = 100;