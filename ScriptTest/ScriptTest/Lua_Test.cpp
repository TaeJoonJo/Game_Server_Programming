#include <iostream>
#include <stdio.h>
#include <string.h> 

using namespace std;

// ������� ����Ǵ� �Լ����� c++�Լ��� �ƴ� c��� ���� ���ش�. c�� c++�� �Լ� ���� ����� �޶� ����� �������� �ʴ´�.
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
		= luaL_newstate();  // ��Ƹ� ����. 
	luaL_openlibs(L);   // ��� ǥ�� ���̺귯���� ����.

	int orc_hp, orc_mp;

	int err = luaL_loadfile(L, "orc.lua");
	if (err != 0)
		error_lua(L, "Error in main() : %s\n", lua_tostring(L, -1));
	lua_pcall(L, 0, 0, 0);
	if (err != 0)
		error_lua(L, "Error in main() : %s\n", lua_tostring(L, -1));

	// lua_getglobal �� ���� �о���°��� �ƴ϶� ����޸� ������ �� ���� �÷����°�
	lua_getglobal(L, "heal_event");
	lua_pushnumber(L, 30);
	lua_pcall(L, 1, 2, 0);		// ���ڰ���, ���ϰ���
	orc_hp = (int)lua_tonumber(L, -2);
	orc_mp = (int)lua_tonumber(L, -1);
	lua_pop(L, 2);

	cout << "ORC hp : " << orc_hp << ", mp : " << orc_mp << "\n";

	lua_getglobal(L, "hp");
	lua_getglobal(L, "mp");
	orc_hp = (int)lua_tonumber(L, -2);		// 2��° ���ڴ� ������ top���� ���° ���� �޾ƿð��ΰ�
	orc_mp = (int)lua_tonumber(L, -1);		// top���� �ƹ��͵� �Ȱ���Ű�� �ְ� ���� �� ������ ����Ű�� ����

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
	//		fprintf(stderr, "%s\n", lua_tostring(L, -1)); lua_pop(L, 1);      //�������κ��Ϳ����޽������̾Ƴ���. 
	//	}
	//	lua_close(L);
	//	return 0;
	//}
}
