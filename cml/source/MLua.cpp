#include "stdafx.h"
#include "MLua.h"
#include "MZFileSystem.h"

MLua::MLua()
{
	m_pState = NULL;
	m_pErrorHandler = NULL;
}

MLua::~MLua()
{
	Destroy();
}

bool MLua::Create()
{
	m_pState = lua_open();

	luaL_openlibs(m_pState);
	lua_settop(m_pState, 0);

	lua_state::open(m_pState);

	return true;
}

void MLua::Destroy()
{
	if(m_pState)
	{
		lua_close(m_pState);
		m_pState = NULL;
	}
}

bool MLua::RunFile(const char *szFileName)
{
	if (luaL_loadfile(m_pState, szFileName) != 0)
	{
		char buf[512];
		sprintf_safe(buf, "Lua Error - Script Load\nScript Name:%s\nError Message:%s\n", szFileName, luaL_checkstring(m_pState, -1));

		if(m_pErrorHandler)
		{
			m_pErrorHandler(buf);
		}

		OutputDebugString(buf);

		return false;
	}
	if (lua_pcall(m_pState, 0, LUA_MULTRET, 0) != 0)
	{
		char buf[512];
		sprintf_safe(buf, "Lua Error - Script Run\nScript Name:%s\nError Message:%s\n", szFileName, luaL_checkstring(m_pState, -1));

		if(m_pErrorHandler)
		{
			m_pErrorHandler(buf);
		}
		OutputDebugString(buf);


		return false;
	}
	return true;
}

bool MLua::RunString(const char* szStream)
{
	if (luaL_loadbuffer(m_pState, szStream, strlen(szStream), NULL) != 0)
	{
		char buf[1024];
		sprintf_safe(buf, "Lua Error - String Load\nString - %s\nError Message:%s\n", szStream, luaL_checkstring(m_pState, -1));

		if(m_pErrorHandler)
		{
			m_pErrorHandler(buf);
		}
		OutputDebugString(buf);


		return false;
	}
	if (0 != lua_pcall(m_pState, 0, LUA_MULTRET, 0))
	{
		char buf[1024];
		sprintf_safe(buf, "Lua Error - String Run\nString - %s\nError Message:%s\n", szStream, luaL_checkstring(m_pState, -1));

		if(m_pErrorHandler)
		{
			m_pErrorHandler(buf);
		}
		OutputDebugString(buf);


		return false;
	}
	return true;
}

const char* MLua::GetErrorString(void)
{
	return luaL_checkstring(m_pState, -1);
}

bool MLua::AddFunction(const char *szFunctionName, LuaFunctionType pFunction)
{
	lua_register(m_pState, szFunctionName, pFunction);
	return true;
}

const char* MLua::GetStringArgument(int nNum, const char* pDefault)
{
	return luaL_optstring(m_pState, nNum, pDefault);
}

double MLua::GetNumberArgument(int nNum, double dDefault)
{
	return luaL_optnumber(m_pState, nNum, dDefault);
}

void MLua::PushString(const char* szString)
{
	lua_pushstring(m_pState, szString);
}

void MLua::PushNumber(double value)
{
	lua_pushnumber(m_pState, value);
}

bool MLua::RunCall(int nargs, int nresults)
{
	if (0 != lua_pcall(m_pState, nargs, nresults, 0))
	{
		char buf[1024];
		sprintf_safe(buf, "Lua Error - String Run\nString - %s\nError Message:%s\n", "", luaL_checkstring(m_pState, -1));

		if(m_pErrorHandler)
		{
			m_pErrorHandler(buf);
		}
		OutputDebugString(buf);


		return false;
	}
	return true;
}