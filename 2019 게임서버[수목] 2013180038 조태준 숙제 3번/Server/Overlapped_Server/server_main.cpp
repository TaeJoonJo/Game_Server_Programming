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

#include "Protocols.h"
#include "Common.h"
#include "Client.h"

vecClients vClients;

void CALLBACK _callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);

VOID Broadcast(VOID *ppacket);
VOID SendPacket(INT id, VOID *ppacket);

VOID SendLogin(INT id, BOOL isBroad = FALSE)
{
	sc_packet_login packet;						// 동적할당을 해줘야하나? // SendPacket에서 동적할당해서 복사해주니까 안해줘도 될듯
	packet.id = id;
	packet.size = sizeof(sc_packet_login);
	packet.x = vClients[id]->GetX();
	packet.y = vClients[id]->GetY();
	INT num{};
	for (int i = 0; i < vClients.size(); ++i)
		if (vClients[i]->isRun() == TRUE)
			++num;
	packet.numclient = num;
	packet.type = sc_PACKETTYPE::Login;

	if (isBroad == TRUE)
		Broadcast(&packet);
	else
		SendPacket(id, &packet);

	for (int i = 0; i < vClients.size(); ++i)
	{
		if (vClients[i] && vClients[i]->isRun() == TRUE)
		{
			if (vClients[i]->GetID() != id)
			{
				sc_packet_put_player packet2;
				packet2.id = vClients[i]->GetID();
				packet2.size = sizeof(sc_packet_put_player);
				packet2.x = vClients[i]->GetX();
				packet2.y = vClients[i]->GetY();
				packet2.type = sc_PACKETTYPE::PutPlayer;

				SendPacket(id, &packet2);
			}
		}
	}
	
}

VOID SendLogout(INT id, BOOL isBroad = FALSE)
{
	sc_packet_logout packet;
	packet.id = id;
	packet.size = sizeof(sc_packet_logout);
	packet.type = sc_PACKETTYPE::Logout;

	if (isBroad == TRUE)
		Broadcast(&packet);
	else
		SendPacket(id, &packet);
}

VOID SendPostion(INT id, BOOL isBroad = FALSE)
{
	sc_packet_pos packet;
	packet.id = id;
	packet.size = sizeof(sc_packet_pos);
	packet.x = vClients[id]->GetX();
	packet.y = vClients[id]->GetY();
	packet.type = sc_PACKETTYPE::Position;

	if (isBroad == TRUE)
		Broadcast(&packet);
	else
		SendPacket(id, &packet);
}
// 목적지와 대상
VOID SendPutPlayer(INT id, BOOL isBroad = FALSE)
{
	sc_packet_put_player packet;
	packet.id = id;
	packet.size = sizeof(sc_packet_put_player);
	packet.x = vClients[id]->GetX();
	packet.y = vClients[id]->GetY();
	packet.type = sc_PACKETTYPE::PutPlayer;

	if (isBroad == TRUE)
		Broadcast(&packet);
	else
		SendPacket(id, &packet);
}

VOID SendRemovePlayer(INT id, BOOL isBroad = FALSE)
{
	sc_packet_remove_player packet;
	packet.id = id;
	packet.size = sizeof(sc_packet_put_player);
	packet.type = sc_PACKETTYPE::RemovePlayer;

	if (isBroad == TRUE)
		Broadcast(&packet);
	else
		SendPacket(id, &packet);
}

// 모든 플레이어에게 뿌림
VOID Broadcast(VOID *ppacket)
{
	for (int i = 0; i < vClients.size(); ++i)
	{
		if(vClients[i])
			SendPacket(vClients[i]->m_ClientInfo.GetId(), ppacket);
	}
}

VOID SendPacket(INT id, VOID *ppacket)
{
	if (vClients[id]->isRun() == FALSE)
		return ;

	SOCKETINFO* psocketInfo = new SOCKETINFO;
	char *p = reinterpret_cast<char *>(ppacket);
	memcpy(psocketInfo->buf, ppacket, p[0]);
	psocketInfo->wsaBuf.buf = psocketInfo->buf;
	psocketInfo->wsaBuf.len = p[0];
	psocketInfo->eventType = e_EventType::Event_Send;
	psocketInfo->id = id;
	ZeroMemory(&psocketInfo->overlapped, sizeof(WSAOVERLAPPED));

	if (WSASend(vClients[id]->GetSocket(), &psocketInfo->wsaBuf, 1, NULL, 0, &psocketInfo->overlapped, _callback) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "Error - Fail WSASend(error_code : " << WSAGetLastError() << ") in SendPacket(), ID : " << id << "\n";
		}
	}
	//cout << "SendPacket to ID : " << id << "\n";
	//delete psocketInfo;					// 할당해제를 해줘도 될까??
}

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
		cout << "클라이언트 접속 : IP 주소 = " << inet_ntoa(clientAddr.sin_addr) << ", 포트 번호 = " << ntohs(clientAddr.sin_port) << "\n";

		// 억셉트하고 해당 클라이언트에 대한 클라이언트 클래스 생성해 아이디를 지정해주고 벡터에 넣어준다. 클라이언트에 아이디를 넘겨주는거는 첫번째 샌드때 Login enum을 이용해 해준다.
		INT id = 0;
		//CChessClient *pClient = NULL;
		//for (int i = 0; i < vClients.size(); ++i)
		//{
		//	if (vClients[i]->isRun() == FALSE)
		//		pClient = vClients[i];
		//}
		//if (pClient == NULL)									// 안쓰는 클라이언트가 없으면 새로운 클라이언트 클래스생성
		//{
			CChessClient *chessClient = new CChessClient();
			if (chessClient->Initial(clientSocket) == FALSE)
			{
				cout << "Initial Fail!\n";
				continue;
			}
			vClients.emplace_back(chessClient);
			id = chessClient->GetID();
		//}
		//else
		//{
		//	if(pClient->Initial(clientSocket) == FALSE)			// 안쓰는 클라이언트가 있다면 기존 클라이언트 클래스 사용
		//	{
		//		cout << "Initial Fail!\n";
		//		continue;
		//	}
		//	id = pClient->GetID();
		//}

		flags = 0;	

		vClients[id]->m_ClientInfo.m_Socket = clientSocket;
		vClients[id]->m_ClientInfo.m_SocketInfo.wsaBuf.len = MAX_BUFFER;
		vClients[id]->m_ClientInfo.m_SocketInfo.wsaBuf.buf = vClients[id]->m_ClientInfo.m_SocketInfo.buf;
		vClients[id]->m_ClientInfo.m_SocketInfo.id = id;
		vClients[id]->m_ClientInfo.m_SocketInfo.eventType = e_EventType::Event_Recv;

		if (WSARecv(clientSocket, &(vClients[id]->m_ClientInfo.m_SocketInfo.wsaBuf), 1, NULL, &flags, &(vClients[id]->m_ClientInfo.m_SocketInfo.overlapped), _callback))	// NULL 을 넣지 않으면 동기식으로 동작 할 수도 있다.
		{
			if (WSAGetLastError() == WSA_IO_PENDING)		// 끝나지 않고 좀더 있어야된다. - 문제 없다.
			{
				cout << "WSA_IO_PENDING in WSARecv\n";
			}
			else
			{
				printf("Error - IO pending Failure\n");
				cout << "ID : " << id << "Logout!\n";
				SendLogout(id, TRUE);
				vClients[id]->Logout();
			}
		}
		else 
		{
			cout << "Non Overlapped Recv return.\n";
			cout << "ID : " << id << "Logout!\n";
			SendLogout(id, TRUE);
			vClients[id]->Logout();
		}
	}

	for (auto d : vClients)
		delete d;
	vClients.clear();

	// 6-2. 리슨 소켓종료
	closesocket(listenSocket);

	// Winsock End
	WSACleanup();

	return 0;
}

void CALLBACK _callback(DWORD Error, DWORD Transferred, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	SOCKETINFO *psocketInfo = reinterpret_cast<SOCKETINFO*>(overlapped);
	int id = psocketInfo->id;
	CChessClient *pClient = vClients[id];
	int dataSize = Transferred;
	DWORD flags = 0;

	cout << " (" << Transferred << ") bytes in _callback from ID : " << id << "\n";

	if (pClient->isRun() == FALSE)
	{
		cout << "ID : " << id << " is Not Run!\n";
		return;
	}
	if (dataSize == 0)		// 클라이언트가 접속 종료 했을시 다른 클라이언트들에게 그 클라이언트의 종료를 알린다.
	{
		cout << "ID : " << id << " LogOut! in callback\n";
		SendLogout(id, TRUE);
		pClient->Logout();
		//SendRemovePlayer(id, TRUE);
		return;
	}

	e_EventType etype = psocketInfo->eventType;
	
	switch (etype)
	{
	case e_EventType::Event_Recv:
	{
		char *buf = pClient->m_ClientInfo.m_SocketInfo.buf;
		int packetSize = 0;
		if (pClient->m_ClientInfo.m_PreSize != 0)
			packetSize = pClient->m_ClientInfo.m_RecvBuf[0];
		while (dataSize > 0)
		{
			if (packetSize == 0)		// 사이즈 저장
				packetSize = buf[0];
			int required = packetSize - pClient->m_ClientInfo.m_PreSize;	// 패킷을 완성하는데 필요한 바이트 수
			if (dataSize < required)	// 리시브 받은 데이터가 필요한 데이터보다 작으면 패킷을 완성 시킬수 없다.
			{
				memcpy(pClient->m_ClientInfo.m_RecvBuf + pClient->m_ClientInfo.m_PreSize, buf, dataSize);
				pClient->m_ClientInfo.m_PreSize += dataSize;
				break;
			}
			else // 데이터를 완성할 수 있다면,,
			{
				memcpy(pClient->m_ClientInfo.m_RecvBuf + pClient->m_ClientInfo.m_PreSize, buf, required);	// 패킷을 완성시키는데 필요한만큼을 복사한다.
				
				e_sc_PacketType escType = pClient->ProcessPacket();											// 복사가 됐으므로 패킷처리해주고 send해주자
				// send 부분 추가하자
				switch (escType)
				{
				case e_sc_PacketType::Login:
				{
					SendLogin(id, TRUE);
				} break;
				case e_sc_PacketType::Logout:
				{
					SendLogout(id, TRUE);
				} break;
				case e_sc_PacketType::Position:
				{
					SendPostion(id, TRUE);
				} break;
				case e_sc_PacketType::PutPlayer:
				{
					SendPutPlayer(id, TRUE);
				} break;
				case e_sc_PacketType::RemovePlayer:
				{
					SendRemovePlayer(id, TRUE);
				} break;
				}
				pClient->m_ClientInfo.m_PreSize = 0;
				dataSize -= required;
				buf += required;
				packetSize = 0;
			}
		}
		if (WSARecv(pClient->m_ClientInfo.GetSocket(), &pClient->m_ClientInfo.m_SocketInfo.wsaBuf, 1, NULL, &flags, &pClient->m_ClientInfo.m_SocketInfo.overlapped, _callback))
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				printf("Error - IO pending Failure\n");
				SendLogout(id, TRUE);
				pClient->Logout();
				
				return;
			}
		}
		else // 연결 끊어주자
		{
			/*cout << "Non Overlapped Recv return.\n";
			SendLogout(id, TRUE);
			pClient->Logout();*/
			
			return;
		}
	} break;
	case e_EventType::Event_Send:
	{
		if (dataSize != psocketInfo->wsaBuf.len)			// 부분적인 send일 경우 접속을 끊어주는 것이 좋다.
		{
			cout << "Part send Error ID : " << id << "\n";
			SendLogout(id, TRUE);
			pClient->Logout();
		}
	} break;
	}
}