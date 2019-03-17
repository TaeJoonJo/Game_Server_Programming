// Server.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Network.h"
#include "NetworkManager.h"

CNetwork *pNetwork;

int main()
{
	pNetwork = new CNetwork();
	if (!pNetwork->Initialize()) {
		printf("Initialize() Error!\n");
		exit(1);
	}

	pNetwork->Accept();
	
	while (1) {
		if (!pNetwork->Update())
			break;
	}

	// 클라이언트 소켓 닫기
	pNetwork->UserLogout();
	
	delete pNetwork;

	return 1;
}