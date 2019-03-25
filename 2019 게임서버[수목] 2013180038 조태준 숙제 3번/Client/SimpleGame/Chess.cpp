#include "stdafx.h"
#include "Chess.h"

CChess::CChess(float x, float y, int color)
{
	/*m_fx = -25;
	m_fy = -175;
	m_fz = 0.f;*/

	m_fsize = DEFAULTSIZE;

	SetColor(1.f, 1.f, 1.f, 1.f);

	if (color == WHITE)
		m_ntexID = CSceneMgr::GetRenderer()->CreatePngTexture("../Resource/White.png");
	else if (color == BLACK)
		m_ntexID = CSceneMgr::GetRenderer()->CreatePngTexture("../Resource/Black.png");

	m_fbulidlevel = 0.1f;
	m_ntype = CHESS;
}

CChess::~CChess()
{
}

void CChess::Render()
{
	CSceneMgr::GetRenderer()->DrawTexturedRect(m_fx, m_fy, m_fz, m_fsize, m_fred, m_fgreen, m_fblue, m_falpha, m_ntexID, m_fbulidlevel);
}

void CChess::Update(float time)
{
	
}

void CChess::KeyInput(int key)
{
	
}

void CChess::Release()
{
	delete this;
}