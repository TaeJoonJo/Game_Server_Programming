#pragma once
#include "stdafx.h"
#include "Network.h"

CNetwork::CNetwork()
	: m_Socket(INVALID_SOCKET)
{
	m_pPlayerManager = new CPlayerManager();
	//ZeroMemory(&m_buf, sizeof(m_buf));
}

CNetwork::~CNetwork()
{
	delete m_pPlayerManager;
	closesocket(m_Socket);

	WSACleanup();
}

bool CNetwork::Initialize()
{
	int retval{ 0 };

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		CNetworkManager::ErrDisplay("[WSAStratup() Error] 윈속 초기화 에러");
		return 1;
	}

	m_Socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0);
	if (m_Socket == INVALID_SOCKET) {
		CNetworkManager::ErrDisplay("[WSASocket Error]");
		return false;
	}

	if (!Connect()) {
		CNetworkManager::ErrDisplay("[connect() Error]");
		return false;
	}

	return true;
}

bool CNetwork::Connect()
{
	int retval{ 0 };
	char serverIP[15 + 1];

	// 서버 아이피 입력
	printf("IP : ");
	fgets(serverIP, sizeof(serverIP), stdin);
	if (serverIP[(strlen(serverIP) - 1)] == '\n')
		serverIP[(strlen(serverIP) - 1)] = '0';
	if (strlen(serverIP) == 0)
		return false;

	// 서버로 커넥트
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(serverIP);
	serverAddr.sin_port = htons(SERVERPORT);
	retval = connect(m_Socket, reinterpret_cast<SOCKADDR *>(&serverAddr), sizeof(serverAddr));
	if (retval == SOCKET_ERROR) {
		return false;
	}

	return true;
}

void CNetwork::FirstRecv()
{
	int retval{ 0 };

	retval = m_pPlayerManager->Recv(m_Socket);
	if (retval == SOCKET_ERROR) {
		CNetworkManager::ErrDisplay("[recv() Error]");
	}
	else if (retval == 0) {
	}

	m_pPlayerManager->Update();
}

bool CNetwork::Update()
{
	int retval{ 0 };

	retval = m_pPlayerManager->Send(m_Socket);
	if (retval == SOCKET_ERROR) {
		return false;
	}

	retval = m_pPlayerManager->Recv(m_Socket);
	if (retval == SOCKET_ERROR) {
		return false;
	}
	else if (retval == 0) {
		return false;
	}

	m_pPlayerManager->Update();

	return true;
}