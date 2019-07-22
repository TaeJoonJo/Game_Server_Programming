#include <iostream>
#include <stdio.h>
#include <string.h> 

using namespace std;

// 헤더에서 선언되는 함수들이 c++함수가 아닌 c라고 정의 해준다. c와 c++는 함수 정의 방식이 달라서 제대로 동작하지 않는다.
extern "C" { 
	#include "include\lua.h"
	#include "include\lauxlib.h"
	#include "include\lualib.h" 
}

void error_lua(const char* msg, const char* err)
{
	
}

void error_lua(lua_State* L, const char* fmt, ...)
{
	va_list argp; 
	va_start(argp, fmt);
	vfprintf(stderr, fmt, argp);
	va_end(argp);
	lua_close(L);
	exit(EXIT_FAILURE); 
}

int addnum_c(lua_State* L)
{
	int a = (int)lua_tonumber(L, -2);
	int b = (int)lua_tonumber(L, -1);
	int result = a + b;
	lua_pop(L, 3);
	lua_pushnumber(L, result);

	return 1;
}

int main(void)
{
	char buff[256];
	int error;
	lua_State* L 
		= luaL_newstate();  // 루아를 연다. 
	luaL_openlibs(L);   // 루아 표준 라이브러리를 연다.

	int orc_hp, orc_mp;

	int err = luaL_loadfile(L, "orc.lua");
	if (err != 0)
		error_lua(L, "Error in main() : %s\n", lua_tostring(L, -1));
	lua_pcall(L, 0, 0, 0);
	if (err != 0)
		error_lua(L, "Error in main() : %s\n", lua_tostring(L, -1));

	// lua_getglobal 그 값을 읽어오는것이 아니라 가상메모리 스택의 맨 위에 올려놓는것
	lua_getglobal(L, "heal_event");
	lua_pushnumber(L, 30);
	lua_pcall(L, 1, 2, 0);		// 인자갯수, 리턴갯수
	orc_hp = (int)lua_tonumber(L, -2);
	orc_mp = (int)lua_tonumber(L, -1);
	lua_pop(L, 2);

	cout << "ORC hp : " << orc_hp << ", mp : " << orc_mp << "\n";

	lua_getglobal(L, "hp");
	lua_getglobal(L, "mp");
	orc_hp = (int)lua_tonumber(L, -2);		// 2번째 인자는 스택의 top에서 몇번째 것을 받아올것인가
	orc_mp = (int)lua_tonumber(L, -1);		// top에는 아무것도 안가르키고 있고 다음 빈 공간을 가르키고 있음

	cout << "ORC hp : " << orc_hp << ", mp : " << orc_mp << "\n";
	lua_pop(L, 2);

	lua_register(L, "c_addnum", addnum_c);
	lua_getglobal(L, "addnum_lua");
	lua_pushnumber(L, 100);
	lua_pushnumber(L, 200);
	lua_pcall(L, 2, 1, 0);
	int result = (int)lua_tonumber(L, -1);
	lua_pop(L, 1);

	cout << "Result : " << result << "\n";

	lua_close(L);

	//const char* pbuff = "print \"Hello World\"";
	//luaL_loadbuffer(L, pbuff, strlen(pbuff), "line");
	//lua_pcall(L, 0, 0, 0);
	//
	//pbuff = "a = 1";
	//luaL_loadbuffer(L, pbuff, strlen(pbuff), "line");
	//lua_pcall(L, 0, 0, 0);
	//
	//pbuff = "b = a + 100";
	//luaL_loadbuffer(L, pbuff, strlen(pbuff), "line");
	//lua_pcall(L, 0, 0, 0);
	//
	//pbuff = "print (b)";
	//luaL_loadbuffer(L, pbuff, strlen(pbuff), "line");
	//lua_pcall(L, 0, 0, 0);
	//
	//lua_close(L);

	//while (fgets(buff, sizeof(buff), stdin) != NULL)
	//{
	//	error = luaL_loadbuffer(L, buff, strlen(buff), "line") 
	//		|| lua_pcall(L, 0, 0, 0);
	//	if (error)
	//	{
	//		fprintf(stderr, "%s\n", lua_tostring(L, -1)); lua_pop(L, 1);      //스택으로부터오류메시지를뽑아낸다. 
	//	}
	//	lua_close(L);
	//	return 0;
	//}
}
