// lua_tinker.cpp
//
// LuaTinker - Simple and light C++ wrapper for Lua.
//
// Copyright (c) 2005 Kwon-il Lee (zupet@hitel.net)
// 
// please check Licence.txt file for licence and legal issues. 

#include "stdafx.h"
#include "lua_tinker.h"


/*---------------------------------------------------------------------------*/ 
/*                                                                           */ 
/*---------------------------------------------------------------------------*/ 
static lua_State *g_L = NULL;

/*---------------------------------------------------------------------------*/ 
/*                                                                           */ 
/*---------------------------------------------------------------------------*/ 
static void show_stack(lua_State* L, int n)
{
    lua_Debug ar;
    if(lua_getstack(L, n, &ar) == 1)
	{
		show_stack(L, n+1);
		lua_getinfo(L, "nSlu", &ar);

		lua_pushstring(L, "_ALERT");
		lua_gettable(L, LUA_GLOBALSINDEX);
		if(lua_isfunction(L, -1))
		{
			if(ar.name)
				lua_pushfstring(L, "	%s() : line %d [%s : line %d]\n", ar.name, ar.currentline, ar.source, ar.linedefined);
			else
				lua_pushfstring(L, "	unknown : line %d [%s : line %d]\n", ar.currentline, ar.source, ar.linedefined);
		}
		else
		{
			lua_pop(L, 1);
		}
	}
}

/*---------------------------------------------------------------------------*/ 
int lua_tinker::_exception(lua_State *L)
{
	lua_pushstring(L, "_ALERT");
	lua_gettable(L, LUA_GLOBALSINDEX);
	if(lua_isfunction(L, -1))
	{
		lua_pushfstring(L, "%s\n", lua_tostring(L, -2));
		lua_call(L, 1, 0);
	}
	else
	{
		lua_pop(L, 1);
	}
	show_stack(L, 0);

	return 0;	
}

/*---------------------------------------------------------------------------*/ 
void lua_tinker::dofile(const char *filename)
{
	dofile(lua_state::L(), filename);
}

/*---------------------------------------------------------------------------*/ 
void lua_tinker::dostring(const char* buff)
{
	dostring(lua_state::L(), buff);
}

/*---------------------------------------------------------------------------*/ 
void lua_tinker::dobuffer(const char* buff, size_t sz)
{
	dobuffer(lua_state::L(), buff, sz);
}

/*---------------------------------------------------------------------------*/ 
void lua_tinker::dofile(lua_State *L, const char *filename)
{
	lua_settop(L, 0);
	lua_pushcclosure(L, lua_tinker::_exception, 0);

    if(luaL_loadfile(L, filename) != 0)
	{
		lua_pcall(L, 1, 0, 0);
	}
	else
	{
        lua_pcall(L, 0, 0, 1);
		lua_remove(L, 1);
	}
}

/*---------------------------------------------------------------------------*/ 
void lua_tinker::dostring(lua_State *L, const char* buff)
{
	lua_tinker::dobuffer(L, buff, strlen(buff));
}

/*---------------------------------------------------------------------------*/ 
void lua_tinker::dobuffer(lua_State *L, const char* buff, size_t sz)
{
	lua_settop(L, 0);
	lua_pushcclosure(L, lua_tinker::_exception, 0);

    if(luaL_loadbuffer(L, buff, sz, "lua_tinker::dostring()") != 0)
	{
		lua_pcall(L, 1, 0, 0);
	}
	else
	{
        lua_pcall(L, 0, 0, 1);
		lua_remove(L, 1);
	}
}

/*---------------------------------------------------------------------------*/ 
/* __s64                                                                     */ 
/*---------------------------------------------------------------------------*/ 
static int tostring_s64(lua_State *L)
{
	char temp[64];
	sprintf(temp, "%I64d", *(__int64*)lua_topointer(L, 1));
	lua_pushstring(L, temp);
	return 1;
}

/*---------------------------------------------------------------------------*/ 
static int eq_s64(lua_State *L)
{
	lua_pushboolean(L, memcmp(lua_topointer(L, 1), lua_topointer(L, 2), sizeof(__int64)) == 0);
	return 1;
}

/*---------------------------------------------------------------------------*/ 
static int lt_s64(lua_State *L)
{
	lua_pushboolean(L, memcmp(lua_topointer(L, 1), lua_topointer(L, 2), sizeof(__int64)) < 0);
	return 1;
}

/*---------------------------------------------------------------------------*/ 
static int le_s64(lua_State *L)
{
	lua_pushboolean(L, memcmp(lua_topointer(L, 1), lua_topointer(L, 2), sizeof(__int64)) <= 0);
	return 1;
}

/*---------------------------------------------------------------------------*/ 
lua_State* lua_tinker::lua_state::L(lua_State *in)
{ 
	if(in) 
		g_L=in; 

	return g_L; 
}
/*---------------------------------------------------------------------------*/ 
void lua_tinker::lua_state::init_s64(lua_State *L)
{
	const char* name = "__s64";
	lua_pushstring(L, name);
	lua_newtable(L);

	lua_pushstring(L, "__name");
	lua_pushstring(L, name);
	lua_rawset(L, -3);

	lua_pushstring(L, "__tostring");
	lua_pushcclosure(L, tostring_s64, 0);
	lua_rawset(L, -3);

	lua_pushstring(L, "__eq");
	lua_pushcclosure(L, eq_s64, 0);
	lua_rawset(L, -3);	

	lua_pushstring(L, "__lt");
	lua_pushcclosure(L, lt_s64, 0);
	lua_rawset(L, -3);	

	lua_pushstring(L, "__le");
	lua_pushcclosure(L, le_s64, 0);
	lua_rawset(L, -3);	

	lua_settable(L, LUA_GLOBALSINDEX);
}

/*---------------------------------------------------------------------------*/ 
/* __u64                                                                     */ 
/*---------------------------------------------------------------------------*/ 
static int tostring_u64(lua_State *L)
{
	char temp[64];
	sprintf(temp, "%I64u", *(unsigned __int64*)lua_topointer(L, 1));
	lua_pushstring(L, temp);
	return 1;
}

/*---------------------------------------------------------------------------*/ 
static int eq_u64(lua_State *L)
{
	lua_pushboolean(L, memcmp(lua_topointer(L, 1), lua_topointer(L, 2), sizeof(unsigned __int64)) == 0);
	return 1;
}

/*---------------------------------------------------------------------------*/ 
static int lt_u64(lua_State *L)
{
	lua_pushboolean(L, memcmp(lua_topointer(L, 1), lua_topointer(L, 2), sizeof(unsigned __int64)) < 0);
	return 1;
}

/*---------------------------------------------------------------------------*/ 
static int le_u64(lua_State *L)
{
	lua_pushboolean(L, memcmp(lua_topointer(L, 1), lua_topointer(L, 2), sizeof(unsigned __int64)) <= 0);
	return 1;
}

/*---------------------------------------------------------------------------*/ 
void lua_tinker::lua_state::init_u64(lua_State *L)
{
	const char* name = "__u64";
	lua_pushstring(L, name);
	lua_newtable(L);

	lua_pushstring(L, "__name");
	lua_pushstring(L, name);
	lua_rawset(L, -3);

	lua_pushstring(L, "__tostring");
	lua_pushcclosure(L, tostring_u64, 0);
	lua_rawset(L, -3);

	lua_pushstring(L, "__eq");
	lua_pushcclosure(L, eq_u64, 0);
	lua_rawset(L, -3);	

	lua_pushstring(L, "__lt");
	lua_pushcclosure(L, lt_u64, 0);
	lua_rawset(L, -3);	

	lua_pushstring(L, "__le");
	lua_pushcclosure(L, le_u64, 0);
	lua_rawset(L, -3);	

	lua_settable(L, LUA_GLOBALSINDEX);
}

/*---------------------------------------------------------------------------*/ 
/*                                                                           */ 
/*---------------------------------------------------------------------------*/ 
void lua_tinker::enum_stack(lua_State *L, int start)
{
	printf("Type:%d\n", lua_gettop(L));
	for(int i=start+1; i<=lua_gettop(L); ++i)
	{
		switch(lua_type(L, i))
		{
		case LUA_TNIL:
			printf("\t%s\n", lua_typename(L, lua_type(L, i)));
			break;
		case LUA_TBOOLEAN:
			printf("\t%s	%s\n", lua_typename(L, lua_type(L, i)), lua_toboolean(L, i)?"true":"false");
			break;
		case LUA_TLIGHTUSERDATA:
			printf("\t%s	0x%08p\n", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
			break;
		case LUA_TNUMBER:
			printf("\t%s	%f\n", lua_typename(L, lua_type(L, i)), lua_tonumber(L, i));
			break;
		case LUA_TSTRING:
			printf("\t%s	%s\n", lua_typename(L, lua_type(L, i)), lua_tostring(L, i));
			break;
		case LUA_TTABLE:
			printf("\t%s	0x%08p\n", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
			break;
		case LUA_TFUNCTION:
			printf("\t%s()\n", lua_typename(L, lua_type(L, i)));
			break;
		case LUA_TUSERDATA:
			printf("\t%s	0x%08p\n", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
			break;
		case LUA_TTHREAD:
			printf("\t%s\n", lua_typename(L, lua_type(L, i)));
			break;
		}
	}
}
 
/*---------------------------------------------------------------------------*/ 
/*                                                                           */ 
/*---------------------------------------------------------------------------*/ 
static void invoke_parent(lua_State *L)
{
	lua_pushstring(L, "__parent");
	lua_rawget(L, -2);
	if(lua_istable(L,-1))
	{
		lua_pushvalue(L,2);
		lua_rawget(L, -2);
		if(!lua_isnil(L,-1))
		{
			lua_remove(L,-2);
		}
		else
		{
			lua_remove(L, -1);
			invoke_parent(L);
			lua_remove(L,-2);
		}
	}
}

/*---------------------------------------------------------------------------*/ 
int lua_tinker::get_var(lua_State *L)
{
	lua_getmetatable(L,1);
	lua_pushvalue(L,2);
	lua_rawget(L,-2);

	if(lua_isuserdata(L,-1))
	{
		user2type<var_base*>::invoke(L,-1)->get(L);
		lua_remove(L, -2);
	}
	else if(lua_isnil(L,-1))
	{
		lua_remove(L,-1);
		invoke_parent(L);
		if(lua_isnil(L,-1))
		{
			lua_pushfstring(L, "can't find '%s' class variable. (forgot registering class variable ?)", lua_tostring(L, 2));
			lua_error(L);
		}
	} 

	lua_remove(L,-2);

	return 1;
}

/*---------------------------------------------------------------------------*/ 
int lua_tinker::set_var(lua_State *L)
{
	lua_getmetatable(L,1);
	lua_pushvalue(L,2);
	lua_rawget(L,-2);

	if(lua_isuserdata(L,-1))
	{
		user2type<var_base*>::invoke(L,-1)->set(L);
	}
	else if(lua_isnil(L, -1))
	{
		lua_pushvalue(L,2);
		lua_pushvalue(L,3);
		lua_rawset(L, -4);
	}
	lua_settop(L, 3);
	return 0;
}

/*---------------------------------------------------------------------------*/ 
