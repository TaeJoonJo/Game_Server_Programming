#include "stdafx.h"
#include "Rect.h"

CRect::CRect(float x, float y, int color)
{
	m_fx = x;
	m_fy = y;
	m_fz = 0.f;

	m_ntexID = 0;

	m_fsize = DEFAULTSIZE;

	if(color == WHITE)
		SetColor(1.f, 1.f, 1.f, 1.f);
	else if(color == BLACK)
		SetColor(0.3f, 0.3f, 0.3f, 1.f);

	m_fbulidlevel = 0.2f;
	m_ntype = BOARD;
}

CRect::~CRect()
{
}

void CRect::Render()
{
	CSceneMgr::GetRenderer()->DrawSolidRect(m_fx, m_fy, m_fz, m_fsize, m_fred, m_fgreen, m_fblue, m_falpha, m_fbulidlevel);
}

void CRect::Update(float time)
{
	
}

void CRect::KeyInput(int key)
{

}

void CRect::Release()
{
	delete this;
}