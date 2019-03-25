#pragma once

#ifndef __PROTOCOLS_H__
#define __PROTOCOLS_H__

// ��Ʈ��ũ�� ���� ����
#define SERVERPORT 3500
#define MAX_BUFFER 512

enum e_EventType
{
	Event_Recv,
	Event_Send
};

struct SOCKETINFO						// Ŭ���̾�Ʈ�� �������ϱ� ������ Ŭ���̾�Ʈ�� ������ �����ϴ� ����ü
{										//
	WSAOVERLAPPED overlapped;			// Ŀ���� ����ϴ� �ڷ� - �ǵ�� �ȵ� - �� ���Ϻ��� �ʿ���
	WSABUF wsaBuf;						// WSARecv �Ҷ� WSABUFŸ���� �ʿ��� 
	e_EventType eventType;
	//SOCKET socket;					// Ŭ���̾�Ʈ�� ������� ����ϴ��� // �ʿ������
	char buf[MAX_BUFFER];				// send�Ҷ� �ʿ��� ���� - �ٵ� �� �̰� �����ִ���? - Ŭ���̾�Ʈ���� �ۼ��� �κп��� ���� ����(�����) - 
										// ���⼭�� ������ 100�� �ϱ� ���۰� 100������(�񵿱��) - Ŀ�κ��� ����Ǵ� ���� - 
										// �������� �ϳ��� ��������� �� �������� �������? - ����Ŀ����� �ϳ��� ���۸� �����ᵵ �������. 
										// �񵿱�Ŀ����� �������� ���Ͽ��� �ۼ����� ���ôٹ������� �̷������ ������ �ѹ��ۿ� ���� ���۰� ��� ���������.
	INT id;
};

typedef enum e_sc_PacketType
{
	Login,
	Logout,
	Position,
	PutPlayer,
	RemovePlayer
}sc_PACKETTYPE;

typedef enum e_cs_PacketType
{
	TryLogin,
	Left,
	Right,
	Up,
	Down,
	IDLE
}cs_PACKETTYPE;

#pragma pack(push, 1)
struct cs_packet_base {
	BYTE size;
	BYTE type;
};
struct cs_packet_try_login {
	BYTE size;
	BYTE type;
};
struct cs_packet_up {
	BYTE size;
	BYTE type;
};
struct cs_packet_down {
	BYTE size;
	BYTE type;
};
struct cs_packet_left {
	BYTE size;
	BYTE type;
};
struct cs_packet_right {
	BYTE size;
	BYTE type;
};
struct cs_packet_idle {
	BYTE size;
	BYTE type;
};

// Server to Client
struct sc_packet_base {
	BYTE size;
	BYTE type;
};
struct sc_packet_login {
	BYTE size;
	BYTE type;
	BYTE numclient;
	INT id;
	float x;
	float y;
};
struct sc_packet_logout {
	BYTE size;
	BYTE type;
	INT id;
};
struct sc_packet_pos {
	BYTE size;
	BYTE type;
	INT id;
	float x;
	float y;
};
struct sc_packet_put_player {
	BYTE size;
	BYTE type;
	INT id;
	//TCHAR name[20];			// ���� ������
	float x;
	float y;
};
struct sc_packet_remove_player {
	BYTE size;
	BYTE type;
	INT id;
};
#pragma pack(pop)

#endif // !__PROTOCOLS_H__