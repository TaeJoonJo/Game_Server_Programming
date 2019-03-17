#pragma once

// ��Ʈ��ũ�� ���� ����
#define SERVERPORT 9000
#define BUFSIZE	   1024

typedef enum ePlayerKeyType {
	m_Up,
	m_Down,
	m_Left,
	m_Right
} PLAYERKEYTYPE;

// ��Ŷ ����ü�� ���߿� �� ����ȭ �ʿ�!
typedef struct ClientPacket {
	ePlayerKeyType m_eKey;
} CLIENTPACKET;

typedef struct ServerPacket {
	float m_fPosition[3];
} SERVERPACKET;