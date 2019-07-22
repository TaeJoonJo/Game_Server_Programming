// PROG14_1_16b.CPP - DirectInput keyboard demo

// INCLUDES ///////////////////////////////////////////////

#include "Common.h"
#include "ClientInfo.h"

// DEFINES ////////////////////////////////////////////////

#define MAX(a,b)	((a)>(b))?(a):(b)
#define	MIN(a,b)	((a)<(b))?(a):(b)

// defines for windows 
#define WINDOW_CLASS_NAME L"WINXCLASS"  // class name

// 680 730

#define	BUF_SIZE				1024
#define	WM_SOCKET				WM_USER + 1

// PROTOTYPES /////////////////////////////////////////////

// game console
int Game_Init(void *parms = NULL);
int Game_Shutdown(void *parms = NULL);
int Game_Main(void *parms = NULL);

// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle = NULL; // save the window handle
HINSTANCE main_instance = NULL; // save the instance
char buffer[80];                // used to print text

								// demo globals
BOB			player;				// 플레이어 Unit
CClientInfo g_ClientInfo;		// 내 정보
BOB			npc[NUM_NPC];      // NPC Unit
BOB         skelaton[MAX_USER];     // the other player skelaton
BOB			item[NUM_ITEM];
BOB			skill[NUM_SKILL];

BITMAP_IMAGE reactor;      // the background   

BITMAP_IMAGE black_tile;
BITMAP_IMAGE white_tile;
BITMAP_IMAGE obstacle_tile;
BITMAP_IMAGE grass_tile;
BITMAP_IMAGE way_tile;
BITMAP_IMAGE stone_tile;
BITMAP_IMAGE quickslot;

BITMAP_IMAGE statusboard;
BITMAP_IMAGE statusplusbutton;

SOCKET g_mysocket;
WSABUF	send_wsabuf;
char 	send_buffer[BUF_SIZE];
WSABUF	recv_wsabuf;
char	recv_buffer[BUF_SIZE];
char	packet_buffer[BUF_SIZE];
DWORD		in_packet_size = 0;
int		saved_packet_size = 0;
int		g_myid;

float	g_left_x = 0;
float	g_top_y = 0;

int		g_MapData[WORLD_HEIGHT][WORLD_WIDTH];

bool	bIsLogin = false;

bool	g_bIsOpenStatus = false;

TCHAR	g_NoticeChat[MAX_STR_LEN] {};

int skill_dir = 99;

// FUNCTIONS //////////////////////////////////////////////
void SendLoginPacket(TCHAR* pname);
void SendSignupPacket(TCHAR* pname);
void TryLogin();
void TrySingup();

void ReadMap()
{
	ifstream mapFile("resources/map.txt");

	int x = 0, y = 0;
	char temp{};
	while (!mapFile.eof())
	{
		mapFile >> temp;
		g_MapData[x++][y] = (int)(temp - 48);
		if (x == WORLD_WIDTH) {
			++y;
			x = 0;
		}
	}
	g_MapData;

	return;
}

void ShowMenu()
{
	//system("clear");

	cout << "\n1. Login\n";
	cout << "\n2. Signup\n";
	cout << "\nInput : ";

	int input{};
	cin >> input;

	switch (input)
	{
	case 1:	TryLogin(); break;
	case 2: TrySingup(); break;
	default: break;
	}
}

void ShowStatus()
{
	if (g_bIsOpenStatus == false)
		return;

	//Set_Pos_BOB32(&status_board, player.x - 10, player.y - 8);
	//Draw_BOB32(&status_board);

	Draw_Bitmap32(&statusboard, STATUSBOARDX, STATUSBOARDY);
	for (int i = 0; i < STATUS_PLUS_BUTTON_NUM; ++i)
		Draw_Bitmap32(&statusplusbutton, STATUSPLUSBUTTONX, STATUSPLUSBUTTONY + (STATUSPLUSBUTTONYDIFF * i));

	wstring str;
	TCHAR temp[10]{};
	for (int i = 0; i < STATUS_PLUS_BUTTON_NUM; ++i) {
		//Set_Pos_BOB32(&status_plus_button[i], player.x - 2, player.y + (5 - (i * 2)));
		//Draw_BOB32(&status_plus_button[i]);
		switch (i)
		{
		case 5:		// Int
		{
			_itow_s(g_ClientInfo.m_nInt, temp, 10);
		} break;
		case 4:		// Will
		{
			_itow_s(g_ClientInfo.m_nWill, temp, 10);
		} break;
		case 3:		// Dex
		{
			_itow_s(g_ClientInfo.m_nDex, temp, 10);
		} break;
		case 2:		// Str
		{
			_itow_s(g_ClientInfo.m_nStr, temp, 10);
		} break;
		case 1:		// MP
		{
			_itow_s(g_ClientInfo.m_nMaxMP, temp, 10);
		} break;
		case 0:		// HP
		{
			_itow_s(g_ClientInfo.m_nMaxHP, temp, 10);
		} break;
		}
		str = temp;
		Draw_Text_D3D((wchar_t*)str.c_str(), STATUSBOARDX + 150, STATUSPLUSBUTTONY + (STATUSPLUSBUTTONYDIFF * i), D3DCOLOR_ARGB(255, 0, 255, 255));
	}

	str = L"Points : ";
	_itow_s(g_ClientInfo.m_nStatPoints, temp, 10);
	str += temp;
	Draw_Text_D3D((wchar_t*)str.c_str(), STATUSBOARDX + 120, STATUSPLUSBUTTONY + 450, D3DCOLOR_ARGB(255, 0, 255, 255));
}

void TryLogin()
{
	wstring nameStr{};
	while (1) {
		cout << "Login ID : ";
		wcin >> nameStr;

		if (nameStr.length() == 0)
			continue;
		TCHAR* name = (TCHAR*)nameStr.c_str();

		SendLoginPacket(name);
		break;
	}
}

void TrySingup()
{
	wstring nameStr{};
	while (1) {
		cout << "Signup ID : ";
		wcin >> nameStr;

		if (nameStr.length() == 0)
			continue;
		TCHAR * name = (TCHAR*)nameStr.c_str();

		SendSignupPacket(name);
		break;
	}
}

void SendLoginPacket(TCHAR* pname)
{
	ZeroMemory(send_buffer, BUF_SIZE);
	cs_packet_login* my_packet = reinterpret_cast<cs_packet_login*>(send_buffer);
	int namelen = wcslen(pname) * 2 + 2;
	memcpy(my_packet->name, pname, namelen);
	my_packet->size = sizeof(cs_packet_login);
	send_wsabuf.len = sizeof(cs_packet_login);
	DWORD iobyte;
	my_packet->type = e_cs_PacketType::Login;
	int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

void SendSignupPacket(TCHAR* pname)
{
	ZeroMemory(send_buffer, BUF_SIZE);
	cs_packet_signup* my_packet = reinterpret_cast<cs_packet_signup*>(send_buffer);
	int namelen = wcslen(pname) * 2 + 2;
	memcpy(my_packet->name, pname, namelen);
	my_packet->size = sizeof(cs_packet_signup);
	send_wsabuf.len = sizeof(cs_packet_signup);
	DWORD iobyte;
	my_packet->type = e_cs_PacketType::Signup;
	int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

void SendAttackPacket()
{
	ZeroMemory(send_buffer, BUF_SIZE);
	cs_packet_attack* my_packet = reinterpret_cast<cs_packet_attack*>(send_buffer);
	my_packet->size = sizeof(cs_packet_attack);
	send_wsabuf.len = sizeof(cs_packet_attack);
	DWORD iobyte;
	my_packet->type = e_cs_PacketType::Attack;
	int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

void SendPickupItemPacket()
{
	ZeroMemory(send_buffer, BUF_SIZE);
	cs_packet_pickup_item* my_packet = reinterpret_cast<cs_packet_pickup_item*>(send_buffer);
	my_packet->size = sizeof(cs_packet_pickup_item);
	send_wsabuf.len = sizeof(cs_packet_pickup_item);
	DWORD iobyte;
	my_packet->type = e_cs_PacketType::PickupItem;
	int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

void SendUseItemPacket(int itemtype)
{
	ZeroMemory(send_buffer, BUF_SIZE);
	cs_packet_use_item* my_packet = reinterpret_cast<cs_packet_use_item*>(send_buffer);
	my_packet->size = sizeof(cs_packet_use_item);
	my_packet->itemtype = itemtype;
	send_wsabuf.len = sizeof(cs_packet_use_item);
	DWORD iobyte;
	my_packet->type = e_cs_PacketType::UseItem;
	int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

void SendInteractPacket()
{
	ZeroMemory(send_buffer, BUF_SIZE);
	cs_packet_interact* my_packet = reinterpret_cast<cs_packet_interact*>(send_buffer);
	my_packet->size = sizeof(cs_packet_interact);
	send_wsabuf.len = sizeof(cs_packet_interact);
	DWORD iobyte;
	my_packet->type = e_cs_PacketType::Interact;
	int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

void SendStatPlusPacket(int stattype)
{
	ZeroMemory(send_buffer, BUF_SIZE);
	cs_packet_stat_plus* my_packet = reinterpret_cast<cs_packet_stat_plus*>(send_buffer);
	my_packet->size = sizeof(cs_packet_stat_plus);
	my_packet->stattype = stattype;
	send_wsabuf.len = sizeof(cs_packet_stat_plus);
	DWORD iobyte;
	my_packet->type = e_cs_PacketType::StatPlus;
	int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

void SendSkillPacket(int dir, int skilltype)
{
	ZeroMemory(send_buffer, BUF_SIZE);
	cs_packet_skill* my_packet = reinterpret_cast<cs_packet_skill*>(send_buffer);
	my_packet->size = sizeof(cs_packet_skill);
	my_packet->skilltype = skilltype;
	my_packet->dir = dir;
	send_wsabuf.len = sizeof(cs_packet_skill);
	DWORD iobyte;
	my_packet->type = e_cs_PacketType::Skill;
	int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

void SendDebugPacket(e_cs_PacketType type)
{
	ZeroMemory(send_buffer, BUF_SIZE);
	cs_packet_moneybug* my_packet = reinterpret_cast<cs_packet_moneybug*>(send_buffer);
	my_packet->size = sizeof(cs_packet_moneybug);
	send_wsabuf.len = sizeof(cs_packet_moneybug);
	DWORD iobyte;
	my_packet->type = type;
	int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

void ProcessPacket(char *ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case e_sc_PacketType::Login_Ok:
	{
		cout << "Login OK!\n";
		sc_packet_login_ok* packet =
			reinterpret_cast<sc_packet_login_ok*>(ptr);
		bIsLogin = true;
		g_myid = packet->id;
	} break;
	case e_sc_PacketType::Login_Fail:
	{
		cout << "Login Fail!\n";
		ShowMenu();
	} break;
	case e_sc_PacketType::Signup_Ok:
	{
		cout << "Signup OK!\n";
		ShowMenu();
	} break;
	case e_sc_PacketType::Signup_Fail:
	{
		cout << "Signup Fail!\n";
		ShowMenu();
	} break;
	case e_sc_PacketType::PlayerInfo:
	{
		sc_packet_player_info* my_packet = reinterpret_cast<sc_packet_player_info*>(ptr);

		g_ClientInfo.m_nLevel = my_packet->level;
		g_ClientInfo.m_nExp = my_packet->exp;
		g_ClientInfo.m_nMaxHP = my_packet->maxhp;
		g_ClientInfo.m_nMaxMP = my_packet->maxmp;
		g_ClientInfo.m_nHP = my_packet->hp;
		g_ClientInfo.m_nMP = my_packet->mp;
		g_ClientInfo.m_nStr = my_packet->str;
		g_ClientInfo.m_nDex = my_packet->dex;
		g_ClientInfo.m_nWill = my_packet->will;
		g_ClientInfo.m_nInt = my_packet->Int;
		g_ClientInfo.m_nStatPoints = my_packet->points;
	} break;
	case e_sc_PacketType::PlayerItem:
	{
		sc_packet_player_item* my_packet = reinterpret_cast<sc_packet_player_item*>(ptr);

		g_ClientInfo.m_nHPPotionNum = my_packet->hppotionnum;
		g_ClientInfo.m_nMPPotionNum = my_packet->mppotionnum;
		g_ClientInfo.m_nMoney = my_packet->money;
	} break;
	case e_sc_PacketType::PutPlayer:
	{
		sc_packet_put_player *my_packet = reinterpret_cast<sc_packet_put_player *>(ptr);
		int id = my_packet->id;
		TCHAR* name = my_packet->name;
		if (id == g_myid) {
			g_left_x = my_packet->x - MAP_SIZE / 2;

			g_top_y = my_packet->y - MAP_SIZE / 2;
			player.x = my_packet->x;
			player.y = my_packet->y;
			player.attr |= BOB_ATTR_VISIBLE;

			g_ClientInfo.m_nX = player.x;
			g_ClientInfo.m_nY = player.y;

			memcpy(g_ClientInfo.m_cName, name, wcslen(name) * 2 + 2);

			wsprintf(player.message, L"%s", my_packet->name);
		}
		else if (id < MAX_USER) {
			skelaton[id].x = my_packet->x;
			skelaton[id].y = my_packet->y;
			skelaton[id].attr |= BOB_ATTR_VISIBLE;

			wsprintf(skelaton[id].message, L"%s", my_packet->name);
		}
		else {
			npc[id].x = my_packet->x;
			npc[id].y = my_packet->y;
			npc[id].attr |= BOB_ATTR_VISIBLE;
		}
	} break;
	case e_sc_PacketType::MovePlayer:
	{
		sc_packet_move_player *my_packet = reinterpret_cast<sc_packet_move_player *>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			g_left_x = my_packet->x - MAP_SIZE / 2;
			g_top_y = my_packet->y - MAP_SIZE / 2;
			player.x = my_packet->x;
			player.y = my_packet->y;

			g_ClientInfo.m_nX = player.x;
			g_ClientInfo.m_nY = player.y;
		}
		else if (other_id < MAX_USER) {
			skelaton[other_id].x = my_packet->x;
			skelaton[other_id].y = my_packet->y;
		}
		else {
			npc[other_id].x = my_packet->x;
			npc[other_id].y = my_packet->y;
		}
	} break;
	case e_sc_PacketType::RemovePlayer:
	{
		sc_packet_remove_player* my_packet = reinterpret_cast<sc_packet_remove_player*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			player.attr &= ~BOB_ATTR_VISIBLE;
		}
		else if (other_id < MAX_USER) {
			skelaton[other_id].attr &= ~BOB_ATTR_VISIBLE;
		}
		else {
			npc[other_id].attr &= ~BOB_ATTR_VISIBLE;
		}
	} break;
	case e_sc_PacketType::MoveNPC:
	{
		sc_packet_move_npc* my_packet = reinterpret_cast<sc_packet_move_npc*>(ptr);
		DWORD other_id = my_packet->id;

		npc[other_id].x = my_packet->x;
		npc[other_id].y = my_packet->y;
	} break;
	case e_sc_PacketType::PutNPC:
	{
		sc_packet_put_npc* my_packet = reinterpret_cast<sc_packet_put_npc*>(ptr);
		DWORD id = my_packet->id;

		int type = (int)my_packet->npctype;
		npc[id].type = type;
		
		switch (type)
		{
		case NPC_FAIRY:		Load_Frame_BOB32(&npc[id], FAIRY_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);	break;
		case NPC_PRIEST:	Load_Frame_BOB32(&npc[id], PRIEST_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);	break;
		case NPC_TREE:		Load_Frame_BOB32(&npc[id], TREE_MONSTER_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);	break;
		case NPC_MUSHROOM:	Load_Frame_BOB32(&npc[id], MUSHROOM_MONSTER_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);	break;
		case NPC_MUD:		Load_Frame_BOB32(&npc[id], MUD_MONSTER_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);	break;
		case NPC_BITE:		Load_Frame_BOB32(&npc[id], BITE_MONSTER_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);	break;
		}

		wsprintf(npc[id].message, L"%3d\n%s", my_packet->level, my_packet->name);
		//wcsncpy_s(npc[id].message, my_packet->name, 256);

		npc[id].x = my_packet->x;
		npc[id].y = my_packet->y;
		npc[id].attr |= BOB_ATTR_VISIBLE;
	} break;
	case e_sc_PacketType::RemoveNPC:
	{
		sc_packet_remove_npc* my_packet = reinterpret_cast<sc_packet_remove_npc*>(ptr);
		DWORD other_id = my_packet->id;

		npc[other_id].attr &= ~BOB_ATTR_VISIBLE;
	} break;
	case e_sc_PacketType::PutItem:
	{
		sc_packet_put_item* my_packet = reinterpret_cast<sc_packet_put_item*>(ptr);
		DWORD item_id = my_packet->id;
		int type = my_packet->itemtype;
		
		switch (type)
		{
		case ITEM_HPPOTION:		Load_Frame_BOB32(&item[item_id], HPPOTION_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);	break;
		case ITEM_MPPOTION:		Load_Frame_BOB32(&item[item_id], MPPOTION_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);	break;
		case ITEM_MONEY:		Load_Frame_BOB32(&item[item_id], MONEY_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);	break;
		}

		item[item_id].x = my_packet->x;
		item[item_id].y = my_packet->y;

		wsprintf(item[item_id].message, L"%3d", my_packet->itemnum);

		item[item_id].attr |= BOB_ATTR_VISIBLE;
	} break;
	case e_sc_PacketType::RemoveItem:
	{
		sc_packet_remove_item* my_packet = reinterpret_cast<sc_packet_remove_item*>(ptr);
		DWORD item_id = my_packet->id;

		item[item_id].attr &= ~BOB_ATTR_VISIBLE;
	} break;
	case e_sc_PacketType::PutSkill:
	{
		sc_packet_put_skill* my_packet = reinterpret_cast<sc_packet_put_skill*>(ptr);
		DWORD id = my_packet->id;

		int type = (int)my_packet->skilltype;
		skill[id].type = type;
		
		int wid{ 0 }, hei{ 0 };
		int x{ 0 }, y{ 0 };
		x = my_packet->x;
		y = my_packet->y;
		switch (type)
		{
		case SKILL_ENERGYBALL:	wid = 31; hei = 32;		Load_Frame_BOB32(&skill[id], SKILL_ENERGYBALL_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);	break;
		case SKILL_FIREWALL:	wid = 31; hei = 32;		Load_Frame_BOB32(&skill[id], SKILL_FIREWALL_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);	break;
		case SKILL_FROZEN:		wid = 31 * 5; hei = 32 * 5; x -= 2; y -= 2;	Load_Frame_BOB32(&skill[id], SKILL_FROZEN_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);	break;
		}
		skill[id].width = wid;
		skill[id].height = hei;
		skill[id].x = x;
		skill[id].y = y;
		skill[id].attr |= BOB_ATTR_VISIBLE;
	} break;
	case e_sc_PacketType::MoveSkill:
	{
		sc_packet_move_skill* my_packet = reinterpret_cast<sc_packet_move_skill*>(ptr);
		DWORD other_id = my_packet->id;
		
		skill[other_id].x = my_packet->x;
		skill[other_id].y = my_packet->y;
	} break;
	case e_sc_PacketType::RemoveSkill:
	{
		
		sc_packet_remove_skill* my_packet = reinterpret_cast<sc_packet_remove_skill*>(ptr);
		DWORD skill_id = my_packet->id;
		
		skill[skill_id].attr &= ~BOB_ATTR_VISIBLE;
	} break;
	case e_sc_PacketType::NPCChat:
	{
		sc_packet_npc_chat* my_packet = reinterpret_cast<sc_packet_npc_chat*>(ptr);
		int npc_id = my_packet->id;

		wcsncpy_s(npc[npc_id].message, my_packet->msg, 256);
		npc[npc_id].message_time = GetTickCount64();
	}  break;
	case e_sc_PacketType::PlayerChat:
	{

	} break;
	case e_sc_PacketType::NoticeChat:
	{
		sc_packet_notice_chat* my_packet = reinterpret_cast<sc_packet_notice_chat*>(ptr);

		wcsncpy_s(g_NoticeChat, my_packet->msg, MAX_STR_LEN);

		wcout << my_packet->msg << "\n";
	} break;
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
	}
}

void ReadPacket(SOCKET sock)
{
	DWORD iobyte, ioflag = 0;

	int ret = WSARecv(sock, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
	if (ret) {
		int err_code = WSAGetLastError();
		printf("Recv Error [%d]\n", err_code);
	}

	BYTE *ptr = reinterpret_cast<BYTE *>(recv_buffer);

	while (0 != iobyte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (iobyte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			iobyte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, iobyte);
			saved_packet_size += iobyte;
			iobyte = 0;
		}
	}
}

void clienterror()
{
	exit(-1);
}

LRESULT CALLBACK WindowProc(HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam)
{
	// this is the main message handler of the system
	PAINTSTRUCT	ps;		   // used in WM_PAINT
	HDC			hdc;	   // handle to a device context

						   // what is the message 
	switch (msg)
	{
	case WM_LBUTTONDOWN:
	{
		if (g_bIsOpenStatus == false) break;
		POINT mousepos;
		GetCursorPos(&mousepos);
		ScreenToClient(hwnd, &mousepos);

		int bx = STATUSPLUSBUTTONX, by= STATUSPLUSBUTTONY;
		for (int i = 0; i < STATUS_PLUS_BUTTON_NUM; ++i) {
			by = STATUSPLUSBUTTONY + (STATUSPLUSBUTTONYDIFF * i);
			if (mousepos.x >= bx && mousepos.x <= bx + STATUSPLUSBUTTONSIZE) {
				if (mousepos.y >= by && mousepos.y <= by + STATUSPLUSBUTTONSIZE) {
					SendStatPlusPacket(i);
					break;
				}
			}
		}
	} break;
	case WM_KEYDOWN: 
	{
		if (wparam == VK_ESCAPE)
		{
			PostMessage(main_window_handle, WM_DESTROY, 0, 0);
		}
		// Open Status UI
		if (wparam == 'p' || wparam == 'P')
		{
			//cout << "Press p\n";
			if (g_bIsOpenStatus == false)
				g_bIsOpenStatus = true;
			else
				g_bIsOpenStatus = false;
			break;
		}
		if (wparam == 'a' || wparam == 'A')
		{
			SendAttackPacket();
			break;
		}
		if (wparam == 'z' || wparam == 'Z')
		{
			SendPickupItemPacket();
			break;
		}
		if (wparam == 'q' || wparam == 'Q')
		{
			SendUseItemPacket(ITEM_HPPOTION);
			break;
		}
		if (wparam == 'w' || wparam == 'W')
		{
			SendUseItemPacket(ITEM_MPPOTION);
			break;
		}
		if (wparam == VK_SPACE)
		{
			SendInteractPacket(); 
			break;
		}
		
		if (wparam == VK_NUMPAD8) skill_dir = DIR_UP;
		if (wparam == VK_NUMPAD2) skill_dir = DIR_DOWN;
		if (wparam == VK_NUMPAD4) skill_dir = DIR_LEFT;
		if (wparam == VK_NUMPAD6) skill_dir = DIR_RIGHT;
		
		if (skill_dir != 99) {
			if (wparam == 'x' || wparam == 'X')
			{
				SendSkillPacket(skill_dir, SKILL_ENERGYBALL);
				break;
			}
			if (wparam == 'c' || wparam == 'C')
			{
				SendSkillPacket(skill_dir, SKILL_FIREWALL);
				break;
			}
			if (wparam == 'v' || wparam == 'V')
			{
				SendSkillPacket(skill_dir, SKILL_FROZEN);
				break;
			}
		}

		// For Debug
		{
			if (wparam == 'm' || wparam == 'M')
			{
				SendDebugPacket(e_cs_PacketType::MoneyBug);
				break;
			}
			if (wparam == 'l' || wparam == 'L')
			{
				SendDebugPacket(e_cs_PacketType::LevelBug);
				break;
			}
		}
		int x = 0, y = 0;
		if (wparam == VK_RIGHT)	x += 1;
		if (wparam == VK_LEFT)	x -= 1;
		if (wparam == VK_UP)	y -= 1;
		if (wparam == VK_DOWN)	y += 1;
		if (x != 0 || y != 0) {
			cs_packet_up* my_packet = reinterpret_cast<cs_packet_up*>(send_buffer);
			my_packet->size = sizeof(my_packet);
			send_wsabuf.len = sizeof(my_packet);
			DWORD iobyte;
			if (0 != x) {
				if (1 == x) my_packet->type = e_cs_PacketType::Right;
				else my_packet->type = e_cs_PacketType::Left;
				int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
				if (ret) {
					int error_code = WSAGetLastError();
					printf("Error while sending packet [%d]", error_code);
				}
			}
			if (0 != y) {
				if (1 == y) my_packet->type = e_cs_PacketType::Down;
				else my_packet->type = e_cs_PacketType::Up;
				WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
			}
		}
		

	} break;
	case WM_KEYUP:
	{
		
		if (wparam == VK_NUMPAD8) skill_dir = 99;
		if (wparam == VK_NUMPAD4) skill_dir = 99;
		if (wparam == VK_NUMPAD2) skill_dir = 99;
		if (wparam == VK_NUMPAD6) skill_dir = 99;
	} break;
	case WM_CREATE:
	{
		// do initialization stuff here
		return(0);
	} break;

	case WM_PAINT:
	{
		// start painting
		hdc = BeginPaint(hwnd, &ps);

		// end painting
		EndPaint(hwnd, &ps);
		return(0);
	} break;

	case WM_DESTROY:
	{
		// kill the application			
		PostQuitMessage(0);
		return(0);
	} break;
	case WM_SOCKET:
	{
		if (WSAGETSELECTERROR(lparam)) {
			closesocket((SOCKET)wparam);
			clienterror();
			break;
		}
		switch (WSAGETSELECTEVENT(lparam)) {
		case FD_READ:
			ReadPacket((SOCKET)wparam);
			break;
		case FD_CLOSE:
			closesocket((SOCKET)wparam);
			clienterror();
			break;
		}
	}

	default:break;

	} // end switch

	  // process any messages that we didn't take care of 
	return (DefWindowProc(hwnd, msg, wparam, lparam));

} // end WinProc

  // WINMAIN ////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE hprevinstance,
	LPSTR lpcmdline,
	int ncmdshow)
{
	// this is the winmain function

	WNDCLASS winclass;	// this will hold the class we create
	HWND	 hwnd;		// generic window handle
	MSG		 msg;		// generic message


						// first fill in the window class stucture
	winclass.style = CS_DBLCLKS | CS_OWNDC |
		CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hinstance;
	winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = WINDOW_CLASS_NAME;

	// register the window class
	if (!RegisterClass(&winclass))
		return(0);

	// create the window, note the use of WS_POPUP
	if (!(hwnd = CreateWindow(WINDOW_CLASS_NAME, // class
		L"Chess Client",	 // title
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0, 0,	   // x,y
		WINDOW_WIDTH,  // width
		WINDOW_HEIGHT, // height
		NULL,	   // handle to parent 
		NULL,	   // handle to menu
		hinstance,// instance
		NULL)))	// creation parms
		return(0);

	// save the window handle and instance in a global
	main_window_handle = hwnd;
	main_instance = hinstance;

	// perform all game console specific initialization
	Game_Init();
	
	// enter main event loop
	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// test if this is a quit
			if (msg.message == WM_QUIT)
				break;

			// translate any accelerator keys
			TranslateMessage(&msg);

			// send the message to the window proc
			DispatchMessage(&msg);
		} // end if
		if(bIsLogin == true)
		  // main game processing goes here
			Game_Main();

	} // end while

	  // shutdown game and release all resources
	Game_Shutdown();

	// return to Windows like this
	return(msg.wParam);

} // end WinMain

  ///////////////////////////////////////////////////////////

  // WINX GAME PROGRAMMING CONSOLE FUNCTIONS ////////////////

int Game_Init(void *parms)
{
	// this function is where you do all the initialization 
	// for your game

	wcout.imbue(locale("korean"));

	// set up screen dimensions
	screen_width = WINDOW_WIDTH;
	screen_height = WINDOW_HEIGHT;
	screen_bpp = 32;

	// initialize directdraw
	DD_Init(screen_width, screen_height, screen_bpp);

	ReadMap();

	// 531 532
	// create and load the reactor bitmap image
	Create_Bitmap32(&reactor, 0, 0, 531, 532);
	Create_Bitmap32(&black_tile, 0, 0, 531, 532);
	Create_Bitmap32(&white_tile, 0, 0, 531, 532);
	Create_Bitmap32(&obstacle_tile, 0, 0, 60, 60);
	Create_Bitmap32(&stone_tile, 0, 0, 1024, 1024);
	Create_Bitmap32(&grass_tile, 0, 0, 32, 32);
	Create_Bitmap32(&way_tile, 0, 0, 1280, 1280);
	Create_Bitmap32(&quickslot, 0, 0, 400, 100);
	Create_Bitmap32(&statusboard, 0, 0, 400, 600);
	Create_Bitmap32(&statusplusbutton, 0, 0, 50, 50);
	Load_Image_Bitmap32(&reactor, L"resources/CHESSMAP.BMP", 0, 0, BITMAP_EXTRACT_MODE_ABS);
	Load_Image_Bitmap32(&black_tile, L"resources/CHESSMAP.BMP", 0, 0, BITMAP_EXTRACT_MODE_ABS);
	black_tile.x = 69;
	black_tile.y = 5;
	black_tile.height = TILE_WIDTH;
	black_tile.width = TILE_WIDTH;
	Load_Image_Bitmap32(&white_tile, L"resources/CHESSMAP.BMP", 0, 0, BITMAP_EXTRACT_MODE_ABS);
	white_tile.x = 5;
	white_tile.y = 5;
	white_tile.height = TILE_WIDTH;
	white_tile.width = TILE_WIDTH;
	Load_Image_Bitmap32(&obstacle_tile, L"resources/OBSTACLE_TILE.BMP", 0, 0, BITMAP_EXTRACT_MODE_ABS);
	obstacle_tile.x = 0;
	obstacle_tile.y = 0;
	obstacle_tile.height = TILE_WIDTH;
	obstacle_tile.width = TILE_WIDTH;
	Load_Image_Bitmap32(&grass_tile, L"resources/GRASS_TILE.BMP", 0, 0, BITMAP_EXTRACT_MODE_ABS);
	grass_tile.x = 0;
	grass_tile.y = 0;
	grass_tile.height = TILE_WIDTH;
	grass_tile.width = TILE_WIDTH;
	Load_Image_Bitmap32(&way_tile, L"resources/WAY_TILE.BMP", 0, 0, BITMAP_EXTRACT_MODE_ABS);
	way_tile.x = 0;
	way_tile.y = 0;
	way_tile.height = TILE_WIDTH;
	way_tile.width = TILE_WIDTH;
	Load_Image_Bitmap32(&stone_tile, L"resources/STONE_TILE.BMP", 0, 0, BITMAP_EXTRACT_MODE_ABS);
	stone_tile.x = 0;
	stone_tile.y = 0;
	stone_tile.height = TILE_WIDTH;
	stone_tile.width = TILE_WIDTH;
	Load_Image_Bitmap32(&quickslot, L"resources/QUICKSLOT.PNG", 0, 0, BITMAP_EXTRACT_MODE_ABS);
	quickslot.x = 0;
	quickslot.y = 0;
	//quickslot.height = TILE_WIDTH * 10;
	//quickslot.width = TILE_WIDTH * 20;
	Load_Image_Bitmap32(&statusboard, L"resources/STATUSUIBOARD.PNG", 0, 0, BITMAP_EXTRACT_MODE_ABS);
	statusboard.x = 0;
	statusboard.y = 0;
	Load_Image_Bitmap32(&statusplusbutton, L"resources/STATUSPLUSBUTTON.PNG", 0, 0, BITMAP_EXTRACT_MODE_ABS);
	statusplusbutton.x = 0;
	statusplusbutton.y = 0;

	// now let's load in all the frames for the skelaton!!!

	Load_Texture(L"resources/CHESS2.PNG", UNIT_TEXTURE, 384 / 2, 64 / 2);

	if (!Create_BOB32(&player, 0, 0, 31, 32, 1, BOB_ATTR_SINGLE_FRAME)) return(0);
	Load_Frame_BOB32(&player, UNIT_TEXTURE, 0, 2, 0, BITMAP_EXTRACT_MODE_CELL);

	// set up texture for npc
	Load_Texture(L"resources/PRIEST.PNG", PRIEST_TEXTURE, 31, 32);//32, 32);
	Load_Texture(L"resources/FAIRY.PNG", FAIRY_TEXTURE, 31, 32);//105, 114);
	Load_Texture(L"resources/TREE_MONSTER.PNG", TREE_MONSTER_TEXTURE, 31, 32);//500, 500);
	Load_Texture(L"resources/MUSHROOM_MONSTER.PNG", MUSHROOM_MONSTER_TEXTURE, 31, 32);//564, 553);
	Load_Texture(L"resources/MUD_MONSTER.PNG", MUD_MONSTER_TEXTURE, 31, 32);//800, 582);
	Load_Texture(L"resources/BITE_MONSTER.PNG", BITE_MONSTER_TEXTURE, 31, 32);//543, 498);

	// set up texture for item
	Load_Texture(L"resources/HP_POTION.PNG", HPPOTION_TEXTURE, 31, 32);
	Load_Texture(L"resources/MP_POTION.PNG", MPPOTION_TEXTURE, 31, 32);
	Load_Texture(L"resources/MONEY.PNG", MONEY_TEXTURE, 31, 32);

	// set up texture for item
	Load_Texture(L"resources/SKILL_ENERGYBALL.PNG", SKILL_ENERGYBALL_TEXTURE, 31, 32);
	Load_Texture(L"resources/SKILL_FIREWALL.PNG", SKILL_FIREWALL_TEXTURE, 31, 32);
	Load_Texture(L"resources/SKILL_FROZEN.PNG", SKILL_FROZEN_TEXTURE, 31 * 5, 32 * 5);

	// set up stating state of skelaton
	Set_Animation_BOB32(&player, 0);
	Set_Anim_Speed_BOB32(&player, 4);
	Set_Vel_BOB32(&player, 0, 0);
	Set_Pos_BOB32(&player, 0, 0);

	// create skelaton bob
	for (int i = 0; i < MAX_USER; ++i) {
		if (!Create_BOB32(&skelaton[i], 0, 0, 31, 32, 1, BOB_ATTR_SINGLE_FRAME))
			return(0);
		Load_Frame_BOB32(&skelaton[i], UNIT_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);

		// set up stating state of skelaton
		Set_Animation_BOB32(&skelaton[i], 0);
		Set_Anim_Speed_BOB32(&skelaton[i], 4);
		Set_Vel_BOB32(&skelaton[i], 0, 0);
		Set_Pos_BOB32(&skelaton[i], 0, 0);
	}

	// create skelaton bob
	for (DWORD i = 0; i < NUM_NPC; ++i) {
		if (!Create_BOB32(&npc[i], 0, 0, 31, 32, 1, BOB_ATTR_SINGLE_FRAME))
			return(0);

		// set up stating state of skelaton
		Set_Animation_BOB32(&npc[i], 0);
		Set_Anim_Speed_BOB32(&npc[i], 4);
		Set_Vel_BOB32(&npc[i], 0, 0);
		Set_Pos_BOB32(&npc[i], 0, 0);
	}

	// create skelaton bob
	for (DWORD i = 0; i < NUM_SKILL; ++i) {
		if (!Create_BOB32(&skill[i], 0, 0, 31, 32, 1, BOB_ATTR_SINGLE_FRAME))
			return(0);

		// set up stating state of skelaton
		Set_Animation_BOB32(&skill[i], 0);
		Set_Anim_Speed_BOB32(&skill[i], 4);
		Set_Vel_BOB32(&skill[i], 0, 0);
		Set_Pos_BOB32(&skill[i], 0, 0);
	}


	for (DWORD i = 0; i < NUM_ITEM; ++i) {
		if (!Create_BOB32(&item[i], 0, 0, 31, 32, 1, BOB_ATTR_SINGLE_FRAME))
			return(0);

		// set up stating state of skelaton
		Set_Animation_BOB32(&item[i], 0);
		Set_Anim_Speed_BOB32(&item[i], 0);
		Set_Vel_BOB32(&item[i], 0, 0);
		Set_Pos_BOB32(&item[i], 0, 0);
	}

	// set clipping rectangle to screen extents so mouse cursor
	// doens't mess up at edges
	//RECT screen_rect = {0,0,screen_width,screen_height};
	//lpddclipper = DD_Attach_Clipper(lpddsback,1,&screen_rect);

	// hide the mouse
	//ShowCursor(FALSE);


	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_mysocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(SERVER_PORT);
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int Result = WSAConnect(g_mysocket, (sockaddr *)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);

	WSAAsyncSelect(g_mysocket, main_window_handle, WM_SOCKET, FD_CLOSE | FD_READ);

	send_wsabuf.buf = send_buffer;
	send_wsabuf.len = BUF_SIZE;
	recv_wsabuf.buf = recv_buffer;
	recv_wsabuf.len = BUF_SIZE;

	ShowMenu();

	// return success
	return(1);

} // end Game_Init

  ///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms)
{
	// this function is where you shutdown your game and
	// release all resources that you allocated

	// kill the reactor
	Destroy_Bitmap32(&black_tile);
	Destroy_Bitmap32(&white_tile);
	Destroy_Bitmap32(&grass_tile);
	Destroy_Bitmap32(&way_tile);
	Destroy_Bitmap32(&stone_tile);
	Destroy_Bitmap32(&obstacle_tile);
	Destroy_Bitmap32(&quickslot);
	Destroy_Bitmap32(&statusboard);
	Destroy_Bitmap32(&statusplusbutton);

	// kill skelaton
	for (int i = 0; i < MAX_USER; ++i) Destroy_BOB32(&skelaton[i]);
	for (int i = 0; i < NUM_NPC; ++i) Destroy_BOB32(&npc[i]);
	for (int i = 0; i < NUM_ITEM; ++i) Destroy_BOB32(&item[i]);
	for (int i = 0; i < NUM_SKILL; ++i) Destroy_BOB32(&skill[i]);

	// shutdonw directdraw
	DD_Shutdown();

	WSACleanup();

	// return success
	return(1);
} // end Game_Shutdown

  ///////////////////////////////////////////////////////////

int Game_Main(void *parms)
{
	// this is the workhorse of your game it will be called
	// continuously in real-time this is like main() in C
	// all the calls for you game go here!
	// check of user is trying to exit
	//if (KEY_DOWN(VK_ESCAPE) || KEY_DOWN(VK_SPACE))
	//	PostMessage(main_window_handle, WM_DESTROY, 0, 0);

	// start the timing clock
	Start_Clock();

	// clear the drawing surface
	DD_Fill_Surface(D3DCOLOR_ARGB(255, 0, 0, 0));

	// get player input

	g_pd3dDevice->BeginScene();
	g_pSprite->Begin(D3DXSPRITE_ALPHABLEND);
	
	// draw the background reactor image
	for (int i = 0; i < MAP_SIZE; ++i)
	{
		for (int j = 0; j < MAP_SIZE; ++j)
		{
			int tile_x = i + g_left_x;
			int tile_y = j + g_top_y;
			if ((tile_x < 0) || (tile_y < 0)) continue;
			if (tile_x > WORLD_WIDTH - 1) continue;
			if (tile_y > WORLD_HEIGHT - 1) continue;
			//if (((tile_x) % 2) == ((tile_y) % 2))
			//	Draw_Bitmap32(&white_tile, TILE_WIDTH * i + (TILE_WIDTH / 2), TILE_WIDTH * j + (TILE_WIDTH / 2));
			//else
			//	Draw_Bitmap32(&black_tile, TILE_WIDTH * i + (TILE_WIDTH / 2), TILE_WIDTH * j + (TILE_WIDTH / 2));
			if(g_MapData[tile_x][tile_y] == MAP_WAY_TILE)
				Draw_Bitmap32(&way_tile, TILE_WIDTH * i + (TILE_WIDTH / 2), TILE_WIDTH * j + (TILE_WIDTH / 2) + g_HeightWeight);
			else if (g_MapData[tile_x][tile_y] == MAP_OBSTACLE_TILE) {
				Draw_Bitmap32(&white_tile, TILE_WIDTH * i + (TILE_WIDTH / 2), TILE_WIDTH * j + (TILE_WIDTH / 2) + g_HeightWeight);
				Draw_Bitmap32(&obstacle_tile, TILE_WIDTH * i + (TILE_WIDTH / 2), TILE_WIDTH * j + (TILE_WIDTH / 2) + g_HeightWeight);
			}
			else if (g_MapData[tile_x][tile_y] == MAP_STONE_TILE)
				Draw_Bitmap32(&stone_tile, TILE_WIDTH * i + (TILE_WIDTH / 2), TILE_WIDTH * j + (TILE_WIDTH / 2) + g_HeightWeight);
			else if (g_MapData[tile_x][tile_y] == MAP_GRASS_TILE)
				Draw_Bitmap32(&grass_tile, TILE_WIDTH * i + (TILE_WIDTH / 2), TILE_WIDTH * j + (TILE_WIDTH / 2) + g_HeightWeight);
		}
	}

	for (int i = 0; i < MAP_SIZE; ++i)
	{
		for (int j = 0; j < MAP_SIZE; ++j)
		{
			int tile_x = i + g_left_x;
			int tile_y = j + g_top_y;
			if ((tile_x < 0) || (tile_y < 0)) continue;
			if (tile_x > WORLD_WIDTH - 1) break;
			if (tile_y > WORLD_HEIGHT - 1) break;
			if (((tile_x % 7 == 0)) && ((tile_y % 7 == 0)))
			{
				wchar_t coorditext[20];
				wsprintf(coorditext, L"(%3d, %3d)", tile_x, tile_y);
				Draw_Text_D3D(coorditext, TILE_WIDTH * i + (TILE_WIDTH / 2), TILE_WIDTH * j + (TILE_WIDTH / 2) + g_HeightWeight, D3DCOLOR_ARGB(255, 255, 0, 0));
			}
		}
	}
	
	// draw the skelaton
	for (int i = 0; i < NUM_SKILL; ++i) Draw_BOB32(&skill[i]);
	Draw_BOB32(&player);
	for (int i = 0; i<MAX_USER; ++i) Draw_BOB32(&skelaton[i]);
	 for (int i = 0; i < NUM_NPC; ++i) Draw_BOB32(&npc[i]);
	 for (int i = 0; i < NUM_ITEM; ++i) Draw_BOB32(&item[i]);
	// draw some text
	 
	 wchar_t text[100]{};
	wsprintf(text, L"LEVEL : %3d | EXP : %3d | HP : %3d / %3d | MP : %3d / %3d", g_ClientInfo.m_nLevel, g_ClientInfo.m_nExp, 
		g_ClientInfo.m_nHP, g_ClientInfo.m_nMaxHP, g_ClientInfo.m_nMP, g_ClientInfo.m_nMaxMP);
	Draw_Text_D3D(text, 10, screen_height - 64, D3DCOLOR_ARGB(255, 255, 255, 255));

	ShowStatus();
	
	// Quickslot
	Draw_Bitmap32(&quickslot, 10, screen_height - (quickslot.height * 2));
	wsprintf(text, L"%3d              %3d              %3d", g_ClientInfo.m_nHPPotionNum, g_ClientInfo.m_nMPPotionNum, g_ClientInfo.m_nMoney);
	Draw_Text_D3D(text, 10, screen_height - (quickslot.height * 2), D3DCOLOR_ARGB(255, 255, 255, 255));


	g_pSprite->End();
	g_pd3dDevice->EndScene();

	// flip the surfaces
	DD_Flip();

	// sync to 3o fps
	Wait_Clock(30);

	// return success
	return(1);

} // end Game_Main

  //////////////////////////////////////////////////////////