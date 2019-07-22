#pragma once

#include "NPC.h"

extern int g_MapData[WORLD_HEIGHT][WORLD_WIDTH];

CNPC::CNPC()
	: m_bIsRun(false)
	, m_bIsSleep(true)
	, m_nX(-1)
	, m_nY(-1)
{
	m_pLastViewPlayer = nullptr;
	//m_eMoveType = e_NPCMoveType::e_RandomMove;
	m_L = luaL_newstate();
	luaL_openlibs(m_L);

	int err = luaL_loadfile(m_L, "npc_ai.lua");
	lua_pcall(m_L, 0, 0, 0);
}

CNPC::~CNPC()
{
	lua_close(m_L);
}

void CNPC::Initalize(UINT dwid, INT ntype)
{
	m_bIsRun = true;

	m_dwID = dwid;
	m_nType = ntype;

	m_nLevel = 1000;

	if (m_nType == NPC_FAIRY) {
		wsprintf(m_Name, L"Fairy");
		m_bIsCanMove = true;
		m_bIsCanAttack = false;
		m_nX = 6;
		m_nY = 6;
	}
	else if (m_nType == NPC_PRIEST) {
		wsprintf(m_Name, L"PRIEST");
		m_bIsCanMove = true;
		m_bIsCanAttack = false;
		m_nX = 6;
		m_nY = 10;
	}

	int err = luaL_loadfile(m_L, "npc_ai.lua");
	lua_pcall(m_L, 0, 0, 0);
	//else {
	//	int x{ 0 }, y{ 0 };
	//	do {
	//		x = RandomINT(0, WORLD_WIDTH - 1);
	//		y = RandomINT(0, WORLD_HEIGHT - 1);
	//		if (x < 15)
	//			if (y < 15) {
	//				x += 15 - x;
	//			}
	//	} while (g_MapData[x][y] != MAP_GRASS_TILE);
	//	m_nX = x;
	//	m_nY = y;
	//}
	//lua_State* L = m_L;
}

void CNPC::Move()
{
	if (m_bIsRun == false)
		return;
	if (m_bIsCanMove == false)
		return;

	int x = m_nX, y = m_nY;

	x += RandomINT(-1, 1);
	y += RandomINT(-1, 1);
	if (g_MapData[x][y] == MAP_WAY_TILE) {
		m_nX = x;
		m_nY = y;
	}

	if (m_nX < 0)
		m_nX += 1;
	else if (m_nX > WORLD_WIDTH)
		m_nX -= 1;
	if (m_nY < 0)
		m_nY += 1;
	else if (m_nY > WORLD_HEIGHT)
		m_nY -= 1;
}

const bool CNPC::WakeUp()
{
	if (m_bIsRun == false)
		return false;

	if (m_bIsSleep == false)
		return false;

	m_bIsSleep = false;

	return true;
}

const bool CNPC::CanGo(int x, int y)
{
	if (x < m_nOriginX - 20)
		return false;
	else if (x > m_nOriginX + 20)
		return false;
	if (y < m_nOriginY - 20)
		return false;
	else if (y > m_nOriginY + 20)
		return false;

	return true;
}

void CNPC::DetectPlayer(CChessClient* pplayer)
{
	//m_eMoveType = e_NPCMoveType::e_RunMove;

	// 방향지정
	for (int i = 0; i < 2; ++i) {
		do {
			m_cRunDir[i] = RandomINT(-1, 1);
		} while (m_cRunDir[i] == 0);
	}
}

INT CNPC::Attack(CChessClient* pplayer, int* pdamage)
{


	return 0;
}

INT CNPC::Attacked(int damage)
{
	return 0;
}

void CNPC::Respawn()
{
}

//////////////////////////////////////////////////////////////////////////////////////////

CMonster::CMonster()
{

}

void CMonster::Initalize(UINT dwid, INT ntype)
{
	int err = luaL_loadfile(m_L, "monster.lua");
	lua_pcall(m_L, 0, 0, 0);

	m_bIsRun = true;

	m_dwID = dwid;
	m_nType = ntype;

	m_bIsAttacked = false;

	switch (ntype)
	{
	case NPC_TREE:
	{
		wsprintf(m_Name, L"TREE");
		m_bIsCanMove = false;
		m_bIsCanAttack = true;

		m_bIsPeace = true;
		m_bIsRoam = false;
	} break;
	case NPC_MUSHROOM:
	{
		wsprintf(m_Name, L"MUSHROOM");
		m_bIsCanMove = true;
		m_bIsCanAttack = true;

		m_bIsPeace = true;
		m_bIsRoam = true;
	} break;
	case NPC_MUD:
	{
		wsprintf(m_Name, L"MUD");
		m_bIsCanMove = false;
		m_bIsCanAttack = true;

		m_bIsPeace = false;
		m_bIsRoam = false;
	} break;
	case NPC_BITE:
	{
		wsprintf(m_Name, L"BITE");
		m_bIsCanMove = true;
		m_bIsCanAttack = true;

		m_bIsPeace = false;
		m_bIsRoam = true;
	} break;
	}

	int x{ 0 }, y{ 0 };
	do {
		x = RandomINT(0, WORLD_WIDTH - 1);
		y = RandomINT(0, WORLD_HEIGHT - 1);
		if (x < 15)
			if (y < 15) {
				x += 15 - x;
			}
	} while (g_MapData[x][y] != MAP_GRASS_TILE);

	int level{ 0 };
	// 0~299
	if (x < 50 && y < 50) {
		level = RandomINT(1, 5);
	}
	else if (x < 125 && y < 125) {
		level = RandomINT(5, 10);
	}
	else if (x < 200 && y < 200) {
		level = RandomINT(10, 15);
	}
	else {
		level = RandomINT(15, 30);
	}
	int damage = 20 + (level * 8);
	int hp = 20 + (level * 5);

	m_Status.m_nDamage = damage;
	m_Status.m_nHP = hp;
	m_nLevel = level;

	m_nX = x;
	m_nY = y;

	m_nOriginX = x;
	m_nOriginY = y;
}

void CMonster::Move()
{
	if (m_bIsRun == false)
		return;
	if (m_bIsCanMove == false)
		return;

	int x = m_nX, y = m_nY;

	if (m_bIsRoam == true) {
		x += RandomINT(-1, 1);
		y += RandomINT(-1, 1);
	}
	
	if (x < 0)
		x += 1;
	else if (x > WORLD_WIDTH)
		x -= 1;
	if (x < m_nOriginX - 20)
		x += 1;
	else if (x > m_nOriginX + 20)
		x -= 1;
	if (y < 0)
		y += 1;
	else if (m_nY > WORLD_HEIGHT)
		y -= 1;
	if (y < m_nOriginY - 20)
		y += 1;
	else if (y > m_nOriginY + 20)
		y -= 1;

	if (g_MapData[x][y] == MAP_GRASS_TILE) {
		m_nX = x;
		m_nY = y;
	}
}

void CMonster::Respawn()
{
	m_bIsRun = true;

	m_bIsAttacked = false;
	//int x{ 0 }, y{ 0 };
	//do {
	//	x = RandomINT(0, WORLD_WIDTH - 1);
	//	y = RandomINT(0, WORLD_HEIGHT - 1);
	//	if (x < 15)
	//		if (y < 15) {
	//			x += 15 - x;
	//		}
	//} while (g_MapData[x][y] != MAP_GRASS_TILE);
	//
	//m_nX = x;
	//m_nY = y;
}

INT CMonster::Attack(CChessClient* pplayer, int* pdamage)
{
	int damage = m_Status.m_nDamage;
	int player_hp = pplayer->Attacked(&damage);
	*pdamage = damage;

	return player_hp;
}

INT CMonster::Attacked(int damage)
{
	m_Status.m_nHP -= damage;

	return m_Status.m_nHP;
}

const INT CMonster::GetExp()
{
	int exp = 0;
	exp = BASIC_MONSTER_EXP + (m_nLevel * 15);

	if (m_bIsPeace == false)
		exp *= 2;
	if (m_bIsRoam == true)
		exp *= 2;

	return exp;
}
  
///////////////////////////////////////////////////////////////

void CFieldItem::Initalize(UINT dwid, INT ntype)
{
	m_dwID = dwid;
	m_nItemType = ntype;
	m_nItemNum = 0;
}

void CFieldItem::Dropped(int x, int y, int type)
{
	m_bIsRun = true;

	m_nX = x;
	m_nY = y;

	m_nItemType = type;

	if (type == ITEM_MONEY) {
		m_nItemNum = RandomINT(1, 3);
		wsprintf(m_Name, L"MONEY");
	}
	else if (type == ITEM_HPPOTION) {
		m_nItemNum = 1;
		wsprintf(m_Name, L"HPPOTION");
	}
	else {
		m_nItemNum = 1;
		wsprintf(m_Name, L"MPPOTION");
	}
}

////////////////////////////////////////////////////////////

void CSkill::Initalize(UINT dwid)
{
	m_bIsRun = false;
	m_dwID = dwid;
}

const bool CSkill::Use(int skill_type, int dirx, int diry, CChessClient* pplayer)
{
	m_pUsedPlayer = pplayer;

	m_nSkillType = skill_type;

	m_nUsedTime = GetTickCount64();

	m_nDir[0] = dirx;
	m_nDir[1] = diry;

	m_nX = pplayer->GetX();
	m_nY = pplayer->GetY();

	switch (skill_type)
	{
	case SKILL_ENERGYBALL:
	{
		//m_nX += m_nDir[0];
		//m_nY += m_nDir[1];

		m_nDurationTime = 10000;
		wcsncpy_s(m_Name, L"ENERGYBAL", MAX_NAME_LEN);
		if (pplayer->m_nLevel < 5)
			return false;
		if (pplayer->m_nMP < 30)
			return false;
		pplayer->m_nMP -= 30;
		//wsprintf(m_Name, L"ENERGYBALL");
		
	} break;
	case SKILL_FIREWALL:
	{
		m_nX += m_nDir[0];
		m_nY += m_nDir[1];

		m_nDurationTime = 5000;
		wsprintf(m_Name, L"FIREWALL");
		if (pplayer->m_nLevel < 10)
			return false;
		if (pplayer->m_nMP < 60)
			return false;
		pplayer->m_nMP -= 60;
		
	} break;
	case SKILL_FROZEN:
	{
		m_nDurationTime = 1000;
		wsprintf(m_Name, L"FROZEN");
		if (pplayer->m_nLevel < 15)
			return false;
		if (pplayer->m_nMP < 100)
			return false;
		pplayer->m_nMP -= 100;
		
	} break;
	}

	m_bIsRun = true;

	return true;
}

const int CSkill::Effect(CChessClient* pplayer, CMonster* pmonster, int* pdamage)
{
	int player_int = pplayer->m_Status.m_nInt;
	int damage = 0;

	switch (m_nSkillType)
	{
	case SKILL_ENERGYBALL:
	{
		damage = SKILL_ENERGYBALL_DAMAGE + ((float)SKILL_ENERGYBALL_DAMAGE / 100.f * (player_int * 10));
	} break;
	case SKILL_FIREWALL:
	{
		damage = SKILL_FIREWALL_DAMAGE + ((float)SKILL_FIREWALL_DAMAGE / 100.f * (player_int * 10));
	} break;
	case SKILL_FROZEN:
	{
		damage = SKILL_FROZEN_DAMAGE + ((float)SKILL_FROZEN_DAMAGE / 100.f * (player_int * 10));
	} break;
	}

	pmonster->m_Status.m_nHP -= damage;
	*pdamage = damage;

	return pmonster->m_Status.m_nHP;
}
