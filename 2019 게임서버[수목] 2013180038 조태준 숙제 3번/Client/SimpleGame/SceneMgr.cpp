#include "stdafx.h"
#include "Renderer.h"
#include "SceneMgr.h"

Renderer *CSceneMgr::g_Renderer = NULL;

vector<CGameObject*> CSceneMgr::m_vGameObjects = vector<CGameObject*>();

CSceneMgr::CSceneMgr()
	: m_pNetwork(NULL), m_fSendTime(0.f)
{
	
}

CSceneMgr::~CSceneMgr()
{
	delete g_Renderer;

	for (auto d : m_vGameObjects)
		delete d;
	m_vGameObjects.clear();
}

bool CSceneMgr::Ready_Renderer(CNetwork *pnetwork)
{
	g_Renderer = new Renderer(WINSIZEX, WINSIZEY);
	if (!g_Renderer->IsInitialized())
		return false;

	m_pNetwork = pnetwork;

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
	float width = -175.f;
	float height = -175.f;
	int color = WHITE;

	for (int i = 0; i < boardsize; ++i)
	{
		for (int j = 0; j < boardsize; ++j)
		{
			Add_Object(width, height, BOARD, color++ % 2);
			width += 50.f;
		}
		++color;
		width = -175.f;
		height += 50.f;
	}

	// 접속시 보내는 처음으로 키값
	m_pNetwork->Update(KEY_FIRST, TRUE);


	return true;
}

void CSceneMgr::Update_Objects(float time)
{
	float elsapedtime = time;
	m_fSendTime += elsapedtime;

	// 1초에 한번씩 샌드해주자
	if (m_fSendTime > 1.f)
	{
		m_pNetwork->Update(KEY_IDLE, TRUE);
		m_fSendTime = 0.f;
	}

	m_pNetwork->Update(KEY_IDLE, FALSE);

	for (int i = 0; i < m_vGameObjects.size(); ++i)
		if (m_vGameObjects[i] != NULL)
			m_vGameObjects[i]->Update(elsapedtime);
}

void CSceneMgr::Draw_Objects()
{
	for (int i = 0; i < m_vGameObjects.size(); ++i)
		if (m_vGameObjects[i] != NULL && m_vGameObjects[i]->isActive() == TRUE)
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
	}
}

void CSceneMgr::Add_Object(int type, int id, bool isplayer)
{
	if (type == CHESS)
	{
		CGameObject* pChess = new CChess(0, 0, BLACK);
		pChess->SetId(id);
		m_vGameObjects.push_back(pChess);
	}
}