#pragma once

// ��Ʈ��ũ�� ���� ����
#define SERVERPORT 9000
#define BUFSIZE	   512

typedef enum ePlayerKeyType {
	m_Up,
	m_Down,
	m_Left,
	m_Right
} PLAYERKEYTYPE;

typedef struct ClientPacket {
	ePlayerKeyType m_eKey;
} CLIENTPACKET;

typedef struct ServerPacket {
	float m_fPosition[3];
} SERVERPACKET;