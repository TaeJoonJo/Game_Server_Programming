#pragma once
#include "stdafx.h"
#include "PlayerManager.h"

CPlayerManager::CPlayerManager()
	: m_pClientPacket(NULL), m_pServerPacket(NULL)
{
	m_fPlayerPosition[0] = -25.f;
	m_fPlayerPosition[1] = -175.f;
	m_fPlayerPosition[2] = 0.f;

	m_pClientPacket = new CLIENTPACKET;
	ZeroMemory(m_pClientPacket, sizeof(CLIENTPACKET));
	m_pServerPacket = new SERVERPACKET;
	ZeroMemory(m_pServerPacket, sizeof(SERVERPACKET));

	m_pServerPacket->m_fPosition[0] = -25.f;
	m_pServerPacket->m_fPosition[1] = -175.f;
	m_pServerPacket->m_fPosition[2] = 0.f;
}

CPlayerManager::~CPlayerManager()
{
	delete m_pClientPacket;
	delete m_pServerPacket;
}

int CPlayerManager::Send(SOCKET &socket)
{
	char *buf = reinterpret_cast<char *>(m_pServerPacket);
	UINT len = sizeof(SERVERPACKET);

	int retval = send(socket, buf, len, 0);
	if (retval == SOCKET_ERROR) {
		CNetworkManager::ErrDisplay("[Send() Error]");
		return retval;
	}

	ZeroMemory(m_pServerPacket, sizeof(SERVERPACKET));
	return retval;
}

int CPlayerManager::Recv(SOCKET &socket)
{
	char buf[BUFSIZE];
	ZeroMemory(buf, sizeof(buf));

	UINT len = sizeof(CLIENTPACKET);
	int retval{};
	retval = Recvn(socket, buf, len, 0);
	if (retval == SOCKET_ERROR) {
		CNetworkManager::ErrDisplay("[recv() Error]");
		return retval;
	}

	buf[retval] = '\0';

	memcpy(m_pClientPacket, buf, len);
	return retval;
}

void CPlayerManager::Update()
{
	switch (m_pClientPacket->m_eKey)
	{
	case m_Up:
		m_fPlayerPosition[1] += DEFAULTSIZE;
		break;
	case m_Down:
		m_fPlayerPosition[1] -= DEFAULTSIZE;
		break;
	case m_Left:
		m_fPlayerPosition[0] -= DEFAULTSIZE;
		break;
	case m_Right:
		m_fPlayerPosition[0] += DEFAULTSIZE;
		break;
	}
	if (m_fPlayerPosition[0] < -(BOARDSIZE * 0.5f))
	{
		m_fPlayerPosition[0] += DEFAULTSIZE;
	}
	if (m_fPlayerPosition[0] > (BOARDSIZE * 0.5f))
	{
		m_fPlayerPosition[0] -= DEFAULTSIZE;
	}
	if (m_fPlayerPosition[1] < -(BOARDSIZE * 0.5f))
	{
		m_fPlayerPosition[1] += DEFAULTSIZE;
	}
	if (m_fPlayerPosition[1] > (BOARDSIZE * 0.5f))
	{
		m_fPlayerPosition[1] -= DEFAULTSIZE;
	}
	for (int i = 0; i < 3; ++i)
		m_pServerPacket->m_fPosition[i] = m_fPlayerPosition[i];

	ZeroMemory(m_pClientPacket, sizeof(CLIENTPACKET));
}