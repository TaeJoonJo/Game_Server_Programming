#include "Protocols.h"
#include "Common.h"
#include "Client.h"
#include "NPC.h"
#include "Timer.h"
#include "DB.h"

HANDLE g_Iocp;
array<CChessClient*, MAX_USER> ChessClients;
array<CNPC*, NUM_NPC> aNPC;
array<CFieldItem*, NUM_ITEM> aFieldItem;
array<CSkill*, NUM_SKILL> aSkill;
unordered_map<INT, CChessClient*>mapChessClients;
mutex g_Mutex;

CTimer g_Timer;
CDB g_DB;

int g_MapData[WORLD_HEIGHT][WORLD_WIDTH];

void CreateMap()
{
	ofstream mapFile("map.txt");

	int obnum = 0;
	int temp = 0;
	for (int i = 0; i < 300; ++i)
	{
		for (int j = 0; j < 300; ++j)
		{
			if (i < 15 && j < 15) {
				mapFile << MAP_WAY_TILE;
			}
			else {
				temp = RandomINT(0, 10);
				if (temp == 2) {
					mapFile << MAP_OBSTACLE_TILE;
					++obnum;
				}
				else {
					mapFile << MAP_GRASS_TILE;
				}
			}
		}
		mapFile << "\n";
	}

	cout << obnum << "\n";
}

void ReadMap()
{
	ifstream mapFile("map.txt");

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

INT AcceptThread();
VOID WorkerThread();
VOID TimerThread();

CChessClient* InsertClient(SOCKET& socket);
const bool DeleteClient(CChessClient* pclient);
const bool DisconnectClient(CChessClient* pclient);

VOID UpdateViewList(INT id);
VOID UpdateNPCViewList(CNPC* pnpc);

VOID Broadcast(VOID *ppacket);
VOID SendPacket(INT id, VOID *ppacket);

VOID SendNPCChatTo(CNPC* pnpc, INT toId, TCHAR* msg);

CChessClient* FindPlayerByName(TCHAR* pname);

bool IsMonsterAvailableXY(int x, int y);
Point PathFinder(int startX, int startY, int targetX, int targetY);

static int API_get_player_x(lua_State* L)
{
	int player_id = (int)lua_tonumber(L, -1);
	lua_pop(L, 2);		// 인자 한개와 함수 한개
	int x = (int)mapChessClients[player_id]->GetX();
	lua_pushnumber(L, x);

	return 1;		// push 해준 갯수
}

static int API_get_player_y(lua_State * L)
{
	int player_id = (int)lua_tonumber(L, -1);
	lua_pop(L, 2);		// 인자 한개와 함수 한개
	int y = (int)mapChessClients[player_id]->GetY();
	lua_pushnumber(L, y);

	return 1;		// push 해준 갯수
}

static int API_get_npc_x(lua_State* L)
{
	int npc_id = (int)lua_tonumber(L, -1);
	lua_pop(L, 2);		// 인자 한개와 함수 한개
	int x = (int)aNPC[npc_id]->GetX();
	lua_pushnumber(L, x);

	return 1;		// push 해준 갯수
}

static int API_get_npc_y(lua_State * L)
{
	int npc_id = (int)lua_tonumber(L, -1);
	lua_pop(L, 2);		// 인자 한개와 함수 한개
	int y = (int)aNPC[npc_id]->GetY();
	lua_pushnumber(L, y);

	return 1;		// push 해준 갯수
}

static int API_SendNPCMessage(lua_State * L)
{
	int client_id = (int)lua_tonumber(L, -3);
	int npc_id = (int)lua_tonumber(L, -2);
	char* msg = (char*)lua_tostring(L, -1);			// 루아에서는 유니코드를 지원하지 않음. -> 게임회사에서는 lua_towstring을 만들어서 씀
	wchar_t wmsg[MAX_STR_LEN];							// 우리는 그정도는 만들지 않음. 그냥 변환해서 씀

	lua_pop(L, 4);		// 인자 한개와 함수 한개			

	size_t wlen, len;
	len = strnlen_s(msg, MAX_STR_LEN);

	mbstowcs_s(&wlen, wmsg, len, msg, _TRUNCATE);

	SendNPCChatTo(aNPC[npc_id], client_id, wmsg);

	return 0;		// push 해준 갯수
}

static int API_NPCDetectPlayer(lua_State* L)
{
	int client_id = (int)lua_tonumber(L, -2);
	int npc_id = (int)lua_tonumber(L, -1);

	lua_pop(L, 3);		// 인자 한개와 함수 한개
	CChessClient* pclinet = mapChessClients[client_id];

	aNPC[npc_id]->DetectPlayer(pclinet);
	g_Timer.AddTimer(GetTickCount64() + 1000, e_EventType::Timer_NPC_Run, pclinet, npc_id);

	return 0;		// push 해준 갯수
}

static int API_AddTimerNPCRun(lua_State* L)
{
	int time = (int)lua_tonumber(L, -3);
	int player_id = (int)lua_tonumber(L, -2);
	int npc_id = (int)lua_tonumber(L, -1);
	lua_pop(L, 4);

	CChessClient* pclinet = mapChessClients[player_id];

	g_Timer.AddTimer(GetTickCount64() + time, e_EventType::Timer_NPC_Run, pclinet, npc_id);

	return 0;		// push 해준 갯수
}

static int API_NPCRunFinish(lua_State* L)
{
	int npc_id = (int)lua_tonumber(L, -1);
	lua_pop(L, 2);

	//aNPC[npc_id]->m_eMoveType = e_NPCMoveType::e_RandomMove;

	return 0;		// push 해준 갯수
}

VOID SendLoginOk(INT id)
{
	sc_packet_login_ok packet;
	packet.size = sizeof(sc_packet_login_ok);
	packet.type = e_sc_PacketType::Login_Ok;
	packet.id = id;

	SendPacket(id, &packet);
}

VOID SendLoginFail(INT id)
{
	sc_packet_login_fail packet;
	packet.size = sizeof(sc_packet_login_fail);
	packet.type = e_sc_PacketType::Login_Fail;

	SendPacket(id, &packet);
}

VOID SendSignupOk(INT id)
{
	sc_packet_signup_ok packet;
	packet.size = sizeof(sc_packet_signup_ok);
	packet.type = e_sc_PacketType::Signup_Ok;

	SendPacket(id, &packet);
}	

VOID SendSignupFail(INT id)
{
	sc_packet_signup_fail packet;
	packet.size = sizeof(sc_packet_signup_fail);
	packet.type = e_sc_PacketType::Signup_Fail;

	SendPacket(id, &packet);
}

VOID RecvLogin(INT id)
{
	g_Mutex.lock();
	auto clients = mapChessClients;
	CChessClient* pclient = clients[id];
	g_Mutex.unlock();

	sc_packet_put_player packet;
	packet.id = id;
	packet.size = sizeof(sc_packet_put_player);
	packet.type = e_sc_PacketType::PutPlayer;
	packet.x = pclient->GetX();
	packet.y = pclient->GetY();

	Broadcast(&packet);
	
	for (auto& client : clients)
	{
		pclient = client.second;
		if (pclient->GetID() == id) continue;

		sc_packet_put_player packet2;
		packet2.id = pclient->GetID();
		packet2.size = sizeof(sc_packet_put_player);
		packet2.x = pclient->GetX();
		packet2.y = pclient->GetY();
		packet2.type = sc_PACKETTYPE::PutPlayer;

		SendPacket(id, &packet2);
	}
}

VOID SendLogout(INT id, bool isBroad = false)
{
	sc_packet_logout packet;
	packet.id = id;
	packet.size = sizeof(sc_packet_logout);
	packet.type = sc_PACKETTYPE::Logout;

	if (isBroad == true)
		Broadcast(&packet);
	else
		SendPacket(id, &packet);
}

VOID SendPlayerInfo(INT id)
{
	sc_packet_player_info packet;
	packet.size = sizeof(sc_packet_player_info);
	packet.type = sc_PACKETTYPE::PlayerInfo;
	
	g_Mutex.lock();
	CChessClient* pplayer = mapChessClients[id];
	g_Mutex.unlock();
	if (pplayer == nullptr)
		return;

	packet.level	= pplayer->m_nLevel;
	packet.exp = pplayer->m_nExp;
	packet.maxhp = pplayer->m_Status.m_nMaxHP;
	packet.maxmp = pplayer->m_Status.m_nMaxMP;
	packet.hp = pplayer->m_nHP;
	packet.mp = pplayer->m_nMP;
	packet.str = pplayer->m_Status.m_nStr;
	packet.dex = pplayer->m_Status.m_nDex;
	packet.will = pplayer->m_Status.m_nWill;
	packet.Int = pplayer->m_Status.m_nInt;
	packet.points = pplayer->m_Status.m_nStatPoints;

	SendPacket(id, &packet);
}

VOID SendPlayerItem(INT id)
{
	sc_packet_player_item packet;
	packet.size = sizeof(sc_packet_player_item);
	packet.type = sc_PACKETTYPE::PlayerItem;

	g_Mutex.lock();
	CChessClient* pplayer = mapChessClients[id];
	g_Mutex.unlock();

	packet.hppotionnum = pplayer->m_Item.m_nHPPotionNum;
	packet.mppotionnum = pplayer->m_Item.m_nMPPotionNum;
	packet.money = pplayer->m_Item.m_nMoney;

	SendPacket(id, &packet);
}

VOID SendMovePlayer(INT id, bool isBroad = false)
{
	g_Mutex.lock();
	CChessClient* pclient = mapChessClients[id];
	g_Mutex.unlock();

	sc_packet_move_player packet;
	packet.id = id;
	packet.size = sizeof(sc_packet_move_player);
	packet.x = pclient->GetX();
	packet.y = pclient->GetY();
	packet.type = sc_PACKETTYPE::MovePlayer;

	if (isBroad == true)
		Broadcast(&packet);
	else
		SendPacket(id, &packet);
}
VOID SendMovePlayerTo(INT myId, INT toId)
{
	g_Mutex.lock();
	CChessClient* pclient = mapChessClients[myId];
	g_Mutex.unlock();

	sc_packet_move_player packet;
	packet.id = myId;
	packet.size = sizeof(sc_packet_move_player);
	packet.x = pclient->GetX();
	packet.y = pclient->GetY();
	packet.type = sc_PACKETTYPE::MovePlayer;

	SendPacket(toId, &packet);
}

// 목적지와 대상
VOID SendPutPlayer(INT id, bool isBroad = false)
{
	g_Mutex.lock();
	CChessClient* pclient = mapChessClients[id];
	g_Mutex.unlock();

	sc_packet_put_player packet;
	packet.id = id;
	packet.size = sizeof(sc_packet_put_player);
	packet.x = pclient->GetX();
	packet.y = pclient->GetY();
	packet.type = sc_PACKETTYPE::PutPlayer;
	wcsncpy_s(packet.name, pclient->m_Name, MAX_NAME_LEN);

	if (isBroad == true)
		Broadcast(&packet);
	else
		SendPacket(id, &packet);
}
VOID SendPutPlayerTo(INT myId, INT toId)
{
	g_Mutex.lock();
	CChessClient* pclient = mapChessClients[myId];
	g_Mutex.unlock();

	sc_packet_put_player packet;
	packet.id = myId;
	packet.size = sizeof(sc_packet_put_player);
	packet.x = pclient->GetX();
	packet.y = pclient->GetY();
	packet.type = sc_PACKETTYPE::PutPlayer;
	wcsncpy_s(packet.name, pclient->m_Name, MAX_NAME_LEN);

	SendPacket(toId, &packet);
}

VOID SendRemovePlayer(INT id, bool isBroad = false)
{
	sc_packet_remove_player packet;
	packet.id = id;
	packet.size = sizeof(sc_packet_remove_player);
	packet.type = sc_PACKETTYPE::RemovePlayer;

	if (isBroad == true)
		Broadcast(&packet);
	else
		SendPacket(id, &packet);
}
VOID SendRemovePlayerTo(INT myId, INT toId)
{
	sc_packet_remove_player packet;
	packet.id = myId;
	packet.size = sizeof(sc_packet_remove_player);
	packet.type = sc_PACKETTYPE::RemovePlayer;

	SendPacket(toId, &packet);
}

VOID SendMoveNPCTo(CNPC* pnpc, INT toId)
{
	sc_packet_move_npc packet;
	packet.id = pnpc->GetID();
	packet.size = sizeof(sc_packet_move_npc);
	packet.x = pnpc->GetX();
	packet.y = pnpc->GetY();
	packet.type = sc_PACKETTYPE::MoveNPC;

	SendPacket(toId, &packet);
}

VOID SendPutNPCTo(CNPC* pnpc, INT toId)
{
	sc_packet_put_npc packet;
	packet.id = pnpc->GetID();
	packet.size = sizeof(sc_packet_put_npc);
	packet.x = pnpc->GetX();
	packet.y = pnpc->GetY();
	packet.type = sc_PACKETTYPE::PutNPC;
	packet.npctype = pnpc->m_nType;
	//wsprintf(packet.name, pnpc->m_Name);
	wcsncpy_s(packet.name, pnpc->m_Name, MAX_NAME_LEN);
	packet.level = pnpc->m_nLevel;

	SendPacket(toId, &packet);
}

VOID SendRemoveNPCTo(CNPC* pnpc, INT toId)
{
	sc_packet_remove_npc packet;
	packet.id = pnpc->GetID();
	packet.size = sizeof(sc_packet_remove_npc);
	packet.type = sc_PACKETTYPE::RemoveNPC;

	SendPacket(toId, &packet);
}

VOID SendPutItem(CFieldItem* pitem, INT toId, bool isbroadcast)
{
	sc_packet_put_item packet;
	packet.id = pitem->GetID();
	packet.size = sizeof(sc_packet_put_item);
	packet.x = pitem->GetX();
	packet.y = pitem->GetY();
	packet.type = sc_PACKETTYPE::PutItem;
	packet.itemtype = pitem->m_nItemType;
	packet.itemnum = pitem->m_nItemNum;

	if(isbroadcast)
		Broadcast(&packet);
	else
		SendPacket(toId, &packet);
}
VOID SendPutItemTo(CFieldItem* pitem, INT toId)
{
	sc_packet_put_item packet;
	packet.id = pitem->GetID();
	packet.size = sizeof(sc_packet_put_item);
	packet.x = pitem->GetX();
	packet.y = pitem->GetY();
	packet.type = sc_PACKETTYPE::PutItem;
	packet.itemtype = pitem->m_nItemType;
	packet.itemnum = pitem->m_nItemNum;

	SendPacket(toId, &packet);
}

VOID SendRemoveItem(CFieldItem* pitem, INT toId, bool isbroadcast)
{
	sc_packet_remove_item packet;
	packet.id = pitem->GetID();
	packet.size = sizeof(sc_packet_remove_item);
	packet.type = sc_PACKETTYPE::RemoveItem;

	if (isbroadcast)
		Broadcast(&packet);
	else
		SendPacket(toId, &packet);
}
VOID SendRemoveItemTo(CFieldItem* pitem, INT toId)
{
	sc_packet_remove_item packet;
	packet.id = pitem->GetID();
	packet.size = sizeof(sc_packet_remove_item);
	packet.type = sc_PACKETTYPE::RemoveItem;

	SendPacket(toId, &packet);
}

VOID SendPutSkill(CSkill* pskill, INT toId, bool isbroadcast)
{
	sc_packet_put_skill packet;
	packet.id = pskill->GetID();
	packet.size = sizeof(sc_packet_put_skill);
	packet.x = pskill->GetX();
	packet.y = pskill->GetY();
	packet.type = sc_PACKETTYPE::PutSkill;
	packet.skilltype = pskill->m_nSkillType;

	if (isbroadcast)
		Broadcast(&packet);
	else
		SendPacket(toId, &packet);
}
VOID SendPutSkillTo(CSkill* pskill, INT toId)
{
	sc_packet_put_skill packet;
	packet.id = pskill->GetID();
	packet.size = sizeof(sc_packet_put_skill);
	packet.x = pskill->GetX();
	packet.y = pskill->GetY();
	packet.type = sc_PACKETTYPE::PutSkill;
	packet.skilltype = pskill->m_nSkillType;

	SendPacket(toId, &packet);
}

VOID SendMoveSkill(CSkill* pskill, INT toId, bool isbroadcast)
{
	sc_packet_move_skill packet;
	packet.id = pskill->GetID();
	packet.size = sizeof(sc_packet_move_skill);
	packet.x = pskill->GetX();
	packet.y = pskill->GetY();
	packet.type = sc_PACKETTYPE::MoveSkill;

	if (isbroadcast)
		Broadcast(&packet);
	else
		SendPacket(toId, &packet);
}
VOID SendMoveSkillTo(CSkill* pskill, INT toId)
{
	sc_packet_move_skill packet;
	packet.id = pskill->GetID();
	packet.size = sizeof(sc_packet_move_skill);
	packet.x = pskill->GetX();
	packet.y = pskill->GetY();
	packet.type = sc_PACKETTYPE::MoveSkill;

	SendPacket(toId, &packet);
}

VOID SendRemoveSkill(CSkill* pskill, INT toId, bool isbroadcast)
{
	sc_packet_remove_skill packet;
	packet.id = pskill->GetID();
	packet.size = sizeof(sc_packet_remove_skill);
	packet.type = sc_PACKETTYPE::RemoveSkill;

	if (isbroadcast)
		Broadcast(&packet);
	else
		SendPacket(toId, &packet);
}
VOID SendRemoveSkillTo(CSkill* pskill, INT toId)
{
	sc_packet_remove_skill packet;
	packet.id = pskill->GetID();
	packet.size = sizeof(sc_packet_remove_skill);
	packet.type = sc_PACKETTYPE::RemoveSkill;

	SendPacket(toId, &packet);
}

VOID SendNPCChatTo(CNPC* pnpc, INT toId, TCHAR* msg)
{
	sc_packet_npc_chat packet;
	packet.id = pnpc->GetID();
	packet.size = sizeof(sc_packet_npc_chat);
	packet.type = sc_PACKETTYPE::NPCChat;
	wcscpy_s(packet.msg, msg);

	SendPacket(toId, &packet);
}

VOID SendNoticeChat(INT toId, TCHAR* msg, bool bboardcasting)
{
	sc_packet_notice_chat packet;
	packet.size = sizeof(sc_packet_notice_chat);
	packet.type = sc_PACKETTYPE::NoticeChat;
	wcscpy_s(packet.msg, msg);

	if (bboardcasting)
		Broadcast(&packet);
	else
		SendPacket(toId, &packet);
}

// 모든 플레이어에게 뿌림
VOID Broadcast(VOID *ppacket)
{
	CChessClient* pclient = nullptr;
	g_Mutex.lock();
	auto clients = mapChessClients;
	g_Mutex.unlock();
	for(auto &client : clients) {
		pclient = client.second;
		if (pclient == nullptr) continue;

		SendPacket(pclient->m_ClientInfo->GetId(), ppacket);
	}
}

VOID SendPacket(INT id, VOID *ppacket)
{
	g_Mutex.lock();
	CChessClient* pclient = mapChessClients[id];
	g_Mutex.unlock();
	if (pclient == nullptr) return;

	SOCKETINFO* psocketInfo = new SOCKETINFO;
	char *p = reinterpret_cast<char *>(ppacket);
	memcpy(psocketInfo->buf, ppacket, p[0]);
	psocketInfo->wsaBuf.buf = psocketInfo->buf;
	psocketInfo->wsaBuf.len = p[0];
	psocketInfo->eventType = e_EventType::Event_Send;
	psocketInfo->targetid = id;
	ZeroMemory(&psocketInfo->overlapped, sizeof(WSAOVERLAPPED));

	//g_Mutex.lock();
	if (WSASend(pclient->GetSocket(), &psocketInfo->wsaBuf, 1, NULL, 0, &psocketInfo->overlapped, 0) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			//cout << "Error - Fail WSASend(error_code : " << WSAGetLastError() << ") in SendPacket(), ID : " << id << "\n";
		}
	}
	//g_Mutex.unlock();
}

bool IsInAttackRange(INT ax, INT ay, INT hx, INT hy)
{
	int dx = abs(ax - hx);
	int dy = abs(ay - hy);

	if (dx <= 1) {
		if (dy == 0)
			return true;
	}
	if(dy <= 1) {
		if (dx == 0)
			return true;
	}

	return false;
}

bool IsInPutRange(INT ax, INT ay, INT bx, INT by)
{
	if (ax == bx && ay == by)
		return true;
	return false;
}

bool IsInEffectRange(INT ax, INT ay, INT bx, INT by, INT range)
{
	int dx = abs(ax - bx);
	int dy = abs(ay - by);

	if (dx <= range && dy <= range)
		return true;

	return false;
}

CFieldItem* PickNotUsedFieldItem()
{
	for (int i = 0; i < NUM_ITEM; ++i) {
		if (aFieldItem[i]->IsRun() == false)
			return aFieldItem[i];
	}

	return nullptr;
}

CSkill* PickNotUsedSkill()
{
	for (int i = 0; i < NUM_SKILL; ++i) {
		if (aSkill[i]->IsRun() == false)
			return aSkill[i];
	}

	return nullptr;
}

void DropItemTo(int dx, int dy, CChessClient* pclient)
{
	int itempercent = RandomINT(0, 99);
	int itemtype = itempercent / 25;

	if (itemtype == 0) return;

	CFieldItem* pitem = PickNotUsedFieldItem();
	pitem->Dropped(dx, dy, itemtype);
	lstrcpy(pitem->m_OwnPlayerName, pclient->m_Name);

	//SendPutItemTo(pitem, pclient->GetID());
	SendPutItem(pitem, pclient->GetID(), true);

	g_Timer.AddTimer(GetTickCount64() + 1000 * 60, e_EventType::Timer_Item_Disappear, pclient, pitem->GetID());
}

void RespawnMonster(CNPC* pmonster)
{
	lua_State* L = pmonster->m_L;

	pmonster->m_lMutex.lock();
	lua_getglobal(L, "get_info");
	lua_pcall(L, 0, 3, 0);
	int x = (int)lua_tonumber(L, -3);
	int y = (int)lua_tonumber(L, -2);
	int hp = (int)lua_tonumber(L, -1);
	lua_pop(L, 3);
	pmonster->m_lMutex.unlock();

	pmonster->m_nX = x;
	pmonster->m_nY = y;
	pmonster->m_Status.m_nHP = hp;

	pmonster->Respawn();

	UpdateNPCViewList(pmonster);
}

void DieMonster(CNPC* pnpc, CChessClient* pplayer)
{
	pnpc->Die();
	CMonster* pmonster = ((CMonster*)pnpc);
	// Exp
	int exp = pmonster->GetExp();
	bool islevelup = pplayer->GetExp(exp);

	// Item
	DropItemTo(pnpc->GetX(), pnpc->GetY(), pplayer);

	pnpc->m_lMutex.lock();
	auto view = pnpc->m_ViewList;
	pnpc->m_lMutex.unlock();

	for (auto player : view) {
		SendRemoveNPCTo(pnpc, player->GetID());
		player->m_npcvlMutex.lock();
		player->m_NPCViewList.erase(pnpc);
		player->m_npcvlMutex.unlock();
	}

	pplayer->m_npcvlMutex.lock();
	pplayer->m_NPCViewList.erase(pnpc);
	pplayer->m_npcvlMutex.unlock();

	g_Timer.AddTimer(GetTickCount64() + NPC_RESPAWN_COOLTIME, e_EventType::Timer_Monster_Respawn, nullptr, pnpc->GetID());

	SendPlayerInfo(pplayer->GetID());

	TCHAR str[MAX_STR_LEN];
	wsprintf(str, L"몬스터 %s에게서 경험치 %3d 획득", pnpc->m_Name, exp);
	SendNoticeChat(pplayer->GetID(), str, false);

	wsprintf(str, L"%3d로 레벨업!", pplayer->m_nLevel);
}

void DiePlayer(CChessClient* pplayer)
{
	TCHAR msg[MAX_STR_LEN]{};
	int exp{}, money{};
	pplayer->Die(&exp, &money);
	g_Timer.AddTimer(GetTickCount64() + PLAYER_RESPAWN_COOLTIME, e_EventType::Timer_Player_Respawn, pplayer);
	wsprintf(msg, L"%3d의 경험치와 %3d의 실링을 잃었습니다.", exp, money);
	SendNoticeChat(pplayer->GetID(), msg, false);
	SendRemovePlayer(pplayer->GetID());
}

void InitalizeObjects() 
{
	cout << "Initalize Objects Start!\n";

	for (int i = 0; i < MAX_USER; ++i)
		ChessClients[i] = new CChessClient;

	aNPC[0] = new CNPC;
	aNPC[0]->Initalize(0, NPC_PRIEST);
	lua_getglobal(aNPC[0]->m_L, "set_info");
	lua_pushnumber(aNPC[0]->m_L, aNPC[0]->GetID());
	char temp1[MAX_NAME_LEN]{};
	WideCharToMultiByte(CP_ACP, 0, aNPC[0]->m_Name, MAX_NAME_LEN, temp1, MAX_NAME_LEN, NULL, NULL);
	lua_pushstring(aNPC[0]->m_L, temp1);
	lua_pcall(aNPC[0]->m_L, 2, 0, 0);

	aNPC[1] = new CNPC;
	aNPC[1]->Initalize(1, NPC_FAIRY);
	lua_getglobal(aNPC[1]->m_L, "set_info");
	lua_pushnumber(aNPC[1]->m_L, aNPC[1]->GetID());
	char temp2[MAX_NAME_LEN]{};
	WideCharToMultiByte(CP_ACP, 0, aNPC[1]->m_Name, MAX_NAME_LEN, temp2, MAX_NAME_LEN, NULL, NULL);
	lua_pushstring(aNPC[1]->m_L, temp2);
	lua_pcall(aNPC[1]->m_L, 2, 0, 0);

	int type{ NPC_TREE };
	for (int i = 2; i < NUM_NPC; ++i) {
		aNPC[i] = new CMonster;
		type = RandomINT(NPC_TREE, NPC_BITE);
		aNPC[i]->Initalize(i, type);
		CMonster* pmonster = ((CMonster*)aNPC[i]);
		lua_State* L = pmonster->m_L;
		lua_getglobal(L, "set_info");
		lua_pushnumber(L, pmonster->GetID());
		char temp[MAX_NAME_LEN]{};
		WideCharToMultiByte(CP_ACP, 0, pmonster->m_Name, MAX_NAME_LEN, temp, MAX_NAME_LEN, NULL, NULL);
		lua_pushstring(L, temp);
		lua_pushnumber(L, pmonster->GetX());
		lua_pushnumber(L, pmonster->GetY());
		lua_pushnumber(L, pmonster->m_nLevel);
		lua_pushnumber(L, pmonster->m_Status.m_nHP);
		lua_pushnumber(L, pmonster->m_Status.m_nDamage);
		lua_pcall(L, 7, 0, 0);
		//lua_pcall(L, 1, 0, 0);
		//lua_register(L, "API_get_player_x", API_get_player_x);
		//lua_register(L, "API_get_player_y", API_get_player_y);
		//lua_register(L, "API_get_npc_x", API_get_npc_x);
		//lua_register(L, "API_get_npc_y", API_get_npc_y);
		//lua_register(L, "API_SendNPCMessage", API_SendNPCMessage);
		//lua_register(L, "API_NPCDetectPlayer", API_NPCDetectPlayer);
		//lua_register(L, "API_AddTimerNPCRun", API_AddTimerNPCRun);
		//lua_register(L, "API_NPCRunFinish", API_NPCRunFinish);
	}

	for (int i = 0; i < NUM_ITEM; ++i) {
		aFieldItem[i] = new CFieldItem;
		aFieldItem[i]->Initalize(i, 0);
	}

	for (int i = 0; i < NUM_SKILL; ++i) {
		aSkill[i] = new CSkill;
		aSkill[i]->Initalize(i);
	}

	cout << "Initalize Objects OK!\n";
}

int main()
{
	//CreateMap();
	ReadMap();
	InitalizeObjects();

	vector<thread> vecworkerThreads;

	g_Iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	g_Timer.Initialize(g_Iocp);

	g_DB.InitalizeDB();
	
	if (g_DB.ConnectDB((SQLWCHAR*)L"Game_Server_2013180038", (SQLWCHAR*)L"TJ", (SQLWCHAR*)L"102030") == false)
		while (1);

	cout << "Server Start!\n";

	for (int i = 0; i < 8; ++i)
		vecworkerThreads.emplace_back(thread{ WorkerThread });
	thread acceptThread{ AcceptThread };
	thread timerThread{ TimerThread };

	timerThread.join();

	acceptThread.join();

	for (auto& thread : vecworkerThreads)
		thread.join();
	
	g_DB.ClearDB();

	g_Mutex.lock();
	for (int i = 0; i < MAX_USER; ++i)
		delete ChessClients[i];
	mapChessClients.clear();
	g_Mutex.unlock();

	for (DWORD i = 0; i < NUM_NPC; ++i)
		delete aNPC[i];

	for (DWORD i = 0; i < NUM_ITEM; ++i)
		delete aFieldItem[i];

	for (DWORD i = 0; i < NUM_SKILL; ++i)
		delete aSkill[i];

	return 0;
}

INT AcceptThread()
{
	// Winsock Start - windock.dll 로드
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << "Error - Can not load 'winsock.dll' file\n";
		return 1;
	}

	// 1. 소켓생성  
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);	// WSA_FLAG_OVERLAPPED 플래그 반드시 넣어줘야 OVEERLAPPED I/O 동작
	if (listenSocket == INVALID_SOCKET)
	{
		printf("Error - Invalid socket\n");
		return 1;
	}

	// 서버정보 객체설정
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// 2. 소켓설정
	if (bind(listenSocket, (struct sockaddr*) & serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		printf("Error - Fail bind\n");
		// 6. 소켓종료
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return 1;
	}

	// 3. 수신대기열생성
	if (listen(listenSocket, 5) == SOCKET_ERROR)
	{
		printf("Error - Fail listen\n");
		// 6. 소켓종료
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return 1;
	}

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);
	SOCKET clientSocket;
	DWORD flags;

	while (1)
	{
		clientSocket = accept(listenSocket, (struct sockaddr*) & clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			printf("Error - Accept Failure\n");
			return 1;
		}
		//cout << "클라이언트 접속 : IP 주소 = " << inet_ntoa(clientAddr.sin_addr) << ", 포트 번호 = " << ntohs(clientAddr.sin_port) << "\n";

		// 억셉트하고 해당 클라이언트에 대한 클라이언트 클래스 생성해 아이디를 지정해주고 벡터에 넣어준다. 클라이언트에 아이디를 넘겨주는거는 첫번째 샌드때 Login enum을 이용해 해준다.
		
		bool optval = true;
		setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, (char*)& optval, sizeof(optval));
		
		CChessClient *pclient = InsertClient(clientSocket);

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), g_Iocp, reinterpret_cast<ULONG_PTR>(pclient), 0);
		flags = 0;
		INT id = pclient->GetID();
		cout << "ID : " << id << " 로 접속!\n";

		//SendLoginOk(id);
		////RecvLogin(id);
		//SendPlayerInfo(id);
		//SendPlayerItem(id);
		//SendPutPlayer(id, false);
		//UpdateViewList(id);

		g_Timer.AddTimer(GetTickCount64() + 5000, e_EventType::Timer_Player_HPMP_Regen, pclient);

		if (WSARecv(clientSocket, &(pclient->m_ClientInfo->m_SocketInfo.wsaBuf), 1, NULL, &flags, &(pclient->m_ClientInfo->m_SocketInfo.overlapped), 0))	// NULL 을 넣지 않으면 동기식으로 동작 할 수도 있다.
		{
			if (WSAGetLastError() == WSA_IO_PENDING)		// 끝나지 않고 좀더 있어야된다. - 문제 없다.
			{
				//cout << "WSA_IO_PENDING in WSARecv\n";
			}
			else
			{
				cout << "Error - IO pending Failure " << WSAGetLastError() << "\n";
				//cout << "ID : " << id << "Logout!\n";
				//SendLogout(id, TRUE);
				//vClients[id]->Logout();
			}
		}
		else
		{
			//cout << "Non Overlapped Recv return.\n";
			//cout << "ID : " << id << "Logout!\n";
			//SendLogout(id, TRUE);
			//vClients[id]->Logout();
		}
	}

	// 6-2. 리슨 소켓종료
	closesocket(listenSocket);

	// Winsock End
	WSACleanup();
}

VOID WorkerThread()
{
	DWORD				ioByte = 0;
	DWORD				retval = 0;
	CChessClient		*pclient = nullptr;
	SOCKETINFO			*poverlapped = nullptr;

	while (1)
	{
		ioByte = 0;
		retval = 0;
		pclient = nullptr;
		poverlapped = nullptr;

		if (false == GetQueuedCompletionStatus(g_Iocp, &ioByte,
			reinterpret_cast<PULONG_PTR>(&pclient),
			reinterpret_cast<LPOVERLAPPED*>(&poverlapped), INFINITE))
			retval = WSAGetLastError();

		if (retval == 0)
		{
			e_EventType etype = poverlapped->eventType;

			// case pclient == nullptr //
			if (etype == e_EventType::Timer_NPC_Move)
			{
				DWORD npcid = poverlapped->targetid;
				CNPC* pnpc = aNPC[npcid];
				
				if (pnpc->m_nType == NPC_FAIRY || pnpc->m_nType == NPC_PRIEST) {
					pnpc->Move();
				}
				else {
					CMonster* pmonster = ((CMonster*)pnpc);
					if (pmonster->m_bIsRoam == true) {
						if (pmonster->m_bIsPeace == true) {
							if (pmonster->m_bIsAttacked == true) {
								pmonster->m_AggroMutex.lock();
								CChessClient* pplayer = pmonster->m_pLastViewPlayer;
								pmonster->m_AggroMutex.unlock();
								if (pplayer == nullptr) {
									pmonster->Move();
								}
								else {
									Point next = PathFinder(pmonster->GetX(), pmonster->GetY(), pplayer->GetX(), pplayer->GetY());
									pmonster->m_nX = next.x;
									pmonster->m_nY = next.y;
									SendMoveNPCTo(pmonster, pplayer->GetID());
									if (pplayer->m_bIsDie == false) {
										if (IsInAttackRange(pmonster->GetX(), pmonster->GetY(), pplayer->GetX(), pplayer->GetY()) == true) {
											int damage{};
											int player_hp = pmonster->Attack(pplayer, &damage);

											TCHAR msg[MAX_STR_LEN]{};
											wsprintf(msg, L"몬스터 %s에게 %3d의 데미지를 받았습니다.", pmonster->m_Name, damage);
											SendNoticeChat(pplayer->GetID(), msg, false);

											if (player_hp <= 0) {
												DiePlayer(pplayer);
												wsprintf(msg, L"몬스터 %s에 의해 죽었습니다.", pmonster->m_Name);
												SendNoticeChat(pplayer->GetID(), msg, false);
												SendPlayerItem(pplayer->GetID());
											}

											SendPlayerInfo(pplayer->GetID());
										}
									}
								}
							}
							else {
								pmonster->Move();
							}
						}
						else {
							pmonster->m_AggroMutex.lock();
							CChessClient* pplayer = pmonster->m_pLastViewPlayer;
							pmonster->m_AggroMutex.unlock();
							if (pplayer == nullptr) {
								pmonster->Move();
							}
							else {
								Point next = PathFinder(pmonster->GetX(), pmonster->GetY(), pplayer->GetX(), pplayer->GetY());
								pmonster->m_nX = next.x;
								pmonster->m_nY = next.y;
								SendMoveNPCTo(pmonster, pplayer->GetID());
								if (pplayer->m_bIsDie == false) {
									if (IsInAttackRange(pmonster->GetX(), pmonster->GetY(), pplayer->GetX(), pplayer->GetY()) == true) {
										int damage{};
										int player_hp = pmonster->Attack(pplayer, &damage);

										TCHAR msg[MAX_STR_LEN]{};
										wsprintf(msg, L"몬스터 %s에게 %3d의 데미지를 받았습니다.", pmonster->m_Name, damage);
										SendNoticeChat(pplayer->GetID(), msg, false);

										if (player_hp <= 0) {
											DiePlayer(pplayer);
											wsprintf(msg, L"몬스터 %s에 의해 죽었습니다.", pmonster->m_Name);
											SendNoticeChat(pplayer->GetID(), msg, false);
											SendPlayerItem(pplayer->GetID());
										}

										SendPlayerInfo(pplayer->GetID());
									}
								}
							}
						}
					}
					else {
						if (pmonster->m_bIsPeace == true) {
							if (pmonster->m_bIsAttacked == true) {
								pmonster->m_lMutex.lock();
								auto view = pmonster->m_ViewList;
								pmonster->m_lMutex.unlock();
								for (auto player : view) {
									if (player->m_bIsDie == true) continue;
									if (IsInAttackRange(pmonster->GetX(), pmonster->GetY(), player->GetX(), player->GetY()) == false)
										continue;
									int damage{};
									int player_hp = pmonster->Attack(player, &damage);

									TCHAR msg[MAX_STR_LEN]{};
									wsprintf(msg, L"몬스터 %s에게 %3d의 데미지를 받았습니다.", pmonster->m_Name, damage);
									SendNoticeChat(player->GetID(), msg, false);

									if (player_hp <= 0) {
										DiePlayer(player);
										wsprintf(msg, L"몬스터 %s에 의해 죽었습니다.", pmonster->m_Name);
										SendNoticeChat(player->GetID(), msg, false);
										SendPlayerItem(player->GetID());
									}

									SendPlayerInfo(player->GetID());
								}
							}
						}
						else {
							pmonster->m_lMutex.lock();
							auto view = pmonster->m_ViewList;
							pmonster->m_lMutex.unlock();
							for (auto player : view) {
								if (player->m_bIsDie == true) continue;
								if (IsInAttackRange(pmonster->GetX(), pmonster->GetY(), player->GetX(), player->GetY()) == false)
									continue;
								int damage{};
								int player_hp = pmonster->Attack(player, &damage);

								TCHAR msg[MAX_STR_LEN]{};
								wsprintf(msg, L"몬스터 %s에게 %3d의 데미지를 받았습니다.", pmonster->m_Name, damage);
								SendNoticeChat(player->GetID(), msg, false);

								if (player_hp <= 0) {
									DiePlayer(player);
									wsprintf(msg, L"몬스터 %s에 의해 죽었습니다.", pmonster->m_Name);
									SendNoticeChat(player->GetID(), msg, false);
									SendPlayerItem(player->GetID());
								}

								SendPlayerInfo(player->GetID());
							}
						}
					}
				}
				
				UpdateNPCViewList(pnpc);

				delete poverlapped;
			}
			else if (etype == e_EventType::Timer_Monster_Respawn)
			{
				int monster_id = poverlapped->targetid;

				RespawnMonster(aNPC[monster_id]);

				delete poverlapped;
			}
			else if (etype == e_EventType::Timer_NPC_Can_Move) {
				aNPC[poverlapped->targetid]->m_bIsCanMove = true;

				delete poverlapped;
			}
			else if(etype == e_EventType::Timer_Monster_Aggro_Reset)
			{
				((CMonster*)aNPC[poverlapped->targetid])->m_bIsAttacked = false;

				delete poverlapped;
			}
			else if(etype == e_EventType::Timer_Item_Disappear)
			{
				int item_id = poverlapped->targetid;
				CFieldItem* pitem = aFieldItem[item_id];
				if (pitem->IsRun()) {
					pitem->Disappear();
					SendRemoveItem(pitem, 9999, true);
					//if (pclient->IsRun() == true)
					//	SendRemoveItemTo(pitem, pclient->GetID());
				}

				delete poverlapped;
			}
			
			if (pclient == nullptr)
				continue;

			if (ioByte == 0) {
				cout << "WorkerThread LogOut ID : " << static_cast<int>(pclient->GetID()) << "\n";
				DisconnectClient(pclient);
				continue;
			}

			switch (etype)
			{
			case e_EventType::Event_Recv:
			{
				//cout << "받은 바이트 : " <<ioByte << "\n";
				char* buf = pclient->m_ClientInfo->m_SocketInfo.buf;
				int packetSize = 0;
				if (pclient->m_ClientInfo->m_PreSize != 0)
					packetSize = pclient->m_ClientInfo->m_RecvBuf[0];
				while (ioByte > 0)
				{
					if (packetSize == 0)		// 사이즈 저장
						packetSize = buf[0];
					int required = packetSize - pclient->m_ClientInfo->m_PreSize;	// 패킷을 완성하는데 필요한 바이트 수
					if (ioByte < required)	// 리시브 받은 데이터가 필요한 데이터보다 작으면 패킷을 완성 시킬수 없다.
					{
						memcpy(pclient->m_ClientInfo->m_RecvBuf + pclient->m_ClientInfo->m_PreSize, buf, ioByte);
						pclient->m_ClientInfo->m_PreSize += ioByte;
						break;
					}
					else // 데이터를 완성할 수 있다면,,
					{
						memcpy(pclient->m_ClientInfo->m_RecvBuf + pclient->m_ClientInfo->m_PreSize, buf, required);	// 패킷을 완성시키는데 필요한만큼을 복사한다.

						cs_packet_base* buf = reinterpret_cast<cs_packet_base*>(pclient->m_ClientInfo->m_RecvBuf);

						BYTE type = buf->type;
						e_sc_PacketType escType = e_sc_PacketType::scIDLE;

						switch (type)
						{
						case e_cs_PacketType::Login:
						{
							cs_packet_login* pbuf = reinterpret_cast<cs_packet_login*>(pclient->m_ClientInfo->m_RecvBuf);

							TCHAR* pname = pbuf->name;
							wcout << L"Try to Login To Name : " << pname << "\n";

							if (FindPlayerByName(pname) != nullptr) {
								escType = e_sc_PacketType::Login_Fail;
								break;
							}

							if (g_DB.LoginToDB(pname, pclient) == true) {
								memcpy(pclient->m_Name, pname, wcslen(pname) * 2 + 2);
								escType = e_sc_PacketType::Login_Ok;
							}
							else
								escType = e_sc_PacketType::Login_Fail;
						} break;
						case e_cs_PacketType::Signup:
						{
							cs_packet_signup* pbuf = reinterpret_cast<cs_packet_signup*>(pclient->m_ClientInfo->m_RecvBuf);

							TCHAR* pname = pbuf->name;
							wcout << L"Try to Signup To Name : " << pname << "\n";

							if (g_DB.SignupToDB(pname) == true) 
								escType = e_sc_PacketType::Signup_Ok;
							else
								escType = e_sc_PacketType::Signup_Fail;
						} break;
						case e_cs_PacketType::Up:
						{
							if (pclient->m_bIsDie == true) break;
							if (pclient->m_bIsCanMove == false) break;
							int x = pclient->m_nX, y = pclient->m_nY;
							--y;
							if(y > 0)
								if(g_MapData[x][y] != MAP_OBSTACLE_TILE && g_MapData[x][y] != MAP_STONE_TILE)		
									--pclient->m_nY;

							pclient->m_bIsCanMove = false;
							g_Timer.AddTimer(GetTickCount64() + PLAYER_MOVE_COOLTIME, e_EventType::Timer_Player_Can_Move, pclient);
							escType = e_sc_PacketType::MovePlayer;
						} break;
						case e_cs_PacketType::Down:
						{
							if (pclient->m_bIsDie == true) break;
							if (pclient->m_bIsCanMove == false) break;
							int x = pclient->m_nX, y = pclient->m_nY;
							++y;
							if (y < WORLD_HEIGHT - 1)
								if (g_MapData[x][y] != MAP_OBSTACLE_TILE && g_MapData[x][y] != MAP_STONE_TILE)
									++pclient->m_nY;

							pclient->m_bIsCanMove = false;
							g_Timer.AddTimer(GetTickCount64() + PLAYER_MOVE_COOLTIME, e_EventType::Timer_Player_Can_Move, pclient);
							escType = e_sc_PacketType::MovePlayer;
						} break;
						case e_cs_PacketType::Left:
						{
							if (pclient->m_bIsDie == true) break;
							if (pclient->m_bIsCanMove == false) break;
							int x = pclient->m_nX, y = pclient->m_nY;
							--x;
							if (x > 0)
								if (g_MapData[x][y] != MAP_OBSTACLE_TILE && g_MapData[x][y] != MAP_STONE_TILE)	
									--pclient->m_nX;

							pclient->m_bIsCanMove = false;
							g_Timer.AddTimer(GetTickCount64() + PLAYER_MOVE_COOLTIME, e_EventType::Timer_Player_Can_Move, pclient);
							escType = e_sc_PacketType::MovePlayer;
						} break;
						case e_cs_PacketType::Right:
						{
							if (pclient->m_bIsDie == true) break;
							if (pclient->m_bIsCanMove == false) break;
							int x = pclient->m_nX, y = pclient->m_nY;
							++x;
							if (x < (WORLD_HEIGHT - 1))
								if (g_MapData[x][y] != MAP_OBSTACLE_TILE && g_MapData[x][y] != MAP_STONE_TILE)
									++pclient->m_nX;

							pclient->m_bIsCanMove = false;
							g_Timer.AddTimer(GetTickCount64() + PLAYER_MOVE_COOLTIME, e_EventType::Timer_Player_Can_Move, pclient);
							escType = e_sc_PacketType::MovePlayer;
						} break;
						case e_cs_PacketType::IDLE:
						{
							escType = e_sc_PacketType::MovePlayer;
						} break;
						case e_cs_PacketType::Attack:
						{
							if (pclient->m_bIsDie == true) break;
							if (pclient->m_bIsCanAttack == false) break;

							int px = pclient->GetX(), py = pclient->GetY();

							pclient->m_npcvlMutex.lock();
							auto npcView = pclient->m_NPCViewList;
							pclient->m_npcvlMutex.unlock();

							for (auto npc : npcView) {
								if (npc->m_nType == NPC_FAIRY || npc->m_nType == NPC_PRIEST) continue;
								if (IsInAttackRange(px, py, npc->GetX(), npc->GetY()) == false) continue;
								int damage{0};
								int monster_hp = pclient->Attack(npc, &damage);
								TCHAR msg[MAX_STR_LEN]{};
								
								wsprintf(msg, L"몬스터 %s에게 %3d의 데미지를 주었습니다.", npc->m_Name, damage);
								SendNoticeChat(pclient->GetID(), msg, false);

								npc->m_AggroMutex.lock();
								npc->m_pLastViewPlayer = pclient;
								npc->m_AggroMutex.unlock();
								((CMonster*)npc)->m_bIsAttacked = true;
								g_Timer.AddTimer(GetTickCount64() + 5000, e_EventType::Timer_Monster_Aggro_Reset, nullptr, npc->GetID());
								if (monster_hp <= 0) {
									DieMonster(npc, pclient);
								}
							}
							pclient->m_bIsCanAttack = false;
							g_Timer.AddTimer(GetTickCount64() + PLAYER_ATTACK_COOLTIME, e_EventType::Timer_Player_Can_Attack, pclient);
							escType = e_sc_PacketType::scIDLE;
						} break;
						case e_cs_PacketType::UseItem:
						{
							if (pclient->m_bIsDie == true) break;
							cs_packet_use_item* pbuf = reinterpret_cast<cs_packet_use_item*>(pclient->m_ClientInfo->m_RecvBuf);

							int type = pbuf->itemtype;
							int id = pclient->GetID();
							
							switch (type)
							{
							case ITEM_HPPOTION:
							{
								if (pclient->m_Item.m_nHPPotionNum > 0) {
									if (pclient->CanHeal(true) == false) break;
									int healcount = pclient->Heal(30, true);
									--pclient->m_Item.m_nHPPotionNum;
									SendPlayerInfo(id);
									SendPlayerItem(id);
									TCHAR msg[MAX_STR_LEN]{};
									wsprintf(msg, L"포션을 사용하여 HP가 %3d 회복되었습니다.", healcount);
									SendNoticeChat(id, msg, false);
								}
							} break;
							case ITEM_MPPOTION:
							{
								if (pclient->m_Item.m_nMPPotionNum > 0) {
									if (pclient->CanHeal(false) == false) break;
									int healcount = pclient->Heal(30, false);
									--pclient->m_Item.m_nMPPotionNum;
									SendPlayerInfo(id);
									SendPlayerItem(id);
									TCHAR msg[MAX_STR_LEN]{};
									wsprintf(msg, L"포션을 사용하여 MP가 %3d 회복되었습니다.", healcount);
									SendNoticeChat(id, msg, false);
								}
							} break;
							default: break;
							}
						} break;
						case e_cs_PacketType::PickupItem:
						{
							if (pclient->m_bIsDie == true) break;
							int x = pclient->GetX(), y = pclient->GetY();
							CFieldItem* pitem = nullptr;
							for (int i = 0; i < NUM_ITEM; ++i) {
								pitem = aFieldItem[i];
								if (pitem->IsRun() == false) continue;
								if (IsInPutRange(x, y, pitem->GetX(), pitem->GetY()) == false) continue;
								if (lstrcmp(pitem->m_OwnPlayerName, pclient->m_Name) != 0) continue;
								int item_type = pitem->m_nItemType, item_num = pitem->m_nItemNum;
								switch (item_type)
								{
								case ITEM_HPPOTION:		pclient->m_Item.m_nHPPotionNum += item_num;	break;
								case ITEM_MPPOTION:		pclient->m_Item.m_nMPPotionNum += item_num;	break;
								case ITEM_MONEY:		pclient->m_Item.m_nMoney += item_num;	break;
								}
								int client_id = pclient->GetID();
								pitem->Disappear();
								TCHAR msg[MAX_STR_LEN]{};
								wsprintf(msg, L"%s를 %3d개 얻었습니다.", pitem->m_Name, pitem->m_nItemNum);
								SendNoticeChat(pclient->GetID(), msg, false);
								//SendRemoveItemTo(pitem, client_id);
								SendRemoveItem(pitem, client_id, true);
								SendPlayerItem(client_id);
								break;
							}
						} break;
						case e_cs_PacketType::Interact:
						{
							if (pclient->m_bIsDie == true) break;
							int x = pclient->GetX(), y = pclient->GetY();
							if (g_MapData[x][y] != MAP_WAY_TILE) break;

							for (int i = 0; i < 2; ++i) {
								CNPC* pnpc = aNPC[i];
								if (pnpc->IsRun() == false) continue;
								if (IsInAttackRange(x, y, pnpc->GetX(), pnpc->GetY()) == false) continue;
								int npc_type = pnpc->m_nType;

								int need_money = 0;
								int player_level = pclient->m_nLevel;
								int player_id = pclient->GetID();

								switch (npc_type)
								{
								case NPC_FAIRY:
								{
									if (player_level == 1) break;
									need_money = player_level * 5;
									if (pclient->m_Item.m_nMoney < need_money) break;
									pclient->m_Item.m_nMoney -= need_money;
									pclient->m_Status.Initalize();
									pclient->m_Status.m_nStatPoints = player_level * 2;
									pclient->Heal(100, true);
									pclient->Heal(100, false);
									SendPlayerInfo(player_id);
									SendPlayerItem(player_id);
									TCHAR msg[MAX_STR_LEN]{};
									wsprintf(msg, L"돈 %3d를 사용하였습니다.", need_money);
									SendNoticeChat(pclient->GetID(), msg, false);
									wsprintf(msg, L"능력치가 초기화되고 %3d만큼의 포인트가 생겼습니다.", player_level*2 );
									SendNoticeChat(pclient->GetID(), msg, false);
								} break;
								case NPC_PRIEST:
								{
									if (pclient->CanHeal(true) == false && pclient->CanHeal(false) == false) break;
									need_money = 10;
									if (pclient->m_Item.m_nMoney < need_money) break;
									pclient->m_Item.m_nMoney -= need_money;
									int hp_heal = pclient->Heal(100, true);
									int mp_heal = pclient->Heal(100, false);
									SendPlayerInfo(player_id);
									SendPlayerItem(player_id);
									TCHAR msg[MAX_STR_LEN]{};
									wsprintf(msg, L"돈 %3d를 사용하였습니다.", need_money);
									SendNoticeChat(pclient->GetID(), msg, false);
									wsprintf(msg, L"성직자가 HP를 %3d, MP를 %3d 회복시켜줬습니다.", hp_heal, mp_heal);
									SendNoticeChat(pclient->GetID(), msg, false);
								} break;
								default: break;
								}
								pnpc->m_bIsCanMove = false;
								g_Timer.AddTimer(GetTickCount64() + 3000, e_EventType::Timer_NPC_Can_Move, nullptr, pnpc->GetID());
							}
						} break;
						case e_cs_PacketType::StatPlus:
						{
							if (pclient->m_Status.m_nStatPoints < 1) break;

							cs_packet_stat_plus* pbuf = reinterpret_cast<cs_packet_stat_plus*>(pclient->m_ClientInfo->m_RecvBuf);
							int stattype = pbuf->stattype;
							switch (stattype)
							{
							case STAT_HP:	pclient->m_Status.m_nMaxHP += 10;	break;
							case STAT_MP:	pclient->m_Status.m_nMaxMP += 10;	break;
							case STAT_Str:	pclient->m_Status.m_nStr += 1;	break;
							case STAT_Dex:	pclient->m_Status.m_nDex += 1;	break;
							case STAT_Will:	pclient->m_Status.m_nWill += 1;	break;
							case STAT_Int:	pclient->m_Status.m_nInt += 1;	break;
							default: while (1);
							}
							pclient->m_Status.m_nStatPoints -= 1;

							SendPlayerInfo(pclient->GetID());
						} break;
						case e_cs_PacketType::Skill:
						{
							if (pclient->m_bIsDie == true) break;
							//if (pclient->m_bIsCanAttack == false) break;

							cs_packet_skill* pbuf = reinterpret_cast<cs_packet_skill*>(pclient->m_ClientInfo->m_RecvBuf);

							int dir = pbuf->dir;
							int skill_type = pbuf->skilltype;

							CSkill* pskill = PickNotUsedSkill();
							
							int dirx{}, diry{};
							switch (dir)
							{
							case DIR_UP:		dirx = 0; diry = -1; break;
							case DIR_DOWN:		dirx = 0; diry = 1; break;
							case DIR_LEFT:		dirx = -1; diry = 0; break;
							case DIR_RIGHT:		dirx = 1; diry = 0; break;
							}

							if (pskill->Use(skill_type, dirx, diry, pclient) == true) {
								g_Timer.AddTimer(pskill->m_nUsedTime + 1000, e_EventType::Timer_Skill_Effect, pclient, pskill->GetID());
								//SendPutSkillTo(pskill, pclient->GetID());
								SendPutSkill(pskill, pclient->GetID(), true);
								SendPlayerInfo(pclient->GetID());
							}
							else {
								TCHAR msg[MAX_STR_LEN]{};
								wsprintf(msg, L"%s를 사용하지 못하였습니다", pskill->m_Name);
								SendNoticeChat(pclient->GetID(), msg, false);
							}
						} break;
						case e_cs_PacketType::Teleport:
						{
							pclient->m_bIsTest = true;
							pclient->m_nHP = 10000;
							pclient->m_Status.m_nWill = 100000;

							int x{}, y{};
							do {
								x = RandomINT(0, WORLD_WIDTH - 1);
								y = RandomINT(0, WORLD_HEIGHT - 1);
							} while (g_MapData[x][y] == MAP_STONE_TILE);
							pclient->m_nX = x;
							pclient->m_nY = y;

							escType = e_sc_PacketType::MovePlayer;
						} break;
						case e_cs_PacketType::ForTest:
						{
							
							pclient->m_bIsTest = true;
							pclient->m_nHP = 10000;
							pclient->m_Status.m_nWill = 100000;

							pclient->m_nX = 6;
							pclient->m_nY = 6;

							escType = e_sc_PacketType::MovePlayer;
						} break;
						// For Debug
						{
							case e_cs_PacketType::MoneyBug:
							{
							pclient->m_Item.m_nMoney = 10000;
							SendPlayerItem(pclient->GetID());
						} break;
							case e_cs_PacketType::LevelBug:
							{
								pclient->LevelUp();
								SendPlayerInfo(pclient->GetID());
							} break;
						}
						}

						int id = pclient->GetID();
						// send 부분 추가하자
						switch (escType)
						{
						case e_sc_PacketType::Login_Ok:
						{
							SendLoginOk(id);
							SendPlayerInfo(id);
							SendPlayerItem(id);
							SendPutPlayer(id, false);
							UpdateViewList(id);
							for (int i = 0; i < NUM_ITEM; ++i) {
								if (aFieldItem[i]->IsRun() == false) continue;
								SendPutItemTo(aFieldItem[i], id);
							}
							for (int i = 0; i < NUM_SKILL; ++i) {
								if (aSkill[i]->IsRun() == false) continue;
								SendPutSkillTo(aSkill[i], id);
							}
						} break;
						case e_sc_PacketType::Login_Fail:
						{
							SendLoginFail(id);
						} break;
						case e_sc_PacketType::Signup_Ok:
						{
							SendSignupOk(id);
						} break;
						case e_sc_PacketType::Signup_Fail:
						{
							SendSignupFail(id);
						} break;
						case e_sc_PacketType::Logout:
						{
							SendLogout(id, true);
						} break;
						case e_sc_PacketType::MovePlayer:
						{
							SendMovePlayer(id, false);
							UpdateViewList(id);
						} break;
						case e_sc_PacketType::PutPlayer:
						{
							SendPutPlayer(id, false);
							//UpdateViewList(id);
						} break;
						case e_sc_PacketType::RemovePlayer:
						{
							SendRemovePlayer(id, false);
							//UpdateViewList(id);
						} break;
						case e_sc_PacketType::scIDLE:
						{

						} break;
						default: break;
						}
						pclient->m_ClientInfo->m_PreSize = 0;
						ioByte -= required;
						buf += required;
						packetSize = 0;
					}
				}
				DWORD flags = 0;
				if (WSARecv(pclient->m_ClientInfo->GetSocket(), &pclient->m_ClientInfo->m_SocketInfo.wsaBuf, 1, NULL, &flags, &pclient->m_ClientInfo->m_SocketInfo.overlapped, 0))
				{
					if (WSAGetLastError() != WSA_IO_PENDING)
					{
						printf("Error - IO pending Failure\n");
						DisconnectClient(pclient);

						return;
					}
				}
				else // 연결 끊어주자
				{
					/*cout << "Non Overlapped Recv return.\n";
					SendLogout(id, TRUE);
					pClient->Logout();*/

					return;
				}
			} break;
			case e_EventType::Event_Send:
			{
				delete poverlapped;
			} break;
			case e_EventType::Event_Player_Move:
			{
				//int npc_id = poverlapped->targetid;
				//lua_State* L = aNPC[npc_id]->m_L;
				//
				//// lua 가상머신은 멀티스레드 고려안됨 lock 필요
				//aNPC[npc_id]->m_lMutex.lock();
				//lua_getglobal(L, "event_player_move");
				//lua_pushnumber(L, pclient->GetID());
				//lua_pcall(L, 1, 0, 0);
				//aNPC[npc_id]->m_lMutex.unlock();

				delete poverlapped;
			} break;
			case e_EventType::Timer_NPC_Run:
			{
				//DWORD npcid = poverlapped->targetid;
				////aNPC[npcid]->m_eMoveType = e_NPCMoveType::e_RandomMove;
				//
				//lua_State* L = aNPC[npcid]->m_L;
				//
				//// lua 가상머신은 멀티스레드 고려안됨 lock 필요
				//aNPC[npcid]->m_lMutex.lock();
				//lua_getglobal(L, "event_npc_run");
				//lua_pushnumber(L, pclient->GetID());
				//lua_pcall(L, 1, 0, 0);
				//aNPC[npcid]->m_lMutex.unlock();

				delete poverlapped;
			} break;
			case e_EventType::Timer_Player_Can_Move:
			{
				pclient->m_bIsCanMove = true;

				delete poverlapped;
			} break;
			case e_EventType::Timer_Player_Can_Attack:
			{
				pclient->m_bIsCanAttack = true;

				delete poverlapped;
			} break;
			case e_EventType::Timer_Player_HPMP_Regen:
			{
				if (pclient->CanHeal(true) || pclient->CanHeal(false)) {
					int heal_hp_amount = pclient->Heal(10, true);

					int heal_mp_amount = pclient->Heal(10, false);

					SendPlayerInfo(pclient->GetID());
				}
				g_Timer.AddTimer(GetTickCount64() + 5000, e_EventType::Timer_Player_HPMP_Regen, pclient);
			} break;
			case e_EventType::Timer_Player_Respawn:
			{
				pclient->Respawn();
				if (pclient->IsRun() == true) {
					SendPutPlayer(pclient->GetID());
					SendPlayerInfo(pclient->GetID());
					UpdateViewList(pclient->GetID());
					TCHAR msg[MAX_STR_LEN]{};
					wsprintf(msg, L"부활하였습니다.");
					SendNoticeChat(pclient->GetID(), msg, false);
				}
				delete poverlapped;
			} break;
			case e_EventType::Timer_Skill_Effect:
			{
				CSkill* pskill = aSkill[poverlapped->targetid];
				if (pclient->IsRun() == false) {
					pskill->Disappear();
					break;
				}
				int sx = pskill->GetX();
				int sy = pskill->GetY();
				int dx = pskill->m_nDir[0], dy = pskill->m_nDir[1];
				int skill_damage = 0;
				int monster_hp = 0;
				switch (pskill->m_nSkillType)
				{
				case SKILL_ENERGYBALL:
				{
					pskill->m_nX += dx;
					pskill->m_nY += dy;
					sx = pskill->m_nX;
					sy = pskill->m_nY;
					if (g_MapData[sx][sy] == MAP_OBSTACLE_TILE || g_MapData[sx][sy] == MAP_STONE_TILE) {
						pskill->Disappear();
						//SendRemoveSkillTo(pskill, pclient->GetID());
						SendRemoveSkill(pskill, pclient->GetID(), true);
						break;
					}
					if (sx < 0 || sx > WORLD_WIDTH
						|| sy < 0 || sy > WORLD_HEIGHT) {
						pskill->Disappear();
						//SendRemoveSkillTo(pskill, pclient->GetID());
						SendRemoveSkill(pskill, pclient->GetID(), true);
						break;
					}

					for (int i = 2; i < NUM_NPC; ++i) {
						CMonster* pmonster = (CMonster*)aNPC[i];
						if (pmonster->IsRun() == false) continue;
						if (IsInEffectRange(sx, sy, pmonster->GetX(), pmonster->GetY(), 0) == false) continue;
						monster_hp = pskill->Effect(pclient, (CMonster*)pmonster, &skill_damage);
						
						pmonster->m_AggroMutex.lock();
						pmonster->m_pLastViewPlayer = pclient;
						pmonster->m_AggroMutex.unlock();
						pmonster->m_bIsAttacked = true;
						g_Timer.AddTimer(GetTickCount64() + 5000, e_EventType::Timer_Monster_Aggro_Reset, nullptr, i);

						TCHAR msg[MAX_STR_LEN]{};
						wsprintf(msg, L"%s로 몬스터 %s에게 %3d 데미지를 주었습니다.", pskill->m_Name, pmonster->m_Name, skill_damage);
						SendNoticeChat(pclient->GetID(), msg, false);

						if (monster_hp <= 0)
							DieMonster(pmonster, pclient);

						//SendRemoveSkillTo(pskill, pclient->GetID());
						SendRemoveSkill(pskill, pclient->GetID(), true);
						pskill->Disappear();

						break;
					}
					if (skill_damage == 0) {
						//pskill->m_nX += dx;
						//pskill->m_nY += dy;
						//SendMoveSkillTo(pskill, pclient->GetID());
						SendMoveSkill(pskill, pclient->GetID(), true);
					}
				} break;
				case SKILL_FIREWALL:
				{
					for (int i = 2; i < NUM_NPC; ++i) {
						CMonster* pmonster = (CMonster*)aNPC[i];
						if (pmonster->IsRun() == false) continue;
						if (IsInEffectRange(sx, sy, pmonster->GetX(), pmonster->GetY(), 0) == false) continue;
						//	if (IsInEffectRange(sx + dx, sy + dy, pmonster->GetX(), pmonster->GetY(), 0) == false) 
						monster_hp = pskill->Effect(pclient, (CMonster*)pmonster, &skill_damage);

						pmonster->m_AggroMutex.lock();
						pmonster->m_pLastViewPlayer = pclient;
						pmonster->m_AggroMutex.unlock();
						pmonster->m_bIsAttacked = true;
						g_Timer.AddTimer(GetTickCount64() + 5000, e_EventType::Timer_Monster_Aggro_Reset, nullptr, i);

						TCHAR msg[MAX_STR_LEN]{};
						wsprintf(msg, L"%s로 몬스터 %s에게 %3d 데미지를 주었습니다.", pskill->m_Name, pmonster->m_Name, skill_damage);
						SendNoticeChat(pclient->GetID(), msg, false);

						if (monster_hp <= 0)
							DieMonster(pmonster, pclient);
					}
				} break;
				case SKILL_FROZEN:
				{
					for (int i = 2; i < NUM_NPC; ++i) {
						CMonster* pmonster = (CMonster*)aNPC[i];
						if (pmonster->IsRun() == false) continue;
						if (IsInEffectRange(sx, sy, pmonster->GetX(), pmonster->GetY(), 2) == false) continue;
						monster_hp = pskill->Effect(pclient, (CMonster*)pmonster, &skill_damage);

						pmonster->m_AggroMutex.lock();
						pmonster->m_pLastViewPlayer = pclient;
						pmonster->m_AggroMutex.unlock();
						pmonster->m_bIsAttacked = true;
						g_Timer.AddTimer(GetTickCount64() + 5000, e_EventType::Timer_Monster_Aggro_Reset, nullptr, i);

						TCHAR msg[MAX_STR_LEN]{};
						wsprintf(msg, L"%s로 몬스터 %s에게 %3d 데미지를 주었습니다.", pskill->m_Name, pmonster->m_Name, skill_damage);
						SendNoticeChat(pclient->GetID(), msg, false);

						if (monster_hp <= 0)
							DieMonster(pmonster, pclient);
					}
				} break;
				}
				if (pskill->m_nUsedTime + pskill->m_nDurationTime > GetTickCount64()) {
					if (pskill->IsRun() == true)
						g_Timer.AddTimer(GetTickCount64() + 1000, e_EventType::Timer_Skill_Effect, pclient, pskill->GetID());
				}
				else {
					if (pskill->IsRun() == true) {
						pskill->Disappear();
						//SendRemoveSkillTo(pskill, pclient->GetID());
						SendRemoveSkill(pskill, pclient->GetID(), true);
					}
				}
				delete poverlapped;
			} break;
			default: break;
			}
		}
		else
		{
			if (WAIT_TIMEOUT == retval)
			{

			}
			else if (ERROR_NETNAME_DELETED == retval)
			{
				cout << "WorkerThread ERROR CODE : ERROR_NETNAME_DELETED\n";
				cout << "ID : " << static_cast<int>(pclient->GetID()) << " LogOut!\n";
				DisconnectClient(pclient);
			}
			else
			{
				cout << "GetQueuedCompletionStatus unknown Error " << WSAGetLastError() << "\n";
				cout << "ID : " << static_cast<int>(pclient->GetID()) << " LogOut!\n";
				DisconnectClient(pclient);
			}

			continue;
		}
	}
}

VOID TimerThread()
{
	while (1)
	{
		Sleep(1);

		g_Timer.ProcessTimer();
	}
}

CChessClient *InsertClient(SOCKET &socket)
{
	CChessClient* pclient = nullptr;
	
	{
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (ChessClients[i]->m_bIsRun == false)
			{
				pclient = ChessClients[i];
				pclient->m_ClientInfo->m_Id = i;
				break;
			}
		}
		if (pclient != nullptr)
		{
			if (pclient->Initial(socket) == false)
				pclient = nullptr;

			g_Mutex.lock();
			mapChessClients[pclient->GetID()] = pclient;
			g_Mutex.unlock();
		}
	}

	return pclient;
}

const bool DeleteClient(CChessClient *pclient)
{
	//g_Mutex.lock();
	{
		pclient->m_bIsRun = false;
		g_Mutex.lock();
		pclient->m_ClientInfo->ClearIoBuffer();
		g_Mutex.unlock();

		CChessClient* potherClient = nullptr;
		// 다른 클라들의 뷰리스트에서 지워주자
		g_Mutex.lock();
		auto clients = mapChessClients;
		g_Mutex.unlock();

		for (auto& client : clients) {
			potherClient = client.second;
			if (potherClient == nullptr)
				continue;
			if (pclient == potherClient)
				continue;
			
			potherClient->m_vlMutex.lock();
			if(potherClient->m_ViewList.count(pclient) > 0)
				potherClient->m_ViewList.erase(pclient);	
			potherClient->m_vlMutex.unlock();
		}
		
		for (DWORD i = 0; i < NUM_NPC; ++i) {
			aNPC[i]->m_vlMutex.lock();
			if (aNPC[i]->m_ViewList.count(pclient) > 0)
				aNPC[i]->m_ViewList.erase(pclient);
			aNPC[i]->m_vlMutex.unlock();
		}
		pclient->m_vlMutex.lock();
		{
			pclient->m_ViewList.clear();
		}
		pclient->m_vlMutex.unlock();
		pclient->m_npcvlMutex.lock();
		{
			pclient->m_NPCViewList.clear();
		}
		pclient->m_npcvlMutex.unlock();

		g_Mutex.lock();
		mapChessClients.erase(pclient->GetID());
		g_Mutex.unlock();
	}
	//g_Mutex.unlock();

	return true;
}

const bool DisconnectClient(CChessClient* pclient)
{
	if(pclient->m_bIsTest == false)
		g_DB.SaveClientInfoInDB(pclient);

	DeleteClient(pclient);

	SendRemovePlayer(pclient->GetID(), true);

	return true;
}

CChessClient* FindPlayerByName(TCHAR* pname)
{
	g_Mutex.lock();
	auto players = mapChessClients;
	g_Mutex.unlock();

	CChessClient* pplayer = nullptr;

	for (auto player : players)
	{
		pplayer = player.second;
		if (lstrcmp(pplayer->m_Name, pname) == 0) 
			break;
		
		pplayer = nullptr;
	}

	return pplayer;
}

// 시야는 7 x 7
VOID UpdateViewList(INT id)
{
	CChessClient* pclient = mapChessClients[id];

	int mx = pclient->GetX();
	int my = pclient->GetY();

	CChessClient* potherClient = nullptr;
	int ox = 0;
	int oy = 0;

	g_Mutex.lock();
	auto clients = mapChessClients;
	g_Mutex.unlock();

	for (auto& client : clients)
	{
		potherClient = client.second;
		if (potherClient == nullptr) 
			continue;
		if (potherClient->IsRun() == false)
			continue;
		if (pclient == potherClient)
			continue;
		
		ox = potherClient->GetX();
		oy = potherClient->GetY();

		// 시야안에 있으면 nearlist에 넣어준다.
		if ((abs(mx - ox) <= VIEW_RADIUS) && (abs(my - oy) <= VIEW_RADIUS))
			pclient->m_NearList.emplace(potherClient);
	}
		
	// 시야 리스트에 있으면
	for (auto& n : pclient->m_NearList) {
		// 시야리스트에서 찾는다.
		pclient->m_vlMutex.lock();
		auto mv = find(pclient->m_ViewList.begin(), pclient->m_ViewList.end(), n);
		// 시야리스트에 없었다면
		if (mv == pclient->m_ViewList.end()) {
			pclient->m_ViewList.emplace(n);
			pclient->m_vlMutex.unlock();
			SendPutPlayerTo(n->GetID(), pclient->GetID());
			// 다른 클라의 시야리스트 검색
			n->m_vlMutex.lock();
			auto ov = find(n->m_ViewList.begin(), n->m_ViewList.end(), pclient);
			if (ov == n->m_ViewList.end()) {
				//n->m_ViewList.emplace_back(pclient);
				n->m_ViewList.emplace(pclient);
				n->m_vlMutex.unlock();
				SendPutPlayerTo(pclient->GetID(), n->GetID());
			}
			else {
				n->m_vlMutex.unlock();
				SendMovePlayerTo(pclient->GetID(), n->GetID());
			}
		}
		else {
			pclient->m_vlMutex.unlock();
			n->m_vlMutex.lock();
			auto ov = find(n->m_ViewList.begin(), n->m_ViewList.end(), pclient);
			if (ov == n->m_ViewList.end()) {
				//n->m_ViewList.emplace_back(pclient);
				n->m_ViewList.emplace(pclient);
				n->m_vlMutex.unlock();
				SendPutPlayerTo(pclient->GetID(), n->GetID());
			}
			else {
				n->m_vlMutex.unlock();
				SendMovePlayerTo(pclient->GetID(), n->GetID());
			}
		}
	}
	pclient->m_vlMutex.lock();
	auto vl = pclient->m_ViewList;
	pclient->m_vlMutex.unlock();
	for (auto& v : vl) {
		auto mn = find(pclient->m_NearList.begin(), pclient->m_NearList.end(), v);
		if (mn == pclient->m_NearList.end()) {
			pclient->m_RemoveList.emplace(v);
			SendRemovePlayerTo(v->GetID(), pclient->GetID());
			v->m_vlMutex.lock();
			auto ov = find(v->m_ViewList.begin(), v->m_ViewList.end(), pclient);
			if (ov == v->m_ViewList.end()) {
				v->m_vlMutex.unlock();
			}
			else {
				v->m_ViewList.erase(ov);
				v->m_vlMutex.unlock();
				SendRemovePlayerTo(pclient->GetID(), v->GetID());
			}
		}
		else {

		}
	}
		
	for (auto& r : pclient->m_RemoveList) {
		pclient->m_vlMutex.lock();
		auto mv = find(pclient->m_ViewList.begin(), pclient->m_ViewList.end(), r);
		if (mv == pclient->m_ViewList.end()) {
			pclient->m_vlMutex.unlock();
		}
		else {
			pclient->m_ViewList.erase(mv);
			pclient->m_vlMutex.unlock();
		}
	}
	
	pclient->m_RemoveList.clear();
	pclient->m_NearList.clear();

	///////////////////////////////////	NPC ///////////////////////////////////
	pclient->m_npcvlMutex.lock();
	auto npcView = pclient->m_NPCViewList;
	pclient->m_npcvlMutex.unlock();
	unordered_set<CNPC*> npcNear;

	int nx = 0;
	int ny = 0;
	for (DWORD i = 0; i < NUM_NPC; ++i)
	{
		if (aNPC[i]->IsRun() == false)
			continue;

		nx = aNPC[i]->GetX();
		ny = aNPC[i]->GetY();

		if ((abs(mx - nx) <= VIEW_RADIUS) && (abs(my - ny) <= VIEW_RADIUS))
			npcNear.emplace(aNPC[i]);
	}

	for (auto& npc : npcView)
	{
		if (npcNear.count(npc) == 0) continue;
		npc->m_vlMutex.lock();
		if (0 < npc->m_ViewList.count(pclient)) {
			npc->m_vlMutex.unlock();
			continue;
		}
		else {
			npc->m_ViewList.emplace(pclient);
			npc->m_vlMutex.unlock();
		}
	}
	for (auto& npc : npcNear)
	{
		if (0 < npcView.count(npc)) continue;
		pclient->m_npcvlMutex.lock();
		pclient->m_NPCViewList.insert(npc);
		pclient->m_npcvlMutex.unlock();
		SendPutNPCTo(npc, pclient->GetID());
		if (npc->WakeUp() == true)
			g_Timer.AddTimer(GetTickCount64() + 1000, e_EventType::Timer_NPC_Move, nullptr, npc->GetID());
		npc->m_vlMutex.lock();
		if (0 == npc->m_ViewList.count(pclient)) {
			npc->m_ViewList.insert(pclient);
		}
		npc->m_vlMutex.unlock();
	}
	for (auto& npc : npcView)
	{
		if (0 < npcNear.count(npc)) continue;
		pclient->m_npcvlMutex.lock();
		pclient->m_NPCViewList.erase(npc);
		pclient->m_npcvlMutex.unlock();
		SendRemoveNPCTo(npc, pclient->GetID());
		npc->m_vlMutex.lock();
		if (0 < npc->m_ViewList.count(pclient)) 
			npc->m_ViewList.erase(pclient);
		npc->m_vlMutex.unlock();
	}

	for (auto& npc : npcNear)
	{
		SOCKETINFO* ex_over = new SOCKETINFO;
		ex_over->eventType = e_EventType::Event_Player_Move;
		ex_over->targetid = npc->GetID();
		PostQueuedCompletionStatus(g_Iocp, 1, reinterpret_cast<ULONG_PTR>(pclient), &ex_over->overlapped);
	}
}

VOID UpdateNPCViewList(CNPC* pnpc)
{
	if (pnpc->IsRun() == false)
		return;
	
	int nx = pnpc->GetX();
	int ny = pnpc->GetY();
	bool isAggroed = false;

	CChessClient* pclient = nullptr;
	int cx = 0;
	int cy = 0;

	if (pnpc->m_pLastViewPlayer != nullptr) {
		cx = pnpc->m_pLastViewPlayer->GetX(); cy = pnpc->m_pLastViewPlayer->GetY();
		isAggroed = true;
		if (   ( pnpc->m_pLastViewPlayer->IsRun() == false )
			|| ( IsMonsterAvailableXY(cx, cy) == false )
			|| ( ((abs(nx - cx) <= MONSTER_AGGRO_RADIUS * 2) && (abs(ny - cy) <= MONSTER_AGGRO_RADIUS * 2)) == false) )
		{
			pnpc->m_AggroMutex.lock();
			pnpc->m_pLastViewPlayer = nullptr;
			pnpc->m_AggroMutex.unlock();
			isAggroed = false;
		}
	}

	pnpc->m_vlMutex.lock();
	auto viewList = pnpc->m_ViewList;
	pnpc->m_vlMutex.unlock();
	unordered_set<CChessClient*> nearList;
	unordered_set<CChessClient*> moveList;

	g_Mutex.lock();
	auto clients = mapChessClients;
	g_Mutex.unlock();

	int mindist{ 10000 };
	for (auto& client : clients)
	{
		pclient = client.second;
		if (pclient == nullptr) continue;

		cx = pclient->GetX();
		cy = pclient->GetY();

		if ((abs(nx - cx) <= VIEW_RADIUS) && (abs(ny - cy) <= VIEW_RADIUS))
			nearList.emplace(pclient);

		if ((abs(nx - cx) <= NPC_MOVE_RADIUS) && (abs(ny - cy) <= NPC_MOVE_RADIUS))
			moveList.emplace(pclient);

		if (isAggroed == false) {
			if ((abs(nx - cx) <= MONSTER_AGGRO_RADIUS) && (abs(ny - cy) <= MONSTER_AGGRO_RADIUS)) {
				if (IsMonsterAvailableXY(cx, cy) == false) continue;
				int dist = int(sqrt(pow(nx - cx, 2) + pow(ny - cx, 2)));
				if (dist > mindist) continue;
				mindist = dist;
				pnpc->m_AggroMutex.lock();
				pnpc->m_pLastViewPlayer = pclient;
				pnpc->m_AggroMutex.unlock();
			}
		}
	}

	//for (auto pl : nearList) {
	//	if (0 == pl->m_NPCViewList.count(pnpc)) {
	//		pl->m_NPCViewList.insert(pnpc);
	//		SendPutNPCTo(pnpc, pl->GetID());
	//		if (viewList.count(pl) == 0)
	//			pnpc->m_ViewList.emplace(pl);
	//	}
	//	else
	//		SendMoveNPCTo(pnpc, pl->GetID());
	//}
	//// 헤어진 플레이어 처리
	//for (auto pl : viewList) {
	//	if (0 == nearList.count(pl)) {
	//		pnpc->m_ViewList.erase(pl);
	//		if (0 < pl->m_NPCViewList.count(pnpc)) {
	//			pl->m_NPCViewList.erase(pnpc);
	//			SendRemoveNPCTo(pnpc, pl->GetID());
	//		}
	//	}
	//}

	for (auto& pl : viewList)
	{
		if (0 == nearList.count(pl)) continue;
		pl->m_npcvlMutex.lock();
		if (0 < pl->m_NPCViewList.count(pnpc)) {
			pl->m_npcvlMutex.unlock();
			SendMoveNPCTo(pnpc, pl->GetID());
		}
		else {
			pl->m_NPCViewList.insert(pnpc);
			pl->m_npcvlMutex.unlock();
			SendPutNPCTo(pnpc, pl->GetID());
		}
	}
	for (auto& pl : nearList)
	{
		if (0 < viewList.count(pl)) continue;
		pnpc->m_vlMutex.lock();
		pnpc->m_ViewList.insert(pl);
		pnpc->m_vlMutex.unlock();
		pl->m_npcvlMutex.lock();
		if (0 == pl->m_NPCViewList.count(pnpc)) {
			pl->m_NPCViewList.insert(pnpc);
			pl->m_npcvlMutex.unlock();
			SendPutNPCTo(pnpc, pl->GetID());
		}
		else {
			pl->m_npcvlMutex.unlock();
			SendMoveNPCTo(pnpc, pl->GetID());
		}
	}
	for (auto& pl : nearList)
	{
		if (0 < viewList.count(pl)) continue;
		pnpc->m_vlMutex.lock();
		pnpc->m_ViewList.erase(pl);
		pnpc->m_vlMutex.unlock();
		pl->m_npcvlMutex.lock();
		if (0 < pl->m_NPCViewList.count(pnpc)) {
			pl->m_NPCViewList.erase(pnpc);
			pl->m_npcvlMutex.unlock();
			SendRemoveNPCTo(pnpc, pl->GetID());
		}
		else
			pl->m_npcvlMutex.unlock();
	}

	if (moveList.empty() == false || pnpc->m_pLastViewPlayer == nullptr)
		g_Timer.AddTimer(GetTickCount64() + NPC_MOVE_COOLTIME, e_EventType::Timer_NPC_Move, nullptr, pnpc->GetID());
	else
		pnpc->m_bIsSleep = true;
}

bool IsMonsterAvailableXY(int x, int y)
{
	if (x < 0)
		return false;
	else if (x > WORLD_WIDTH - 1)
		return false;
	if (y < 0)
		return false;
	else if (y > WORLD_HEIGHT - 1)
		return false;
	if (g_MapData[x][y] != MAP_GRASS_TILE)
		return false;

	return true;
}

Point PathFinder(int startX, int startY, int targetX, int targetY)
{
	//g_Mutex.lock();
	int tempX{}, tempY{}, x{}, y{};
	vector<Node> open;
	Point nextpath{startX, startY};
	x = startX;
	y = startY;
	int g{}, h{};
	tempX = x - 1;
	tempY = y - 1;
	if (IsMonsterAvailableXY(tempX, tempY) == true) {
		Node temp;
		temp.x = tempX;
		temp.y = tempY;
		g = int(pow(tempX - x, 2) + pow(tempY - y, 2));
		h = int(pow(targetX - tempX, 2) + pow(targetY - tempY, 2));
		temp.value = g + h;
		open.emplace_back(temp);
	}
	tempX = x - 1;
	tempY = y;
	if (IsMonsterAvailableXY(tempX, tempY) == true) {
		Node temp;
		temp.x = tempX;
		temp.y = tempY;
		g = int(pow(tempX - x, 2) + pow(tempY - y, 2));
		h = int(pow(targetX - tempX, 2) + pow(targetY - tempY, 2));
		temp.value = g + h;
		open.emplace_back(temp);
	}
	tempX = x - 1;
	tempY = y + 1;
	if (IsMonsterAvailableXY(tempX, tempY) == true) {
		Node temp;
		temp.x = tempX;
		temp.y = tempY;
		g = int(pow(tempX - x, 2) + pow(tempY - y, 2));
		h = int(pow(targetX - tempX, 2) + pow(targetY - tempY, 2));
		temp.value = g + h;
		open.emplace_back(temp);
	}
	tempX = x;
	tempY = y + 1;
	if (IsMonsterAvailableXY(tempX, tempY) == true) {
		Node temp;
		temp.x = tempX;
		temp.y = tempY;
		g = int(pow(tempX - x, 2) + pow(tempY - y, 2));
		h = int(pow(targetX - tempX, 2) + pow(targetY - tempY, 2));
		temp.value = g + h;
		open.emplace_back(temp);
	}
	tempX = x + 1;
	tempY = y + 1;
	if (IsMonsterAvailableXY(tempX, tempY) == true) {
		Node temp;
		temp.x = tempX;
		temp.y = tempY;
		g = int(pow(tempX - x, 2) + pow(tempY - y, 2));
		h = int(pow(targetX - tempX, 2) + pow(targetY - tempY, 2));
		temp.value = g + h;
		open.emplace_back(temp);
	}
	tempX = x + 1;
	tempY = y;
	if (IsMonsterAvailableXY(tempX, tempY) == true) {
		Node temp;
		temp.x = tempX;
		temp.y = tempY;
		g = int(pow(tempX - x, 2) + pow(tempY - y, 2));
		h = int(pow(targetX - tempX, 2) + pow(targetY - tempY, 2));
		temp.value = g + h;
		open.emplace_back(temp);
	}
	tempX = x + 1;
	tempY = y - 1;
	if (IsMonsterAvailableXY(tempX, tempY) == true) {
		Node temp;
		temp.x = tempX;
		temp.y = tempY;
		g = int(pow(tempX - x, 2) + pow(tempY - y, 2));
		h = int(pow(targetX - tempX, 2) + pow(targetY - tempY, 2));
		temp.value = g + h;
		open.emplace_back(temp);
	}
	tempX = x;
	tempY = y - 1;
	if (IsMonsterAvailableXY(tempX, tempY) == true) {
		Node temp;
		temp.x = tempX;
		temp.y = tempY;
		g = int(pow(tempX - x, 2) + pow(tempY - y, 2));
		h = int(pow(targetX - tempX, 2) + pow(targetY - tempY, 2));
		temp.value = g + h;
		open.emplace_back(temp);
	}

	if (open.empty() == true)
		return nextpath;

	Node* temp = &open[0];
	
	for (int i = 0; i < open.size(); ++i) {
		if (temp->value > open[i].value)
			temp = &open[i];
	}
	nextpath.x = temp->x;
	nextpath.y = temp->y;

	if (nextpath.x == targetX && nextpath.y == targetY) {
		nextpath.x = startX;
		nextpath.y = startY;
	}
	//g_Mutex.unlock();
	return nextpath;
}