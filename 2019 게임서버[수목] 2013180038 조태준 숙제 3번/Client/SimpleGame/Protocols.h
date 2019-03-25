#pragma once

#ifndef __PROTOCOLS_H__
#define __PROTOCOLS_H__

// 네트워크를 위한 정보
#define SERVERPORT 3500
#define MAX_BUFFER 512

enum e_EventType
{
	Event_Recv,
	Event_Send
};

struct SOCKETINFO						// 클라이언트가 여러개니까 각각의 클라이언트의 정보를 저장하는 구조체
{										//
	WSAOVERLAPPED overlapped;			// 커널이 사용하는 자료 - 건들면 안됨 - 각 소켓별로 필요함
	WSABUF wsaBuf;						// WSARecv 할때 WSABUF타입이 필요함 
	e_EventType eventType;
	//SOCKET socket;					// 클라이언트가 어떤소켓을 사용하는지 // 필요없을듯
	char buf[MAX_BUFFER];				// send할때 필요한 버퍼 - 근데 왜 이게 여기있느냐? - 클라이언트에는 송수신 부분에서 버퍼 정의(동기식) - 
										// 여기서는 소켓이 100개 니까 버퍼가 100개생김(비동기식) - 커널별로 저장되는 버퍼 - 
										// 전역으로 하나만 있으면되지 왜 여러개를 만드느냐? - 동기식에서는 하나의 버퍼를 돌려써도 상관없다. 
										// 비동기식에서는 여러개의 소켓에서 송수신이 동시다발적으로 이루어지기 때문에 한버퍼에 쓰면 버퍼가 계속 덮어씌어진다.
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
	//TCHAR name[20];			// 아직 사용안함
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