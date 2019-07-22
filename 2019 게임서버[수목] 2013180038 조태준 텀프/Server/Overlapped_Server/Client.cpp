
#include "Client.h"

CClientInfo::CClientInfo()
	: m_Socket(INVALID_SOCKET)
	, m_Id(0xffff)
	, m_CurSize(0), m_PreSize(0)
{
	memset(m_RecvBuf, 0x00, MAX_BUFFER);
	ZeroMemory(&m_SocketInfo, sizeof(SOCKETINFO));
}

CClientInfo::~CClientInfo()
{
	closesocket(m_Socket);
}

const bool CClientInfo::Initial(SOCKET& psocket)
{
	if (psocket == INVALID_SOCKET)
		return false;

	// 소켓 연결, 버퍼 연결
	m_Socket = psocket;
	m_SocketInfo.wsaBuf.len = MAX_BUFFER;
	m_SocketInfo.wsaBuf.buf = m_SocketInfo.buf;
	m_SocketInfo.eventType = e_EventType::Event_Recv;
	// 아이디 설정
	//m_Id = ID++;

	return true;
}

const bool CClientInfo::ClearIoBuffer()
{
	ZeroMemory(&m_SocketInfo, sizeof(SOCKETINFO));

	closesocket(m_Socket);

	return true;
}


///////////////////////////////////////////////////////////////////

CChessClient::CChessClient()
	: m_nX(-1), m_nY(-1)
	, m_nHP(-1), m_nMP(-1)
	, m_nLevel(-1), m_nExp(-1)
	, m_bIsTest(false)
{
	m_ClientInfo = new CClientInfo;
	memset(m_Name, 0x00, sizeof(m_Name));
	
	m_NearList.clear();
	m_RemoveList.clear();
	m_ViewList.clear();

	m_bIsCanAttack = true;
}

CChessClient::~CChessClient()
{
	delete m_ClientInfo;

	m_NearList.clear();
	m_RemoveList.clear();
	m_ViewList.clear();
}

const bool CChessClient::Initial(SOCKET& psocket)
{
	if (this->m_ClientInfo->Initial(psocket) == false)
		return false;

	m_nX = 6;
	m_nY = 6;

	m_bIsDie = false;
	//m_nX = (float)RandomINT(1, WORLD_WIDTH - 1);
	//m_nY = (float)RandomINT(1, WORLD_HEIGHT - 1);

	m_bIsRun = true;

	m_bIsCanMove = true;

	return true;
}

VOID CChessClient::Logout()
{
	m_ClientInfo->Logout();
}

const INT CChessClient::Attack(CNPC* pmonster, int* pdamage)
{
	int damage = BASIC_PLAYER_DAMAGE + (m_Status.m_nStr * STAT_WEIGHT);
	*pdamage = damage;

	return pmonster->Attacked(damage);
}

const INT CChessClient::Attacked(int* damage)
{
	ClientStatus stat = m_Status;

	int counteddamage = 0;
	int dodgepercent = stat.m_nDex * 2;
	int temp = RandomINT(0, 100);

	if(temp > dodgepercent)
		counteddamage = *damage - (stat.m_nWill);
	m_nHP -= counteddamage;

	*damage = counteddamage;

	return m_nHP;
}

const INT CChessClient::Heal(int amount, bool bhp)
{
	int heal_amount{0};

	if (bhp) {
		int my_max_hp = m_Status.m_nMaxHP;
		int my_hp = m_nHP;

		heal_amount = ((float)my_max_hp / 100.f) * amount;
		my_hp += heal_amount;
		if (my_hp > my_max_hp) {
			heal_amount = heal_amount - (my_hp - my_max_hp);
			my_hp = my_max_hp;
		}
		m_nHP = my_hp;
	}
	else {
		int my_max_mp = m_Status.m_nMaxMP;
		int my_mp = m_nMP;

		heal_amount = ((float)my_max_mp / 100.f) * amount;
		my_mp += heal_amount;
		if (my_mp > my_max_mp) {
			heal_amount = heal_amount - (my_mp - my_max_mp);
			my_mp = my_max_mp;
		}
		m_nMP = my_mp;
	}

	return heal_amount;
}

const bool CChessClient::CanHeal(bool bhp)
{
	if (m_bIsDie == true)
		return false;

	if (bhp) {
		if (m_nHP == m_Status.m_nMaxHP)
			return false;
	}
	else {
		if (m_nMP == m_Status.m_nMaxMP)
			return false;
	}

	return true;
}

const bool CChessClient::GetExp(int nexp)
{
	int my_exp = m_nExp;
	my_exp += nexp;

	int my_need_exp = BASIC_NEED_EXP * m_nLevel;

	if (my_exp >= my_need_exp) {
		LevelUp();
		return true;
	}

	m_nExp = my_exp;

	return false;
}

void CChessClient::LevelUp()
{
	m_nLevel += 1;

	m_nHP = m_Status.m_nMaxHP;
	m_nMP = m_Status.m_nMaxMP;

	m_Status.m_nStatPoints += 2;

	m_nExp = 0;
}

VOID CChessClient::Die(int* pexp, int* pmoney)
{
	m_bIsDie = true;

	int lost_exp = m_nExp / 2;
	int lost_money = m_Item.m_nMoney / 10;

	*pexp = lost_exp;
	*pmoney = lost_money;

	m_nExp -= lost_exp;
	m_Item.m_nMoney -= lost_money;
}

VOID CChessClient::Respawn()
{
	m_bIsDie = false;

	m_nHP = m_Status.m_nMaxHP;
	m_nMP = m_Status.m_nMaxMP;

	m_nX = BASIC_PLAYER_X;
	m_nY = BASIC_PLAYER_Y;
}

// 안씀
const e_sc_PacketType CChessClient::ProcessPacket()
{
	cs_packet_base *buf = reinterpret_cast<cs_packet_base *>(m_ClientInfo->m_RecvBuf);
	char *pbuf = reinterpret_cast<char *>(m_ClientInfo->m_RecvBuf);;
	
	BYTE type = buf->type;
	e_sc_PacketType escType{};

	switch (type)
	{
		//case e_cs_PacketType::
	case e_cs_PacketType::Up:
	{
		if (m_nY > 0)					m_nY--;
		escType = e_sc_PacketType::MovePlayer;
	}break;
	case e_cs_PacketType::Down:
	{
		if (m_nY < WORLD_HEIGHT - 1)		m_nY++;
		escType = e_sc_PacketType::MovePlayer;
	} break;
	case e_cs_PacketType::Left:
	{
		if (m_nX > 0)					m_nX--;
		escType = e_sc_PacketType::MovePlayer;
	} break;
	case e_cs_PacketType::Right:
	{
		if (m_nX < (WORLD_HEIGHT - 1))	m_nX++;
		escType = e_sc_PacketType::MovePlayer;
	} break;
	case e_cs_PacketType::IDLE:
	{
		escType = e_sc_PacketType::MovePlayer;
	} break;
	}

	/*if (m_X < -(BOARDSIZE * 0.5f))
	{
		m_X += DEFAULTSIZE;
	}
	if (m_X > (BOARDSIZE * 0.5f))
	{
		m_X -= DEFAULTSIZE;
	}
	if (m_Y < -(BOARDSIZE * 0.5f))
	{
		m_Y += DEFAULTSIZE;
	}
	if (m_Y > (BOARDSIZE * 0.5f))
	{
		m_Y -= DEFAULTSIZE;
	}*/

	return escType;
}

VOID CChessClient::UpdateViewList()
{
	// 시야는 7x7


	return VOID();
}
