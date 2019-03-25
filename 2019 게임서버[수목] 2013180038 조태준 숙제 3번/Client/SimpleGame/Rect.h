#pragma once

#include "GameObject.h"
#include "SceneMgr.h"
#include "Renderer.h"

class CRect :public CGameObject
{
protected:

public:
	CRect(float x, float y, int color);
	~CRect();
public:
	virtual void Render();
	virtual void Update(float time);
	virtual void KeyInput(int key);
public:
	virtual void Release();
};