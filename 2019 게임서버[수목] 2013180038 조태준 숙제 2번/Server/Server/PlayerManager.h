#pragma once

#include "stdafx.h"
#include "NetworkManager.h"
#include "Network.h"

class CPlayerManager : public CNetworkManager
{
public:
	CPlayerManager();
	~CPlayerManager();
protected:
	float	m_fPlayerPosition[3];
	ClientPacket *m_pClientPacket;
	ServerPacket *m_pServerPacket;
public:
	virtual int Send(SOCKET &socket);
	virtual int Recv(SOCKET &socket);
	virtual void Update();
};