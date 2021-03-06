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

// 목적지와 대상
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

// 모든 플레이어에게 뿌림
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
	//delete psocketInfo;					// 할당해제를 해줘도 될까??
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
	if (bind(listenSocket, (struct sockaddr*) & serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
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
		clientSocket = accept(listenSocket, (struct sockaddr*) & clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			printf("Error - Accept Failure\n");
			return 1;
		}
		cout << "클라이언트 접속 : IP 주소 = " << inet_ntoa(clientAddr.sin_addr) << ", 포트 번호 = " << ntohs(clientAddr.sin_port) << "\n";

		// 억셉트하고 해당 클라이언트에 대한 클라이언트 클래스 생성해 아이디를 지정해주고 벡터에 넣어준다. 클라이언트에 아이디를 넘겨주는거는 첫번째 샌드때 Login enum을 이용해 해준다.
		
		
		CChessClient *pclient = InsertClient(clientSocket);

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), g_Iocp, reinterpret_cast<ULONG_PTR>(pclient), 0);
		flags = 0;
		INT id = pclient->GetID();
		cout << "ID : " << id << " 로 접속!\n";

		SendLoginOk(id);
		//RecvLogin(id);
		SendPutPlayer(id, false);
		UpdateViewList(id);

		if (WSARecv(clientSocket, &(pclient->m_ClientInfo->m_SocketInfo.wsaBuf), 1, NULL, &flags, &(pclient->m_ClientInfo->m_SocketInfo.overlapped), 0))	// NULL 을 넣지 않으면 동기식으로 동작 할 수도 있다.
		{
			if (WSAGetLastError() == WSA_IO_PENDING)		// 끝나지 않고 좀더 있어야된다. - 문제 없다.
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

	// 6-2. 리슨 소켓종료
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
					if (packetSize == 0)		// 사이즈 저장
						packetSize = buf[0];
					int required = packetSize - pclient->m_ClientInfo->m_PreSize;	// 패킷을 완성하는데 필요한 바이트 수
					if (ioByte < required)	// 리시브 받은 데이터가 필요한 데이터보다 작으면 패킷을 완성 시킬수 없다.
					{
						memcpy(pclient->m_ClientInfo->m_RecvBuf + pclient->m_ClientInfo->m_PreSize, buf, ioByte);
						pclient->m_ClientInfo->m_PreSize += ioByte;
						break;
					}
					else // 데이터를 완성할 수 있다면,,
					{
						memcpy(pclient->m_ClientInfo->m_RecvBuf + pclient->m_ClientInfo->m_PreSize, buf, required);	// 패킷을 완성시키는데 필요한만큼을 복사한다.

						e_sc_PacketType escType = pclient->ProcessPacket();											// 복사가 됐으므로 패킷처리해주고 send해주자
						int id = pclient->GetID();
						// send 부분 추가하자
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
						default: cout << "알 수 없는 패킷 도착\n"; break;
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
		// 다른 클라들의 뷰리스트에서 지워주자
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

// 시야는 7 x 7
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

			// 시야안에 있으면 nearlist에 넣어준다.
			if ((abs(mx - ox) < 4) && (abs(my - oy) < 4))
				pclient->m_NearList.emplace_back(potherClient);
		}
		// 시야 리스트에 있으면
		if (pclient->m_NearList.size() > 0) {
			for (auto& n : pclient->m_NearList) {
				// 시야리스트에서 찾는다.
				auto mv = find(pclient->m_ViewList.begin(), pclient->m_ViewList.end(), n);
				// 시야리스트에 없었다면
				if (mv == pclient->m_ViewList.end()) {
					pclient->m_ViewList.emplace_back(n);
					SendPutPlayerTo(n->GetID(), pclient->GetID());
					// 다른 클라의 시야리스트 검색
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