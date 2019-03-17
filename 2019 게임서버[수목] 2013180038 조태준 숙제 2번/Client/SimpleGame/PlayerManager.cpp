#pragma once
#include "stdafx.h"
#include "PlayerManager.h"

CPlayerManager::CPlayerManager()
	: m_pClientPacket(NULL), m_pServerPacket(NULL)
{
	m_pClientPacket = new CLIENTPACKET;
	ZeroMemory(m_pClientPacket, sizeof(CLIENTPACKET));
	m_pServerPacket = new SERVERPACKET;
	ZeroMemory(m_pServerPacket, sizeof(SERVERPACKET));
}

CPlayerManager::~CPlayerManager()
{
	delete m_pClientPacket;
	delete m_pServerPacket;
}

int CPlayerManager::Send(SOCKET &socket)
{
	char *buf = reinterpret_cast<char *>(m_pClientPacket);
	UINT len = sizeof(CLIENTPACKET);
	
	int retval = send(socket, buf, len, 0);
	if (retval == SOCKET_ERROR) {
		CNetworkManager::ErrDisplay("[Send() Error]");
		return retval;
	}

	ZeroMemory(m_pClientPacket, sizeof(SERVERPACKET));

	return retval;
}

int CPlayerManager::Recv(SOCKET &socket)
{
	char buf[BUFSIZE];	
	ZeroMemory(buf, sizeof(buf));

	UINT len = sizeof(SERVERPACKET);
	int retval{};
	retval = Recvn(socket, buf, len, 0);
	if (retval == SOCKET_ERROR) {
		CNetworkManager::ErrDisplay("[recv() Error]");
		return retval;
	}

	buf[retval] = '\0';

	memcpy(m_pServerPacket, buf, len);

	return retval;
}

void CPlayerManager::Update()
{
	m_pGameObject->SetPosition(m_pServerPacket->m_fPosition[0], m_pServerPacket->m_fPosition[1], m_pServerPacket->m_fPosition[2]);

	ZeroMemory(m_pServerPacket, sizeof(CLIENTPACKET));
}

void CPlayerManager::KeyInput(void *key)
{
	int keytype = (int)key;

	if (keytype == 0x0064)		//LEFT
	{
		m_pClientPacket->m_eKey = m_Left;
	}

	if (keytype == 0x0066)		// RIGHT
	{
		m_pClientPacket->m_eKey = m_Right;
	}

	if (keytype == 0x0065)		// UP
	{
		m_pClientPacket->m_eKey = m_Up;
	}

	if (keytype == 0x0067)		// DOWN
	{
		m_pClientPacket->m_eKey = m_Down;
	}
}