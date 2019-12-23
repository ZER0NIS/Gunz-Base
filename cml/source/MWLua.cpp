#include "stdafx.h"
#include "MWLua.h"
#include "windows.h"
#include <string>

MWLua::LUA_MAP	MWLua::mapLua;

#define WLUA_IS_CPP(T) std::string(T) == "=[C]"

static int set_table_var(lua_State *L)
{
	const char* name = NULL;
	if (lua_isstring(L, -2))
	{
		name = lua_tostring(L, -2);
	}

	lua_pushfstring(L, "WLua Error: attempt to table member var `%s'", name);
	MWLua::GetWLua(L)->Error();

	return 0;
}

static void GetLastestStack(lua_State* L, lua_Debug& ar)
{
	if (lua_getstack(L, 0, &ar) == 1)
		lua_getinfo(L, "nSlu", &ar);
	else
		assert(0);

	if (ar.currentline == -1 &&
		ar.name == NULL)
	{
		if (lua_getstack(L, 1, &ar) == 1)
			lua_getinfo(L, "nSlu", &ar);
	}
}

static void DefaultWLuaLog(const char* text)
{
	std::cerr << text;

	//	OutputDebugString(text);
}

bool DisplayCallInfo(MWLua* L, lua_Debug& ar, bool& bIsCpp)
{
	bool bPrint = true;
	bIsCpp = WLUA_IS_CPP(ar.source);

	// C
	if (bIsCpp)
	{
		CPP_FUNC_INFO* info = L->AdvanceCppCallInfo();

		if (info &&
			info->nLine != -1)
		{
			sprintf_s(L->TempBuff(), WLUA_TEMP_BUF_LEN, "\t%*s: [%s:%3d]\n", 
				WLUA_MAX_NAME_BUF_LEN, info->strFuncName, info->strFileName, info->nLine);	
		}
		else
		{
			sprintf_s(L->TempBuff(), WLUA_TEMP_BUF_LEN, "[C] CallStackInfo Empty.\n");	
		}
	}
	// Lua
	else
	{
		if (std::string(ar.source) != std::string(""))
		{
			ar.source++; // '@' 제거

			if(ar.name)
			{
				sprintf_s(L->TempBuff(), WLUA_TEMP_BUF_LEN, "\t%*s: [%s:%3d]\n", 
					WLUA_MAX_NAME_BUF_LEN, ar.name, ar.source, ar.currentline);
			}
			else
			{
				CPP_FUNC_INFO* prev_info = L->PrevCppCallInfo();

				sprintf_s(L->TempBuff(), WLUA_TEMP_BUF_LEN, "\t%*s: [%s:%3d]\n", 
					WLUA_MAX_NAME_BUF_LEN, (prev_info)?prev_info->strCallFuncName:"UnknownPrev", ar.source, ar.currentline);
			}
		}
		else
		{
			sprintf_s(L->TempBuff(), WLUA_TEMP_BUF_LEN, "[Lua] CallStackInfo Empty.\n");	
		}
	}

	if (bPrint)
		L->Log(L->TempBuff());

	return bPrint;
}

void ShowCallStack(lua_State* L, int n)
{
	//#ifdef WLUA_USE_DEBUG_STACK
	int nCppStackIndexLocal = MWLua::GetWLua(L)->GetCppStackIndex();

	MWLua::GetWLua(L)->Log("Call Stack >>\n");

	lua_Debug ar;
	bool bIsCpp=false;
	bool bPrintAtLeastOne=false;

	for (int i=n; lua_getstack(L, i, &ar)==1; i++)
	{
		lua_getinfo(L, "nSlu", &ar);

		bool bPrint = 
			DisplayCallInfo(MWLua::GetWLua(L), ar, bIsCpp);

		if (bPrintAtLeastOne == false &&
			bPrint == true)
			bPrintAtLeastOne = true;
	}

	if (bIsCpp)
	{
		DisplayCallInfo(MWLua::GetWLua(L), ar, bIsCpp);
		bPrintAtLeastOne = true;
	}

	if (bPrintAtLeastOne == false)
	{
		MWLua::GetWLua(L)->Log("\t(Nothing)\n");
	}

	MWLua::GetWLua(L)->SetCppStackIndex(nCppStackIndexLocal);
	//#endif
}


static int _exception(lua_State *L)
{
	std::string strError = "\n";

	if (lua_isstring(L, -1))
		strError += lua_tostring(L, -1);
	else
		strError += "No Error Message.";

	strError += "\n";
	MWLua::GetWLua(L)->Log(strError.c_str());
	lua_remove(L, -1);

	ShowCallStack(L, 0);

	// Clean Up
	MWLua::GetWLua(L)->InitCppCallInfo();

	return 0;	
}
/*---------------------------------------------------------------------------*/ 
static void invoke_parent(lua_State *L)
{
	if (lua_gettop(L) <= 1)		return;

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
int get_var(lua_State *L)
{
	lua_getmetatable(L,1);
	lua_pushvalue(L,2);
	lua_rawget(L,-2);

	if(lua_isuserdata(L,-1))
	{
		MWLua::user2type<MWLua::var_base*>::invoke(L,-1)->get(L);
		lua_remove(L, -2);
	}
	else if(lua_isnil(L,-1))
	{
		lua_remove(L,-1);
		invoke_parent(L);
		if(lua_isnil(L,-1))
		{
			lua_pushfstring(L, "can't find '%s' class variable. (forgot registering class variable ?)\n", lua_tostring(L, 2));
			MWLua::GetWLua(L)->Error();
		}
	} 

	lua_remove(L,-2);

	return 1;
}

/*---------------------------------------------------------------------------*/ 
int set_var(lua_State *L)
{
	lua_getmetatable(L,1);
	lua_pushvalue(L,2);
	lua_rawget(L,-2);

	if(lua_isuserdata(L,-1))
	{
		MWLua::user2type<MWLua::var_base*>::invoke(L,-1)->set(L);
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

/*---------------------------------------------------------------------------*/ 
/* __s64                                                                     */ 
/*---------------------------------------------------------------------------*/ 
static int tostring_s64(lua_State *L)
{
	char temp[64];
	sprintf_s(temp, 64, "%I64d", *(__int64*)lua_topointer(L, 1));
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
void MWLua::init_s64(lua_State *L)
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
	sprintf_s(temp, 64, "%I64u", *(unsigned __int64*)lua_topointer(L, 1));
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
void MWLua::init_u64(lua_State *L)
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



MWLua::MWLua():
L(NULL),
pfnException(NULL),
pfnLog(NULL),
nCppStackIndex(0),
#ifdef WLUA_USE_DEBUG_TIMEOUT
TimeoutThread(NULL),
#endif
nCallTimeoutSec(0),
pfnPreCall(0),
pfnPostCall(0)
{ 
	CppStacks.resize(WLUA_MAX_CALLSTACK_DEPTH); 

#ifdef WLUA_USE_DEBUG_TIMEOUT
	InitializeCriticalSection(&cs);

	TimeoutThread = new MWLuaThread(this); 
#endif
}

MWLua::~MWLua() 
{ 
#ifdef WLUA_USE_DEBUG_TIMEOUT
	if (TimeoutThread) 
	{ 
		delete TimeoutThread; 
		TimeoutThread = NULL; 
	} 

	DeleteCriticalSection(&cs);
#endif
}

int 
MWLua::Open(int nCallTimeoutSec_) 
{

	L = lua_open();

	luaL_openlibs(L);

	init_s64(L);
	init_u64(L);

	// 유저 테이블용 메타테이블 생성
	luaL_newmetatable(L, "__user_table__");
	lua_pushstring(L, "__newindex");
	lua_pushcfunction(L, set_table_var);
	lua_rawset(L, -3);


	this->SetExceptionFunc(_exception);
	this->SetLogFunc(DefaultWLuaLog);
	SetCallTimeoutSec(nCallTimeoutSec_);

	lua_settop(L, 0);

	mapLua.insert(LUA_MAP::value_type(L, this));

	// WLua 라이브러리 불러오기
	Def<fnLog>("Log", pfnLog);

#ifdef WLUA_USE_DEBUG_TIMEOUT
	TimeoutThread->Create();
#endif

	return 0;
}

int 
MWLua::Close() 
{ 
	if (L == NULL)
		return 0;

	//Log("WLua(%x) ------ CallStack\n", L);
	//ShowCallStack(L, 0);

	//Log("WLua(%x) ------ EnumStack\n", L);
	//EnumStack();

#ifdef WLUA_USE_DEBUG_TIMEOUT
	TimeoutThread->Destroy();
#endif

	mapLua.erase(L);

	lua_close(L);

	L = NULL;

	return 0; 
}

void 
MWLua::Lock()			
{ 
#ifdef WLUA_USE_DEBUG_TIMEOUT
	EnterCriticalSection(&cs); 
#endif
}

void 
MWLua::Unlock()		
{ 
#ifdef WLUA_USE_DEBUG_TIMEOUT
	LeaveCriticalSection(&cs); 
#endif
}

void 
MWLua::SetExceptionFunc(lua_CFunction pfn)
{
	pfnException = pfn;
}

void 
MWLua::SetLogFunc(MWLua::fnLog pfn)
{
	pfnLog = pfn;
}

void 
MWLua::SetCallbackFunc( fnCallback pfnPre, fnCallback pfnPost )
{
	pfnPreCall = pfnPre;
	pfnPostCall = pfnPost;
}


// File
bool 
MWLua::RunFile(const char* name, WLUA_DEBUG_ARG_DECL)
{
	if (L == NULL) return false;

	RegisterCppDebugInfo(NULL, name, WLUA_DEBUG_ARG_DECL_NAME);

	PreCall();

	if (luaL_loadfile(L, name) != 0)
	{
		pfnException(L);
		//		RemoveCppDebugInfo();

		return false;
	}	

	bool bSuccess = 
		RawCall(0, 0) == 0;

	RemoveCppDebugInfo();

	return bSuccess;
}

// String
bool 
MWLua::RunString(std::string text, WLUA_DEBUG_ARG_DECL)
{
	return RunString(text.c_str(), text.size(), WLUA_DEBUG_ARG_DECL_NAME);
}

bool 
MWLua::RunString(const char* text, size_t len, WLUA_DEBUG_ARG_DECL)
{
	if (L == NULL) return false;

	RegisterCppDebugInfo(NULL, text, WLUA_DEBUG_ARG_DECL_NAME);

	PreCall();

	if (luaL_loadbuffer(L, text, len, pszFunc))
	{
		pfnException(L);

		return false;
	}

	bool bRet = RawCall(0, 0) == 0;

	RemoveCppDebugInfo();

	return bRet;
}

void 
MWLua::EnumStack(int start)
{
	Log("Enum Stack >>\nMaxLevel:%d\n", lua_gettop(L));
	for(int i=start+1; i<=lua_gettop(L); ++i)
	{
		switch(lua_type(L, i))
		{
		case LUA_TNIL:
			Log("\t%s\n", lua_typename(L, lua_type(L, i)));
			break;
		case LUA_TBOOLEAN:
			Log("\t%s	%s\n", lua_typename(L, lua_type(L, i)), lua_toboolean(L, i)?"true":"false");
			break;
		case LUA_TLIGHTUSERDATA:
			Log("\t%s	0x%08p\n", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
			break;
		case LUA_TNUMBER:
			Log("\t%s	%f\n", lua_typename(L, lua_type(L, i)), lua_tonumber(L, i));
			break;
		case LUA_TSTRING:
			Log("\t%s	%s\n", lua_typename(L, lua_type(L, i)), lua_tostring(L, i));
			break;
		case LUA_TTABLE:
			Log("\t%s	0x%08p\n", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
			break;
		case LUA_TFUNCTION:
			Log("\t%s()\n", lua_typename(L, lua_type(L, i)));
			break;
		case LUA_TUSERDATA:
			Log("\t%s	0x%08p\n", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
			break;
		case LUA_TTHREAD:
			Log("\t%s\n", lua_typename(L, lua_type(L, i)));
			break;
		}
	}
}

void 
MWLua::Dump()
{
	WCALL(this, "_dump", WNULL);
}


/*---------------------------------------------------------------------------*/ 
/* table object on stack                                                     */ 
/*---------------------------------------------------------------------------*/ 
MWLua::table_obj::table_obj(lua_State* L, int index)
:m_L(L)
,m_index(index)
,m_ref(0)
,m_pointer(NULL)
{
	m_pointer = lua_topointer(m_L, m_index);
}

MWLua::table_obj::~table_obj()
{
	if(validate())
	{
		lua_remove(m_L, m_index);
	}
}

void MWLua::table_obj::inc_ref()
{
	++m_ref;
}

void MWLua::table_obj::dec_ref()
{
	if(--m_ref == 0)
		delete this;
}

bool MWLua::table_obj::validate()
{
	if(m_pointer != NULL)
	{
		if(m_pointer == lua_topointer(m_L, m_index))
		{
			return true;
		}
		else
		{
			int top = lua_gettop(m_L);

			for(int i=1; i<=top; ++i)
			{
				if(m_pointer == lua_topointer(m_L, i))
				{
					m_index = i;
					return true;
				}
			}

			m_pointer = NULL;
			return false;
		}
	}
	else
	{
		return false;
	}
}

/*---------------------------------------------------------------------------*/ 
/* Table Object Holder                                                       */ 
/*---------------------------------------------------------------------------*/ 

MWLua::table::table()
: m_obj(NULL)
{
}

MWLua::table::table(lua_State* L)
: m_obj(NULL)
{
	if (L)
	{
		lua_newtable(L);
		m_obj = new table_obj(L, lua_gettop(L));

		m_obj->inc_ref();
	}
	
}

// 테이블 이름 지정은 지원하지 않음
//MWLua::table::table(lua_State* L, const char* name)
//{
//	lua_pushstring(L, name);
//	lua_gettable(L, LUA_GLOBALSINDEX);
//
//	if(lua_istable(L, -1) == 0)
//	{
//		lua_pop(L, 1);
//
//		lua_newtable(L);
//		lua_pushstring(L, name);
//		lua_pushvalue(L, -2);
//		lua_settable(L, LUA_GLOBALSINDEX);
//	}
//
//	m_obj = new table_obj(L, lua_gettop(L));
//
//	m_obj->inc_ref();
//}

MWLua::table::table(lua_State* L, int index)
: m_obj(NULL)
{
	if (L)
	{
		attach(L, index);
	}
}

MWLua::table::table(const table& input)
: m_obj(NULL)
{
	m_obj = input.m_obj;

	m_obj->inc_ref();
}

MWLua::table::~table()
{
	if (m_obj)
		m_obj->dec_ref();
}

void MWLua::table::attach(lua_State* L, int index)
{
	if(index < 0)
	{
		index = lua_gettop(L) + index + 1;
	}

	m_obj = new table_obj(L, index);

	m_obj->inc_ref();
}

bool MWLua::table::validate()
{
	if (!m_obj)		return false;
	return m_obj->validate();
}
/*---------------------------------------------------------------------------*/ 





#ifdef WLUA_USE_DEBUG_TIMEOUT

void MWLuaThread::Create()
{
	//	m_hThread = CreateThread(NULL, 0, ThreadProc, this, 0, &m_idThread); 
}

void MWLuaThread::Destroy()
{
	m_KillEvent.SetEvent();

	//if (m_hThread) {
	//	WaitForSingleObject(m_hThread, INFINITE);
	//	CloseHandle(m_hThread);
	//	m_hThread = NULL;
	//}
}

DWORD WINAPI MWLuaThread::ThreadProc(LPVOID pParam)
{
	MWLuaThread* pThread = (MWLuaThread*)pParam;
	pThread->Run();
	ExitThread(0);

	return 0;
}

void MWLuaThread::Run()
{
	const unsigned int LUA_THREAD_TICK = 1000;

	while(true)
	{
		DWORD nResult = WaitForSingleObject(m_KillEvent.GetEvent(), LUA_THREAD_TICK);
		if (nResult == WAIT_OBJECT_0) return;

		if (GetBeginTime() != 0)
		{
			if (GetBeginTime()+(m_pLua->GetCallTimeoutSec()*1000) < timeGetTime())
			{
				m_pLua->Lock(); // CppStacks
				CPP_FUNC_INFO f_info = m_pLua->CppStacks.front();
				m_pLua->Unlock(); // CppStacks

				if (m_pLua->GetCppStackIndex() != 0)
					ShowCallStack(m_pLua->GetState(), 0);

				this->BeginTimeWatch();
			}
		}
	}
}

#endif