
#pragma once

#ifndef __PROTOCOLS_H__
#define __PROTOCOLS_H__

//#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_BUFFER        1024
#define SERVER_PORT        3500

constexpr int WORLD_WIDTH = 800;
constexpr int WORLD_HEIGHT = 800;

constexpr int MAX_USER = 100;

constexpr int MAX_STR_LEN = 50;

constexpr unsigned int NUM_NPC = 20000;

constexpr int MAP_SIZE = 21;

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
	RemoveNPC,
	PlayerChat,
	NPCChat
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
	char size;
	char type;
};
struct cs_packet_login {
	char size;
	char type;
	wchar_t name[10];
};
struct cs_packet_signup {
	char size;
	char type;
	wchar_t name[10];
};
struct cs_packet_up {
	char size;
	char type;
};
struct cs_packet_down {
	char size;
	char type;
};
struct cs_packet_left {
	char size;
	char type;
};
struct cs_packet_right {
	char size;
	char type;
};
struct cs_packet_idle {
	char size;
	char type;
};

// Server to Client
struct sc_packet_base {
	char size;
	char type;
};
struct sc_packet_login_ok {
	char size;
	char type;
	char id;
};
struct sc_packet_login_fail {
	char size;
	char type;
};
struct sc_packet_signup_ok {
	char size;
	char type;
};
struct sc_packet_signup_fail {
	char size;
	char type;
};
struct sc_packet_logout {
	char size;
	char type;
	char id;
};
struct sc_packet_move_player {
	char size;
	char type;
	char id;
	float x;
	float y;
};
struct sc_packet_put_player {
	char size;
	char type;
	char id;
	//TCHAR name[20];			// 아직 사용안함
	float x;
	float y;
};
struct sc_packet_remove_player {
	char size;
	char type;
	char id;
};
struct sc_packet_move_npc {
	char size;
	char type;
	unsigned int id;
	float x;
	float y;
};
struct sc_packet_put_npc {
	char size;
	char type;
	unsigned int id;
	//TCHAR name[20];			// 아직 사용안함
	float x;
	float y;
};
struct sc_packet_remove_npc {
	char size;
	char type;
	unsigned int id;
};
struct sc_packet_player_chat {
	char size;
	char type;
	unsigned int id;
	wchar_t msg[MAX_STR_LEN];
};
struct sc_packet_npc_chat {
	char size;
	char type;
	unsigned int id;
	wchar_t msg[MAX_STR_LEN];
};
#pragma pack(pop)
#endif // !__PROTOCOLS_H__