#include "Protocols.h"
#include "Common.h"
#include "Client.h"

HANDLE g_Iocp;
CChessClient *ChessClients[MAX_USER];
map<int, CChessClient*>mapChessClients;
mutex g_Mutex;

INT AcceptThread();
VOID WorkerThread();

CChessClient* InsertClient(SOCKET& socket);
const bool DeleteClient(CChessClient* pclient);
const bool DisconnectClient(CChessClient* pclient);

VOID UpdateViewList(INT id);

VOID Broadcast(VOID *ppacket);
VOID SendPacket(INT id, VOID *ppacket);

VOID SendLoginOk(INT id)
{
	sc_packet_login_ok packet;
	packet.size = sizeof(sc_packet_login_ok);
	packet.type = e_sc_PacketType::Login_Ok;
	packet.id = id;

	SendPacket(id, &packet);
}

VOID RecvLogin(INT id)
{
	sc_packet_put_player packet;
	packet.id = id;
	packet.size = sizeof(sc_packet_put_player);
	packet.type = e_sc_PacketType::PutPlayer;
	packet.x = mapChessClients[id]->GetX();
	packet.y = mapChessClients[id]->GetY();

	Broadcast(&packet);
	
	for (auto& client : mapChessClients)
	{
		CChessClient* pclient = client.second;
		if (pclient->GetID() == id) continue;

		sc_packet_put_player packet2;
		packet2.id = pclient->GetID();
		packet2.size = sizeof(sc_packet_put_player);
		packet2.x = pclient->GetX();
		packet2.y = pclient->GetY();
		packet2.type = sc_PACKETTYPE::PutPlayer;

		SendPacket(id, &packet2);
	}
}

VOID SendLogout(INT id, bool isBroad = false)
{
	sc_packet_logout packet;
	packet.id = id;
	packet.size = sizeof(sc_packet_logout);
	packet.type = sc_PACKETTYPE::Logout;

	if (isBroad == true)
		Broadcast(&packet);
	else
		SendPacket(id, &packet);
}

VOID SendMovePlayer(INT id, bool isBroad = false)
{
	sc_packet_move_player packet;
	packet.id = id;
	packet.size = sizeof(sc_packet_move_player);
	packet.x = mapChessClients[id]->GetX();
	packet.y = mapChessClients[id]->GetY();
	packet.type = sc_PACKETTYPE::MovePlayer;

	if (isBroad == true)
		Broadcast(&packet);
	else
		SendPacket(id, &packet);
}
VOID SendMovePlayerTo(INT myId, INT toId)
{
	sc_packet_move_player packet;
	packet.id = myId;
	packet.size = sizeof(sc_packet_move_player);
	packet.x = mapChessClients[myId]->GetX();
	packet.y = mapChessClients[myId]->GetY();
	packet.type = sc_PACKETTYPE::MovePlayer;

	SendPacket(toId, &packet);
}

// �������� ���
VOID SendPutPlayer(INT id, bool isBroad = false)
{
	sc_packet_put_player packet;
	packet.id = id;
	packet.size = sizeof(sc_packet_put_player);
	packet.x = mapChessClients[id]->GetX();
	packet.y = mapChessClients[id]->GetY();
	packet.type = sc_PACKETTYPE::PutPlayer;

	if (isBroad == true)
		Broadcast(&packet);
	else
		SendPacket(id, &packet);
}
VOID SendPutPlayerTo(INT myId, INT toId)
{
	sc_packet_put_player packet;
	packet.id = myId;
	packet.size = sizeof(sc_packet_put_player);
	packet.x = mapChessClients[myId]->GetX();
	packet.y = mapChessClients[myId]->GetY();
	packet.type = sc_PACKETTYPE::PutPlayer;

	SendPacket(toId, &packet);
}

VOID SendRemovePlayer(INT id, bool isBroad = false)
{
	sc_packet_remove_player packet;
	packet.id = id;
	packet.size = sizeof(sc_packet_remove_player);
	packet.type = sc_PACKETTYPE::RemovePlayer;

	if (isBroad == true)
		Broadcast(&packet);
	else
		SendPacket(id, &packet);
}
VOID SendRemovePlayerTo(INT myId, INT toId)
{
	sc_packet_remove_player packet;
	packet.id = myId;
	packet.size = sizeof(sc_packet_remove_player);
	packet.type = sc_PACKETTYPE::RemovePlayer;

	SendPacket(toId, &packet);
}

// ��� �÷��̾�� �Ѹ�
VOID Broadcast(VOID *ppacket)
{
	CChessClient* pclient = nullptr;

	for(auto &client : mapChessClients) {
		pclient = client.second;
			SendPacket(pclient->m_ClientInfo->GetId(), ppacket);
	}
}

VOID SendPacket(INT id, VOID *ppacket)
{
	SOCKETINFO* psocketInfo = new SOCKETINFO;
	char *p = reinterpret_cast<char *>(ppacket);
	memcpy(psocketInfo->buf, ppacket, p[0]);
	psocketInfo->wsaBuf.buf = psocketInfo->buf;
	psocketInfo->wsaBuf.len = p[0];
	psocketInfo->eventType = e_EventType::Event_Send;
	psocketInfo->id = id;
	ZeroMemory(&psocketInfo->overlapped, sizeof(WSAOVERLAPPED));

	if (WSASend(mapChessClients[id]->GetSocket(), &psocketInfo->wsaBuf, 1, NULL, 0, &psocketInfo->overlapped, 0) == SOCKET_ERROR)
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
	for (int i = 0; i < MAX_USER; ++i)
		ChessClients[i] = new CChessClient;

	vector<thread> vecworkerThreads;

	g_Iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	for (int i = 0; i < 4; ++i)
		vecworkerThreads.emplace_back(thread{ WorkerThread });
	thread acceptThread{ AcceptThread };

	acceptThread.join();
	for (auto& thread : vecworkerThreads)
		thread.join();
	
	for (int i = 0; i < MAX_USER; ++i)
		delete ChessClients[i];

	return 0;
}

INT AcceptThread()
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
	if (bind(listenSocket, (struct sockaddr*) & serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
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
		clientSocket = accept(listenSocket, (struct sockaddr*) & clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			printf("Error - Accept Failure\n");
			return 1;
		}
		cout << "Ŭ���̾�Ʈ ���� : IP �ּ� = " << inet_ntoa(clientAddr.sin_addr) << ", ��Ʈ ��ȣ = " << ntohs(clientAddr.sin_port) << "\n";

		// ���Ʈ�ϰ� �ش� Ŭ���̾�Ʈ�� ���� Ŭ���̾�Ʈ Ŭ���� ������ ���̵� �������ְ� ���Ϳ� �־��ش�. Ŭ���̾�Ʈ�� ���̵� �Ѱ��ִ°Ŵ� ù��° ���嶧 Login enum�� �̿��� ���ش�.
		
		
		CChessClient *pclient = InsertClient(clientSocket);

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), g_Iocp, reinterpret_cast<ULONG_PTR>(pclient), 0);
		flags = 0;
		INT id = pclient->GetID();
		cout << "ID : " << id << " �� ����!\n";

		SendLoginOk(id);
		//RecvLogin(id);
		SendPutPlayer(id, false);
		UpdateViewList(id);

		if (WSARecv(clientSocket, &(pclient->m_ClientInfo->m_SocketInfo.wsaBuf), 1, NULL, &flags, &(pclient->m_ClientInfo->m_SocketInfo.overlapped), 0))	// NULL �� ���� ������ ��������� ���� �� ���� �ִ�.
		{
			if (WSAGetLastError() == WSA_IO_PENDING)		// ������ �ʰ� ���� �־�ߵȴ�. - ���� ����.
			{
				//cout << "WSA_IO_PENDING in WSARecv\n";
			}
			else
			{
				cout << "Error - IO pending Failure " << WSAGetLastError() << "\n";
				cout << "ID : " << id << "Logout!\n";
				SendLogout(id, TRUE);
				//vClients[id]->Logout();
			}
		}
		else
		{
			cout << "Non Overlapped Recv return.\n";
			cout << "ID : " << id << "Logout!\n";
			SendLogout(id, TRUE);
			//vClients[id]->Logout();
		}
	}

	// 6-2. ���� ��������
	closesocket(listenSocket);

	// Winsock End
	WSACleanup();
}

VOID WorkerThread()
{
	DWORD				ioByte = 0;
	DWORD				retval = 0;
	CChessClient		*pclient = nullptr;
	SOCKETINFO			*poverlapped = nullptr;

	while (1)
	{
		ioByte = 0;
		retval = 0;
		pclient = nullptr;
		poverlapped = nullptr;

		if (false == GetQueuedCompletionStatus(g_Iocp, &ioByte,
			reinterpret_cast<PULONG_PTR>(&pclient),
			reinterpret_cast<LPOVERLAPPED*>(&poverlapped), INFINITE))
			retval = WSAGetLastError();

		if (retval == 0)
		{
			if (pclient == nullptr)
				continue;

			if (ioByte == 0) {
				cout << "WorkerThread LogOut ID : " << static_cast<int>(pclient->GetID()) << "\n";
				DisconnectClient(pclient);
				continue;
			}

			e_EventType etype = poverlapped->eventType;

			switch (etype)
			{
			case e_EventType::Event_Recv:
			{
				char* buf = pclient->m_ClientInfo->m_SocketInfo.buf;
				int packetSize = 0;
				if (pclient->m_ClientInfo->m_PreSize != 0)
					packetSize = pclient->m_ClientInfo->m_RecvBuf[0];
				while (ioByte > 0)
				{
					if (packetSize == 0)		// ������ ����
						packetSize = buf[0];
					int required = packetSize - pclient->m_ClientInfo->m_PreSize;	// ��Ŷ�� �ϼ��ϴµ� �ʿ��� ����Ʈ ��
					if (ioByte < required)	// ���ú� ���� �����Ͱ� �ʿ��� �����ͺ��� ������ ��Ŷ�� �ϼ� ��ų�� ����.
					{
						memcpy(pclient->m_ClientInfo->m_RecvBuf + pclient->m_ClientInfo->m_PreSize, buf, ioByte);
						pclient->m_ClientInfo->m_PreSize += ioByte;
						break;
					}
					else // �����͸� �ϼ��� �� �ִٸ�,,
					{
						memcpy(pclient->m_ClientInfo->m_RecvBuf + pclient->m_ClientInfo->m_PreSize, buf, required);	// ��Ŷ�� �ϼ���Ű�µ� �ʿ��Ѹ�ŭ�� �����Ѵ�.

						e_sc_PacketType escType = pclient->ProcessPacket();											// ���簡 �����Ƿ� ��Ŷó�����ְ� send������
						int id = pclient->GetID();
						// send �κ� �߰�����
						switch (escType)
						{
						case e_sc_PacketType::Logout:
						{
							SendLogout(id, true);
						} break;
						case e_sc_PacketType::MovePlayer:
						{
							SendMovePlayer(id, false);
							UpdateViewList(id);
						} break;
						case e_sc_PacketType::PutPlayer:
						{
							SendPutPlayer(id, false);
							//UpdateViewList(id);
						} break;
						case e_sc_PacketType::RemovePlayer:
						{
							SendRemovePlayer(id, false);
							//UpdateViewList(id);
						} break;
						default: cout << "�� �� ���� ��Ŷ ����\n"; break;
						}
						pclient->m_ClientInfo->m_PreSize = 0;
						ioByte -= required;
						buf += required;
						packetSize = 0;
					}
				}
				DWORD flags = 0;
				if (WSARecv(pclient->m_ClientInfo->GetSocket(), &pclient->m_ClientInfo->m_SocketInfo.wsaBuf, 1, NULL, &flags, &pclient->m_ClientInfo->m_SocketInfo.overlapped, 0))
				{
					if (WSAGetLastError() != WSA_IO_PENDING)
					{
						printf("Error - IO pending Failure\n");
						DisconnectClient(pclient);

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
				delete poverlapped;
			} break;
			default: continue;
			}
		}
		else
		{
			if (WAIT_TIMEOUT == retval)
			{

			}
			else if (ERROR_NETNAME_DELETED == retval)
			{
				cout << "WorkerThread ERROR CODE : ERROR_NETNAME_DELETED\n";
				cout << "ID : " << static_cast<int>(pclient->GetID()) << " LogOut!\n";
				DisconnectClient(pclient);
			}
			else
			{
				cout << "GetQueuedCompletionStatus unknown Error " << WSAGetLastError() << "\n";
				cout << "ID : " << static_cast<int>(pclient->GetID()) << " LogOut!\n";
				DisconnectClient(pclient);
			}

			continue;
		}
	}
}

CChessClient *InsertClient(SOCKET &socket)
{
	CChessClient* pclient = nullptr;
	g_Mutex.lock();
	{
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (ChessClients[i]->m_bIsRun == false)
			{
				pclient = ChessClients[i];
				break;
			}
		}
		if (pclient != nullptr)
		{
			if (pclient->Initial(socket) == false)
				pclient = nullptr;
			mapChessClients[pclient->GetID()] = pclient;
		}
	}
	g_Mutex.unlock();

	return pclient;
}

const bool DeleteClient(CChessClient *pclient)
{
	g_Mutex.lock();
	{
		pclient->m_bIsRun = false;
		pclient->m_ClientInfo->ClearIoBuffer();

		CChessClient* potherClient = nullptr;
		// �ٸ� Ŭ����� �丮��Ʈ���� ��������
		for (auto& client : mapChessClients) {
			potherClient = client.second;
			if (pclient == potherClient)
				continue;

			auto ov = find(potherClient->m_ViewList.begin(), potherClient->m_ViewList.end(), pclient);
			if (ov != potherClient->m_ViewList.end())
				potherClient->m_ViewList.erase(ov);
		}
		pclient->m_ViewList.clear();

		mapChessClients.erase(pclient->GetID());
	}
	g_Mutex.unlock();
	return true;
}

const bool DisconnectClient(CChessClient* pclient)
{
	DeleteClient(pclient);

	SendRemovePlayer(pclient->GetID(), true);

	return true;
}

// �þߴ� 7 x 7
VOID UpdateViewList(INT id)
{
	g_Mutex.lock();
	{
		CChessClient* pclient = mapChessClients[id];
		float mx = pclient->GetX();
		float my = pclient->GetY();

		CChessClient* potherClient = nullptr;
		float ox = 0;
		float oy = 0;
		for (auto& client : mapChessClients)
		{
			potherClient = client.second;

			if (pclient == potherClient)
				continue;

			ox = potherClient->GetX();
			oy = potherClient->GetY();

			// �þ߾ȿ� ������ nearlist�� �־��ش�.
			if ((abs(mx - ox) < 4) && (abs(my - oy) < 4))
				pclient->m_NearList.emplace_back(potherClient);
		}
		// �þ� ����Ʈ�� ������
		if (pclient->m_NearList.size() > 0) {
			for (auto& n : pclient->m_NearList) {
				// �þ߸���Ʈ���� ã�´�.
				auto mv = find(pclient->m_ViewList.begin(), pclient->m_ViewList.end(), n);
				// �þ߸���Ʈ�� �����ٸ�
				if (mv == pclient->m_ViewList.end()) {
					pclient->m_ViewList.emplace_back(n);
					SendPutPlayerTo(n->GetID(), pclient->GetID());
					// �ٸ� Ŭ���� �þ߸���Ʈ �˻�
					auto ov = find(n->m_ViewList.begin(), n->m_ViewList.end(), pclient);
					if (ov == n->m_ViewList.end()) {
						n->m_ViewList.emplace_back(pclient);
						SendPutPlayerTo(pclient->GetID(), n->GetID());
					}
					else {
						SendMovePlayerTo(pclient->GetID(), n->GetID());
					}
				}
				else {
					auto ov = find(n->m_ViewList.begin(), n->m_ViewList.end(), pclient);
					if (ov == n->m_ViewList.end()) {
						n->m_ViewList.emplace_back(pclient);
						SendPutPlayerTo(pclient->GetID(), n->GetID());
					}
					else {
						SendMovePlayerTo(pclient->GetID(), n->GetID());
					}
				}
			}
		}
		if (pclient->m_ViewList.size() > 0) {
			for (auto& v : pclient->m_ViewList) {
				auto mn = find(pclient->m_NearList.begin(), pclient->m_NearList.end(), v);
				if (mn == pclient->m_NearList.end()) {
					pclient->m_RemoveList.emplace_back(v);
					SendRemovePlayerTo(v->GetID(), pclient->GetID());

					auto ov = find(v->m_ViewList.begin(), v->m_ViewList.end(), pclient);
					if (ov == v->m_ViewList.end()) {

					}
					else {
						v->m_ViewList.erase(ov);
						SendRemovePlayerTo(pclient->GetID(), v->GetID());
					}
				}
				else {

				}
			}
		}
		if (pclient->m_RemoveList.size() > 0) {
			for (auto& r : pclient->m_RemoveList) {
				auto mv = find(pclient->m_ViewList.begin(), pclient->m_ViewList.end(), r);
				if (mv == pclient->m_ViewList.end()) {

				}
				else {
					pclient->m_ViewList.erase(mv);
				}
			}
		}
		pclient->m_RemoveList.clear();
		pclient->m_NearList.clear();
	}
	g_Mutex.unlock();
}