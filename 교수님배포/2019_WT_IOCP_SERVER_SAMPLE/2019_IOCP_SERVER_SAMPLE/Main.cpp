#include <iostream>
#include <map>
#include <vector>
#include <thread>

using namespace std;

#include <winsock2.h>
#include "Protocol.h"

#pragma comment(lib, "Ws2_32.lib")

struct OVER_EX {
	WSAOVERLAPPED over;
	WSABUF dataBuffer;
	char messageBuffer[MAX_BUFFER];
	bool is_recv;
};

class SOCKETINFO						// Ŭ���̾�Ʈ�� �������ϱ� ������ Ŭ���̾�Ʈ�� ������ �����ϴ� ����ü
{										//
public:
	SOCKETINFO() {}
	SOCKETINFO(SOCKET &s) {
		socket = s;
		ZeroMemory(&over_ex.over, sizeof(over_ex.over));
		over_ex.dataBuffer.len = MAX_BUFFER;
		over_ex.dataBuffer.buf = over_ex.messageBuffer;
		x = y = 4;
	};
public:
	OVER_EX over_ex;
	SOCKET socket;			
	// ��Ŷ �������� ���� �迭�� ����
	char packet_buffer[MAX_BUFFER];
	int prev_size;
	char x, y;
};

HANDLE g_iocp;

// �����̾ƴ� id��
map <char, SOCKETINFO> clients;		// STL�� �ڷᱸ�� - �� �ڷ����� �ε����� �ϴ� �� �ڷ����� �迭

void do_recv(char id)
{
	DWORD flags{ 0 };

	clients[id].over_ex.dataBuffer.buf = clients[id].over_ex.messageBuffer;

	if (WSARecv(clients[id].socket, &clients[id].over_ex.dataBuffer, 1, NULL, &flags, &(clients[id].over_ex.over), 0))	// NULL �� ���� ������ ��������� ���� �� ���� �ִ�.
	{
		if (WSAGetLastError() != WSA_IO_PENDING)		// ������ �ʰ� ���� �־�ߵȴ�. - ���� ����.
		{
			cout << "Error - IO pending Failure\n";
			cout << WSAGetLastError() << "\n";
			while (true);
		}
	}
	else {
		cout << "Non Overlapped Recv return.\n";
		while (true);
	}
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
			printf("Error - Fail WSASend(error_code : %d)\n", WSAGetLastError());
			while (true);
		}
	}
	else {
		/*cout << "Non Overlapped Send Return\n";
		while (true);*/
	}
}

void send_pos_packet(char client)
{
	sc_packet_move_player packet;
	packet.id = client;
	packet.size = sizeof(packet);
	packet.type = SC_MOVE_PLAYER;
	packet.x = clients[client].x;
	packet.y = clients[client].y;

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

void send_put_player_packet(char new_id)
{
	sc_packet_put_player packet;				// ����, ���������ŵ� ���⼭ ������
	packet.id = new_id;
	packet.size = sizeof(packet);
	packet.type = SC_PUT_PLAYER;
	packet.x = clients[new_id].x;
	packet.y = clients[new_id].y;

	send_packet(new_id, &packet);
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
		wcout << L"���ǵ��� ���� ��Ŷ ���� ����!\n";
		while (true);
	}

	clients[client].x = x;
	clients[client].y = y;
	send_pos_packet(client);
}

void worker_thread()
{
	while (true) {
		DWORD io_byte;
		ULONGLONG l_key;
		//LPWSAOVERLAPPED over;
		OVER_EX *over_ex = nullptr;

		int is_error = GetQueuedCompletionStatus(g_iocp, &io_byte, &l_key, reinterpret_cast<LPWSAOVERLAPPED *>(over_ex), INFINITE);					// WSAOVERLAPPED ����ü�� recv���� send���� �˾Ƴ�����
		if (is_error == 0) {
			wcout << L"GQCS Error!\n";
			while (true);
		}
		char key = static_cast<char>(l_key);
		if (true == over_ex->is_recv) {
			wcout << L"Packet from Client : " << key << "\n";
			// RECV
			// ��Ŷ����
			int rest = io_byte;
			char *ptr = over_ex->messageBuffer;
			char packet_size = 0;
			if (0 < clients[key].prev_size)	// ��Ŷ�� �̿ϼ��� ������ ù��°���� ��Ŷ�� ����� ����
				packet_size = clients[key].packet_buffer[0];
			while (0 < rest) {
				if (0 == packet_size)		// ���ο� ��Ŷ����
					packet_size = ptr[0];
				int required = packet_size - clients[key].prev_size;
				if (required <= rest) {	// ��Ŷ�� ����� ����
					memcpy(clients[key].packet_buffer + clients[key].prev_size, ptr, required);
					process_packet(key, clients[key].packet_buffer);
					rest -= required;
					ptr += required;
					packet_size = 0;
					clients[key].prev_size = 0;
				}
				else {	// ��Ŷ�� ����� ����
					memcpy(clients[key].packet_buffer + clients[key].prev_size, ptr, rest);
					clients[key].prev_size += rest;
					rest = 0;
				}
			
			}
			do_recv(key);
		}
		else {	
			// SEND
			delete over_ex;
		}
	}
}

int do_accpet()
{
	// Winsock Start - windock.dll �ε�
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << "Error - Can not load 'winsock.dll' file\n";
		return 1;
	}

	// 1. ���ϻ���  
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);	// WSA_FLAG_OVERLAPPED �÷��� �ݵ�� �־���� OVEERLAPPED I/O ����
	if (listenSocket == INVALID_SOCKET)
	{
		cout << "Error - Invalid socket\n";
		return 1;
	}

	// �������� ��ü����
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// 2. ���ϼ���
	if ((::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN))) == SOCKET_ERROR)
	{
		cout << "Error - Fail bind\n";
		// 6. ��������
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return 1;
	}

	// 3. ���Ŵ�⿭����
	if (listen(listenSocket, 5) == SOCKET_ERROR)
	{
		cout << "Error - Fail listen\n";
		// 6. ��������
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
			if (0 == clients.count(i)) {
				new_id = i;
				break;
			}
		}
		if (-1 == new_id) {
			cout << "MAX USER overflow\n";
			closesocket(clientSocket);
			continue;
		}

		clients[new_id] = SOCKETINFO{clientSocket};												// Recv �غ�
		flags = 0;

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), g_iocp, new_id, 0);

		send_login_ok_packet(new_id);
		send_put_player_packet(new_id);

		do_recv(new_id);
	}

	// 6-2. ���� ��������
	closesocket(listenSocket);

	// Winsock End
	WSACleanup();

	return 0;
}

int main()
{
	vector<thread> worker_threads;
	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	for (int i = 0; i < 4; ++i)	// ���� �ھ�ϱ� 4��
		worker_threads.emplace_back(thread{ worker_thread });
	thread accept_thread{ do_accpet };

	// �����Ų ��������� ���������� ���
	accept_thread.join();
	for (auto &th : worker_threads) th.join();

	return 1;
}