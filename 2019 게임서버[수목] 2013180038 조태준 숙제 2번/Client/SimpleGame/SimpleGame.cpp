#include "stdafx.h"
#include <iostream>
#include "Dependencies\glew.h"
#include "Dependencies\freeglut.h"

#include "Renderer.h"
#include "SceneMgr.h"
#include "Network.h"

#include "GameObject.h"
#include "Rect.h"

CSceneMgr *g_Scene = NULL;
CNetwork *pNetwork = NULL;

DWORD g_prevTime = 0;

void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	DWORD currTime = timeGetTime();
	DWORD elapsedTime = currTime - g_prevTime;
	g_prevTime = currTime;

	float elapsedTime_per_time = elapsedTime * 0.001f;

	if (g_Scene)
	{
		g_Scene->Update_Objects((float)elapsedTime);
		g_Scene->Draw_Objects();
	}

	glutSwapBuffers();
}

void Idle(void)
{
	RenderScene();
}

void MouseInput(int button, int state, int x, int y)
{
	/*int RenderScenex = x - WINHALFSIZEX;
	int RenderSceney = -(y - WINHALFSIZEY);*/

	RenderScene();
}

void KeyInput(unsigned char key, int x, int y)
{

	RenderScene();
}

void SpecialKeyInput(int key, int x, int y)
{
	//g_Scene->GetPlayer()->KeyInput(key);
	pNetwork->GetPlayerManager()->KeyInput((void *)key);
	if (!pNetwork->Update())
		exit(1);

	RenderScene();
}

int main(int argc, char **argv)
{
	srand((unsigned)time(NULL));

	pNetwork = new CNetwork();
	if (!pNetwork->Initialize())
		printf("네트워크 초기화 오류\n");

	// Initialize GL things
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINSIZEX, WINSIZEY);
	glutCreateWindow("KPU GSE JoTaeJoon");

	glewInit();
	if (glewIsSupported("GL_VERSION_3_0"))
	{
		std::cout << " GLEW Version is 3.0\n ";
	}
	else
	{
		std::cout << "GLEW 3.0 not supported\n ";
	}
	
	g_Scene = new CSceneMgr();
	if (!g_Scene->Ready_Renderer(pNetwork))
		std::cout << "렌더러 오류" << std::endl;

	glutDisplayFunc(RenderScene);
	glutIdleFunc(Idle);
	glutKeyboardFunc(KeyInput);
	glutMouseFunc(MouseInput);
	glutSpecialFunc(SpecialKeyInput);

	glutMainLoop();

	delete g_Scene;
	delete pNetwork;

    return 0;
}