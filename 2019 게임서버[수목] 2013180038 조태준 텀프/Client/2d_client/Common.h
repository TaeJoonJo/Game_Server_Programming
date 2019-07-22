#pragma once

#define WIN32_LEAN_AND_MEAN  
#define INITGUID
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <windows.h>   // include important windows stuff
#include <windowsx.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

#include <d3d9.h>     // directX includes
#include "d3dx9tex.h"     // directX includes
#include "gpdumb1.h"
#include "../../Server/Overlapped_Server/Protocols.h"

using namespace std;

#pragma comment (linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma comment (lib, "ws2_32.lib")

#define WINDOW_WIDTH    700   // size of window
#define WINDOW_HEIGHT   800

#define UNIT_TEXTURE						0
#define STATUS_PLUS_BUTTON_TEXTURE			1
#define STATUS_BOARD_TEXTURE				2
#define TREE_MONSTER_TEXTURE				3
#define MUSHROOM_MONSTER_TEXTURE			4
#define MUD_MONSTER_TEXTURE					5
#define BITE_MONSTER_TEXTURE				6
#define FAIRY_TEXTURE						7
#define PRIEST_TEXTURE						8
#define HPPOTION_TEXTURE					9
#define MPPOTION_TEXTURE					10
#define MONEY_TEXTURE						11
#define SKILL_ENERGYBALL_TEXTURE			12
#define SKILL_FIREWALL_TEXTURE				13
#define SKILL_FROZEN_TEXTURE				14

constexpr int g_HeightWeight = -90;

constexpr int STATUSBOARDX = WINDOW_WIDTH / 4;
constexpr int STATUSBOARDY = 0;

constexpr int STATUSPLUSBUTTONSIZE = 50;

constexpr int STATUSPLUSBUTTONX = STATUSBOARDX + 300;
constexpr int STATUSPLUSBUTTONY = STATUSBOARDY + 120;
constexpr int STATUSPLUSBUTTONYDIFF = 70;