#pragma once
#include <iostream>

// include Lua, assumes it is local to this file
extern "C"
{
#include "lua540/include/lua.h"
#include "lua540/include/lauxlib.h"
#include "lua540/include/lualib.h"
}

// Link to lua library
#ifdef _WIN32
#pragma comment(lib, "lua540/liblua54.a")
#endif

// Little error checking utility function
bool CheckLua(lua_State* L, int r)
{
	if (r != LUA_OK)
	{
		std::string errormsg = lua_tostring(L, -1);
		std::cout << errormsg << std::endl;
		return false;
	}
	return true;
}