#pragma once

#ifndef __NPC_H__
#define __NPC_H__

#include "Common.h"
#include "Client.h"

enum e_NPCMoveType {
	e_RandomMove,
	e_RunMove
};

class CChessClient;

class CNPC
{
public:
	CNPC();
	~CNPC();
private:
	bool			m_bIsRun;
	UINT			m_dwID;
public:
	volatile bool	m_bIsSleep;
	//TCHAR			m_Name[20];
	float			m_fX;
	float			m_fY;

	e_NPCMoveType	m_eMoveType;
	char			m_cRunDir[2];

	mutex			m_vlMutex;
	mutex			m_lMutex;
	unordered_set<CChessClient*>		m_ViewList;
	lua_State*		m_L;
public:
	void			Initalize(UINT dwid);

	void			Move();
	bool			WakeUp();

	void			DetectPlayer(CChessClient* pplayer);
public:
	inline float	GetX() {
		return m_fX;
	}
	inline float	GetY() {
		return m_fY;
	}
	inline UINT		GetID() {
		return m_dwID;
	}
};

#endif // !__NPC_H__
