
#pragma once

#include <iostream>
#include <map>
#include <unordered_map>
#include <array>
#include <queue>
#include <random>
#include <vector>
#include <thread>
#include <unordered_set>
#include <mutex>
using namespace std;

#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULTSIZE 50.f
#define BOARDSIZE 400.f
#define STARTX -175.f
#define STARTY -175.f

constexpr int WORLD_WIDTH = 800;
constexpr int WORLD_HEIGHT = 800;

constexpr int VIEW_RADIUS = 7;
constexpr int NPC_MOVE_RADIUS = 10;

static INT RandomINT()
{
	//< 1단계. 시드 설정
	random_device rn;
	mt19937_64 rnd(rn());

	//< 2단계. 분포 설정 ( 정수 )
	uniform_int_distribution<int> range(RAND_MAX, -RAND_MAX);

	//< 3단계. 값 추출
	return range(rnd);
}

static INT RandomINT(INT min, INT max)
{
	//< 1단계. 시드 설정
	random_device rn;
	mt19937_64 rnd(rn());

	//< 2단계. 분포 설정 ( 정수 )
	uniform_int_distribution<int> range(min, max);

	//< 3단계. 값 추출
	return range(rnd);
}

static FLOAT RandomFLOAT(float fmin, float fmax)
{
	//< 1단계. 시드 설정
	random_device rn;
	mt19937_64 rnd(rn());

	//< 2단계. 분포 설정
	uniform_real_distribution<float> range(fmin, fmax);

	//< 3단계. 값 추출
	return range(rnd);
}