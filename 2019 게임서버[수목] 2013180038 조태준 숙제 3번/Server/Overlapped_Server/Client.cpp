
#include "Client.h"

CClientInfo::CClientInfo()
	: m_Socket(INVALID_SOCKET)
	, m_Id(0), m_isRun(FALSE)
	, m_CurSize(0), m_PreSize(0)
{
	memset(m_RecvBuf, 0x00, MAX_BUFFER);
}

CClientInfo::~CClientInfo()
{
}

const BOOL CClientInfo::Initial(SOCKET& psocket)
{
	if (psocket == INVALID_SOCKET)
		return FALSE;

	m_isRun = TRUE;
	// 소켓 연결, 버퍼 연결
	m_Socket = psocket;
	m_SocketInfo.wsaBuf.len = MAX_BUFFER;
	m_SocketInfo.wsaBuf.buf = m_SocketInfo.buf;
	m_SocketInfo.eventType = e_EventType::Event_Recv;
	// 아이디 설정
	m_Id = ID++;

	return TRUE;
}


///////////////////////////////////////////////////////////////////

CChessClient::CChessClient()
	: m_X(0), m_Y(0)
{
	memset(m_Name, 0x00, sizeof(m_Name));
	memset(&m_ClientInfo, 0x00, sizeof(m_ClientInfo));
}

CChessClient::~CChessClient()
{

}

const BOOL CChessClient::Initial(SOCKET& psocket)
{
	if (this->m_ClientInfo.Initial(psocket) == FALSE)
		return FALSE;

	int id = m_ClientInfo.GetId();
	float x = STARTX + (id * DEFAULTSIZE);
	float y = -175.f;
	if (id > 8)
	{
		x = STARTX + ((id - 9) * DEFAULTSIZE);
		y = 175.f;
	}

	m_X = x;
	m_Y = y;

	return TRUE;
}

VOID CChessClient::Logout()
{
	m_ClientInfo.Logout();
}

const e_sc_PacketType CChessClient::ProcessPacket()
{
	cs_packet_base *buf = reinterpret_cast<cs_packet_base *>(m_ClientInfo.m_RecvBuf);
	char *pbuf = reinterpret_cast<char *>(m_ClientInfo.m_RecvBuf);;
	
	BYTE type = buf->type;
	e_sc_PacketType escType;

	switch (type)
	{
	case e_cs_PacketType::TryLogin:
	{
		escType = e_sc_PacketType::Login;
	} break;
	case e_cs_PacketType::Up:
	{
		m_Y += DEFAULTSIZE;
		escType = e_sc_PacketType::Position;
	}break;
	case e_cs_PacketType::Down:
	{
		m_Y -= DEFAULTSIZE;
		escType = e_sc_PacketType::Position;
	} break;
	case e_cs_PacketType::Left:
	{
		m_X -= DEFAULTSIZE;
		escType = e_sc_PacketType::Position;
	} break;
	case e_cs_PacketType::Right:
	{
		m_X += DEFAULTSIZE;
		escType = e_sc_PacketType::Position;
	} break;
	case e_cs_PacketType::IDLE:
	{
		escType = e_sc_PacketType::Position;
	}
	}

	if (m_X < -(BOARDSIZE * 0.5f))
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
	}

	return escType;
}