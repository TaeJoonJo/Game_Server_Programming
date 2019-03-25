#pragma once

#define WINSIZEX 600.f
#define WINSIZEY 500.f

#define WINHALFSIZEX WINSIZEX * 0.5f
#define WINHALFSIZEY WINSIZEY * 0.5f

#define DEFAULTSIZE 50.f
#define BOARDSIZE 400.f

#define BOARD 0
#define CHESS 1

#define BLACK 0
#define WHITE 1

#define VECTORITERATOR std::vector<CGameObject*>::iterator

// Key Input
#define KEY_FIRST 0x0000
#define KEY_LEFT 0x0064
#define KEY_RIGHT 0x0066
#define KEY_UP 0x0065
#define KEY_DOWN 0x0067
#define KEY_IDLE 0x0001

#define NOT_REGISTED 0x1111