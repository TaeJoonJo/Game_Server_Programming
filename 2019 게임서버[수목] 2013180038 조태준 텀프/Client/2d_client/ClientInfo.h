#pragma once

#ifndef __CLIENTINFO_H__
#define __CLIENTINFO_H__

#include "Common.h"

class CClientInfo
{
public:
	CClientInfo();
	~CClientInfo() {};
public:
	wchar_t m_cName[MAX_NAME_LEN];

	int m_nX;
	int m_nY;

	int m_nLevel;
	int m_nExp;

	int m_nHP;
	int m_nMP;

	int m_nMaxHP;
	int m_nMaxMP;
	int m_nStr;
	int m_nDex;
	int m_nWill;
	int m_nInt;

	int m_nStatPoints;

	int m_nHPPotionNum;
	int m_nMPPotionNum;

	int m_nMoney;
};

#endif // !__CLIENTINFO_H__
