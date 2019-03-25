#pragma once

#include "stdafx.h"
#include "NetworkManager.h"
#include "Network.h"
#include "SceneMgr.h"

class CPlayerManager : public CNetworkManager
{
public:
	CPlayerManager();
	~CPlayerManager();
protected:
	char m_IoBuf[MAX_BUFFER];
	INT m_Id;
public:
	virtual void StartIO(SOCKET &socket, int key);
	virtual int Send(SOCKET &socket, int key);
	virtual int Recv(SOCKET &socket);
	virtual void Update(SOCKET &socket);
	virtual void KeyInput(void *key);
	virtual void Disconnect();
};