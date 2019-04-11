#include "stdafx.h"
#include "Renderer.h"
#include "SceneMgr.h"

Renderer *CSceneMgr::g_Renderer = NULL;

vector<CGameObject*> CSceneMgr::m_vGameObjects = vector<CGameObject*>();

CSceneMgr::CSceneMgr()
{
	
}

CSceneMgr::~CSceneMgr()
{
	delete g_Renderer;

	for (auto d : m_vGameObjects)
		delete d;
	m_vGameObjects.clear();
}

bool CSceneMgr::Ready_Renderer()
{
	g_Renderer = new Renderer(WINSIZEX, WINSIZEY);
	if (!g_Renderer->IsInitialized())
		return false;

	if (!Ready_Objects())
	{
		std::cout << "오브젝트 오류" << std::endl;
		return false;
	}
	return true;
}

bool CSceneMgr::Ready_Objects()
{
	int boardsize = 8;
	int width = -175;
	int height = -175;
	int color = WHITE;

	for (int i = 0; i < boardsize; ++i)
	{
		for (int j = 0; j < boardsize; ++j)
		{
			Add_Object(width, height, BOARD, color++ % 2);
			width += 50;
		}
		++color;
		width = -175;
		height += 50;
	}
	Add_Object(0, 0, CHESS, BLACK);
	
	return true;
}

void CSceneMgr::Update_Objects(float time)
{
	float elsapedtime = time * 0.001f;
	if (elsapedtime > 10.f)
		elsapedtime = 10.f;
	
	for (int i = 0; i < m_vGameObjects.size(); ++i)
		if (m_vGameObjects[i] != NULL)
			m_vGameObjects[i]->Update(elsapedtime);
}

void CSceneMgr::Draw_Objects()
{
	for (int i = 0; i < m_vGameObjects.size(); ++i)
		if (m_vGameObjects[i] != NULL)
			m_vGameObjects[i]->Render();
}


void CSceneMgr::Add_Object(float x, float y, int type, int team)
{
	if (type == BOARD)
	{
		CGameObject* nRect = new CRect(x, y, team);

		m_vGameObjects.push_back(nRect);
	}
	else if (type == CHESS)
	{
		CGameObject* pChess = new CChess(x, y, team);

		m_vGameObjects.push_back(pChess);
		m_pChess = pChess;
	}
}
