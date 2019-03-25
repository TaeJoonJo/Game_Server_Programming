#include "stdafx.h"
#include "GameObject.h"


CGameObject::CGameObject()
	: m_fx(0), m_fy(0), m_fz(0),
	m_fred(0), m_fblue(0), m_fgreen(0), m_falpha(0),
	m_fsize(0), m_ntype(0), m_ntexID(0), m_fbulidlevel(0), m_nid(999),
	m_isActive(TRUE)
{
}

CGameObject::~CGameObject()
{
}

void CGameObject::Render()
{

}

void CGameObject::Update(float time)
{

}