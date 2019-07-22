
#pragma once

#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "Common.h"
#include "Protocols.h"
#include "NPC.h"

class CNPC;

// 아이디를 위한 전역변수
static UINT ID;

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
//private:
	INT			m_Id;
public:
	const bool Initial(SOCKET& psocket);
	const bool ClearIoBuffer();
public:
	inline VOID Logout() {
		closesocket(m_Socket);
	}
	inline const INT GetId() {
		return m_Id;
	}
	inline const SOCKET& GetSocket() {
		return m_Socket;
	}
};

struct ClientStatus {
	int				m_nMaxHP = -1;//-1;
	int				m_nMaxMP = -1;//-1;

	int				m_nStr = -1;//-1;
	int				m_nDex = -1;
	int				m_nWill = -1;
	int				m_nInt = -1;

	int				m_nStatPoints = -1;
	void Initalize() {
		m_nMaxHP = 100;
		m_nMaxMP = 100;

		m_nStr = 1;
		m_nDex = 1;
		m_nWill = 1;
		m_nInt = 1;
		
		m_nStatPoints = 0;
	}
};

struct ClientItem {
	int		m_nHPPotionNum = -1;
	int		m_nMPPotionNum = -1;

	int		m_nMoney = -1;
};

class CChessClient
{
public:
	CChessClient();
	~CChessClient();
public:
	CClientInfo		*m_ClientInfo;
	volatile bool	m_bIsRun;
	volatile bool	m_bIsTest;
	lua_State*		m_L;

	mutex			m_vlMutex;
	mutex			m_npcvlMutex;
	unordered_set<CChessClient*>		m_NearList;
	unordered_set<CChessClient*>		m_ViewList;
	unordered_set<CChessClient*>		m_RemoveList;

	unordered_set<CNPC*>		m_NPCViewList;
public:
	bool			m_bIsDie;

	TCHAR			m_Name[10];

	int				m_nX;
	int				m_nY;

	int				m_nLevel;
	int				m_nExp;

	int				m_nHP;
	int				m_nMP;

	volatile bool	m_bIsCanAttack;
	volatile bool	m_bIsCanMove;

	ClientStatus	m_Status;
	ClientItem		m_Item;
public:
	const bool					Initial(SOCKET& psocket);
	
	// 패킷 처리 프로세스
	const e_sc_PacketType		ProcessPacket();

	VOID		UpdateViewList();
	VOID		Logout();

	// 대상의 남은 hp return
	const INT	Attack(CNPC* pmonster, int* pdamage);
	// 대상의 남은 hp return
	const INT	Attacked(int* damage);
	const INT	Heal(int amount, bool bhp);
	const bool	CanHeal(bool bhp);

	// 레벨업시 true 아니면 false
	const bool	GetExp(int nexp);
	void		LevelUp();

	VOID		Die(int* pexp, int* pmoney);
	VOID		Respawn();
public:
	inline const bool IsRun() {
		return m_bIsRun;
	}
	inline const INT GetX() {
		return m_nX;
	}
	inline const INT GetY() {
		return m_nY;
	}
	inline const INT GetID() {
		return m_ClientInfo->GetId();
	}
	inline const SOCKET& GetSocket(){
		return m_ClientInfo->GetSocket();
	}
public:
	inline VOID SetX(INT x) {
		m_nX = x;
	}
	inline VOID SetY(INT y) {
		m_nY = y;
	}
	inline VOID SetID(INT id) {
		m_ClientInfo->m_Id = id;
	}
};

typedef vector<CChessClient *> vecClients;
#endif // !__CLIENT_H__
