#ifndef __MAIET_WILD_LUA_H__
#define __MAIET_WILD_LUA_H__

#ifdef __cplusplus
extern "C" 
{
#endif
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
#ifdef __cplusplus
}
#endif

#include <string>
#include <map>
#include <deque>
#include <iostream>
#include <assert.h>
#include "MSync.h"
#include "RLib.h"

// new 매크로를 지웁니다.
#pragma push_macro ("new")
#ifdef new
#undef new
#endif

class MWLua;


// === Option Defines ===
#define WLUA_USE_DEBUG_STACK
#define WLUA_USE_DEBUG_TIMEOUT

// === Const Defines ===
#define WLUA_TEMP_BUF_LEN			512
#define WLUA_MAX_NAME_BUF_LEN		28
#define WLUA_MAX_FILE_NAME_BUF_LEN	50
#define WLUA_MAX_CALLSTACK_DEPTH	32
#define WLUA_DEFAULT_CALL_TIMEOUT	3

#define WLUA_DEBUG_ARG_DECL			const char* pszFile, long nLine, const char* pszFunc
#define WLUA_DEBUG_ARG_DECL_NAME	pszFile, nLine, pszFunc
#define WLUA_DEBUG_ARG				__FILE__, __LINE__, __FUNCTION__


// === Prototype Functions ===
CML2_API void ShowCallStack(lua_State* L, int n);
static bool DisplayCallInfo(MWLua* L, lua_Debug& ar, bool& bIsCpp);

CML2_API int get_var(lua_State *L);
CML2_API int set_var(lua_State *L);

struct CPP_FUNC_INFO
{	
	char strFileName[WLUA_MAX_FILE_NAME_BUF_LEN+1];
	char strTableName[WLUA_MAX_NAME_BUF_LEN+1];
	char strFuncName[WLUA_MAX_NAME_BUF_LEN+1];
	char strCallFuncName[WLUA_MAX_NAME_BUF_LEN+1];
	long nLine;
	bool bCppCall;

	CPP_FUNC_INFO():
	nLine(-1), bCppCall(false)
	{}
};

class MWLua;


#ifdef WLUA_USE_DEBUG_TIMEOUT

#define WIN32_LEAN_AND_MEAN		
#include <windows.h>
#include <MMSystem.h>
#pragma comment(lib,"winmm.lib")

class MWLuaThread
{
public:
	MWLuaThread(MWLua* pLua): m_bRunning(false), m_uBeginTime(0), m_pLua(pLua) {}

	void Create();
	void Destroy();
	static DWORD WINAPI ThreadProc(LPVOID lpParameter);

	void Run();

	inline void BeginTimeWatch();

	inline void EndTimeWatch();

	inline unsigned long GetBeginTime();

private:
	HANDLE			m_hThread;
	DWORD			m_idThread;

	bool			m_bRunning;
	MWLua*			m_pLua;
	MSignalEvent	m_KillEvent;

	// 쓰레드에 안전하지 않음
	unsigned long	m_uBeginTime;
};

#endif

class CML2_API MWLua
{
private:
	typedef std::map<lua_State*, MWLua*> LUA_MAP;
	typedef void (*fnLog) (const char* text);
	typedef void (*fnCallback) (void);
public:
	MWLua();
	virtual ~MWLua();
public:
	virtual int Open(int nCallTimeoutSec_ = WLUA_DEFAULT_CALL_TIMEOUT);
	virtual int Close();
	bool IsOpened() { return (L != NULL); }

	void SetExceptionFunc(lua_CFunction pfn);
	void SetLogFunc(MWLua::fnLog pfn);
	void SetCallbackFunc(MWLua::fnCallback pfnPre, MWLua::fnCallback pfnPost);

	void Dump();

	inline lua_State* GetState() 
	{ 
		Lock();
		lua_State* retL = L;
		Unlock();

		return retL;
	}

	static void			init_s64(lua_State *L);
	static void			init_u64(lua_State *L);

	struct lua_obj
	{
		lua_obj& operator,(const lua_obj& obj) { return *this; }
	};

	struct module
	{
		module(lua_State *L){}
		void operator[](const lua_obj& obj){}
	};

	struct lua_value
	{
		virtual void to_lua(lua_State *L) = 0;
	};

	struct table_obj;

	// Table Object Holder
	struct CML2_API table
	{
		table();
		table(lua_State* L);
		table(lua_State* L, int index);
		//table(lua_State* L, const char* name);
		table(const table& input);
		~table();

		void attach(lua_State* L, int index);
		bool validate();

		template<typename T>
		void set(const char* name, T object)
		{
			if (!m_obj)	return;
			m_obj->set(name, object);
		}

		template<typename T>
		T get(const char* name)
		{
			if (!m_obj)	return T();
			return m_obj->get<T>(name);
		}

		bool is_exist(const char* name)
		{
			if (!m_obj)	return false;
			return m_obj->is_exist(name);
		}

		void iteration()
		{
			// table is in the stack at index `t' 
			lua_pushnil(m_obj->m_L); // first key 
		}

		// 반환값은 계속 진행될지 여부
		template<typename RVal>
		bool next(RVal* poutVal)
		{
			if (!poutVal)							return false;

			if (!lua_next(m_obj->m_L, m_obj->m_index))
				return false;

			*poutVal = pop_<RVal>(m_obj->m_L, -1);
			lua_pop(m_obj->m_L, 1); // removes `value'; keeps `key' for next iteration 
			return true;
		}

		// 테이블 멤버 변수를 추가할 수 있게 설정
		void set_writable()
		{
			if (!m_obj)	return;
			m_obj->set_writable();
		}

		// 테이블 멤버 변수를 추가할 수 없게 설정
		void set_unwritable()
		{
			if (!m_obj)	return;
			m_obj->set_unwritable();
		}


		table_obj*		m_obj;
	};

	// Table Object on Stack
	struct CML2_API table_obj
	{
		table_obj(lua_State* L, int index);
		~table_obj();

		void inc_ref();
		void dec_ref();

		bool validate();

		template<typename T>
		void set(const char* name, T object)
		{
			if(validate())
			{
				lua_pushstring(m_L, name);
				push_(m_L, object);
				lua_settable(m_L, m_index);
			}
		}

		template<>
		void set(const char* name, table t)
		{
			if(validate())
			{
				lua_pushstring(m_L, name);
				push_(m_L, t);
				lua_settable(m_L, m_index);
				t.set_writable();
			}
		}

		template<typename T>
		T get(const char* name)
		{
			if(validate())
			{
				lua_pushstring(m_L, name);
				lua_gettable(m_L, m_index);

				if(lua_isnil(m_L,-1))
				{
					lua_pushfstring(m_L, "can't find '%s' table member variable.\n", name);
					MWLua::GetWLua(m_L)->Error();
				}
			}
			else
			{
				lua_pushnil(m_L);
			}

			return pop_<T>(m_L, -1);
		}

		bool is_exist(const char* name)
		{
			if(!validate())
				return false;

			lua_pushstring(m_L, name);
			lua_gettable(m_L, m_index);

			if(lua_isnil(m_L,-1))
				return false;

			return true;
		}

		// 테이블 멤버 변수를 추가할 수 있게 설정
		void set_writable()
		{
			lua_pushvalue(m_L, m_index);
			lua_pushvalue(m_L, m_index);
			lua_setmetatable(m_L, -2);
			lua_pop(m_L, 1);
		}

		// 테이블 멤버 변수를 추가할 수 없게 설정
		void set_unwritable()
		{
			lua_pushvalue(m_L, m_index);
			luaL_getmetatable(m_L, "__user_table__");
			lua_setmetatable(m_L, -2);
			lua_pop(m_L, 1);
		}

		template<>
		table get(const char* name);

		lua_State*		m_L;
		int				m_index;
		const void*		m_pointer;
		int				m_ref;
	};

	// type trait
	template<typename T> struct class_;

	template<bool C, typename A, typename B> struct if_ {};
	template<typename A, typename B>		struct if_<true, A, B> { typedef A type; };
	template<typename A, typename B>		struct if_<false, A, B> { typedef B type; };

	template<typename A>
	struct is_ptr { static const bool value = false; };
	template<typename A>
	struct is_ptr<A*> { static const bool value = true; };

	template<typename A>
	struct is_ref { static const bool value = false; };
	template<typename A>
	struct is_ref<A&> { static const bool value = true; };

	template<typename A>
	struct remove_const { typedef A type; };
	template<typename A>
	struct remove_const<const A> { typedef A type; };

	template<typename A>
	struct base_type { typedef A type; };
	template<typename A>
	struct base_type<A*> { typedef A type; };
	template<typename A>
	struct base_type<A&> { typedef A type; };

	template<typename A>
	struct class_type { typedef typename remove_const<typename base_type<A>::type>::type type; };

	/////////////////////////////////
	enum { no = 1, yes = 2 }; 
	typedef char (& no_type )[no]; 
	typedef char (& yes_type)[yes]; 

	struct int_conv_type { int_conv_type(int); }; 

	static no_type int_conv_tester (...); 
	static yes_type int_conv_tester (int_conv_type); 

	static no_type vfnd_ptr_tester (const volatile char *); 
	static no_type vfnd_ptr_tester (const volatile short *); 
	static no_type vfnd_ptr_tester (const volatile int *); 
	static no_type vfnd_ptr_tester (const volatile long *); 
	static no_type vfnd_ptr_tester (const volatile double *); 
	static no_type vfnd_ptr_tester (const volatile float *); 
	static no_type vfnd_ptr_tester (const volatile bool *); 
	static yes_type vfnd_ptr_tester (const volatile void *); 

	template <typename T> static T* add_ptr(T&); 

	template <bool C> struct bool_to_yesno { typedef no_type type; }; 
	template <> struct bool_to_yesno<true> { typedef yes_type type; }; 

	template <typename T> 
	struct is_enum 
	{ 
		static T arg; 
		static const bool value = ( (sizeof(int_conv_tester(arg)) == sizeof(yes_type)) && (sizeof(vfnd_ptr_tester(add_ptr(arg))) == sizeof(yes_type)) ); 
	}; 
	/////////////////////////////////

	// from lua
	template<typename T>
	struct void2val { static T invoke(void* input){ return *(T*)input; } };
	template<typename T>
	struct void2ptr { static T* invoke(void* input){ return (T*)input; } };
	template<typename T>
	struct void2ref { static T& invoke(void* input){ return *(T*)input; } };

	template<typename T>  
	struct void2type
	{
		static T invoke(void* ptr)
		{
			return	if_<is_ptr<T>::value,
				void2ptr<base_type<T>::type>,
				if_<is_ref<T>::value,
				void2ref<base_type<T>::type>,
				void2val<base_type<T>::type>
				>::type
			>::type::invoke(ptr);
		}
	};

	template<typename T>  
	struct user2type { static T invoke(lua_State *L, int index) { return void2type<T>::invoke(lua_touserdata(L, index)); } };

	template<typename T>
	struct lua2enum { static T invoke(lua_State *L, int index) { return (T)(int)lua_tonumber(L, index); } };

	template<typename T>
	struct lua2object
	{ 
		static T invoke(lua_State *L, int index) 
		{ 
			if(!lua_isuserdata(L,index))
			{
				MWLua::GetWLua(L)->EnumStack();
				lua_pushstring(L, "형변환을 시도했으나, Userdata가 아닙니다.\n함수 인자와 다른 형의 인자를 넣었거나, 멤버변수를 멤버 함수로 접근했을 수 있습니다.\n");
				MWLua::GetWLua(L)->Error();
				return T();
			}
			return void2type<T>::invoke(user2type<user*>::invoke(L,index)->m_p); 
		} 
	};

	template<typename T>
	static T lua2type(lua_State *L, int index)
	{
		return	if_<is_enum<T>::value
			,lua2enum<T>
			,lua2object<T> 
		>::type::invoke(L, index);
	}

	struct user
	{
		user(void* p) : m_p(p) {}
		virtual ~user() {}
		void* m_p;
	};

	template<typename T>
	struct val2user : user
	{
		val2user() : user(new T) {}

		template<typename T1>
		val2user(T1 t1) : user(new T(t1)) {}

		template<typename T1, typename T2>
		val2user(T1 t1, T2 t2) : user(new T(t1, t2)) {}

		template<typename T1, typename T2, typename T3>
		val2user(T1 t1, T2 t2, T3 t3) : user(new T(t1, t2, t3)) {}

		template<typename T1, typename T2, typename T3, typename T4>
		val2user(T1 t1, T2 t2, T3 t3, T4 t4) : user(new T(t1, t2, t3, t4)) {}

		template<typename T1, typename T2, typename T3, typename T4, typename T5>
		val2user(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) : user(new T(t1, t2, t3, t4, t5)) {}

		template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
		val2user(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) : user(new T(t1, t2, t3, t4, t5, t6)) {}

		template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
		val2user(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) : user(new T(t1, t2, t3, t4, t5, t6, t7)) {}

		template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
		val2user(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8) : user(new T(t1, t2, t3, t4, t5, t6, t7, t8)) {}
		~val2user() { delete ((T*)m_p); }
	};

	template<typename T>
	struct ptr2user : user
	{
		ptr2user(T* t) : user((void*)t) {}
	};

	template<typename T>
	struct ref2user : user
	{
		ref2user(T& t) : user(&t) {}
	};

	// to lua
	template<typename T>
	struct val2lua { static void invoke(lua_State *L, T& input){ new(lua_newuserdata(L, sizeof(val2user<T>))) val2user<T>(input); } };
	template<typename T>
	struct ptr2lua { static void invoke(lua_State *L, T* input){ if(input) new(lua_newuserdata(L, sizeof(ptr2user<T>))) ptr2user<T>(input); else lua_pushnil(L); } };
	template<typename T>
	struct ref2lua { static void invoke(lua_State *L, T& input){ new(lua_newuserdata(L, sizeof(ref2user<T>))) ref2user<T>(input); } };

	template<typename T>
	struct enum2lua { static void invoke(lua_State *L, T val) { lua_pushnumber(L, (int)val); } };

	template<typename T>
	struct object2lua 
	{ 
		static void invoke(lua_State *L, T val) 
		{ 
			if_<is_ptr<T>::value
				,ptr2lua<base_type<T>::type>
				,if_<is_ref<T>::value
				,ref2lua<base_type<T>::type>
				,val2lua<base_type<T>::type>
				>::type
			>::type::invoke(L, val);

			class_<class_type<T>::type>::push_meta(L);
			lua_setmetatable(L, -2);
		} 
	};

	template<typename T>
	static void type2lua(lua_State *L, T val)
	{
		if_<is_enum<T>::value
			,enum2lua<T>
			,object2lua<T>
		>::type::invoke(L, val);
	}

	// get value from cclosure
	template<typename T>  
	static T upvalue_(lua_State *L)
	{
		return user2type<T>::invoke(L, lua_upvalueindex(1));
	}

	struct ret_int2
	{
		int val1;
		int val2;

		ret_int2(int a1, int a2): val1(a1), val2(a2) {}
	};

	struct ret_double2
	{
		double val1;
		double val2;

		ret_double2(double a1, double a2): val1(a1), val2(a2) {}
	};

	struct ret_int4
	{
		int val1;
		int val2;
		int val3;
		int val4;

		ret_int4(int a1, int a2, int a3, int a4): val1(a1), val2(a2), val3(a3), val4(a4) {}
	};

	// pop a value from lua stack 
	template<typename T>  
	T static pop_(lua_State *L, int index)					{ return lua2type<T>(L, index);						}


	template<>	static char*			pop_(lua_State *L, int index);
	template<>	static const char*		pop_(lua_State *L, int index);
	template<>	static char				pop_(lua_State *L, int index);
	template<>	static unsigned char	pop_(lua_State *L, int index);
	template<>	static short			pop_(lua_State *L, int index);
	template<>	static unsigned short	pop_(lua_State *L, int index);
	template<>	static long				pop_(lua_State *L, int index);
	template<>	static unsigned long	pop_(lua_State *L, int index);
	template<>	static int				pop_(lua_State *L, int index);
	template<>	static unsigned int		pop_(lua_State *L, int index);
	template<>	static float			pop_(lua_State *L, int index);
	template<>	static double			pop_(lua_State *L, int index);
	template<>	static bool				pop_(lua_State *L, int index);
	template<>	static void				pop_(lua_State *L, int index);
	template<>	static __int64			pop_(lua_State *L, int index);
	template<>	static unsigned __int64	pop_(lua_State *L, int index);
	template<>	static void*			pop_(lua_State *L, int index)	{ return (void*)0; }
	template<>	static table			pop_(lua_State *L, int index);

	// push a value to lua stack 
	template<typename T>  
	void static push_(lua_State *L, T ret)					{ type2lua<T>(L, ret);							}

	template<>	static void push_(lua_State *L, char ret);
	template<>	static void push_(lua_State *L, unsigned char ret);
	template<>	static void push_(lua_State *L, short ret);
	template<>	static void push_(lua_State *L, unsigned short ret);
	template<>	static void push_(lua_State *L, long ret);
	template<>	static void push_(lua_State *L, unsigned long ret);
	template<>	static void push_(lua_State *L, int ret);
	template<>	static void push_(lua_State *L, unsigned int ret);
	template<>	static void push_(lua_State *L, float ret);
	template<>	static void push_(lua_State *L, double ret);
	template<>	static void push_(lua_State *L, char* ret);
	template<>	static void push_(lua_State *L, const char* ret);
	template<>	static void push_(lua_State *L, bool ret);
	template<>	static void push_(lua_State *L, lua_value* ret);
	template<>	static void push_(lua_State *L, __int64 ret);
	template<>	static void push_(lua_State *L, unsigned __int64 ret);
	template<>	static void push_(lua_State *L, const ret_int2 ret);
	template<>	static void push_(lua_State *L, const ret_double2 ret);
	template<>	static void push_(lua_State *L, const ret_int4 ret);
	template<>	static void push_(lua_State *L, table ret);

	template<typename T>
	struct ret_ { static const int value = 1; };
	template<>
	struct ret_<void> { static const int value = 0; };
	template<>	
	struct ret_<ret_int2> { static const int value = 2; };
	template<>	
	struct ret_<ret_double2> { static const int value = 2; };
	template<>	
	struct ret_<ret_int4> { static const int value = 4; };

	// functor
	template<typename T1=void, typename T2=void, typename T3=void, typename T4=void, typename T5=void, typename T6=void, typename T7=void, typename T8=void>
	struct functor
	{
		template<typename RVal>
		static int invoke(lua_State *L) { push_(L,upvalue_<RVal(*)(T1,T2,T3,T4,T5,T6,T7,T8)>(L)(pop_<T1>(L,1),pop_<T2>(L,2),pop_<T3>(L,3),pop_<T4>(L,4),pop_<T5>(L,5),pop_<T6>(L,6),pop_<T7>(L,7),pop_<T8>(L,8))); return ret_<RVal>::value; }
		template<>
		static int invoke<void>(lua_State *L) { upvalue_<void(*)(T1,T2,T3,T4,T5,T6,T7,T8)>(L)(pop_<T1>(L,1),pop_<T2>(L,2),pop_<T3>(L,3),pop_<T4>(L,4),pop_<T5>(L,5),pop_<T6>(L,6),pop_<T7>(L,7),pop_<T8>(L,8)); return 0; }
	};

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	struct functor<T1,T2,T3,T4,T5,T6,T7>
	{
		template<typename RVal>
		static int invoke(lua_State *L) { push_(L,upvalue_<RVal(*)(T1,T2,T3,T4,T5,T6,T7)>(L)(pop_<T1>(L,1),pop_<T2>(L,2),pop_<T3>(L,3),pop_<T4>(L,4),pop_<T5>(L,5),pop_<T6>(L,6),pop_<T7>(L,7))); return ret_<RVal>::value; }
		template<>
		static int invoke<void>(lua_State *L) { upvalue_<void(*)(T1,T2,T3,T4,T5,T6,T7)>(L)(pop_<T1>(L,1),pop_<T2>(L,2),pop_<T3>(L,3),pop_<T4>(L,4),pop_<T5>(L,5),pop_<T6>(L,6),pop_<T7>(L,7)); return 0; }
	};

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	struct functor<T1,T2,T3,T4,T5,T6>
	{
		template<typename RVal>
		static int invoke(lua_State *L) { push_(L,upvalue_<RVal(*)(T1,T2,T3,T4,T5,T6)>(L)(pop_<T1>(L,1),pop_<T2>(L,2),pop_<T3>(L,3),pop_<T4>(L,4),pop_<T5>(L,5),pop_<T6>(L,6))); return ret_<RVal>::value; }
		template<>
		static int invoke<void>(lua_State *L) { upvalue_<void(*)(T1,T2,T3,T4,T5,T6)>(L)(pop_<T1>(L,1),pop_<T2>(L,2),pop_<T3>(L,3),pop_<T4>(L,4),pop_<T5>(L,5),pop_<T6>(L,6)); return 0; }
	};

	template<typename T1, typename T2, typename T3, typename T4, typename T5>
	struct functor<T1,T2,T3,T4,T5>
	{
		template<typename RVal>
		static int invoke(lua_State *L) { push_(L,upvalue_<RVal(*)(T1,T2,T3,T4,T5)>(L)(pop_<T1>(L,1),pop_<T2>(L,2),pop_<T3>(L,3),pop_<T4>(L,4),pop_<T5>(L,5))); return ret_<RVal>::value; }
		template<>
		static int invoke<void>(lua_State *L) { upvalue_<void(*)(T1,T2,T3,T4,T5)>(L)(pop_<T1>(L,1),pop_<T2>(L,2),pop_<T3>(L,3),pop_<T4>(L,4),pop_<T5>(L,5)); return 0; }
	};

	template<typename T1, typename T2, typename T3, typename T4>
	struct functor<T1,T2,T3,T4> 
	{
		template<typename RVal>
		static int invoke(lua_State *L) { push_(L,upvalue_<RVal(*)(T1,T2,T3,T4)>(L)(pop_<T1>(L,1),pop_<T2>(L,2),pop_<T3>(L,3),pop_<T4>(L,4))); return ret_<RVal>::value; }
		template<>
		static int invoke<void>(lua_State *L) { upvalue_<void(*)(T1,T2,T3,T4)>(L)(pop_<T1>(L,1),pop_<T2>(L,2),pop_<T3>(L,3),pop_<T4>(L,4)); return 0; }
	};

	template<typename T1, typename T2, typename T3>
	struct functor<T1,T2,T3> 
	{
		template<typename RVal>
		static int invoke(lua_State *L) { push_(L,upvalue_<RVal(*)(T1,T2,T3)>(L)(pop_<T1>(L,1),pop_<T2>(L,2),pop_<T3>(L,3))); return ret_<RVal>::value; }
		template<>
		static int invoke<void>(lua_State *L) { upvalue_<void(*)(T1,T2,T3)>(L)(pop_<T1>(L,1),pop_<T2>(L,2),pop_<T3>(L,3)); return 0; }
	};

	template<typename T1, typename T2>
	struct functor<T1,T2> 
	{
		template<typename RVal>
		static int invoke(lua_State *L) { push_(L,upvalue_<RVal(*)(T1,T2)>(L)(pop_<T1>(L,1),pop_<T2>(L,2))); return ret_<RVal>::value; }
		template<>
		static int invoke<void>(lua_State *L) { upvalue_<void(*)(T1,T2)>(L)(pop_<T1>(L,1),pop_<T2>(L,2)); return 0; }
	};

	template<typename T1>
	struct functor<T1> 
	{
		template<typename RVal>
		static int invoke(lua_State *L) { push_(L,upvalue_<RVal(*)(T1)>(L)(pop_<T1>(L,1))); return ret_<RVal>::value; }
		template<>
		static int invoke<void>(lua_State *L) { upvalue_<void(*)(T1)>(L)(pop_<T1>(L,1)); return 0; }
	};

	template<>
	struct functor<>
	{
		template<typename RVal>
		static int invoke(lua_State *L) { push_(L,upvalue_<RVal(*)()>(L)()); return ret_<RVal>::value; }
		template<>
		static int invoke<void>(lua_State *L) { upvalue_<void(*)()>(L)(); return 0; }
	};

	// push_functor
	template<typename RVal> 
	static void push_functor(lua_State *L, RVal (*func)())
	{
		lua_pushcclosure(L, functor<>::invoke<RVal>, 1);
	}

	template<typename RVal, typename T1> 
	static void push_functor(lua_State *L, RVal (*func)(T1))
	{ 
		lua_pushcclosure(L, functor<T1>::invoke<RVal>, 1);
	}

	template<typename RVal, typename T1, typename T2> 
	static void push_functor(lua_State *L, RVal (*func)(T1,T2))
	{ 
		lua_pushcclosure(L, functor<T1,T2>::invoke<RVal>, 1);
	}

	template<typename RVal, typename T1, typename T2, typename T3> 
	static void push_functor(lua_State *L, RVal (*func)(T1,T2,T3))
	{ 
		lua_pushcclosure(L, functor<T1,T2,T3>::invoke<RVal>, 1);
	}

	template<typename RVal, typename T1, typename T2, typename T3, typename T4> 
	static void push_functor(lua_State *L, RVal (*func)(T1,T2,T3,T4))
	{ 
		lua_pushcclosure(L, functor<T1,T2,T3,T4>::invoke<RVal>, 1);
	}

	template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5> 
	static void push_functor(lua_State *L, RVal (*func)(T1,T2,T3,T4,T5))
	{ 
		lua_pushcclosure(L, functor<T1,T2,T3,T4,T5>::invoke<RVal>, 1);
	}

	template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> 
	static void push_functor(lua_State *L, RVal (*func)(T1,T2,T3,T4,T5,T6))
	{ 
		lua_pushcclosure(L, functor<T1,T2,T3,T4,T5,T6>::invoke<RVal>, 1);
	}

	template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> 
	static void push_functor(lua_State *L, RVal (*func)(T1,T2,T3,T4,T5,T6,T7))
	{ 
		lua_pushcclosure(L, functor<T1,T2,T3,T4,T5,T6,T7>::invoke<RVal>, 1);
	}

	template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> 
	static void push_functor(lua_State *L, RVal (*func)(T1,T2,T3,T4,T5,T6,T7,T8))
	{ 
		lua_pushcclosure(L, functor<T1,T2,T3,T4,T5,T6,T7,T8>::invoke<RVal>, 1);
	}

	template<typename RVal> 
	static void push_func(lua_State *L, RVal (*func)())
	{
		lua_pushcclosure(L, functor<>::invoke<RVal>, 1);
	}

	template<typename RVal,class T1> 
	static void push_func(lua_State *L, RVal (*func)(T1))
	{ 
		lua_pushcclosure(L, functor<T1>::invoke<RVal>, 1);
	}

	template<typename RVal,class T1,class T2> 
	static void push_func(lua_State *L, RVal (*func)(T1,T2))
	{ 
		lua_pushcclosure(L, functor<T1,T2>::invoke<RVal>, 1);
	}

	template<typename RVal,class T1,class T2, typename T3> 
	static void push_func(lua_State *L, RVal (*func)(T1,T2,T3))
	{ 
		lua_pushcclosure(L, functor<T1,T2,T3>::invoke<RVal>, 1);
	}

	template<typename RVal,class T1,class T2, typename T3, typename T4> 
	static void push_func(lua_State *L, RVal (*func)(T1,T2,T3,T4))
	{ 
		lua_pushcclosure(L, functor<T1,T2,T3,T4>::invoke<RVal>, 1);
	}

	template<typename RVal,class T1,class T2, typename T3, typename T4, typename T5> 
	static void push_func(lua_State *L, RVal (*func)(T1,T2,T3,T4,T5))
	{ 
		lua_pushcclosure(L, functor<T1,T2,T3,T4,T5>::invoke<RVal>, 1);
	}

	template<typename RVal,class T1,class T2, typename T3, typename T4, typename T5, typename T6> 
	static void push_func(lua_State *L, RVal (*func)(T1,T2,T3,T4,T5,T6))
	{ 
		lua_pushcclosure(L, functor<T1,T2,T3,T4,T5,T6>::invoke<RVal>, 1);
	}

	template<typename RVal,class T1,class T2, typename T3, typename T4, typename T5, typename T6, typename T7> 
	static void push_func(lua_State *L, RVal (*func)(T1,T2,T3,T4,T5,T6,T7))
	{ 
		lua_pushcclosure(L, functor<T1,T2,T3,T4,T5,T6,T7>::invoke<RVal>, 1);
	}

	template<typename RVal,class T1,class T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> 
	static void push_func(lua_State *L, RVal (*func)(T1,T2,T3,T4,T5,T6,T7,T8))
	{ 
		lua_pushcclosure(L, functor<T1,T2,T3,T4,T5,T6,T7,T8>::invoke<RVal>, 1);
	}

	template<typename RVal, typename T>
	static void push_func(lua_State *L, RVal (T::*func)()) 
	{ 
		lua_pushcclosure(L, mem_functor<T>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T>
	static void push_func(lua_State *L, RVal (T::*func)() const) 
	{ 
		lua_pushcclosure(L, mem_functor<T>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1>
	static void push_func(lua_State *L, RVal (T::*func)(T1)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1>
	static void push_func(lua_State *L, RVal (T::*func)(T1) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2>
	static void push_func(lua_State *L, RVal (T::*func)(T1,T2)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2>
	static void push_func(lua_State *L, RVal (T::*func)(T1,T2) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3>
	static void push_func(lua_State *L, RVal (T::*func)(T1,T2,T3)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3>
	static void push_func(lua_State *L, RVal (T::*func)(T1,T2,T3) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4>
	static void push_func(lua_State *L, RVal (T::*func)(T1,T2,T3,T4)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4>
	static void push_func(lua_State *L, RVal (T::*func)(T1,T2,T3,T4) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
	static void push_func(lua_State *L, RVal (T::*func)(T1,T2,T3,T4,T5)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4,T5>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
	static void push_func(lua_State *L, RVal (T::*func)(T1,T2,T3,T4,T5) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4,T5>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	static void push_func(lua_State *L, RVal (T::*func)(T1,T2,T3,T4,T5,T6)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4,T5,T6>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	static void push_func(lua_State *L, RVal (T::*func)(T1,T2,T3,T4,T5,T6) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4,T5,T6>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	static void push_func(lua_State *L, RVal (T::*func)(T1,T2,T3,T4,T5,T6,T7)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4,T5,T6,T7>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	static void push_func(lua_State *L, RVal (T::*func)(T1,T2,T3,T4,T5,T6,T7) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4,T5,T6,T7>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	static void push_func(lua_State *L, RVal (T::*func)(T1,T2,T3,T4,T5,T6,T7,T8)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4,T5,T6,T7,T8>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	static void push_func(lua_State *L, RVal (T::*func)(T1,T2,T3,T4,T5,T6,T7,T8) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4,T5,T6,T7,T8>::invoke<RVal>, 1); 
	}

	// member variable
	struct var_base
	{
		virtual void get(lua_State *L) = 0;
		virtual void set(lua_State *L) = 0;
	};

	template<typename T, typename V>
	struct mem_var : var_base
	{
		V T::*_var;
		mem_var(V T::*val) : _var(val) {}
		void get(lua_State *L)	
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return;

			push_(L, pObj->*(_var));		
		}
		void set(lua_State *L)	
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return;

			pObj->*(_var) = pop_<V>(L, 3);	
		}
	};

	// member function
	template<typename T, typename T1=void, typename T2=void, typename T3=void, typename T4=void, typename T5=void, typename T6=void, typename T7=void, typename T8=void>
	struct mem_functor
	{
		template<typename RVal>
		static int invoke(lua_State *L) 
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return 0;

			push_(L,(pObj->*upvalue_<RVal(T::*)(T1,T2,T3,T4,T5,T6,T7,T8)>(L))(pop_<T1>(L,2),pop_<T2>(L,3),pop_<T3>(L,4),pop_<T4>(L,5),pop_<T5>(L,6),pop_<T6>(L,7),pop_<T7>(L,8),pop_<T8>(L,9)));;
			return MWLua::ret_<RVal>::value; 
		}
		template<>
		static int invoke<void>(lua_State *L)  
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return 0;

			(pObj->*upvalue_<void(T::*)(T1,T2,T3,T4,T5,T6,T7,T8)>(L))(pop_<T1>(L,2),pop_<T2>(L,3),pop_<T3>(L,4),pop_<T4>(L,5),pop_<T5>(L,6),pop_<T6>(L,7),pop_<T7>(L,8),pop_<T8>(L,9)); 
			return 0; 
		}
	};

	template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	struct mem_functor<T,T1,T2,T3,T4,T5,T6,T7>
	{
		template<typename RVal>
		static int invoke(lua_State *L) 
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return 0;

			push_(L,(pObj->*upvalue_<RVal(T::*)(T1,T2,T3,T4,T5,T6,T7)>(L))(pop_<T1>(L,2),pop_<T2>(L,3),pop_<T3>(L,4),pop_<T4>(L,5),pop_<T5>(L,6),pop_<T6>(L,7),pop_<T7>(L,8)));;
			return MWLua::ret_<RVal>::value; 
		}
		template<>
		static int invoke<void>(lua_State *L)  
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return 0;

			(pObj->*upvalue_<void(T::*)(T1,T2,T3,T4,T5,T6,T7)>(L))(pop_<T1>(L,2),pop_<T2>(L,3),pop_<T3>(L,4),pop_<T4>(L,5),pop_<T5>(L,6),pop_<T6>(L,7),pop_<T7>(L,8)); 
			return 0; 
		}
	};

	template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	struct mem_functor<T,T1,T2,T3,T4,T5,T6>
	{
		template<typename RVal>
		static int invoke(lua_State *L) 
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return 0;

			push_(L,(pObj->*upvalue_<RVal(T::*)(T1,T2,T3,T4,T5,T6)>(L))(pop_<T1>(L,2),pop_<T2>(L,3),pop_<T3>(L,4),pop_<T4>(L,5),pop_<T5>(L,6),pop_<T6>(L,7)));;
			return MWLua::ret_<RVal>::value; 
		}
		template<>
		static int invoke<void>(lua_State *L)  
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return 0;

			(pObj->*upvalue_<void(T::*)(T1,T2,T3,T4,T5,T6)>(L))(pop_<T1>(L,2),pop_<T2>(L,3),pop_<T3>(L,4),pop_<T4>(L,5),pop_<T5>(L,6),pop_<T6>(L,7)); 
			return 0; 
		}
	};

	template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
	struct mem_functor<T,T1,T2,T3,T4,T5>
	{
		template<typename RVal>
		static int invoke(lua_State *L) 
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return 0;

			push_(L,(pObj->*upvalue_<RVal(T::*)(T1,T2,T3,T4,T5)>(L))(pop_<T1>(L,2),pop_<T2>(L,3),pop_<T3>(L,4),pop_<T4>(L,5),pop_<T5>(L,6)));;
			return MWLua::ret_<RVal>::value; 
		}
		template<>
		static int invoke<void>(lua_State *L)  
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return 0;

			(pObj->*upvalue_<void(T::*)(T1,T2,T3,T4,T5)>(L))(pop_<T1>(L,2),pop_<T2>(L,3),pop_<T3>(L,4),pop_<T4>(L,5),pop_<T5>(L,6)); 
			return 0; 
		}
	};

	template<typename T, typename T1, typename T2, typename T3, typename T4> 
	struct mem_functor<T,T1,T2,T3,T4>
	{
		template<typename RVal>
		static int invoke(lua_State *L) 
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return 0;

			push_(L,(pObj->*upvalue_<RVal(T::*)(T1,T2,T3,T4)>(L))(pop_<T1>(L,2),pop_<T2>(L,3),pop_<T3>(L,4),pop_<T4>(L,5))); 
			return MWLua::ret_<RVal>::value; 
		}
		template<>
		static int invoke<void>(lua_State *L)  
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return 0;

			(pObj->*upvalue_<void(T::*)(T1,T2,T3,T4)>(L))(pop_<T1>(L,2),pop_<T2>(L,3),pop_<T3>(L,4),pop_<T4>(L,5)); 
			return 0; 
		}
	};

	template<typename T, typename T1, typename T2, typename T3> 
	struct mem_functor<T,T1,T2,T3>
	{
		template<typename RVal>
		static int invoke(lua_State *L) 
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return 0;

			push_(L,(pObj->*upvalue_<RVal(T::*)(T1,T2,T3)>(L))(pop_<T1>(L,2),pop_<T2>(L,3),pop_<T3>(L,4))); 
			return MWLua::ret_<RVal>::value; 
		}
		template<>
		static int invoke<void>(lua_State *L)  
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return 0;

			(pObj->*upvalue_<void(T::*)(T1,T2,T3)>(L))(pop_<T1>(L,2),pop_<T2>(L,3),pop_<T3>(L,4)); 
			return 0; 
		}
	};

	template<typename T, typename T1, typename T2> 
	struct mem_functor<T,T1, T2>
	{
		template<typename RVal>
		static int invoke(lua_State *L) 
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return 0;

			push_(L,(pObj->*upvalue_<RVal(T::*)(T1,T2)>(L))(pop_<T1>(L,2),pop_<T2>(L,3))); 
			return MWLua::ret_<RVal>::value; 
		}
		template<>
		static int invoke<void>(lua_State *L)  
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return 0;

			(pObj->*upvalue_<void(T::*)(T1,T2)>(L))(pop_<T1>(L,2),pop_<T2>(L,3)); 
			return 0; 
		}
	};

	template<typename T, typename T1> 
	struct mem_functor<T,T1>
	{
		template<typename RVal>
		static int invoke(lua_State *L) 
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return 0;

			push_(L,(pObj->*upvalue_<RVal(T::*)(T1)>(L))(pop_<T1>(L,2))); 
			return MWLua::ret_<RVal>::value; 
		}
		template<>
		static int invoke<void>(lua_State *L)  
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return 0;

			(pObj->*upvalue_<void(T::*)(T1)>(L))(pop_<T1>(L,2)); 
			return 0; 
		}
	};

	template<typename T> 
	struct mem_functor<T>
	{
		template<typename RVal>
		static int invoke(lua_State *L) 
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return 0;

			push_(L,(pObj->*upvalue_<RVal(T::*)()>(L))()); 
			return MWLua::ret_<RVal>::value; 
		}
		template<>
		static int invoke<void>(lua_State *L)  
		{ 
			T* pObj = pop_<T*>(L,1);
			if (!pObj)	return 0;

			(pObj->*upvalue_<void(T::*)()>(L))(); 
			return 0; 
		}
	};

	// push_functor
	template<typename RVal, typename T>
	static void push_functor(lua_State *L, RVal (T::*func)()) 
	{ 
		lua_pushcclosure(L, mem_functor<T>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T>
	static void push_functor(lua_State *L, RVal (T::*func)() const) 
	{ 
		lua_pushcclosure(L, mem_functor<T>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1>
	static void push_functor(lua_State *L, RVal (T::*func)(T1)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1>
	static void push_functor(lua_State *L, RVal (T::*func)(T1) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2>
	static void push_functor(lua_State *L, RVal (T::*func)(T1,T2)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2>
	static void push_functor(lua_State *L, RVal (T::*func)(T1,T2) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3>
	static void push_functor(lua_State *L, RVal (T::*func)(T1,T2,T3)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3>
	static void push_functor(lua_State *L, RVal (T::*func)(T1,T2,T3) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4>
	static void push_functor(lua_State *L, RVal (T::*func)(T1,T2,T3,T4)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4>
	static void push_functor(lua_State *L, RVal (T::*func)(T1,T2,T3,T4) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
	static void push_functor(lua_State *L, RVal (T::*func)(T1,T2,T3,T4,T5)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4,T5>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
	static void push_functor(lua_State *L, RVal (T::*func)(T1,T2,T3,T4,T5) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4,T5>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	static void push_functor(lua_State *L, RVal (T::*func)(T1,T2,T3,T4,T5,T6)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4,T5,T6>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	static void push_functor(lua_State *L, RVal (T::*func)(T1,T2,T3,T4,T5,T6) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4,T5,T6>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	static void push_functor(lua_State *L, RVal (T::*func)(T1,T2,T3,T4,T5,T6,T7)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4,T5,T6,T7>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	static void push_functor(lua_State *L, RVal (T::*func)(T1,T2,T3,T4,T5,T6,T7) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4,T5,T6,T7>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	static void push_functor(lua_State *L, RVal (T::*func)(T1,T2,T3,T4,T5,T6,T7,T8)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4,T5,T6,T7,T8>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	static void push_functor(lua_State *L, RVal (T::*func)(T1,T2,T3,T4,T5,T6,T7,T8) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4,T5,T6,T7,T8>::invoke<RVal>, 1); 
	}

	// constructor
	template<typename T1=void, typename T2=void, typename T3=void, typename T4=void, typename T5=void, typename T6=void, typename T7=void, typename T8=void, typename T9=void>
	struct constructor {};

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	struct constructor<T1,T2,T3,T4,T5,T6,T7,T8>
	{
		template<typename T>
		static void invoke(lua_State *L)
		{
			new(lua_newuserdata(L, sizeof(val2user<T>))) val2user<T>(pop_<T1>(L,2),pop_<T2>(L,3),pop_<T3>(L,4),pop_<T4>(L,5),pop_<T5>(L,6),pop_<T6>(L,7),pop_<T7>(L,8),pop_<T8>(L,9));
		}
	};

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	struct constructor<T1,T2,T3,T4,T5,T6,T7>
	{
		template<typename T>
		static void invoke(lua_State *L)
		{
			new(lua_newuserdata(L, sizeof(val2user<T>))) val2user<T>(pop_<T1>(L,2),pop_<T2>(L,3),pop_<T3>(L,4),pop_<T4>(L,5),pop_<T5>(L,6),pop_<T6>(L,7),pop_<T7>(L,8));
		}
	};

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	struct constructor<T1,T2,T3,T4,T5,T6>
	{
		template<typename T>
		static void invoke(lua_State *L)
		{
			new(lua_newuserdata(L, sizeof(val2user<T>))) val2user<T>(pop_<T1>(L,2),pop_<T2>(L,3),pop_<T3>(L,4),pop_<T4>(L,5),pop_<T5>(L,6),pop_<T6>(L,7));
		}
	};

	template<typename T1, typename T2, typename T3, typename T4, typename T5>
	struct constructor<T1,T2,T3,T4,T5>
	{
		template<typename T>
		static void invoke(lua_State *L)
		{
			new(lua_newuserdata(L, sizeof(val2user<T>))) val2user<T>(pop_<T1>(L,2),pop_<T2>(L,3),pop_<T3>(L,4),pop_<T4>(L,5),pop_<T5>(L,6));
		}
	};

	template<typename T1, typename T2, typename T3, typename T4>
	struct constructor<T1,T2,T3,T4>
	{
		template<typename T>
		static void invoke(lua_State *L)
		{
			new(lua_newuserdata(L, sizeof(val2user<T>))) val2user<T>(pop_<T1>(L,2),pop_<T2>(L,3),pop_<T3>(L,4),pop_<T4>(L,5));
		}
	};


	template<typename T1, typename T2, typename T3>
	struct constructor<T1,T2,T3>
	{
		template<typename T>
		static void invoke(lua_State *L)
		{
			new(lua_newuserdata(L, sizeof(val2user<T>))) val2user<T>(pop_<T1>(L,2),pop_<T2>(L,3),pop_<T3>(L,4));
		}
	};


	template<typename T1, typename T2>
	struct constructor<T1,T2>
	{
		template<typename T>
		static void invoke(lua_State *L)
		{
			new(lua_newuserdata(L, sizeof(val2user<T>))) val2user<T>(pop_<T1>(L,2),pop_<T2>(L,3));
		}
	};

	template<typename T1>
	struct constructor<T1>
	{
		template<typename T>
		static void invoke(lua_State *L)
		{
			new(lua_newuserdata(L, sizeof(val2user<T>))) val2user<T>(pop_<T1>(L,2));
		}
	};

	template<>
	struct constructor<void>
	{ 
		template<typename T>
		static void invoke(lua_State *L) 
		{ 
			new(lua_newuserdata(L, sizeof(val2user<T>))) val2user<T>();
		} 
	};

	template<typename T>
	struct creator
	{
		template<typename CONSTRUCTOR>
		static int invoke(lua_State *L) 
		{ 
			CONSTRUCTOR::invoke<T>(L);
			class_<class_type<T>::type>::push_meta(L);
			lua_setmetatable(L, -2);

			return 1; 
		}
	};

	// destroyer
	template<typename T>
	static int destroyer(lua_State *L) 
	{ 
		((user*)lua_touserdata(L, 1))->~user();
		return 0;
	}

	// Global Function
	template<typename F> 
	void Def(const char* name, F func)
	{ 
		lua_pushstring(L, name);
		lua_pushlightuserdata(L, func);
		push_functor(L, func);
		lua_settable(L, LUA_GLOBALSINDEX);
	}

	// Global Variable
	template<typename T>
	void SetVar(const char* name, T object)
	{
		if (IsExistGlobal(name))
		{
			lua_pushfstring(L, "WLua Error: attempt to register same symbol global `%s'", name);
			pfnException(L);			
			return;
		}
		lua_settop(L, 0);
		lua_pushstring(L, name);
		push_<T>(L, object);
		lua_settable(L, LUA_GLOBALSINDEX);
	}

	void SetNull(const char* name)
	{
		lua_settop(L, 0);
		lua_pushstring(L, name);
		lua_pushnil(L);
		lua_settable(L, LUA_GLOBALSINDEX);
	}

	template<typename T>
	void SetVarRef(const char* name, T& object)
	{
		if (IsExistGlobal(name))
		{
			lua_pushfstring(L, "WLua Error: attempt to register same symbol global `%s'", name);
			pfnException(L);			
			return;
		}
		lua_settop(L, 0);
		lua_pushstring(L, name);
		push_(L, object);
		lua_settable(L, LUA_GLOBALSINDEX);
	}

	template<typename RVal>
	RVal GetVar(const char* name)
	{
		lua_settop(L, 0);
		lua_pushstring(L, name);
		lua_gettable(L, LUA_GLOBALSINDEX);

		return pop_<RVal>(L, -1);
	}

	// Table Variable
	template<typename T>
	void SetMemberVar(const char* table, const char* name, T object)
	{
		lua_settop(L, 0);
		lua_pushstring(L, table);
		lua_gettable(L, LUA_GLOBALSINDEX);
		if(lua_istable(L,-1) || lua_isuserdata(L, -1))
		{
			lua_pushstring(L, name);
			push_(L, object);
			lua_settable(L, -3);
		}
		else
		{
			lua_pushfstring(L, "WLua Error: attempt to call global `%s' (not a table)", table);
			pfnException(L);
		}
	}

	template<typename T>
	void SetMemberVarRef(const char* table, const char* name, T& object)
	{
		lua_settop(L, 0);
		lua_pushstring(L, table);
		lua_gettable(L, LUA_GLOBALSINDEX);
		if(lua_istable(L,-1) || lua_isuserdata(L, -1))
		{
			lua_pushstring(L, name);
			push_(L, object);
			lua_settable(L, -2);
		}
		else
		{
			lua_pushfstring(L, "WLua Error: attempt to call global `%s' (not a table)", table);
			pfnException(L);
		}
	}

	template<typename RVal>
	RVal GetMemberVar(const char* table, const char* name)
	{
		lua_settop(L, 0);
		lua_pushstring(L, table);
		lua_gettable(L, LUA_GLOBALSINDEX);
		if(lua_istable(L,-1) || lua_isuserdata(L, -1))
		{
			lua_pushstring(L, name);
			lua_gettable(L, -2);
		}
		else
		{
			lua_pushfstring(L, "WLua Error: attempt to call global `%s' (not a table)", table);
			pfnException(L);
		}

		return pop_<RVal>(L, -1);
	}

	inline void RegisterCppDebugInfo(const char* pszCallTableName, const char* pszCallFuncName, WLUA_DEBUG_ARG_DECL)
	{
		int nCppStackIndexLocal = GetCppStackIndex();
#ifdef WLUA_USE_DEBUG_STACK
		if (WLUA_MAX_CALLSTACK_DEPTH <= nCppStackIndexLocal)
		{
			lua_pushfstring(L, "CppCallStack is full. check define 'WLUA_MAX_CALLSTACK_DEPTH'.");
			pfnException(L);
			return;
		}

		Lock(); // CppStacks
		strncpy_s(CppStacks[nCppStackIndexLocal].strFileName,		pszFile, WLUA_MAX_FILE_NAME_BUF_LEN);
		strncpy_s(CppStacks[nCppStackIndexLocal].strFuncName,		pszFunc, WLUA_MAX_NAME_BUF_LEN);
		strncpy_s(CppStacks[nCppStackIndexLocal].strCallFuncName,	pszCallFuncName, WLUA_MAX_NAME_BUF_LEN);
		if (pszCallTableName)
			strncpy_s(CppStacks[nCppStackIndexLocal].strTableName,		pszCallTableName, WLUA_MAX_NAME_BUF_LEN);
		else
			CppStacks[nCppStackIndexLocal].strTableName[0] = '\0';

		CppStacks[nCppStackIndexLocal].nLine				= nLine;
		Unlock(); // CppStacks
#endif

#ifdef WLUA_USE_DEBUG_TIMEOUT
		if (nCppStackIndexLocal == 0)
		{
			TimeoutThread->BeginTimeWatch();
		}
#endif

		SetCppStackIndex(nCppStackIndexLocal+1);

	}	

	inline void RemoveCppDebugInfo()
	{
		SetCppStackIndex(GetCppStackIndex()-1);

		if (0 >= GetCppStackIndex())
		{
#ifdef WLUA_USE_DEBUG_TIMEOUT
			TimeoutThread->EndTimeWatch();
#endif
			SetCppStackIndex(0);
		}
	}

private:
	inline bool _PreCall(const char* name, WLUA_DEBUG_ARG_DECL)
	{
		if (pfnPreCall)
			pfnPreCall();

		RegisterCppDebugInfo(NULL, name, WLUA_DEBUG_ARG_DECL_NAME);

		PreCall();

		lua_pushstring(L, name);
		lua_gettable(L, LUA_GLOBALSINDEX);
		if(!lua_isfunction(L,-1))	
		{
			lua_pushfstring(L, "WLua Error: attempt to call global `%s' (not a function)", name);
			pfnException(L);
			return false;
		}

		return true;
	}

	inline bool _PostCall(int nCallRet)
	{
		RemoveCppDebugInfo();

		if (nCallRet != 0)
		{
			if (pfnPostCall)
				pfnPostCall();

			return false;
		}

		if (pfnPostCall)
			pfnPostCall();

		return true;
	}

	template<typename RVal>
	inline void _GetReturnValue(RVal* poutVal)
	{
		if (!poutVal)					return;
		if (lua_gettop(L) < 1)			return;

		if (!lua_isnoneornil(L, -1) &&
			!lua_isfunction(L, -1))
		{
			*poutVal = pop_<RVal>(L, -1);
		}

		lua_remove(L, 1);
	}

	template<>
	inline void _GetReturnValue(void* poutVal)	{}

	template<>
	inline void _GetReturnValue(table* poutVal)	
	{
		if (!poutVal)					return;
		poutVal->attach(L, -1);
		poutVal->set_writable();
	}

public:
	// Call
	template<typename RVal>
	bool Call(const char* name, WLUA_DEBUG_ARG_DECL, RVal* poutVal)
	{
		int nCallRet = -1;
		if (_PreCall(name, WLUA_DEBUG_ARG_DECL_NAME))
		{
			nCallRet = RawCall(0, ret_<RVal>::value);
		}

		if (!_PostCall(nCallRet))	return false;

		_GetReturnValue(poutVal);

		return true;
	}

	template<typename RVal, typename T1>
	bool Call(const char* name, T1 arg, WLUA_DEBUG_ARG_DECL, RVal* poutVal)
	{
		int nCallRet = -1;
		if (_PreCall(name, WLUA_DEBUG_ARG_DECL_NAME))
		{
			push_<T1>(L, arg);
			nCallRet = RawCall(1, ret_<RVal>::value);
		}

		if (!_PostCall(nCallRet))	return false;

		_GetReturnValue(poutVal);

		return true;
	}


	template<typename RVal, typename T1, typename T2>
	bool Call(const char* name, T1 arg1, T2 arg2, WLUA_DEBUG_ARG_DECL, RVal* poutVal)
	{
		int nCallRet = -1;
		if (_PreCall(name, WLUA_DEBUG_ARG_DECL_NAME))
		{
			push_<T1>(L, arg1);
			push_<T2>(L, arg2);
			nCallRet = RawCall(2, ret_<RVal>::value);
		}

		if (!_PostCall(nCallRet))	return false;

		_GetReturnValue(poutVal);

		return true;
	}

	template<typename RVal, typename T1, typename T2, typename T3>
	bool Call(const char* name, T1 arg1, T2 arg2, T3 arg3, WLUA_DEBUG_ARG_DECL, RVal* poutVal)
	{
		int nCallRet = -1;
		if (_PreCall(name, WLUA_DEBUG_ARG_DECL_NAME))
		{
			push_<T1>(L, arg1);
			push_<T2>(L, arg2);
			push_<T3>(L, arg3);
			nCallRet = RawCall(3, ret_<RVal>::value);
		}

		if (!_PostCall(nCallRet))	return false;

		_GetReturnValue(poutVal);

		return true;
	}

	template<typename RVal, typename T1, typename T2, typename T3, typename T4>
	bool Call(const char* name, T1 arg1, T2 arg2, T3 arg3, T4 arg4, WLUA_DEBUG_ARG_DECL, RVal* poutVal)
	{
		int nCallRet = -1;
		if (_PreCall(name, WLUA_DEBUG_ARG_DECL_NAME))
		{
			push_<T1>(L, arg1);
			push_<T2>(L, arg2);
			push_<T3>(L, arg3);
			push_<T4>(L, arg4);
			nCallRet = RawCall(4, ret_<RVal>::value);
		}

		if (!_PostCall(nCallRet))	return false;

		_GetReturnValue(poutVal);

		return true;
	}

	template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5>
	bool Call(const char* name, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, WLUA_DEBUG_ARG_DECL, RVal* poutVal)
	{
		int nCallRet = -1;
		if (_PreCall(name, WLUA_DEBUG_ARG_DECL_NAME))
		{
			push_<T1>(L, arg1);
			push_<T2>(L, arg2);
			push_<T3>(L, arg3);
			push_<T4>(L, arg4);
			push_<T5>(L, arg5);
			nCallRet = RawCall(5, ret_<RVal>::value);
		}

		if (!_PostCall(nCallRet))	return false;

		_GetReturnValue(poutVal);

		return true;
	}

	template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	bool Call(const char* name, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, WLUA_DEBUG_ARG_DECL, RVal* poutVal)
	{
		int nCallRet = -1;
		if (_PreCall(name, WLUA_DEBUG_ARG_DECL_NAME))
		{
			push_<T1>(L, arg1);
			push_<T2>(L, arg2);
			push_<T3>(L, arg3);
			push_<T4>(L, arg4);
			push_<T5>(L, arg5);
			push_<T6>(L, arg6);
			nCallRet = RawCall(6, ret_<RVal>::value);
		}

		if (!_PostCall(nCallRet))	return false;

		_GetReturnValue(poutVal);

		return true;
	}

	template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	bool Call(const char* name, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, WLUA_DEBUG_ARG_DECL, RVal* poutVal)
	{
		int nCallRet = -1;
		if (_PreCall(name, WLUA_DEBUG_ARG_DECL_NAME))
		{
			push_<T1>(L, arg1);
			push_<T2>(L, arg2);
			push_<T3>(L, arg3);
			push_<T4>(L, arg4);
			push_<T5>(L, arg5);
			push_<T6>(L, arg6);
			push_<T7>(L, arg7);
			nCallRet = RawCall(7, ret_<RVal>::value);
		}

		if (!_PostCall(nCallRet))	return false;

		_GetReturnValue(poutVal);

		return true;
	}

	template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	bool Call(const char* name, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, WLUA_DEBUG_ARG_DECL, RVal* poutVal)
	{
		int nCallRet = -1;
		if (_PreCall(name, WLUA_DEBUG_ARG_DECL_NAME))
		{
			push_<T1>(L, arg1);
			push_<T2>(L, arg2);
			push_<T3>(L, arg3);
			push_<T4>(L, arg4);
			push_<T5>(L, arg5);
			push_<T6>(L, arg6);
			push_<T7>(L, arg7);
			push_<T8>(L, arg8);
			nCallRet = RawCall(8, ret_<RVal>::value);
		}

		if (!_PostCall(nCallRet))	return false;

		_GetReturnValue(poutVal);

		return true;
	}


private:
	inline bool _PreMemberCall(const char* table, const char* name, WLUA_DEBUG_ARG_DECL)
	{
		RegisterCppDebugInfo(table, name, WLUA_DEBUG_ARG_DECL_NAME);

		PreCall();

		lua_pushstring(L, table);
		lua_gettable(L, LUA_GLOBALSINDEX);
		int nCallRet = -1;
		if(lua_istable(L,-1) || lua_isuserdata(L, -1))
		{
			lua_pushstring(L, name);
			lua_gettable(L, -2);
			lua_insert(L, -2); 
			if(lua_isfunction(L,-2))
			{
				return true;
			}
			else
			{
				lua_pushfstring(L, "WLua Error: attempt to call global `%s:%s' (not a function)", table, name);
				pfnException(L);
				return false;
			}
		}
		else
		{
			lua_pushfstring(L, "WLua Error: attempt to call global `%s' (not a table)", table);
			pfnException(L);
			return false;
		}

		return false;
	}

	inline bool _PostMemberCall(int nCallRet)
	{
		RemoveCppDebugInfo();

		if (nCallRet != 0)
		{
			lua_settop(L, 0);
			return false;
		}

		return true;
	}

public:
	template<typename RVal>
	bool MemberCall(const char* table, const char* name, WLUA_DEBUG_ARG_DECL, RVal* poutVal)
	{
		int nCallRet = -1;
		if (_PreMemberCall(table, name, WLUA_DEBUG_ARG_DECL_NAME))
		{
			nCallRet = RawCall(0+1, ret_<RVal>::value);
		}

		if (!_PostMemberCall(nCallRet))	return false;

		_GetReturnValue(poutVal);

		return true;
	}

	template<typename RVal, typename T1>
	bool MemberCall(const char* table, const char* name, T1 arg1, WLUA_DEBUG_ARG_DECL, RVal* poutVal)
	{
		int nCallRet = -1;
		if (_PreMemberCall(table, name, WLUA_DEBUG_ARG_DECL_NAME))
		{
			push_(L, arg1);
			nCallRet = RawCall(1+1, ret_<RVal>::value);
		}

		if (!_PostMemberCall(nCallRet))	return false;

		_GetReturnValue(poutVal);

		return true;
	}

	template<typename RVal, typename T1, typename T2>
	bool MemberCall(const char* table, const char* name, T1 arg1, T2 arg2, WLUA_DEBUG_ARG_DECL, RVal* poutVal)
	{
		int nCallRet = -1;
		if (_PreMemberCall(table, name, WLUA_DEBUG_ARG_DECL_NAME))
		{
			push_(L, arg1);
			push_(L, arg2);
			nCallRet = RawCall(2+1, ret_<RVal>::value);
		}

		if (!_PostMemberCall(nCallRet))	return false;

		_GetReturnValue(poutVal);

		return true;
	}

	template<typename RVal, typename T1, typename T2, typename T3>
	bool MemberCall(const char* table, const char* name, T1 arg1, T2 arg2, T3 arg3, WLUA_DEBUG_ARG_DECL, RVal* poutVal)
	{
		int nCallRet = -1;
		if (_PreMemberCall(table, name, WLUA_DEBUG_ARG_DECL_NAME))
		{
			push_(L, arg1);
			push_(L, arg2);
			push_(L, arg3);
			nCallRet = RawCall(3+1, ret_<RVal>::value);
		}

		if (!_PostMemberCall(nCallRet))	return false;

		_GetReturnValue(poutVal);

		return true;
	}

	template<typename RVal, typename T1, typename T2, typename T3, typename T4>
	bool MemberCall(const char* table, const char* name, T1 arg1, T2 arg2, T3 arg3, T4 arg4, WLUA_DEBUG_ARG_DECL, RVal* poutVal)
	{
		int nCallRet = -1;
		if (_PreMemberCall(table, name, WLUA_DEBUG_ARG_DECL_NAME))
		{
			push_(L, arg1);
			push_(L, arg2);
			push_(L, arg3);
			push_(L, arg4);
			nCallRet = RawCall(4+1, ret_<RVal>::value);
		}

		if (!_PostMemberCall(nCallRet))	return false;

		_GetReturnValue(poutVal);

		return true;
	}

	template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5>
	bool MemberCall(const char* table, const char* name, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, WLUA_DEBUG_ARG_DECL, RVal* poutVal)
	{
		int nCallRet = -1;
		if (_PreMemberCall(table, name, WLUA_DEBUG_ARG_DECL_NAME))
		{
			push_(L, arg1);
			push_(L, arg2);
			push_(L, arg3);
			push_(L, arg4);
			push_(L, arg5);
			nCallRet = RawCall(5+1, ret_<RVal>::value);
		}

		if (!_PostMemberCall(nCallRet))	return false;

		_GetReturnValue(poutVal);

		return true;
	}

	template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	bool MemberCall(const char* table, const char* name, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, WLUA_DEBUG_ARG_DECL, RVal* poutVal)
	{
		int nCallRet = -1;
		if (_PreMemberCall(table, name, WLUA_DEBUG_ARG_DECL_NAME))
		{
			push_(L, arg1);
			push_(L, arg2);
			push_(L, arg3);
			push_(L, arg4);
			push_(L, arg5);
			push_(L, arg6);
			nCallRet = RawCall(6+1, ret_<RVal>::value);
		}

		if (!_PostMemberCall(nCallRet))	return false;

		_GetReturnValue(poutVal);

		return true;
	}

	template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	bool MemberCall(const char* table, const char* name, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, WLUA_DEBUG_ARG_DECL, RVal* poutVal)
	{
		int nCallRet = -1;
		if (_PreMemberCall(table, name, WLUA_DEBUG_ARG_DECL_NAME))
		{
			push_(L, arg1);
			push_(L, arg2);
			push_(L, arg3);
			push_(L, arg4);
			push_(L, arg5);
			push_(L, arg6);
			push_(L, arg7);
			nCallRet = RawCall(7+1, ret_<RVal>::value);
		}

		if (!_PostMemberCall(nCallRet))	return false;

		_GetReturnValue(poutVal);

		return true;
	}

	template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	bool MemberCall(const char* table, const char* name, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, WLUA_DEBUG_ARG_DECL, RVal* poutVal)
	{
		int nCallRet = -1;
		if (_PreMemberCall(table, name, WLUA_DEBUG_ARG_DECL_NAME))
		{
			push_(L, arg1);
			push_(L, arg2);
			push_(L, arg3);
			push_(L, arg4);
			push_(L, arg5);
			push_(L, arg6);
			push_(L, arg7);
			push_(L, arg8);
			nCallRet = RawCall(8+1, ret_<RVal>::value);
		}

		if (!_PostMemberCall(nCallRet))	return false;

		_GetReturnValue(poutVal);

		return true;
	}

	// Check
	bool IsExistGlobal(const char* szName)
	{
		if (!L)		return false;

		// 전역 함수 있는지 체크
		lua_pushstring(L, szName);
		lua_gettable(L, LUA_GLOBALSINDEX);
		bool bExist = 
			(lua_isnil(L, -1) == 0);

		lua_pop(L, 1);
		return bExist;
	}

	bool IsExistGlobalTable(const char* szFuncName)
	{
		if (!L)		return false;

		// 전역 함수 있는지 체크
		lua_pushstring(L, szFuncName);
		lua_gettable(L, LUA_GLOBALSINDEX);
		bool bExist = 
			(lua_istable(L, -1) != 0);

		lua_pop(L, 1);
		return bExist;
	}

	bool IsExistGlobalFunc(const char* szFuncName)
	{
		if (!L)		return false;

		// 전역 함수 있는지 체크
		lua_pushstring(L, szFuncName);
		lua_gettable(L, LUA_GLOBALSINDEX);
		bool bExist = 
			(lua_isfunction(L, -1) != 0);

		lua_pop(L, 1);
		return bExist;
	}

	bool IsExistMemberFunc(const char* szTableName, const char* szFuncName)
	{
		if (!L)		return false;

		// 전역 함수 있는지 체크
		lua_pushstring(L, szTableName);
		lua_gettable(L, LUA_GLOBALSINDEX);
		if (!lua_istable(L, -1) && !lua_isuserdata(L, -1))	
		{
			lua_pop(L, 1);
			return false;
		}

		lua_pushstring(L, szFuncName);
		lua_gettable(L, -2);
		lua_insert(L, -2); 
		bool bExist = 
			(lua_isfunction(L, -2) != 0);

		lua_pop(L, 2);
		return bExist;
	}

	// File
	bool RunFile(const char* name, WLUA_DEBUG_ARG_DECL);

	// String
	bool RunString(std::string text, WLUA_DEBUG_ARG_DECL);
	bool RunString(const char* text, size_t len, WLUA_DEBUG_ARG_DECL);

	void EnumStack(int start=0);

	inline static MWLua* GetWLua(lua_State *L)
	{
		LUA_MAP::iterator iter = mapLua.find(L)	;

		if (iter == mapLua.end())
			return NULL;

		return (*iter).second;
	}
	void Log(const char* pFormat, ...)
	{
		Lock();
		char temp[1024];
		va_list args;
		va_start(args, pFormat);
		vsprintf_s(temp, pFormat, args);
		va_end(args);

		if (pfnLog)
			pfnLog(temp);
		Unlock();
	}

	// Class
	template<typename T>
	struct class_ : lua_obj
	{
		// initialize
		class_(lua_State* l, const char* name) 
			:	m_L(l)
		{ 
			ClassName(name); 

			lua_pushstring(m_L, name);
			lua_newtable(m_L);

			lua_pushstring(m_L, "__name");
			lua_pushstring(m_L, name);
			lua_rawset(m_L, -3);

			lua_pushstring(m_L, "__index");
			lua_pushcclosure(m_L, &::get_var, 0);
			lua_rawset(m_L, -3);

			lua_pushstring(m_L, "__newindex");
			lua_pushcclosure(m_L, &::set_var, 0);
			lua_rawset(m_L, -3);

			lua_pushstring(m_L, "__gc");
			lua_pushcclosure(m_L, destroyer<T>, 0);
			lua_rawset(m_L, -3);

			lua_settable(m_L, LUA_GLOBALSINDEX);
		}

		// inheritence
		template<typename P>
		class_<T>& inh()
		{
			class_<class_type<T>::type>::push_meta(m_L);
			lua_pushstring(m_L, "__parent");
			class_<class_type<P>::type>::push_meta(m_L);
			lua_rawset(m_L, -3);
			lua_pop(m_L,1);
			return *this; 
		}

		// functions
		template<typename F>
		class_<T>& def(const char* name, F func) 
		{ 
			class_<class_type<T>::type>::push_meta(m_L);

			lua_pushstring(m_L, name);
			new(lua_newuserdata(m_L,sizeof(F))) F(func);
			push_functor(m_L, func);
			lua_rawset(m_L, -3);

			lua_pop(m_L,1);
			return *this; 
		}

		// variables
		template<typename BASE, typename VAR>
		class_<T>& def_readwrite(const char* name, VAR BASE::*val) 
		{ 
			class_<class_type<T>::type>::push_meta(m_L);

			lua_pushstring(m_L, name);
			new(lua_newuserdata(m_L,sizeof(mem_var<BASE,VAR>))) mem_var<BASE,VAR>(val);
			lua_rawset(m_L, -3);

			lua_pop(m_L,1);
			return *this; 
		}

		// metatable
		static void push_meta(lua_State *L)
		{
			if(ClassName() == "")
			{
				lua_pushnil(L);
			}
			else
			{
				lua_pushstring(L, ClassName().c_str());
				lua_gettable(L, LUA_GLOBALSINDEX);				
			}
		}

		// misc
		static inline std::string& ClassName(std::string strClassNameBuffer="")
		{
			static std::string m_strClassNameBuffer="";

			if (strClassNameBuffer.length() > 0)
				m_strClassNameBuffer = strClassNameBuffer;

			return m_strClassNameBuffer;
		}

		// constructor
		template<typename CONSTRUCTOR>
		class_<T>& def(CONSTRUCTOR)
		{
			class_<class_type<T>::type>::push_meta(m_L);

			lua_newtable(m_L);
			lua_pushstring(m_L, "__call");
			lua_pushcclosure(m_L, creator<T>::invoke<CONSTRUCTOR>, 0);
			lua_rawset(m_L, -3);
			lua_setmetatable(m_L, -2);
			lua_pop(m_L,1);
			return *this; 
		}

		lua_State* m_L;	    
	};

	// Call
	inline void PreCall()
	{
		//	lua_settop(L, 0);

		if (pfnException)
		{
			lua_pushcclosure(L, pfnException, 0);
		}
	}

	inline int RawCall(int nArg, int nRet)
	{
		int nStatus = 
			lua_pcall(L, nArg, nRet, 1);

		lua_remove(L, 1);

		return nStatus;
	}

	void			InitCppCallInfo()
	{
#ifdef WLUA_USE_DEBUG_STACK
		SetCppStackIndex(0);
#endif
	}

	CPP_FUNC_INFO*	AdvanceCppCallInfo()
	{
#ifdef WLUA_USE_DEBUG_STACK
		int nCppStackIndexLocal = GetCppStackIndex();

		if (0							>	nCppStackIndexLocal ||
			WLUA_MAX_CALLSTACK_DEPTH	<=	nCppStackIndexLocal)
		{
			SetCppStackIndex(nCppStackIndexLocal-1);
			return NULL;
		}

		CPP_FUNC_INFO* pRet = &CppStacks[nCppStackIndexLocal];

		SetCppStackIndex(nCppStackIndexLocal-1);

		return pRet;
#else
		return NULL;
#endif
	}

	CPP_FUNC_INFO*	PrevCppCallInfo()
	{
#ifdef WLUA_USE_DEBUG_STACK
		int nCppStackIndexLocal = GetCppStackIndex();

		if (0							>	nCppStackIndexLocal ||
			WLUA_MAX_CALLSTACK_DEPTH	<=	nCppStackIndexLocal)
		{
			return NULL;
		}

		return &CppStacks[nCppStackIndexLocal]; // 인덱스는 변경되지 않음
#else
		return NULL;
#endif
	}

	// Debug
	inline void Error()
	{
		pfnException(L);
	}

	void Lock();
	void Unlock();

	// Getter
	inline int GetCppStackIndex()
	{
		Lock();
		int nRet = nCppStackIndex;
		Unlock();

		return nRet;
	}

	inline int GetCallTimeoutSec()
	{
		Lock();
		int nRet = nCallTimeoutSec;
		Unlock();

		return nRet;
	}

	// Setter
	inline void SetCppStackIndex(int nValue)
	{
		Lock();
		nCppStackIndex = nValue;
		Unlock();
	}

	inline void SetCallTimeoutSec(int nValue)
	{
		Lock();
		nCallTimeoutSec = nValue;
		Unlock();
	}

	inline char* TempBuff() 
	{
		Lock();
		char* pszRet = szTemp;
		Unlock();

		return pszRet;
	}

protected:
#ifdef WLUA_USE_DEBUG_TIMEOUT
	friend MWLuaThread;
	MWLuaThread*	TimeoutThread;
	CRITICAL_SECTION	cs;
#endif

	lua_State*		L;				///< 루아 상태기계
	lua_CFunction	pfnException;	///< 예외 함수 포인터
	fnLog			pfnLog;			///< 로그 함수 포인터
	fnCallback		pfnPreCall;		///< 호출전 콜백함수 포인터
	fnCallback		pfnPostCall;	///< 호출후 콜백함수 포인터
	static LUA_MAP	mapLua;			///< 생성된 인스턴트들의 맵

	char			szTableNameBuff[WLUA_MAX_NAME_BUF_LEN*2+1+1];
	char			szTemp[WLUA_TEMP_BUF_LEN];

	// 쓰레드에 안전하지 않음
	std::deque<CPP_FUNC_INFO> CppStacks;

	int				nCppStackIndex;
	int				nCallTimeoutSec;
};


#ifdef WLUA_USE_DEBUG_TIMEOUT

inline 
void 
MWLuaThread::BeginTimeWatch()		
{ 
	m_pLua->Lock(); 
	m_uBeginTime = timeGetTime(); 
	m_pLua->Unlock();
}

inline 
void 
MWLuaThread::EndTimeWatch()			
{ 
	m_pLua->Lock(); 
	m_uBeginTime = 0; 
	m_pLua->Unlock();
}

inline 
unsigned long 
MWLuaThread::GetBeginTime()
{
	m_pLua->Lock(); 
	unsigned long ret = m_uBeginTime;
	m_pLua->Unlock();
	return ret;
}

#endif


/*---------------------------------------------------------------------------*/ 
/*                                                                           */ 
/*---------------------------------------------------------------------------*/ 
template<>
char* MWLua::pop_(lua_State *L, int index)
{
	return (char*)lua_tostring(L, index);				
}

template<>
const char* MWLua::pop_(lua_State *L, int index)
{
	return (const char*)lua_tostring(L, index);		
}

template<>
char MWLua::pop_(lua_State *L, int index)
{
	return (char)lua_tonumber(L, index);				
}

template<>
unsigned char MWLua::pop_(lua_State *L, int index)
{
	return (unsigned char)lua_tonumber(L, index);		
}

template<>
short MWLua::pop_(lua_State *L, int index)
{
	return (short)lua_tonumber(L, index);				
}

template<>
unsigned short MWLua::pop_(lua_State *L, int index)
{
	return (unsigned short)lua_tonumber(L, index);	
}

template<>
long MWLua::pop_(lua_State *L, int index)
{
	return (long)lua_tonumber(L, index);				
}

template<>
unsigned long MWLua::pop_(lua_State *L, int index)
{
	return (unsigned long)lua_tonumber(L, index);		
}

template<>
int MWLua::pop_(lua_State *L, int index)
{
	return (int)lua_tonumber(L, index);				
}

template<>
unsigned int MWLua::pop_(lua_State *L, int index)
{
	return (unsigned int)lua_tonumber(L, index);		
}

template<>
float MWLua::pop_(lua_State *L, int index)
{
	return (float)lua_tonumber(L, index);				
}

template<>
double MWLua::pop_(lua_State *L, int index)
{
	return (double)lua_tonumber(L, index);			
}

template<>
bool MWLua::pop_(lua_State *L, int index)
{
	return lua_toboolean(L, index) != 0;				
}

template<>
void MWLua::pop_(lua_State *L, int index)
{
	return;											
}

template<>
__int64 MWLua::pop_(lua_State *L, int index)
{
	if(lua_isnumber(L,index))
		return (__int64)lua_tonumber(L, index);
	else
		return *(__int64*)lua_touserdata(L, index);
}
template<>
unsigned __int64 MWLua::pop_(lua_State *L, int index)
{
	if(lua_isnumber(L,index))
		return (unsigned __int64)lua_tonumber(L, index);
	else
		return *(unsigned __int64*)lua_touserdata(L, index);
}

template<>
MWLua::table MWLua::pop_( lua_State *L, int index )
{
	return table(L, index);
}

/*---------------------------------------------------------------------------*/ 
/*                                                                           */ 
/*---------------------------------------------------------------------------*/ 
template<>
void MWLua::push_(lua_State *L, char ret)
{
	lua_pushnumber(L, ret);						
}

template<>
void MWLua::push_(lua_State *L, unsigned char ret)
{
	lua_pushnumber(L, ret);						
}

template<>
void MWLua::push_(lua_State *L, short ret)
{
	lua_pushnumber(L, ret);						
}

template<>
void MWLua::push_(lua_State *L, unsigned short ret)
{
	lua_pushnumber(L, ret);						
}

template<>
void MWLua::push_(lua_State *L, long ret)
{
	lua_pushnumber(L, ret);						
}

template<>
void MWLua::push_(lua_State *L, unsigned long ret)
{
	lua_pushnumber(L, ret);						
}

template<>
void MWLua::push_(lua_State *L, int ret)
{
	lua_pushnumber(L, ret);						
}

template<>
void MWLua::push_(lua_State *L, unsigned int ret)
{
	lua_pushnumber(L, ret);						
}

template<>
void MWLua::push_(lua_State *L, float ret)
{
	lua_pushnumber(L, ret);						
}

template<>
void MWLua::push_(lua_State *L, double ret)
{
	lua_pushnumber(L, ret);						
}

template<>
void MWLua::push_(lua_State *L, char* ret)
{
	lua_pushstring(L, ret);						
}

template<>
void MWLua::push_(lua_State *L, const char* ret)
{
	lua_pushstring(L, ret);						
}

template<>
void MWLua::push_(lua_State *L, bool ret)
{
	lua_pushboolean(L, ret);						
}

template<>
void MWLua::push_(lua_State *L, lua_value* ret)
{
	if(ret) ret->to_lua(L); else lua_pushnil(L);	
}

template<>
void MWLua::push_(lua_State *L, __int64 ret)			
{ 
	*(__int64*)lua_newuserdata(L, sizeof(__int64)) = ret;
	lua_pushstring(L, "__s64");
	lua_gettable(L, LUA_GLOBALSINDEX);
	lua_setmetatable(L, -2);
}
template<>
void MWLua::push_(lua_State *L, unsigned __int64 ret)
{
	*(unsigned __int64*)lua_newuserdata(L, sizeof(unsigned __int64)) = ret;
	lua_pushstring(L, "__u64");
	lua_gettable(L, LUA_GLOBALSINDEX);
	lua_setmetatable(L, -2);
}

template<>
void MWLua::push_(lua_State *L, const ret_int2 ret)
{
	lua_pushnumber(L, ret.val1);
	lua_pushnumber(L, ret.val2);
}

template<>
void MWLua::push_(lua_State *L, const ret_double2 ret)
{
	lua_pushnumber(L, ret.val1);
	lua_pushnumber(L, ret.val2);
}

template<>
void MWLua::push_(lua_State *L, const ret_int4 ret)
{
	lua_pushnumber(L, ret.val1);
	lua_pushnumber(L, ret.val2);
	lua_pushnumber(L, ret.val3);
	lua_pushnumber(L, ret.val4);
}

template<>
void MWLua::push_(lua_State *L, MWLua::table ret)
{
	ret.set_unwritable();
	lua_pushvalue(L, ret.m_obj->m_index);
}

template<>
MWLua::table 
MWLua::table_obj::get<MWLua::table>( const char* name )
{
	if(validate())
	{
		lua_pushstring(m_L, name);
		lua_gettable(m_L, m_index);
	}
	else
	{
		lua_pushnil(m_L);
	}

	MWLua::table ret(m_L, lua_gettop(m_L));
	ret.set_writable();
	return ret; 
}



// 새로운 매크로를 저장된 값으로 복원
#pragma pop_macro ("new")

#define WDCALL(L, FN, DA, RET)												((L) ? (L)->Call((FN), DA, (RET)) : false)
#define WDCALL1(L, FN, A1, DA, RET)											((L) ? (L)->Call((FN), (A1), DA, (RET)) : false)
#define WDCALL2(L, FN, A1, A2, DA, RET)										((L) ? (L)->Call((FN), (A1), (A2), DA, (RET)) : false)
#define WDCALL3(L, FN, A1, A2, A3, DA, RET)									((L) ? (L)->Call((FN), (A1), (A2), (A3), DA, (RET)) : false)
#define WDCALL4(L, FN, A1, A2, A3, A4, DA, RET)								((L) ? (L)->Call((FN), (A1), (A2), (A3), (A4), DA, (RET)) : false)
#define WDCALL5(L, FN, A1, A2, A3, A4, A5, DA, RET)							((L) ? (L)->Call((FN), (A1), (A2), (A3), (A4), (A5), DA, (RET)) : false)
#define WDCALL6(L, FN, A1, A2, A3, A4, A5, A6, DA, RET)						((L) ? (L)->Call((FN), (A1), (A2), (A3), (A4), (A5), (A6), DA, (RET)) : false)
#define WDCALL7(L, FN, A1, A2, A3, A4, A5, A6, A7, DA, RET)					((L) ? (L)->Call((FN), (A1), (A2), (A3), (A4), (A5), (A6), (A7), DA, (RET)) : false)
#define WDCALL8(L, FN, A1, A2, A3, A4, A5, A6, A7, A8, DA, RET)				((L) ? (L)->Call((FN), (A1), (A2), (A3), (A4), (A5), (A6), (A7), (A8), DA, (RET)) : false)

#define WCALL(L, FN, RET)													WDCALL((L), (FN), WLUA_DEBUG_ARG, (RET))
#define WCALL1(L, FN, A1, RET)												WDCALL1((L), (FN), (A1), WLUA_DEBUG_ARG, (RET))
#define WCALL2(L, FN, A1, A2, RET)											WDCALL2((L), (FN), (A1), (A2), WLUA_DEBUG_ARG, (RET))
#define WCALL3(L, FN, A1, A2, A3, RET)										WDCALL3((L), (FN), (A1), (A2), (A3), WLUA_DEBUG_ARG, (RET))
#define WCALL4(L, FN, A1, A2, A3, A4, RET)									WDCALL4((L), (FN), (A1), (A2), (A3), (A4), WLUA_DEBUG_ARG, (RET))
#define WCALL5(L, FN, A1, A2, A3, A4, A5, RET)								WDCALL5((L), (FN), (A1), (A2), (A3), (A4), (A5), WLUA_DEBUG_ARG, (RET))
#define WCALL6(L, FN, A1, A2, A3, A4, A5, A6, RET)							WDCALL6((L), (FN), (A1), (A2), (A3), (A4), (A5), (A6), WLUA_DEBUG_ARG, (RET))
#define WCALL7(L, FN, A1, A2, A3, A4, A5, A6, A7, RET)						WDCALL7((L), (FN), (A1), (A2), (A3), (A4), (A5), (A6), (A7), WLUA_DEBUG_ARG, (RET))
#define WCALL8(L, FN, A1, A2, A3, A4, A5, A6, A7, A8, RET)					WDCALL8((L), (FN), (A1), (A2), (A3), (A4), (A5), (A6), (A7), (A8), WLUA_DEBUG_ARG, (RET))

#define WDMEMCALL(L, TABLE, FN, DA, RET)									((L) ? (L)->MemberCall((TABLE), (FN), DA, RET) : false)
#define WDMEMCALL1(L, TABLE, FN, A1, DA, RET)								((L) ? (L)->MemberCall((TABLE), (FN), (A1), DA, RET) : false)
#define WDMEMCALL2(L, TABLE, FN, A1, A2, DA, RET)							((L) ? (L)->MemberCall((TABLE), (FN), (A1), (A2), DA, RET) : false)
#define WDMEMCALL3(L, TABLE, FN, A1, A2, A3, DA, RET)						((L) ? (L)->MemberCall((TABLE), (FN), (A1), (A2), (A3), DA, RET) : false)
#define WDMEMCALL4(L, TABLE, FN, A1, A2, A3, A4, DA, RET)					((L) ? (L)->MemberCall((TABLE), (FN), (A1), (A2), (A3), (A4), DA, RET) : false)
#define WDMEMCALL5(L, TABLE, FN, A1, A2, A3, A4, A5, DA, RET)				((L) ? (L)->MemberCall((TABLE), (FN), (A1), (A2), (A3), (A4), (A5), DA, RET) : false)
#define WDMEMCALL6(L, TABLE, FN, A1, A2, A3, A4, A5, A6, DA, RET)			((L) ? (L)->MemberCall((TABLE), (FN), (A1), (A2), (A3), (A4), (A5), (A6), DA, RET) : false)
#define WDMEMCALL7(L, TABLE, FN, A1, A2, A3, A4, A5, A6, A7, DA, RET)		((L) ? (L)->MemberCall((TABLE), (FN), (A1), (A2), (A3), (A4), (A5), (A6), (A7), DA, RET) : false)
#define WDMEMCALL8(L, TABLE, FN, A1, A2, A3, A4, A5, A6, A7, A8, DA, RET)	((L) ? (L)->MemberCall((TABLE), (FN), (A1), (A2), (A3), (A4), (A5), (A6), (A7), (A8), DA, RET) : false)

#define WMEMCALL(L, TABLE, FN, RET)											WDMEMCALL((L), (TABLE), (FN), WLUA_DEBUG_ARG, (RET))
#define WMEMCALL1(L, TABLE, FN, A1, RET)									WDMEMCALL1((L), (TABLE), (FN), (A1), WLUA_DEBUG_ARG, (RET))
#define WMEMCALL2(L, TABLE, FN, A1, A2, RET)								WDMEMCALL2((L), (TABLE), (FN), (A1), (A2), WLUA_DEBUG_ARG, (RET))
#define WMEMCALL3(L, TABLE, FN, A1, A2, A3, RET)							WDMEMCALL3((L), (TABLE), (FN), (A1), (A2), (A3), WLUA_DEBUG_ARG, (RET))
#define WMEMCALL4(L, TABLE, FN, A1, A2, A3, A4, RET)						WDMEMCALL4((L), (TABLE), (FN), (A1), (A2), (A3), (A4), WLUA_DEBUG_ARG, (RET))
#define WMEMCALL5(L, TABLE, FN, A1, A2, A3, A4, A5, RET)					WDMEMCALL5((L), (TABLE), (FN), (A1), (A2), (A3), (A4), (A5), WLUA_DEBUG_ARG, (RET))
#define WMEMCALL6(L, TABLE, FN, A1, A2, A3, A4, A5, A6, RET)				WDMEMCALL6((L), (TABLE), (FN), (A1), (A2), (A3), (A4), (A5), (A6), WLUA_DEBUG_ARG, (RET))
#define WMEMCALL7(L, TABLE, FN, A1, A2, A3, A4, A5, A6, A7, RET)			WDMEMCALL7((L), (TABLE), (FN), (A1), (A2), (A3), (A4), (A5), (A6), (A7), WLUA_DEBUG_ARG, (RET))
#define WMEMCALL8(L, TABLE, FN, A1, A2, A3, A4, A5, A6, A7, A8, RET)		WDMEMCALL8((L), (TABLE), (FN), (A1), (A2), (A3), (A4), (A5), (A6), (A7), (A8), WLUA_DEBUG_ARG, (RET))


#define WRUNSTRING(L, TEXT)		((L) ? (L)->RunString((TEXT), WLUA_DEBUG_ARG) : false)
#define WRUNFILE(L, FILE)		((L) ? (L)->RunFile((FILE), WLUA_DEBUG_ARG) : false)

#define WNULL	(void*)0


#endif // __MAIET_WILD_LUA_H__

