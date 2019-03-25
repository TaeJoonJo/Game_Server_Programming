
#pragma once

#ifndef __PROTOCOLS_H__
#define __PROTOCOLS_H__

#include "Common.h"

//#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_BUFFER        1024
#define SERVER_PORT        3500

enum e_EventType
{
	Event_Recv,
	Event_Send
};

struct SOCKETINFO	
{									
	WSAOVERLAPPED overlapped;	
	WSABUF wsaBuf;
	e_EventType eventType;
	char buf[MAX_BUFFER];
	INT id;
};

typedef enum e_sc_PacketType
{
	Login,
	Logout,
	Position,
	PutPlayer,
	RemovePlayer
}sc_PACKETTYPE;

typedef enum e_cs_PacketType
{
	TryLogin,
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
struct cs_packet_try_login {
	BYTE size;
	BYTE type;
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
struct sc_packet_login {
	BYTE size;
	BYTE type;
	BYTE numclient;
	INT id;
	float x;
	float y;
};
struct sc_packet_logout {
	BYTE size;
	BYTE type;
	INT id;
};
struct sc_packet_pos {
	BYTE size;
	BYTE type;
	INT id;
	float x;
	float y;
};
struct sc_packet_put_player {
	BYTE size;
	BYTE type;
	INT id;
	//TCHAR name[20];			// 아직 사용안함
	float x;
	float y;
};
struct sc_packet_remove_player {
	BYTE size;
	BYTE type;
	INT id;
};
#pragma pack(pop)
#endif // !__PROTOCOLS_H__