#pragma once
#include "stdafx.h"
#include "Network.h"

CNetwork::CNetwork()
	: m_ListenSocket(INVALID_SOCKET), m_ClientSocket(INVALID_SOCKET)
	, m_pPlayerManager(NULL)
{
	m_pPlayerManager = new CPlayerManager();
}

CNetwork::~CNetwork()
{
	delete m_pPlayerManager;
	closesocket(m_ListenSocket);

	WSACleanup();
}

bool CNetwork::Initialize()
{
	int retval{ 0 };

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		CNetworkManager::ErrDisplay("[WSAStratup() Error] ���� �ʱ�ȭ ����");
		return false;
	}

	// ���� ���� ����
	m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0);
	if (m_ListenSocket == INVALID_SOCKET) {
		CNetworkManager::ErrDisplay("[WSASocket() Error] ���ϻ��� ����");
		return false;
	}

	// ���� �ּ� ���ε�
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(SERVERPORT);
	retval = bind(m_ListenSocket, reinterpret_cast<SOCKADDR *>(&serverAddr), sizeof(serverAddr));
	if (retval == SOCKET_ERROR) {
		CNetworkManager::ErrDisplay("[bind() Error] ���ε� ����");
		return false;
	}

	// listen
	retval = listen(m_ListenSocket, SOMAXCONN);
	if (retval == SOCKET_ERROR) {
		CNetworkManager::ErrDisplay("[listen() Error] ���� ����");
		return false;
	}

	return true;
}

void CNetwork::Accept()
{
	SOCKADDR_IN clientAddr;
	int addrLen;

	// Accept
	addrLen = sizeof(clientAddr);
	m_ClientSocket = WSAAccept(m_ListenSocket, reinterpret_cast<SOCKADDR *>(&clientAddr), &addrLen, NULL, NULL);
	if (m_ClientSocket == INVALID_SOCKET) {
		CNetworkManager::ErrDisplay("[accept() Error]");
	}
	// ������ Ŭ���̾�Ʈ ���� ���
	printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
}

void CNetwork::UserLogout()
{
	closesocket(m_ClientSocket);

	printf("Ŭ���̾�Ʈ ���� ����\n");
}

bool CNetwork::Update()
{
	int retval{ 0 };

	retval = m_pPlayerManager->Send(m_ClientSocket);
	if (retval == SOCKET_ERROR) {
		return false;
	}

	retval = m_pPlayerManager->Recv(m_ClientSocket);
	if (retval == SOCKET_ERROR) {
		return false;
	}
	else if (retval == 0) {
		return false;
	}

	m_pPlayerManager->Update();

	return true;
}