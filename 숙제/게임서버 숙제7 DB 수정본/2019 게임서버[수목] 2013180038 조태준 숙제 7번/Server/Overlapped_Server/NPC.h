#pragma once

#ifndef __NPC_H__
#define __NPC_H__

#include "Protocols.h"
#include "Client.h"

class CChessClient;

class CNPC
{
public:
	CNPC();
	~CNPC();
private:
	bool			m_bIsRun;
	DWORD			m_dwID;
public:
	volatile bool	m_bIsSleep;
	//TCHAR			m_Name[20];
	float			m_fX;
	float			m_fY;

	mutex			m_vlMutex;
	unordered_set<CChessClient*>		m_ViewList;
public:
	void			Initalize(DWORD dwid);

	void			Move();
	bool			WakeUp();
public:
	inline float	GetX() {
		return m_fX;
	}
	inline float	GetY() {
		return m_fY;
	}
	inline DWORD	GetID() {
		return m_dwID;
	}
};

#endif // !__NPC_H__
