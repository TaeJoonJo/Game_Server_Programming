
#pragma once

#ifndef __PROTOCOLS_H__
#define __PROTOCOLS_H__

//#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_BUFFER        1024
#define SERVER_PORT        3500

constexpr int WORLD_WIDTH	= 300;
constexpr int WORLD_HEIGHT	= 300;

constexpr int MAX_USER = 10000;

constexpr int MAX_STR_LEN	= 50;
constexpr int MAX_NAME_LEN	= 10;

constexpr unsigned int NUM_NPC = 2000;

constexpr int MAP_SIZE = 21;
constexpr int MAP_OBSTACLE_TILE			= 0;
constexpr int MAP_GRASS_TILE			= 1;
constexpr int MAP_WAY_TILE				= 2;
constexpr int MAP_STONE_TILE			= 3;

constexpr int STATUS_PLUS_BUTTON_NUM = 6;

constexpr int NPC_PRIEST		= 0;
constexpr int NPC_FAIRY			= 1;
constexpr int NPC_TREE			= 2;
constexpr int NPC_MUSHROOM		= 3;
constexpr int NPC_MUD			= 4;
constexpr int NPC_BITE			= 5;

constexpr int NUM_ITEM = 10000;

constexpr int ITEM_HPPOTION = 1;
constexpr int ITEM_MPPOTION = 2;
constexpr int ITEM_MONEY	= 3;

constexpr int STAT_HP	= 0;
constexpr int STAT_MP	= 1;
constexpr int STAT_Str	= 2;
constexpr int STAT_Dex	= 3;
constexpr int STAT_Will	= 4;
constexpr int STAT_Int	= 5;

constexpr int NUM_SKILL = 10000;

constexpr int SKILL_ENERGYBALL	= 0;
constexpr int SKILL_FIREWALL	= 1;
constexpr int SKILL_FROZEN		= 2;

constexpr int DIR_UP		= 0;
constexpr int DIR_DOWN		= 1;
constexpr int DIR_LEFT		= 2;
constexpr int DIR_RIGHT		= 3;

typedef enum e_sc_PacketType
{
	Login_Ok,
	Login_Fail,
	Signup_Ok,
	Signup_Fail,
	Logout,
	PlayerInfo,
	PlayerItem,
	MovePlayer,
	PutPlayer,
	RemovePlayer,
	MoveNPC,
	PutNPC,
	RemoveNPC,
	PutItem,
	RemoveItem,
	PutSkill,
	MoveSkill,
	RemoveSkill,
	PlayerChat,
	NPCChat,
	NoticeChat,
	scIDLE
}sc_PACKETTYPE;

typedef enum e_cs_PacketType
{
	Login,
	Signup,
	Left,
	Right,
	Up,
	Down,
	IDLE,
	Attack,
	PickupItem,
	UseItem,
	Interact,
	StatPlus,
	Skill,
	ForTest,
	Teleport,
	MoneyBug,
	LevelBug
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
	wchar_t name[MAX_NAME_LEN];
};
struct cs_packet_signup {
	char size;
	char type;
	wchar_t name[MAX_NAME_LEN];
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
struct cs_packet_attack {
	char size;
	char type;
};
struct cs_packet_pickup_item {
	char size;
	char type;
};
struct cs_packet_use_item {
	char size;
	char type;
	char itemtype;
};
struct cs_packet_interact {
	char size;
	char type;
};
struct cs_packet_stat_plus {
	char size;
	char type;
	char stattype;
};
struct cs_packet_skill {
	char size;
	char type;
	char dir;
	char skilltype;
};

// Server to Client
struct sc_packet_base {
	char size;
	char type;
};
struct sc_packet_login_ok {
	char size;
	char type;
	int id;
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
	int id;
};
struct sc_packet_player_info {
	char size;
	char type;
	int level;
	int exp;
	int maxhp;
	int maxmp;
	int hp;
	int mp;
	int str;
	int dex;
	int will;
	int Int;
	int points;
};
struct sc_packet_player_item {
	char size;
	char type;
	int hppotionnum;
	int mppotionnum;
	int money;
};
struct sc_packet_move_player {
	char size;
	char type;
	int id;
	int x;
	int y;
};
struct sc_packet_put_player {
	char size;
	char type;
	int id;
	wchar_t name[MAX_NAME_LEN];			// 아직 사용안함
	int x;
	int y;
};
struct sc_packet_remove_player {
	char size;
	char type;
	int id;
};
struct sc_packet_move_npc {
	char size;
	char type;
	unsigned int id;
	int x;
	int y;
};
struct sc_packet_put_npc {
	char size;
	char type;
	unsigned int id;
	wchar_t name[MAX_NAME_LEN];			// 아직 사용안함
	int level;
	int x;
	int y;
	char npctype;
};
struct sc_packet_remove_npc {
	char size;
	char type;
	unsigned int id;
};
struct sc_packet_put_item {
	char size;
	char type;
	unsigned int id;
	int x;
	int y;
	char itemtype;
	char itemnum;
};
struct sc_packet_remove_item {
	char size;
	char type;
	unsigned int id;
};
struct sc_packet_put_skill {
	char size;
	char type;
	unsigned int id;
	int x;
	int y;
	char skilltype;
};
struct sc_packet_move_skill {
	char size;
	char type;
	unsigned int id;
	int x;
	int y;
};
struct sc_packet_remove_skill {
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
struct sc_packet_notice_chat {
	char size;
	char type;
	wchar_t msg[MAX_STR_LEN];
};

// For StressTest
struct cs_packet_teleport {
	char size;
	char type;
};
// For Debug
struct cs_packet_moneybug {
	char size;
	char type;
};

#pragma pack(pop)
#endif // !__PROTOCOLS_H__