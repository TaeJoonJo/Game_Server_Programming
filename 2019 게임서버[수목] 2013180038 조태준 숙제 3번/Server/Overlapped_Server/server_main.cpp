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

#include "Protocols.h"
#include "Common.h"
#include "Client.h"

vecClients vClients;

void CALLBACK _callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);

VOID Broadcast(VOID *ppacket);
VOID SendPacket(INT id, VOID *ppacket);

VOID SendLogin(INT id, BOOL isBroad = FALSE)
{
	sc_packet_login packet;						// �����Ҵ��� ������ϳ�? // SendPacket���� �����Ҵ��ؼ� �������ִϱ� �����൵ �ɵ�
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
// �������� ���
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

// ��� �÷��̾�� �Ѹ�
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
	//delete psocketInfo;					// �Ҵ������� ���൵ �ɱ�??
}

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
		cout << "Ŭ���̾�Ʈ ���� : IP �ּ� = " << inet_ntoa(clientAddr.sin_addr) << ", ��Ʈ ��ȣ = " << ntohs(clientAddr.sin_port) << "\n";

		// ���Ʈ�ϰ� �ش� Ŭ���̾�Ʈ�� ���� Ŭ���̾�Ʈ Ŭ���� ������ ���̵� �������ְ� ���Ϳ� �־��ش�. Ŭ���̾�Ʈ�� ���̵� �Ѱ��ִ°Ŵ� ù��° ���嶧 Login enum�� �̿��� ���ش�.
		INT id = 0;
		//CChessClient *pClient = NULL;
		//for (int i = 0; i < vClients.size(); ++i)
		//{
		//	if (vClients[i]->isRun() == FALSE)
		//		pClient = vClients[i];
		//}
		//if (pClient == NULL)									// �Ⱦ��� Ŭ���̾�Ʈ�� ������ ���ο� Ŭ���̾�Ʈ Ŭ��������
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
		//	if(pClient->Initial(clientSocket) == FALSE)			// �Ⱦ��� Ŭ���̾�Ʈ�� �ִٸ� ���� Ŭ���̾�Ʈ Ŭ���� ���
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

		if (WSARecv(clientSocket, &(vClients[id]->m_ClientInfo.m_SocketInfo.wsaBuf), 1, NULL, &flags, &(vClients[id]->m_ClientInfo.m_SocketInfo.overlapped), _callback))	// NULL �� ���� ������ ��������� ���� �� ���� �ִ�.
		{
			if (WSAGetLastError() == WSA_IO_PENDING)		// ������ �ʰ� ���� �־�ߵȴ�. - ���� ����.
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

	// 6-2. ���� ��������
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
	if (dataSize == 0)		// Ŭ���̾�Ʈ�� ���� ���� ������ �ٸ� Ŭ���̾�Ʈ�鿡�� �� Ŭ���̾�Ʈ�� ���Ḧ �˸���.
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
			if (packetSize == 0)		// ������ ����
				packetSize = buf[0];
			int required = packetSize - pClient->m_ClientInfo.m_PreSize;	// ��Ŷ�� �ϼ��ϴµ� �ʿ��� ����Ʈ ��
			if (dataSize < required)	// ���ú� ���� �����Ͱ� �ʿ��� �����ͺ��� ������ ��Ŷ�� �ϼ� ��ų�� ����.
			{
				memcpy(pClient->m_ClientInfo.m_RecvBuf + pClient->m_ClientInfo.m_PreSize, buf, dataSize);
				pClient->m_ClientInfo.m_PreSize += dataSize;
				break;
			}
			else // �����͸� �ϼ��� �� �ִٸ�,,
			{
				memcpy(pClient->m_ClientInfo.m_RecvBuf + pClient->m_ClientInfo.m_PreSize, buf, required);	// ��Ŷ�� �ϼ���Ű�µ� �ʿ��Ѹ�ŭ�� �����Ѵ�.
				
				e_sc_PacketType escType = pClient->ProcessPacket();											// ���簡 �����Ƿ� ��Ŷó�����ְ� send������
				// send �κ� �߰�����
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
		else // ���� ��������
		{
			/*cout << "Non Overlapped Recv return.\n";
			SendLogout(id, TRUE);
			pClient->Logout();*/
			
			return;
		}
	} break;
	case e_EventType::Event_Send:
	{
		if (dataSize != psocketInfo->wsaBuf.len)			// �κ����� send�� ��� ������ �����ִ� ���� ����.
		{
			cout << "Part send Error ID : " << id << "\n";
			SendLogout(id, TRUE);
			pClient->Logout();
		}
	} break;
	}
}