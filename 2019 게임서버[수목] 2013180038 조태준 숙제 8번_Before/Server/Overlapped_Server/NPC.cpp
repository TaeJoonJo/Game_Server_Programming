#pragma once

#include "NPC.h"

CNPC::CNPC()
	: m_bIsRun(false)
	, m_bIsSleep(true)
	, m_fX(0.f)
	, m_fY(0.f)
{
	m_eMoveType = e_NPCMoveType::e_RandomMove;

	m_L = luaL_newstate();
	luaL_openlibs(m_L);

	int err = luaL_loadfile(m_L, "npc_ai.lua");
	lua_pcall(m_L, 0, 0, 0);
}

CNPC::~CNPC()
{
	lua_close(m_L);
}

void CNPC::Initalize(UINT dwid)
{
	m_bIsRun = true;

	m_dwID = dwid;

	m_fX = (float)RandomINT(0, WORLD_WIDTH - 1);
	m_fY = (float)RandomINT(0, WORLD_HEIGHT - 1);

	//lua_State* L = m_L;
}

void CNPC::Move()
{
	switch (m_eMoveType)
	{
	case e_NPCMoveType::e_RandomMove:
	{
		m_fX += (float)RandomINT(-1, 1);
		m_fY += (float)RandomINT(-1, 1);
	} break;
	case e_NPCMoveType::e_RunMove:
	{
		m_fX += (float)m_cRunDir[0];
		m_fY += (float)m_cRunDir[1];
	} break;
	default: break;
	}

	if (m_fX < 0.f)
		m_fX += 1.f;
	else if (m_fX > WORLD_WIDTH)
		m_fX -= 1.f;
	if (m_fY < 0.f)
		m_fY += 1.f;
	else if (m_fY > WORLD_HEIGHT)
		m_fY -= 1.f;
}

bool CNPC::WakeUp()
{
	if (m_bIsSleep == false)
		return false;

	m_bIsSleep = false;

	return true;
}

void CNPC::DetectPlayer(CChessClient* pplayer)
{
	m_eMoveType = e_NPCMoveType::e_RunMove;

	// 방향지정
	for (int i = 0; i < 2; ++i) {
		do {
			m_cRunDir[i] = RandomINT(-1, 1);
		} while (m_cRunDir[i] == 0);
	}
}
