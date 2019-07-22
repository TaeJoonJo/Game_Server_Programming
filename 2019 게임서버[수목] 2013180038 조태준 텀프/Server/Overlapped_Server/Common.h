
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
#include <fstream>
using namespace std;

#include <winsock2.h>

#include "Protocols.h"

extern "C" {
#include "include/lauxlib.h"
#include "include/lua.h"
#include "include/lualib.h"
}

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "lua53.lib")

#define DEFAULTSIZE 50.f
#define BOARDSIZE 400.f
#define STARTX -175.f
#define STARTY -175.f

constexpr int VIEW_RADIUS = 7;
constexpr int NPC_MOVE_RADIUS = 10;
constexpr int MONSTER_AGGRO_RADIUS = 5;

constexpr int DEBUG_TELEPORT_X = 200;
constexpr int DEBUG_TELEPORT_Y = 200;

constexpr int BASIC_PLAYER_X = 6;
constexpr int BASIC_PLAYER_Y = 6;

constexpr int BASIC_PLAYER_HP = 100;
constexpr int BASIC_PLAYER_MP = 100;
constexpr int BASIC_PLAYER_DAMAGE = 10;

constexpr int BASIC_NEED_EXP = 100;

constexpr int BASIC_MONSTER_EXP = 30;

constexpr int STAT_WEIGHT = 5;

constexpr int SKILL_ENERGYBALL_DAMAGE	 = 20;
constexpr int SKILL_FIREWALL_DAMAGE		 = 30;
constexpr int SKILL_FROZEN_DAMAGE		 = 40;

constexpr int PLAYER_MOVE_COOLTIME = 100;
constexpr int PLAYER_ATTACK_COOLTIME = 1000;
constexpr int PLAYER_RESPAWN_COOLTIME = 1000 * 5;
constexpr int NPC_MOVE_COOLTIME = 1000;
constexpr int NPC_RESPAWN_COOLTIME = 1000 * 30;
enum e_EventType
{
	Event_Recv,
	Event_Send,
	Timer_NPC_Move,
	Timer_NPC_Run,
	Event_Player_Move,
	Timer_Player_Can_Move,
	Timer_Player_Can_Attack,
	Timer_Player_HPMP_Regen,
	Timer_Player_Respawn,
	Timer_NPC_Can_Move,
	Timer_Item_Disappear,
	Timer_Monster_Respawn,
	Timer_Monster_Aggro_Reset,
	Timer_Skill_Effect
};

struct SOCKETINFO
{
	WSAOVERLAPPED overlapped;
	WSABUF wsaBuf;
	e_EventType eventType;
	char buf[MAX_BUFFER];
	DWORD targetid;
};

struct Point
{
	int x;
	int y;
};

struct Node
{
	int value;
	int x, y;
};

static INT RandomINT()
{
	//< 1단계. 시드 설정
	random_device rn;
	mt19937_64 rnd(rn());

	//< 2단계. 분포 설정 ( 정수 )
	uniform_int_distribution<int> range(-RAND_MAX, RAND_MAX);

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