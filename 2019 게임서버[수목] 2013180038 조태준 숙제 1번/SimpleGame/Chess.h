#pragma once

#include "GameObject.h"
#include "SceneMgr.h"
#include "Renderer.h"

class CChess :public CGameObject
{
protected:

public:
	CChess(float x, float y, int color);
	~CChess();
public:
	virtual void Render();
	virtual void Update(float time);
	virtual void KeyInput(int key);
public:
	virtual void Release();
};