
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
	m_Id = ID++;

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
	: m_X(0), m_Y(0)
{
	m_ClientInfo = new CClientInfo;
	memset(m_Name, 0x00, sizeof(m_Name));
	
	m_NearList.clear();
	m_RemoveList.clear();
	m_ViewList.clear();
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

	//int id = m_ClientInfo->GetId();
	//float x = STARTX + (id * DEFAULTSIZE);
	//float y = -175.f;
	//if (id > 8)
	//{
	//	x = STARTX + ((id - 9) * DEFAULTSIZE);
	//	y = 175.f;
	//}

	m_X = 10.f;
	m_Y = 10.f;

	m_bIsRun = true;
	return true;
}

VOID CChessClient::Logout()
{
	m_ClientInfo->Logout();
}

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
		if (m_Y > 0)					m_Y--;
		escType = e_sc_PacketType::MovePlayer;
	}break;
	case e_cs_PacketType::Down:
	{
		if (m_Y < WORLD_HEIGHT - 1)		m_Y++;
		escType = e_sc_PacketType::MovePlayer;
	} break;
	case e_cs_PacketType::Left:
	{
		if (m_X > 0)					m_X--;
		escType = e_sc_PacketType::MovePlayer;
	} break;
	case e_cs_PacketType::Right:
	{
		if (m_X < (WORLD_HEIGHT - 1))	m_X++;
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
