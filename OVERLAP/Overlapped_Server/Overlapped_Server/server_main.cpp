/*
## 소켓 서버 : 1 v n - overlapped callback
1. socket()            : 소켓생성
2. bind()            : 소켓설정
3. listen()            : 수신대기열생성
4. accept()            : 연결대기
5. read()&write()
WIN recv()&send    : 데이터 읽고쓰기
6. close()
WIN closesocket    : 소켓종료
*/

#include <iostream>
#include <map>

using namespace std;

#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER        1024
#define SERVER_PORT        3500

struct SOCKETINFO						// 클라이언트가 여러개니까 각각의 클라이언트의 정보를 저장하는 구조체
{										//
	WSAOVERLAPPED overlapped;			// 커널이 사용하는 자료 - 건들면 안됨 - 각 소켓별로 필요함
	WSABUF dataBuffer;					// WSARecv 할때 WSABUF타입이 필요함 
	SOCKET socket;						// 클라이언트가 어떤소켓을 사용하는지
	char messageBuffer[MAX_BUFFER];		// send할때 필요한 버퍼 - 근데 왜 이게 여기있느냐? - 클라이언트에는 송수신 부분에서 버퍼 정의(동기식) - 
										// 여기서는 소켓이 100개 니까 버퍼가 100개생김(비동기식) - 커널별로 저장되는 버퍼 - 
										// 전역으로 하나만 있으면되지 왜 여러개를 만드느냐? - 동기식에서는 하나의 버퍼를 돌려써도 상관없다. 
										// 비동기식에서는 여러개의 소켓에서 송수신이 동시다발적으로 이루어지기 때문에 한버퍼에 쓰면 버퍼가 계속 덮어씌어진다.
	int receiveBytes;					// 
	int sendBytes;						// 
};

map <SOCKET, SOCKETINFO> clients;		// STL의 자료구조 - 앞 자료형을 인덱스로 하는 뒤 자료형의 배열

void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);	// 
void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);	// 

int main()
{
	// Winsock Start - windock.dll 로드
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << "Error - Can not load 'winsock.dll' file\n";
		return 1;
	}

	// 1. 소켓생성  
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);	// WSA_FLAG_OVERLAPPED 플래그 반드시 넣어줘야 OVEERLAPPED I/O 동작
	if (listenSocket == INVALID_SOCKET)
	{
		printf("Error - Invalid socket\n");
		return 1;
	}

	// 서버정보 객체설정
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// 2. 소켓설정
	if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		printf("Error - Fail bind\n");
		// 6. 소켓종료
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return 1;
	}

	// 3. 수신대기열생성
	if (listen(listenSocket, 5) == SOCKET_ERROR)
	{
		printf("Error - Fail listen\n");
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
			printf("Error - Accept Failure\n");
			return 1;
		}

		clients[clientSocket] = SOCKETINFO{};												// Recv 준비
		memset(&clients[clientSocket], 0x00, sizeof(struct SOCKETINFO));					//
		clients[clientSocket].socket = clientSocket;										//
		clients[clientSocket].dataBuffer.len = MAX_BUFFER;									//
		clients[clientSocket].dataBuffer.buf = clients[clientSocket].messageBuffer;			// WSA버퍼에 진짜 버퍼 연결
		flags = 0;

		// 중첩 소캣을 지정하고 완료시 실행될 함수를 넘겨준다.
		clients[clientSocket].overlapped.hEvent = (HANDLE)clients[clientSocket].socket;		// Callback모델에서는 hEvent를 안쓰니 소켓을 넣었다 -> 꼼수

		if (WSARecv(clients[clientSocket].socket, &clients[clientSocket].dataBuffer, 1, NULL, &flags, &(clients[clientSocket].overlapped), recv_callback))	// NULL 을 넣지 않으면 동기식으로 동작 할 수도 있다.
		{
			if (WSAGetLastError() != WSA_IO_PENDING)		// 끝나지 않고 좀더 있어야된다. - 문제 없다.
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

	// 6-2. 리슨 소켓종료
	closesocket(listenSocket);

	// Winsock End
	WSACleanup();

	return 0;
}

void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	SOCKET client_s = reinterpret_cast<int>(overlapped->hEvent);			// 어디서 온 소켓인지 읽음

	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	if (dataBytes == 0)								// dataBytes : 몇바이트 받은지
	{
		closesocket(clients[client_s].socket);
		clients.erase(client_s);
		return;
	}  // 클라이언트가 closesocket을 했을 경우

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
	}  // 클라이언트가 closesocket을 했을 경우

	{
		// WSASend(응답에 대한)의 콜백일 경우
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

// 이걸 이용해 어떻게 다중 접속을 구현하느냐
// 이대로 사용하면 안된다. -> hEvent 사용하면 안된다.
// recv_callback 데이터처리 -> WSARecv를 바로 호출해줘야 한다.