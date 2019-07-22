#include "ClientInfo.h"

CClientInfo::CClientInfo()
	: m_nLevel(-1), m_nExp(-1)
	, m_nX(-1), m_nY(-1)
	, m_nHP(-1), m_nMP(-1)
	, m_nMaxHP(-1), m_nMaxMP(-1), m_nStr(-1), m_nDex(-1), m_nWill(-1), m_nInt(-1)
	, m_nStatPoints(-1)
	, m_nHPPotionNum(-1), m_nMPPotionNum(-1), m_nMoney(-1)
{
	ZeroMemory(m_cName, MAX_NAME_LEN);
}
