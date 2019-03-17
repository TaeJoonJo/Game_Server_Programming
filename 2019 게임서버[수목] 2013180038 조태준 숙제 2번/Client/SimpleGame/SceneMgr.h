#pragma once

#include "GameObject.h"
#include "Rect.h"
#include "Chess.h"
#include "Network.h"
#include "PlayerManager.h"

class Renderer;

class CSceneMgr
{
protected:
	static Renderer *g_Renderer;
	CNetwork *m_pNetwork;
	static vector<CGameObject*> m_vGameObjects;
	CGameObject* m_pChess;
public:
	CSceneMgr();
	~CSceneMgr();
public:
	bool Ready_Renderer(CNetwork *pnetwork);
	bool Ready_Objects();
	void Update_Objects(float time);
	void Draw_Objects();

	static Renderer* GetRenderer() {
		return g_Renderer;
	}
public:
	inline CGameObject* Get_Object(int index) {
		return (m_vGameObjects[index]);
	}
	inline CGameObject* GetPlayer() {
		return m_pChess;
	}

public:
	void Add_Object(float x, float y, int type, int team);
};
