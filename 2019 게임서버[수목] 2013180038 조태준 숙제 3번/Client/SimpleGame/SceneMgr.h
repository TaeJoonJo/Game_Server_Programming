#pragma once

#include "GameObject.h"
#include "Rect.h"
#include "Chess.h"
#include "Network.h"
#include "PlayerManager.h"

class Renderer;
class CNetwork;

class CSceneMgr
{
protected:
	static Renderer *g_Renderer;
	CNetwork *m_pNetwork;
	static vector<CGameObject*> m_vGameObjects;
	CGameObject* m_pChess;
	float m_fSendTime;
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
	static inline CGameObject* Get_Object(int index) {
		return (m_vGameObjects[index]);
	}
	static inline CGameObject* Get_Object_By_ID(int id) {
		CGameObject *pobject = NULL;
		for (int i = 0; i < m_vGameObjects.size(); ++i) 
			if (m_vGameObjects[i]->GetId() == id) {
				pobject = m_vGameObjects[i];	break;
			} 
		return pobject;
	}
	inline CGameObject* GetPlayer() {
		return m_pChess;
	}

public:
	static void Add_Object(float x, float y, int type, int team);
	static void Add_Object(int type, int id, bool isplayer);
};
