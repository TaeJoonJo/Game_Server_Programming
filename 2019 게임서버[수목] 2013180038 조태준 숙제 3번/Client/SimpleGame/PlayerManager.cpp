#pragma once
#include "stdafx.h"
#include "PlayerManager.h"

CPlayerManager::CPlayerManager()
	: m_Id(NOT_REGISTED)
{
	memset(m_IoBuf, 0x00, MAX_BUFFER);
}

CPlayerManager::~CPlayerManager()
{
	
}

void CPlayerManager::StartIO(SOCKET &socket, int key)
{
	Send(socket, key);

}

int CPlayerManager::Send(SOCKET &socket, int key)
{
	INT len = 0;

	char buf[MAX_BUFFER]{};

	switch (key)
	{
	case KEY_FIRST:
	{
		cs_packet_try_login cspacket;
		cspacket.size = sizeof(cs_packet_try_login);
		cspacket.type = e_cs_PacketType::TryLogin;
		len = sizeof(cs_packet_try_login);
		memcpy(buf, reinterpret_cast<char *>(&cspacket), len);
	} break;
	case KEY_UP:
	{
		cs_packet_up cspacket;
		cspacket.size = sizeof(cs_packet_up);
		cspacket.type = e_cs_PacketType::Up;
		len = sizeof(cs_packet_up);
		memcpy(buf, reinterpret_cast<char *>(&cspacket), len);
	} break;
	case KEY_DOWN:
	{
		cs_packet_down cspacket;
		cspacket.size = sizeof(cs_packet_down);
		cspacket.type = e_cs_PacketType::Down;
		len = sizeof(cs_packet_down);
		memcpy(buf, reinterpret_cast<char *>(&cspacket), len);
	} break;
	case KEY_LEFT:
	{
		cs_packet_left cspacket;
		cspacket.size = sizeof(cs_packet_left);
		cspacket.type = e_cs_PacketType::Left;
		len = sizeof(cs_packet_left);
		memcpy(buf, reinterpret_cast<char *>(&cspacket), len);
	} break;
	case KEY_RIGHT:
	{
		cs_packet_right cspacket;
		cspacket.size = sizeof(cs_packet_right);
		cspacket.type = e_cs_PacketType::Right;
		len = sizeof(cs_packet_right);
		memcpy(buf, reinterpret_cast<char *>(&cspacket), len);
	} break;
	case KEY_IDLE:
	{
		cs_packet_idle cspacket;
		cspacket.size = sizeof(cs_packet_idle);
		cspacket.type = e_cs_PacketType::IDLE;
		len = sizeof(cs_packet_idle);
		memcpy(buf, reinterpret_cast<char *>(&cspacket), len);
	} break;
	}
	
	int retval = send(socket, buf, len, 0);
	if (WSAGetLastError() == WSAEWOULDBLOCK)
		return 0;
	if (retval == SOCKET_ERROR) {
		CNetworkManager::ErrDisplay("[Send() Error]");
		return retval;
	}

	return retval;
}

int CPlayerManager::Recv(SOCKET &socket)
{
	char *pbuf = m_IoBuf;
	
	// 패킷의 크기와 종류를 먼저 받아온다.
	INT len = sizeof(sc_packet_base);
	int retval = 0;
	retval = Recvn(socket, pbuf, len, 0);
	if (retval == SOCKET_ERROR) {
		if (WSAGetLastError() != WSAEWOULDBLOCK) 
			CNetworkManager::ErrDisplay("[recv() Error]");
		return retval;
	}
	// 나머지 데이터를 받아온다.
	INT packetlen = pbuf[0] - len;
	retval = Recvn(socket, pbuf + len, packetlen, 0);
	if (retval == SOCKET_ERROR) {
		if (WSAGetLastError() != WSAEWOULDBLOCK)
			CNetworkManager::ErrDisplay("[recv() Error]");
		return retval;
	}

	// 패킷 처리
	this->Update(socket);

	return retval;
}

void CPlayerManager::Update(SOCKET &socket)
{
	sc_packet_base *buf = reinterpret_cast<sc_packet_base *>(m_IoBuf);

	// 받은 데이터를 이용해 배치
	switch (buf->type)
	{
	case e_sc_PacketType::Login:
	{
		sc_packet_login *ppacket = reinterpret_cast<sc_packet_login *>(m_IoBuf);
		if (m_Id == NOT_REGISTED)
		{
			m_isRun = TRUE;
			m_Id = ppacket->id;
			CSceneMgr::Add_Object(CHESS, ppacket->id, TRUE);
			CGameObject *pObject = CSceneMgr::Get_Object_By_ID(ppacket->id);
			m_pGameObject = pObject;
			m_pGameObject->SetPosition(ppacket->x, ppacket->y, 0.f);
			int num = ppacket->numclient;
			for (int i = 0; i < num; ++i)		// 전에 있던 클라이언트 수만큼 데이터를 더 받는다.
			{
				Recv(socket);
				Update(socket);
			}
		}
		else
		{
			CSceneMgr::Add_Object(CHESS, ppacket->id, TRUE);
		}
	} break;
	case e_sc_PacketType::Logout:		// 같은 아이디면 종료 다른 아이디면 그아이디 찾아서 비활성화 해주자
	{
		sc_packet_logout *ppacket = reinterpret_cast<sc_packet_logout *>(m_IoBuf);
		if (m_Id == ppacket->id)
		{
			cout << "Logout!\n";
			m_isRun = FALSE;
			exit(1);
		}
		m_pGameObject->SetActive(FALSE);
	} break;
	case e_sc_PacketType::Position:
	{
		sc_packet_pos *ppacket = reinterpret_cast<sc_packet_pos *>(m_IoBuf);

		CGameObject *pObject = CSceneMgr::Get_Object_By_ID(ppacket->id);
		if (pObject == NULL)
		{
			cout << "Can't Find Object By ID\n";
			//exit(1);
			return ;
		}
		pObject->SetPosition(ppacket->x, ppacket->y, 0.f);
	} break;
	case e_sc_PacketType::PutPlayer:
	{
		sc_packet_put_player *ppacket = reinterpret_cast<sc_packet_put_player *>(m_IoBuf);
		
		if (m_Id == ppacket->id && m_pGameObject != NULL)
		{
			m_pGameObject->SetPosition(ppacket->x, ppacket->y, 0.f);
		}
		else
		{
			CGameObject *pObject = CSceneMgr::Get_Object_By_ID(ppacket->id);
			if (pObject == NULL)
			{
				CSceneMgr::Add_Object(CHESS, ppacket->id, TRUE);
				pObject = CSceneMgr::Get_Object_By_ID(ppacket->id);
			}
			pObject->SetPosition(ppacket->x, ppacket->y, 0.f);
		}
	} break;
	case e_sc_PacketType::RemovePlayer:		// 찾아서 비활성화
	{
		sc_packet_remove_player *ppacket = reinterpret_cast<sc_packet_remove_player *>(m_IoBuf);
		CGameObject *pObject = CSceneMgr::Get_Object_By_ID(ppacket->id);
		if (pObject == NULL)
		{
			cout << "Can't Find Object By ID\n";
			//exit(1);
			return ;
		}
		pObject->SetActive(FALSE);
	} break;
	}

}

void CPlayerManager::KeyInput(void *key)
{
	int keytype = (int)key;

	/*switch(keytype)
	{
	case UP:
	{

	} break;
	case DOWN:
	{

	} break;
	case LEFT:
	{

	} break;
	case RIGHT:
	{

	} break;
	}*/
}

void CPlayerManager::Disconnect()
{
	m_Id = NOT_REGISTED;
}