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
private:
	SOCKET m_ListenSocket;
	SOCKET m_ClientSocket;
	CNetworkManager *m_pPlayerManager;
public:
	bool Initialize();
	void Accept();
	void UserLogout();
	bool Update();
};