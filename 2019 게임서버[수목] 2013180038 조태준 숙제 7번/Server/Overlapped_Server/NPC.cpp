#pragma once

#include "NPC.h"

CNPC::CNPC()
	: m_bIsRun(false)
	, m_bIsSleep(true)
	, m_fX(0.f)
	, m_fY(0.f)
{

}

CNPC::~CNPC()
{
}

void CNPC::Initalize(DWORD dwid)
{
	m_bIsRun = true;

	m_dwID = dwid;

	m_fX = (float)RandomINT(0, WORLD_WIDTH - 1);
	m_fY = (float)RandomINT(0, WORLD_HEIGHT - 1);
}

void CNPC::Move()
{
	m_fX += (float)RandomINT(-1, 1);
	m_fY += (float)RandomINT(-1, 1);

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
