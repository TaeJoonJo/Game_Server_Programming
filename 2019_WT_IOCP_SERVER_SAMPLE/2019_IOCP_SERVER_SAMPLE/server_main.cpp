#include <iostream>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
using namespace std;
#include <winsock2.h>
#include "protocol.h"

#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER        1024

struct OVER_EX {
	WSAOVERLAPPED	over;
	WSABUF dataBuffer;
	char messageBuffer[MAX_BUFFER];
	bool is_recv;
};

class SOCKETINFO
{
public:
	mutex access_lock;
	bool in_use;
	OVER_EX over_ex;
	SOCKET socket;
	char packet_buffer[MAX_BUFFER];
	int	prev_size;
	char x, y;

	SOCKETINFO() {
		in_use = false;
		socket = INVALID_SOCKET;
		over_ex.dataBuffer.len = MAX_BUFFER;
		over_ex.dataBuffer.buf = over_ex.messageBuffer;
		over_ex.is_recv = true;
	}
};

HANDLE g_iocp;
SOCKETINFO clients[MAX_USER];

void error_display(const char *msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	cout << msg;
	wcout << L"에러 [" << err_no << L"]" << lpMsgBuf << "\n";

	while(true);
	LocalFree(lpMsgBuf);
}

void send_packet(char client, void *packet)
{
	char *p = reinterpret_cast<char *>(packet);
	OVER_EX *ov = new OVER_EX;
	ov->dataBuffer.len = p[0];
	ov->dataBuffer.buf = ov->messageBuffer;
	ov->is_recv = false;
	memcpy(ov->messageBuffer, p, p[0]);
	ZeroMemory(&ov->over, sizeof(WSAOVERLAPPED));

	int error = WSASend(clients[client].socket, &ov->dataBuffer, 1, 0, 0, &ov->over, NULL);
	if (0 != error) {
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			error_display("Error - Fail WSASend(error_code : %d)\n", WSAGetLastError());
			//printf("Error - Fail WSASend(error_code : %d)\n", WSAGetLastError());
			while (true);
		}
	}
	else {
		/*cout << "Non Overlapped Send Return\n";
		while (true);*/
	}
}



void send_pos_packet(char client, char pl)
{
	sc_packet_move_player packet;
	packet.id = pl;
	packet.size = sizeof(packet);
	packet.type = SC_MOVE_PLAYER;
	packet.x = clients[pl].x;
	packet.y = clients[pl].y;

	send_packet(client, &packet);
}

void send_login_ok_packet(char new_id)
{
	sc_packet_login_ok packet;
	packet.id = new_id;
	packet.size = sizeof(packet);
	packet.type = SC_LOGIN_OK;

	send_packet(new_id, &packet);
}

void send_put_player_packet(char client, char new_id)
{
	sc_packet_put_player packet;				// 종족, 성별같은거도 여기서 정해줌
	packet.id = new_id;
	packet.size = sizeof(packet);
	packet.type = SC_PUT_PLAYER;
	packet.x = clients[new_id].x;
	packet.y = clients[new_id].y;

	send_packet(client, &packet);
}

void send_remove_player_packet(char client, char pl)
{
	sc_packet_remove_player packet;	
	packet.id = pl;
	packet.size = sizeof(packet);
	packet.type = SC_REMOVE_PLAYER;

	send_packet(client, &packet);
}

void dissconect_client(char id)
{
	for (int i = 0; i < MAX_USER; ++i) {
		if (false == clients[i].in_use) continue;
		if (i == id) continue;
		send_remove_player_packet(i, id);
	}

	closesocket(clients[id].socket);
	clients[id].in_use = false;
}

void process_packet(char client, char *packet)
{
	cs_packet_up *p = reinterpret_cast<cs_packet_up *>(packet);

	int x = clients[client].x;
	int y = clients[client].y;

	switch (p->type)
	{
	case CS_UP:			if (y > 0)					y--;	break;
	case CS_DOWN:		if (y < WORLD_HEIGHT - 1)	y++;	break;
	case CS_LEFT:		if (x > 0)					x--;	break;
	case CS_RIGHT:		if (x < (WORLD_HEIGHT - 1))	x++;	break;
	default:
		wcout << L"정의되지 않은 패킷 도착 오류!\n";
		while (true);
	}

	clients[client].x = x;
	clients[client].y = y;

	for(int i = 0; i < MAX_USER; ++i)
		if(true == clients[i].in_use)
			send_pos_packet(i, client);
}

void do_recv(char id)
{
	DWORD flags = 0;

	if (WSARecv(clients[id].socket, &clients[id].over_ex.dataBuffer, 1,
		NULL, &flags, &(clients[id].over_ex.over), 0))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			error_display("Error - IO pending Failure\n", WSAGetLastError());
			//cout << "Error - IO pending Failure\n";
			while (true);
		}
	}
	else {
		error_display("Non Overlapped Recv return.\n", WSAGetLastError());
		//cout << "Non Overlapped Recv return.\n";
		while (true);
	}
}

void worker_thread()
{
	while (true) {
		DWORD io_byte;
		ULONG key;
		OVER_EX *over_ex;
		bool is_error = GetQueuedCompletionStatus(g_iocp, &io_byte,
			reinterpret_cast<PULONG_PTR>(&key), reinterpret_cast<LPWSAOVERLAPPED *>(&over_ex), 
			INFINITE);

		if (0 == is_error) {
			int err_no = WSAGetLastError();
			if (64 == err_no) {
				dissconect_client(key);
				continue;
			}
			else error_display("GQCS :", err_no);
		}

		if (0 == io_byte) {
			dissconect_client(key);
			continue;
		}

		if (true == over_ex->is_recv) {
			wcout << L"Packet From : " << key << "\n";
			// 패킷조립
			int rest = io_byte;
			char *ptr = over_ex->messageBuffer;
			char packet_size = 0;
			if (0 < clients[key].prev_size)
				packet_size = clients[key].packet_buffer[0];
			while (0 < rest) {
				if (0 == packet_size) packet_size = ptr[0];
				int required = packet_size - clients[key].prev_size;
				if (required <= rest) {
					memcpy(clients[key].packet_buffer + clients[key].prev_size,
						ptr, required);
					process_packet(key, clients[key].packet_buffer);
					rest -= required;
					ptr += required;
					packet_size = 0;
					clients[key].prev_size = 0;
				}
				else {
					memcpy(clients[key].packet_buffer + clients[key].prev_size,
						ptr, rest);
					rest = 0;
					clients[key].prev_size += rest;
				}
			}
			do_recv(key);
		}
		else {
			// SEND
		}
	}
}

int do_accept()
{
	// Winsock Start - windock.dll 로드
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << "Error - Can not load 'winsock.dll' file\n";
		return 1;
	}

	// 1. 소켓생성  
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET)
	{
		cout << "Error - Invalid socket\n";
		return 1;
	}

	// 서버정보 객체설정
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// 2. 소켓설정
	if (::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		cout << "Error - Fail bind\n";
		// 6. 소켓종료
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return 1;
	}

	// 3. 수신대기열생성
	if (listen(listenSocket, 5) == SOCKET_ERROR)
	{
		cout << "Error - Fail listen\n";
		// 6. 소켓종료
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return 1;
	}

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);
	SOCKET clientSocket;
	DWORD flags;

	while (1)
	{
		clientSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			cout << "Error - Accept Failure\n";
			return 1;
		}

		int new_id = -1;
		for (int i = 0; i < MAX_USER; ++i) {
			if (false == clients[i].in_use) {
				new_id = i;
				break;
			}
		}
		if (-1 == new_id) {
			cout << "MAX USER overflow\n";
			continue;
		}

		clients[new_id].socket = clientSocket;
		clients[new_id].prev_size = 0;
		clients[new_id].x = clients[new_id].y = 4;

		ZeroMemory(&clients[new_id].over_ex.over, sizeof(clients[new_id].over_ex.over));

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket),
			g_iocp, new_id, 0);

		clients[new_id].in_use = true;		// inuse가 트루면 데이터를 보내는데 iocp가연결안되있으면 에러가 막뜨기때문에 연결 후 트루로 바꿔주자.

		send_login_ok_packet(new_id);
		for (int i = 0; i < MAX_USER; ++i)
			if (true == clients[i].in_use)
				send_put_player_packet(i, new_id);
		for (int i = 0; i < MAX_USER; ++i) {
			if (false == clients[i].in_use) continue;
			if (i == new_id) continue;
				send_put_player_packet(new_id, i);
		}
		do_recv(new_id);
	}

	// 6-2. 리슨 소켓종료
	closesocket(listenSocket);

	// Winsock End
	WSACleanup();

	return 0;
}

int main()
{
	vector <thread> worker_threads;


	wcout.imbue(std::locale("korean"));

	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	for (int i = 0; i < 4; ++i)
		worker_threads.emplace_back(thread{ worker_thread });
	thread accept_thread{ do_accept };
	accept_thread.join();
	for (auto &th : worker_threads) th.join();
}