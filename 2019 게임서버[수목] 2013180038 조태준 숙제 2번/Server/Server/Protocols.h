#pragma once

// 네트워크를 위한 정보
#define SERVERPORT 9000
#define BUFSIZE	   1024

typedef enum ePlayerKeyType {
	m_Up,
	m_Down,
	m_Left,
	m_Right
} PLAYERKEYTYPE;

// 패킷 구조체는 나중에 더 세분화 필요!
typedef struct ClientPacket {
	ePlayerKeyType m_eKey;
} CLIENTPACKET;

typedef struct ServerPacket {
	float m_fPosition[3];
} SERVERPACKET;