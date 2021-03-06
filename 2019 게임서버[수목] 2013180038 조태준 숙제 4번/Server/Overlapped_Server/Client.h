
#pragma once

#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "Common.h"
#include "Protocols.h"

// 아이디를 위한 전역변수
static BYTE ID;

class CClientInfo
{
public:
	CClientInfo();
	~CClientInfo();
public:
	SOCKETINFO		m_SocketInfo;
	UINT			m_CurSize;
	UINT			m_PreSize;
	char			m_RecvBuf[MAX_BUFFER];
	SOCKET			m_Socket;
private:
	BYTE			m_Id;
public:
	const bool Initial(SOCKET& psocket);
	const bool ClearIoBuffer();
public:
	inline VOID Logout() {
		closesocket(m_Socket);
	}
	inline const BYTE GetId() {
		return m_Id;
	}
	inline const SOCKET& GetSocket() {
		return m_Socket;
	}
};

class CChessClient
{
public:
	CChessClient();
	~CChessClient();
public:
	CClientInfo		*m_ClientInfo;
	bool			m_bIsRun;

	list<CChessClient*>		m_NearList;
	list<CChessClient*>		m_ViewList;
	list<CChessClient*>		m_RemoveList;
private:
	TCHAR			m_Name[20];
	float				m_X;
	float				m_Y;
public:
	const bool Initial(SOCKET& psocket);
	
	// 패킷 처리 프로세스
	const e_sc_PacketType ProcessPacket();
	VOID UpdateViewList();
	VOID Logout();
public:
	inline float GetX() {
		return m_X;
	}
	inline float GetY() {
		return m_Y;
	}
	inline const BYTE GetID() {
		return m_ClientInfo->GetId();
	}
	inline const SOCKET& GetSocket(){
		return m_ClientInfo->GetSocket();
	}
};

//typedef vector<CChessClient> vecClients;
typedef vector<CChessClient *> vecClients;
#endif // !__CLIENT_H__
