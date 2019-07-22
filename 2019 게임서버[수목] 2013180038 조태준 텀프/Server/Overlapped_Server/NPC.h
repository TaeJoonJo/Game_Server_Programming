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

struct NPCStatus {
	int m_nHP;

	int m_nDamage;
};

class CNPC
{
public:
	CNPC();
	~CNPC();
protected:
	bool			m_bIsRun;
	UINT			m_dwID;
public:
	volatile bool	m_bIsSleep;
	volatile bool	m_bIsCanMove;
	volatile bool	m_bIsCanAttack;

	TCHAR			m_Name[MAX_NAME_LEN];

	int				m_nX;
	int				m_nY;

	int				m_nOriginX;
	int				m_nOriginY;

	int				m_nLevel;

	NPCStatus		m_Status;
	int				m_nType;

	//e_NPCMoveType	m_eMoveType;
	char			m_cRunDir[2];

	mutex			m_vlMutex;
	mutex			m_lMutex;
	mutex			m_AggroMutex;
	unordered_set<CChessClient*>		m_ViewList;
	CChessClient*	m_pLastViewPlayer;
	lua_State*		m_L;
public:
	virtual void	Initalize(UINT dwid, INT ntype);

	virtual void	Move();
	const bool		WakeUp();
	const bool		CanGo(int x, int y);

	void			DetectPlayer(CChessClient* pplayer);

	virtual INT		Attack(CChessClient* pplayer, int* pdamage);
	virtual INT		Attacked(int damage);
	void			Die() {
		m_bIsRun = false;
	}
	virtual void	Respawn();
public:
	inline const INT	GetX() {
		return m_nX;
	}
	inline const INT	GetY() {
		return m_nY;
	}
	inline const UINT	GetID() {
		return m_dwID;
	}
	inline const bool	IsRun() {
		return m_bIsRun;
	}
};

class CMonster : public CNPC
{
public:
	CMonster();
	~CMonster() {};
public:
	//int		m_nLevel;

	bool	m_bIsAttacked;

	bool	m_bIsPeace;
	bool	m_bIsRoam;
public:
	virtual void		Initalize(UINT dwid, INT ntype);

	virtual void		Move();
	virtual void		Respawn();

	virtual INT			Attack(CChessClient* pplayer, int* pdamage);
	virtual INT			Attacked(int damage);

	const INT			GetExp();
};

class CFieldItem : public CNPC
{
public:
	CFieldItem() {};
public:
	int m_nItemType;
	int m_nItemNum;

	TCHAR m_OwnPlayerName[MAX_NAME_LEN];
public:
	virtual void		Initalize(UINT dwid, INT ntype);

	void				Dropped(int x, int y, int type);
	void				Disappear() {
		m_bIsRun = false;
	}
};

class CSkill : public CNPC
{
public:
	CSkill() {};
public:
	int			m_nSkillType;

	int			m_nDir[2];

	UINT		m_nUsedTime;
	UINT		m_nDurationTime;

	CChessClient* m_pUsedPlayer;
public:
	virtual void		Initalize(UINT dwid);

	const bool			Use(int skill_type, int dirx, int diry, CChessClient* pplayer);
	const int			Effect(CChessClient* pplayer, CMonster* pmonster, int* pdamage);
	void				Disappear() {
		m_bIsRun = false;
	}
};

#endif // !__NPC_H__
