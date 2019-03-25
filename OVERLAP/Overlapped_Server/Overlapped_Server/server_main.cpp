/*
## ���� ���� : 1 v n - overlapped callback
1. socket()            : ���ϻ���
2. bind()            : ���ϼ���
3. listen()            : ���Ŵ�⿭����
4. accept()            : ������
5. read()&write()
WIN recv()&send    : ������ �а���
6. close()
WIN closesocket    : ��������
*/

#include <iostream>
#include <map>

using namespace std;

#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER        1024
#define SERVER_PORT        3500

struct SOCKETINFO						// Ŭ���̾�Ʈ�� �������ϱ� ������ Ŭ���̾�Ʈ�� ������ �����ϴ� ����ü
{										//
	WSAOVERLAPPED overlapped;			// Ŀ���� ����ϴ� �ڷ� - �ǵ�� �ȵ� - �� ���Ϻ��� �ʿ���
	WSABUF dataBuffer;					// WSARecv �Ҷ� WSABUFŸ���� �ʿ��� 
	SOCKET socket;						// Ŭ���̾�Ʈ�� ������� ����ϴ���
	char messageBuffer[MAX_BUFFER];		// send�Ҷ� �ʿ��� ���� - �ٵ� �� �̰� �����ִ���? - Ŭ���̾�Ʈ���� �ۼ��� �κп��� ���� ����(�����) - 
										// ���⼭�� ������ 100�� �ϱ� ���۰� 100������(�񵿱��) - Ŀ�κ��� ����Ǵ� ���� - 
										// �������� �ϳ��� ��������� �� �������� �������? - ����Ŀ����� �ϳ��� ���۸� �����ᵵ �������. 
										// �񵿱�Ŀ����� �������� ���Ͽ��� �ۼ����� ���ôٹ������� �̷������ ������ �ѹ��ۿ� ���� ���۰� ��� ���������.
	int receiveBytes;					// 
	int sendBytes;						// 
};

map <SOCKET, SOCKETINFO> clients;		// STL�� �ڷᱸ�� - �� �ڷ����� �ε����� �ϴ� �� �ڷ����� �迭

void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);	// 
void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);	// 

int main()
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
		printf("Error - Invalid socket\n");
		return 1;
	}

	// �������� ��ü����
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// 2. ���ϼ���
	if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		printf("Error - Fail bind\n");
		// 6. ��������
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return 1;
	}

	// 3. ���Ŵ�⿭����
	if (listen(listenSocket, 5) == SOCKET_ERROR)
	{
		printf("Error - Fail listen\n");
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
			printf("Error - Accept Failure\n");
			return 1;
		}

		clients[clientSocket] = SOCKETINFO{};												// Recv �غ�
		memset(&clients[clientSocket], 0x00, sizeof(struct SOCKETINFO));					//
		clients[clientSocket].socket = clientSocket;										//
		clients[clientSocket].dataBuffer.len = MAX_BUFFER;									//
		clients[clientSocket].dataBuffer.buf = clients[clientSocket].messageBuffer;			// WSA���ۿ� ��¥ ���� ����
		flags = 0;

		// ��ø ��Ĺ�� �����ϰ� �Ϸ�� ����� �Լ��� �Ѱ��ش�.
		clients[clientSocket].overlapped.hEvent = (HANDLE)clients[clientSocket].socket;		// Callback�𵨿����� hEvent�� �Ⱦ��� ������ �־��� -> �ļ�

		if (WSARecv(clients[clientSocket].socket, &clients[clientSocket].dataBuffer, 1, NULL, &flags, &(clients[clientSocket].overlapped), recv_callback))	// NULL �� ���� ������ ��������� ���� �� ���� �ִ�.
		{
			if (WSAGetLastError() != WSA_IO_PENDING)		// ������ �ʰ� ���� �־�ߵȴ�. - ���� ����.
			{
				printf("Error - IO pending Failure\n");
				return 1;
			}
		}
		else {
			cout << "Non Overlapped Recv return.\n";
			return 1;
		}
	}

	// 6-2. ���� ��������
	closesocket(listenSocket);

	// Winsock End
	WSACleanup();

	return 0;
}

void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	SOCKET client_s = reinterpret_cast<int>(overlapped->hEvent);			// ��� �� �������� ����

	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	if (dataBytes == 0)								// dataBytes : �����Ʈ ������
	{
		closesocket(clients[client_s].socket);
		clients.erase(client_s);
		return;
	}  // Ŭ���̾�Ʈ�� closesocket�� ���� ���

	cout << "TRACE - Receive message : "
		<< clients[client_s].messageBuffer
		<< " (" << dataBytes << ") bytes)\n";

	clients[client_s].dataBuffer.len = dataBytes;
	memset(&(clients[client_s].overlapped), 0x00, sizeof(WSAOVERLAPPED));
	clients[client_s].overlapped.hEvent = (HANDLE)client_s;
	if (WSASend(client_s, &(clients[client_s].dataBuffer), 1, &dataBytes, 0, &(clients[client_s].overlapped), send_callback) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			printf("Error - Fail WSASend(error_code : %d)\n", WSAGetLastError());
		}
	}

}

void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	SOCKET client_s = reinterpret_cast<int>(overlapped->hEvent);

	if (dataBytes == 0)
	{
		closesocket(clients[client_s].socket);
		clients.erase(client_s);
		return;
	}  // Ŭ���̾�Ʈ�� closesocket�� ���� ���

	{
		// WSASend(���信 ����)�� �ݹ��� ���
		clients[client_s].dataBuffer.len = MAX_BUFFER;
		clients[client_s].dataBuffer.buf = clients[client_s].messageBuffer;

		cout << "TRACE - Send message : "
			<< clients[client_s].messageBuffer
			<< " (" << dataBytes << " bytes)\n";
		memset(&(clients[client_s].overlapped), 0x00, sizeof(WSAOVERLAPPED));
		clients[client_s].overlapped.hEvent = (HANDLE) client_s;
		if (WSARecv(client_s, &clients[client_s].dataBuffer, 1, NULL, &flags, &(clients[client_s].overlapped), recv_callback) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				printf("Error - Fail WSARecv(error_code : %d)\n", WSAGetLastError());
			}
		}
	}
}

// �̰� �̿��� ��� ���� ������ �����ϴ���
// �̴�� ����ϸ� �ȵȴ�. -> hEvent ����ϸ� �ȵȴ�.
// recv_callback ������ó�� -> WSARecv�� �ٷ� ȣ������� �Ѵ�.