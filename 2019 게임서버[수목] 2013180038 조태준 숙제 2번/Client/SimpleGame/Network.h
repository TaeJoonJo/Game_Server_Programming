#pragma once

#include "stdafx.h"
#include "PlayerManager.h"
#include "NetworkManager.h"
class CNetworkManager;

class CNetwork
{
public:
	CNetwork();
	~CNetwork();
protected:
	SOCKET m_Socket;
	CNetworkManager *m_pPlayerManager;
public:
	bool Initialize();
	bool Connect();
	bool Update();
	void FirstRecv();
public:
	inline CNetworkManager *GetPlayerManager() {
		return m_pPlayerManager;
	}
};