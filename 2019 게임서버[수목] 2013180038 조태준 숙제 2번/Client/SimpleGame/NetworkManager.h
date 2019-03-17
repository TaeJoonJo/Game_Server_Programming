#pragma once

#include "stdafx.h"
#include "Network.h"
#include "PlayerManager.h"
#include "GameObject.h"

class CNetworkManager
{
public:
	inline CNetworkManager() {};
	inline ~CNetworkManager() {};
protected:
	CGameObject *m_pGameObject = NULL;
public:
	virtual int Send(SOCKET &socket) = 0;
	virtual int Recv(SOCKET &socket) = 0;
public:
	inline void SetGameObject(CGameObject *pgameObject) {
		m_pGameObject = pgameObject;
	}
public:
	virtual void Update() = 0;
	virtual void KeyInput(void *key) = 0;
public:
	// 임시용
	inline static int Recvn(SOCKET s, char *buf, int len, int flags) {
		int received;
		char *ptr = buf;
		int left = len;

		while (left > 0) {
			received = recv(s, ptr, left, flags);
			if (received == SOCKET_ERROR)
				return SOCKET_ERROR;
			else if (received == 0)
				break;
			left -= received;
			ptr += received;
		}

		return (len - left);
	}
public:
	// 에러출력용 
	inline static void ErrDisplay(const char *msg) {
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, WSAGetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf, 0, NULL);
		printf("[%s] %s\n", msg, reinterpret_cast<char *>(lpMsgBuf));
		LocalFree(lpMsgBuf);
	}
	inline static void ErrQuit(const char *msg) {
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, WSAGetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf, 0, NULL);
		MessageBox(NULL, reinterpret_cast<LPCWSTR>(lpMsgBuf), reinterpret_cast<LPCWSTR>(msg), MB_ICONERROR);
		LocalFree(lpMsgBuf);
		exit(1);
	}
};