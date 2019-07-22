
#pragma once

#ifndef __PROTOCOLS_H__
#define __PROTOCOLS_H__

#include "Common.h"

//#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_BUFFER        1024
#define SERVER_PORT        3500

constexpr int MAX_USER = 100;
constexpr DWORD NUM_NPC = 200000;

constexpr int MAP_SIZE = 21;

enum e_EventType
{
	Event_Recv,
	Event_Send,
	Timer_NPC_Move
};

struct SOCKETINFO	
{									
	WSAOVERLAPPED overlapped;	
	WSABUF wsaBuf;
	e_EventType eventType;
	char buf[MAX_BUFFER];
	DWORD targetid;
};

typedef enum e_sc_PacketType
{
	Login_Ok,
	Login_Fail,
	Signup_Ok,
	Signup_Fail,
	Logout,
	MovePlayer,
	PutPlayer,
	RemovePlayer,
	MoveNPC,
	PutNPC,
	RemoveNPC
}sc_PACKETTYPE;

typedef enum e_cs_PacketType
{
	Login,
	Signup,
	Left,
	Right,
	Up,
	Down,
	IDLE
}cs_PACKETTYPE;

#pragma pack(push, 1)
// Client to Server
struct cs_packet_base {
	BYTE size;
	BYTE type;
};
struct cs_packet_login {
	BYTE size;
	BYTE type;
	TCHAR name[10];
};
struct cs_packet_signup {
	BYTE size;
	BYTE type;
	TCHAR name[10];
};
struct cs_packet_up {
	BYTE size;
	BYTE type;
};
struct cs_packet_down {
	BYTE size;
	BYTE type;
};
struct cs_packet_left {
	BYTE size;
	BYTE type;
};
struct cs_packet_right {
	BYTE size;
	BYTE type;
};
struct cs_packet_idle {
	BYTE size;
	BYTE type;
};

// Server to Client
struct sc_packet_base {
	BYTE size;
	BYTE type;
};
struct sc_packet_login_ok {
	BYTE size;
	BYTE type;
	BYTE id;
};
struct sc_packet_login_fail {
	BYTE size;
	BYTE type;
};
struct sc_packet_signup_ok {
	BYTE size;
	BYTE type;
};
struct sc_packet_signup_fail {
	BYTE size;
	BYTE type;
};
struct sc_packet_logout {
	BYTE size;
	BYTE type;
	BYTE id;
};
struct sc_packet_move_player {
	BYTE size;
	BYTE type;
	BYTE id;
	float x;
	float y;
};
struct sc_packet_put_player {
	BYTE size;
	BYTE type;
	BYTE id;
	//TCHAR name[20];			// 아직 사용안함
	float x;
	float y;
};
struct sc_packet_remove_player {
	BYTE size;
	BYTE type;
	BYTE id;
};
struct sc_packet_move_npc {
	BYTE size;
	BYTE type;
	DWORD id;
	float x;
	float y;
};
struct sc_packet_put_npc {
	BYTE size;
	BYTE type;
	DWORD id;
	//TCHAR name[20];			// 아직 사용안함
	float x;
	float y;
};
struct sc_packet_remove_npc {
	BYTE size;
	BYTE type;
	DWORD id;
};
#pragma pack(pop)
#endif // !__PROTOCOLS_H__