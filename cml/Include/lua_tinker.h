// lua_tinker.h
//
// LuaTinker - Simple and light C++ wrapper for Lua.
//
// Copyright (c) 2005 Kwon-il Lee (zupet@hitel.net)
// 
// please check Licence.txt file for licence and legal issues. 
//
// - 멀티리턴타입 관련 수정됨 - bird(2006/02/06)
// - 인클루드 폴더 lua/ 로 수정 - dubble(2006/04/20)

#if !defined(_LUA_TINKER_H_)
#define _LUA_TINKER_H_

#include <new>
extern "C" 
{
	#include "lua/lua.h"
	#include "lua/lualib.h"
	#include "lua/lauxlib.h"
}


namespace lua_tinker
{
	// debug helpers
	void	enum_stack(lua_State *L, int start=0);
	int		_exception(lua_State *L);

	void	dofile(const char *filename);
	void	dostring(const char* buff);
	void	dobuffer(const char* buff, size_t sz);

	void	dofile(lua_State *L, const char *filename);
	void	dostring(lua_State *L, const char* buff);
	void	dobuffer(lua_State *L, const char* buff, size_t sz);

	// basic Object
	struct lua_state
	{
		static void open(lua_State *in)
		{ 
			L(in); 
			init_s64(in);
			init_u64(in);
		}

		static lua_State*	L(lua_State *in=NULL);
		static void			init_s64(lua_State *L);
		static void			init_u64(lua_State *L);
	};

	// for LuaBind
	struct luabind : lua_state
	{
	};

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

	// for multi return value - by bird
	struct ret_int2
	{
		int val1;
		int val2;
	};
	struct ret_int3
	{
		int val1;
		int val2;
		int val3;
	};
	struct ret_float2
	{
		float val1;
		float val2;
	};
	struct ret_float3
	{
		float val1;
		float val2;
		float val3;
	};

	// type trait
	template<typename T> struct class_;

	template<bool C, typename A, typename B> 
	struct if_ {};
	template<typename A, typename B> 
	struct if_<true, A, B> { typedef A type; };
	template<typename A, typename B> 
	struct if_<false, A, B> { typedef B type; };

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

	no_type int_conv_tester (...); 
	yes_type int_conv_tester (int_conv_type); 

	no_type vfnd_ptr_tester (const volatile char *); 
	no_type vfnd_ptr_tester (const volatile short *); 
	no_type vfnd_ptr_tester (const volatile int *); 
	no_type vfnd_ptr_tester (const volatile long *); 
	no_type vfnd_ptr_tester (const volatile double *); 
	no_type vfnd_ptr_tester (const volatile float *); 
	no_type vfnd_ptr_tester (const volatile bool *); 
	yes_type vfnd_ptr_tester (const volatile void *); 

	template <typename T> T* add_ptr(T&); 

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
			return	if_<is_ptr<T>::value
						,void2ptr<base_type<T>::type>
						,if_<is_ref<T>::value
							,void2ref<base_type<T>::type>
							,void2val<base_type<T>::type>
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
				lua_pushstring(L, "no class at first argument. (forgot ':' expression ?)");
				lua_error(L);
			}
			return void2type<T>::invoke(user2type<user*>::invoke(L,index)->m_p); 
		} 
	};

	template<typename T>
	struct lua2type
	{
		static T invoke(lua_State *L, int index)
		{
			return	if_<is_enum<T>::value
						,lua2enum<T>
						,lua2object<T> 
					>::type::invoke(L, index);
		}
	};

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
	struct type2lua
	{
		static void invoke(lua_State *L, T val)
		{
			if_<is_enum<T>::value
				,enum2lua<T>
				,object2lua<T>
			>::type::invoke(L, val);
		};
	};

	//
	template<typename T>  
	T func_(lua_State *L)
	{
		return user2type<T>::invoke(L, lua_upvalueindex(1));
	}

	// arguments
	struct pop_ 
	{
		template<typename T>  
		static T invoke(lua_State *L, int index)				{ return lua2type<T>::invoke(L, index);					}
		template<> 
		static char* invoke(lua_State *L, int index)			{ return (char*)lua_tostring(L, index);					}
		template<> 
		static const char* invoke(lua_State *L, int index)		{ return (const char*)lua_tostring(L, index);			}
		template<> 
		static char invoke(lua_State *L, int index)				{ return (char)lua_tonumber(L, index);					}
		template<> 
		static unsigned char invoke(lua_State *L, int index)	{ return (unsigned char)lua_tonumber(L, index);			}
		template<> 
		static short invoke(lua_State *L, int index)			{ return (short)lua_tonumber(L, index);					}
		template<> 
		static unsigned short invoke(lua_State *L, int index)	{ return (unsigned short)lua_tonumber(L, index);		}
		template<> 
		static long invoke(lua_State *L, int index)				{ return (long)lua_tonumber(L, index);					}
		template<> 
		static unsigned long invoke(lua_State *L, int index)	{ return (unsigned long)lua_tonumber(L, index);			}
		template<> 
		static int invoke(lua_State *L, int index)				{ return (int)lua_tonumber(L, index);					}
		template<> 
		static unsigned int invoke(lua_State *L, int index)		{ return (unsigned int)lua_tonumber(L, index);			}
		template<> 
		static float invoke(lua_State *L, int index)			{ return (float)lua_tonumber(L, index);					}
		template<> 
		static double invoke(lua_State *L, int index)			{ return (double)lua_tonumber(L, index);				}
		template<> 
		static bool invoke(lua_State *L, int index)				{ return lua_toboolean(L, index) != 0;					}
		template<> 
		static void invoke(lua_State *L, int index)				{ return;												}
		template<> 
		static __int64 invoke(lua_State *L, int index)
		{
			if(lua_isnumber(L,index))
				return (__int64)lua_tonumber(L, index);
			else
				return *(__int64*)lua_touserdata(L, index);
		}
		template<> 
		static unsigned __int64 invoke(lua_State *L, int index)
		{
			if(lua_isnumber(L,index))
				return (unsigned __int64)lua_tonumber(L, index);
			else
				return *(unsigned __int64*)lua_touserdata(L, index);
		}
	};

	// return value
	struct push_
	{
		template<typename T>  
		static void invoke(lua_State *L, T ret)					{ type2lua<T>::invoke(L, ret);	}
		template<>
		static void invoke(lua_State *L, char ret)				{ lua_pushnumber(L, ret);		}
		template<>
		static void invoke(lua_State *L, unsigned char ret)		{ lua_pushnumber(L, ret);		}
		template<>
		static void invoke(lua_State *L, short ret)				{ lua_pushnumber(L, ret);		}
		template<>
		static void invoke(lua_State *L, unsigned short ret)	{ lua_pushnumber(L, ret);		}
		template<>
		static void invoke(lua_State *L, long ret)				{ lua_pushnumber(L, ret);		}
		template<>
		static void invoke(lua_State *L, unsigned long ret)		{ lua_pushnumber(L, ret);		}
		template<>
		static void invoke(lua_State *L, int ret)				{ lua_pushnumber(L, ret);		}
		template<>
		static void invoke(lua_State *L, unsigned int ret)		{ lua_pushnumber(L, ret);		}
		template<>
		static void invoke(lua_State *L, float ret)				{ lua_pushnumber(L, ret);		}
		template<>
		static void invoke(lua_State *L, double ret)			{ lua_pushnumber(L, ret);		}
		template<>
		static void invoke(lua_State *L, char* ret)				{ lua_pushstring(L, ret);		}
		template<>
		static void invoke(lua_State *L, const char* ret)		{ lua_pushstring(L, ret);		}
		template<>
		static void invoke(lua_State *L, bool ret)				{ lua_pushboolean(L, ret);		}
		template<>
		static void invoke(lua_State *L, lua_value* ret)		{ if(ret) ret->to_lua(L); else lua_pushnil(L);	}
		template<> 
		static void invoke(lua_State *L, __int64 ret)			
		{ 
			*(__int64*)lua_newuserdata(L, sizeof(__int64)) = ret;
			lua_pushstring(L, "__s64");
			lua_gettable(L, LUA_GLOBALSINDEX);
			lua_setmetatable(L, -2);
		}
		template<> 
		static void invoke(lua_State *L, unsigned __int64 ret)
		{
			*(unsigned __int64*)lua_newuserdata(L, sizeof(unsigned __int64)) = ret;
			lua_pushstring(L, "__u64");
			lua_gettable(L, LUA_GLOBALSINDEX);
			lua_setmetatable(L, -2);
		}

		// 추가 - bird
		template<>
		static void invoke(lua_State *L, const ret_int2 ret)
		{
			lua_pushnumber(L, ret.val1);
			lua_pushnumber(L, ret.val2);
		}

	};

	template<typename T>
	struct ret_ { static const int value = 1; };
	// 추가 - bird
	template<>
	struct ret_<ret_int2> { static const int value = 2; };
	template<>
	struct ret_<void> { static const int value = 0; };

	// caller
	template<typename T1=void, typename T2=void, typename T3=void, typename T4=void, typename T5=void>
	struct caller
	{
		template<typename RVal>
		static void invoke(lua_State *L) { push_::invoke(L,func_<RVal(*)(T1,T2,T3,T4,T5)>(L)(pop_::invoke<T1>(L,1),pop_::invoke<T2>(L,2),pop_::invoke<T3>(L,3),pop_::invoke<T4>(L,4),pop_::invoke<T5>(L,5))); }
		template<>
		static void invoke<void>(lua_State *L) { func_<void(*)(T1,T2,T3,T4,T5)>(L)(pop_::invoke<T1>(L,1),pop_::invoke<T2>(L,2),pop_::invoke<T3>(L,3),pop_::invoke<T4>(L,4),pop_::invoke<T5>(L,5)); }
	};

	template<typename T1, typename T2, typename T3, typename T4>
	struct caller<T1,T2,T3,T4> 
	{
		template<typename RVal>
		static void invoke(lua_State *L) { push_::invoke(L,func_<RVal(*)(T1,T2,T3,T4)>(L)(pop_::invoke<T1>(L,1),pop_::invoke<T2>(L,2),pop_::invoke<T3>(L,3),pop_::invoke<T4>(L,4))); }
		template<>
		static void invoke<void>(lua_State *L) { func_<void(*)(T1,T2,T3,T4)>(L)(pop_::invoke<T1>(L,1),pop_::invoke<T2>(L,2),pop_::invoke<T3>(L,3),pop_::invoke<T4>(L,4)); }
	};

	template<typename T1, typename T2, typename T3>
	struct caller<T1,T2,T3> 
	{
		template<typename RVal>
		static void invoke(lua_State *L) { push_::invoke(L,func_<RVal(*)(T1,T2,T3)>(L)(pop_::invoke<T1>(L,1),pop_::invoke<T2>(L,2),pop_::invoke<T3>(L,3))); }
		template<>
		static void invoke<void>(lua_State *L) { func_<void(*)(T1,T2,T3)>(L)(pop_::invoke<T1>(L,1),pop_::invoke<T2>(L,2),pop_::invoke<T3>(L,3)); }
	};

	template<typename T1, typename T2>
	struct caller<T1,T2> 
	{
		template<typename RVal>
		static void invoke(lua_State *L) { push_::invoke(L,func_<RVal(*)(T1,T2)>(L)(pop_::invoke<T1>(L,1),pop_::invoke<T2>(L,2))); }
		template<>
		static void invoke<void>(lua_State *L) { func_<void(*)(T1,T2)>(L)(pop_::invoke<T1>(L,1),pop_::invoke<T2>(L,2)); }
	};

	template<typename T1>
	struct caller<T1> 
	{
		template<typename RVal>
		static void invoke(lua_State *L) { push_::invoke(L,func_<RVal(*)(T1)>(L)(pop_::invoke<T1>(L,1))); }
		template<>
		static void invoke<void>(lua_State *L) { func_<void(*)(T1)>(L)(pop_::invoke<T1>(L,1)); }
	};

	template<>
	struct caller<void> 
	{
		template<typename RVal>
		static void invoke(lua_State *L) { push_::invoke(L,func_<RVal(*)()>(L)()); }
		template<>
		static void invoke<void>(lua_State *L) { func_<void(*)()>(L)(); }
	};

	// function
	template<typename T1=void, typename T2=void, typename T3=void, typename T4=void, typename T5=void> 
	struct functor
	{
		template<typename RVal>
		static int invoke(lua_State *L) { caller<T1,T2,T3,T4,T5>::invoke<RVal>(L); return ret_<RVal>::value; }
	};

	template<typename RVal> 
	void push_func(lua_State *L, RVal (*func)())
	{
		lua_pushcclosure(L, functor<>::invoke<RVal>, 1);
	}

	template<typename RVal,class T1> 
	void push_func(lua_State *L, RVal (*func)(T1))
	{ 
		lua_pushcclosure(L, functor<T1>::invoke<RVal>, 1);
	}

	template<typename RVal,class T1,class T2> 
	void push_func(lua_State *L, RVal (*func)(T1,T2))
	{ 
		lua_pushcclosure(L, functor<T1,T2>::invoke<RVal>, 1);
	}

	template<typename RVal,class T1,class T2, typename T3> 
	void push_func(lua_State *L, RVal (*func)(T1,T2,T3))
	{ 
		lua_pushcclosure(L, functor<T1,T2,T3>::invoke<RVal>, 1);
	}

	template<typename RVal,class T1,class T2, typename T3, typename T4> 
	void push_func(lua_State *L, RVal (*func)(T1,T2,T3,T4))
	{ 
		lua_pushcclosure(L, functor<T1,T2,T3,T4>::invoke<RVal>, 1);
	}

	template<typename RVal,class T1,class T2, typename T3, typename T4, typename T5> 
	void push_func(lua_State *L, RVal (*func)(T1,T2,T3,T4,T5))
	{ 
		lua_pushcclosure(L, functor<T1,T2,T3,T4,T5>::invoke<RVal>, 1);
	}

	// member variable
	template<typename T>
	T* this_(lua_State *L) 
	{ 
		return pop_::invoke<T*>(L, 1); 
	}

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
		void get(lua_State *L)	{ push_::invoke(L, this_<T>(L)->*(_var));		}
		void set(lua_State *L)	{ this_<T>(L)->*(_var) = pop_::invoke<V>(L, 3);	}
	};

	// member function
	template<typename T,class T1=void, typename T2=void, typename T3=void, typename T4=void, typename T5=void>
	struct mem_caller
	{
		template<typename RVal>
		static void invoke(lua_State *L) { push_::invoke(L,(this_<T>(L)->*func_<RVal(T::*)(T1,T2,T3,T4,T5)>(L))(pop_::invoke<T1>(L,2),pop_::invoke<T2>(L,3),pop_::invoke<T3>(L,4),pop_::invoke<T4>(L,5),pop_::invoke<T5>(L,6))); }
		template<>
		static void invoke<void>(lua_State *L)  { (this_<T>(L)->*func_<void(T::*)(T1,T2,T3,T4,T5)>(L))(pop_::invoke<T1>(L,2),pop_::invoke<T2>(L,3),pop_::invoke<T3>(L,4),pop_::invoke<T4>(L,5),pop_::invoke<T5>(L,6)); }
	};

	template<typename T, typename T1, typename T2, typename T3, typename T4> 
	struct mem_caller<T,T1,T2,T3,T4>
	{
		template<typename RVal>
		static void invoke(lua_State *L) { push_::invoke(L,(this_<T>(L)->*func_<RVal(T::*)(T1,T2,T3,T4)>(L))(pop_::invoke<T1>(L,2),pop_::invoke<T2>(L,3),pop_::invoke<T3>(L,4),pop_::invoke<T4>(L,5))); }
		template<>
		static void invoke<void>(lua_State *L)  { (this_<T>(L)->*func_<void(T::*)(T1,T2,T3,T4)>(L))(pop_::invoke<T1>(L,2),pop_::invoke<T2>(L,3),pop_::invoke<T3>(L,4),pop_::invoke<T4>(L,5)); }
	};

	template<typename T, typename T1, typename T2, typename T3> 
	struct mem_caller<T,T1,T2,T3>
	{
		template<typename RVal>
		static void invoke(lua_State *L) { push_::invoke(L,(this_<T>(L)->*func_<RVal(T::*)(T1,T2,T3)>(L))(pop_::invoke<T1>(L,2),pop_::invoke<T2>(L,3),pop_::invoke<T3>(L,4))); }
		template<>
		static void invoke<void>(lua_State *L)  { (this_<T>(L)->*func_<void(T::*)(T1,T2,T3)>(L))(pop_::invoke<T1>(L,2),pop_::invoke<T2>(L,3),pop_::invoke<T3>(L,4)); }
	};

	template<typename T, typename T1, typename T2> 
	struct mem_caller<T,T1, T2>
	{
		template<typename RVal>
		static void invoke(lua_State *L) { push_::invoke(L,(this_<T>(L)->*func_<RVal(T::*)(T1,T2)>(L))(pop_::invoke<T1>(L,2),pop_::invoke<T2>(L,3))); }
		template<>
		static void invoke<void>(lua_State *L)  { (this_<T>(L)->*func_<void(T::*)(T1,T2)>(L))(pop_::invoke<T1>(L,2),pop_::invoke<T2>(L,3)); }
	};

	template<typename T, typename T1> 
	struct mem_caller<T,T1>
	{
		template<typename RVal>
		static void invoke(lua_State *L) { push_::invoke(L,(this_<T>(L)->*func_<RVal(T::*)(T1)>(L))(pop_::invoke<T1>(L,2))); }
		template<>
		static void invoke<void>(lua_State *L)  { (this_<T>(L)->*func_<void(T::*)(T1)>(L))(pop_::invoke<T1>(L,2)); }
	};

	template<typename T> 
	struct mem_caller<T>
	{
		template<typename RVal>
		static void invoke(lua_State *L) { push_::invoke(L,(this_<T>(L)->*func_<RVal(T::*)()>(L))()); }
		template<>
		static void invoke<void>(lua_State *L)  { (this_<T>(L)->*func_<void(T::*)()>(L))(); }
	};
	
	// 
	template<typename T, typename T1=void, typename T2=void, typename T3=void, typename T4=void, typename T5=void> 
	struct mem_functor
	{
		template<typename RVal>
		static int invoke(lua_State *L) { mem_caller<T,T1,T2,T3,T4,T5>::invoke<RVal>(L); return ret_<RVal>::value; }
	};

	template<typename RVal, typename T>
	void push_func(lua_State *L, RVal (T::*func)()) 
	{ 
		lua_pushcclosure(L, mem_functor<T>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T>
	void push_func(lua_State *L, RVal (T::*func)() const) 
	{ 
		lua_pushcclosure(L, mem_functor<T>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1>
	void push_func(lua_State *L, RVal (T::*func)(T1)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1>
	void push_func(lua_State *L, RVal (T::*func)(T1) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2>
	void push_func(lua_State *L, RVal (T::*func)(T1,T2)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2>
	void push_func(lua_State *L, RVal (T::*func)(T1,T2) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3>
	void push_func(lua_State *L, RVal (T::*func)(T1,T2,T3)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3>
	void push_func(lua_State *L, RVal (T::*func)(T1,T2,T3) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4>
	void push_func(lua_State *L, RVal (T::*func)(T1,T2,T3,T4)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4>
	void push_func(lua_State *L, RVal (T::*func)(T1,T2,T3,T4) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
	void push_func(lua_State *L, RVal (T::*func)(T1,T2,T3,T4,T5)) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4,T5>::invoke<RVal>, 1); 
	}

	template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
	void push_func(lua_State *L, RVal (T::*func)(T1,T2,T3,T4,T5) const) 
	{ 
		lua_pushcclosure(L, mem_functor<T,T1,T2,T3,T4,T5>::invoke<RVal>, 1); 
	}

	// constructor
	template<typename T1=void, typename T2=void, typename T3=void, typename T4=void>
	struct constructor {};

	template<typename T1, typename T2, typename T3>
	struct constructor<T1,T2,T3>
	{
		template<typename T>
		static void invoke(lua_State *L)
		{
			new(lua_newuserdata(L, sizeof(val2user<T>))) val2user<T>(pop_::invoke<T1>(L,2),pop_::invoke<T2>(L,3),pop_::invoke<T3>(L,4));
		}
	};


	template<typename T1, typename T2>
	struct constructor<T1,T2>
	{
		template<typename T>
		static void invoke(lua_State *L)
		{
			new(lua_newuserdata(L, sizeof(val2user<T>))) val2user<T>(pop_::invoke<T1>(L,2),pop_::invoke<T2>(L,3));
		}
	};

	template<typename T1>
	struct constructor<T1>
	{
		template<typename T>
		static void invoke(lua_State *L)
		{
			new(lua_newuserdata(L, sizeof(val2user<T>))) val2user<T>(pop_::invoke<T1>(L,2));
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
	struct destroyer
	{
		static int invoke(lua_State *L) 
		{ 
			((user*)lua_touserdata(L, 1))->~user();
			return 0;
		}
	};

	// get
	int get_var(lua_State *L);
	int set_var(lua_State *L);

	// Tinker Function
	template<typename F> 
	lua_obj def(const char* name, F func)
	{ 
		lua_State *L = lua_state::L();
		lua_pushstring(L, name);
		lua_pushlightuserdata(L, func);
		push_func(L, func);
		lua_settable(L, LUA_GLOBALSINDEX);

		return lua_obj();
	}

	// Tinker Global
	template<typename T>
	lua_obj decl(const char* name, T object)
	{
		lua_State *L = lua_state::L();
		lua_pushstring(L, name);
		push_::invoke(L, object);
		lua_settable(L, LUA_GLOBALSINDEX);

		return lua_obj();
	}

	// Tinker Call
	template<typename RVal>
	RVal call(const char* name)
	{
		lua_State *L = lua_state::L();

		lua_pushcclosure(L, _exception, 0);
		lua_pushstring(L, name);
		lua_gettable(L, LUA_GLOBALSINDEX);
		if(lua_isfunction(L,-1))
		{
			lua_pcall(L, 0, ret_<RVal>::value, 1);
		}
		else
		{
			lua_pushfstring(L, "lua_tinker : attempt to call global `%s' (not a function)", name);
//			_exception(L);
		}

		lua_remove(L, 1);
		return pop_::invoke<RVal>(L, -1);
	}

	template<typename RVal, typename T1>
	RVal call(const char* name, T1 arg)
	{
		lua_State *L = lua_state::L();

		lua_pushcclosure(L, _exception, 0);
		lua_pushstring(L, name);
		lua_gettable(L, LUA_GLOBALSINDEX);
		push_::invoke(L, arg);
		lua_pcall(L, 1, ret_<RVal>::value, 1);
		lua_remove(L, 1);
		return pop_::invoke<RVal>(L, -1);
	}

	template<typename RVal, typename T1, typename T2>
	RVal call(const char* name, T1 arg1, T2 arg2)
	{
		lua_State *L = lua_state::L();

		lua_pushcclosure(L, _exception, 0);
		lua_pushstring(L, name);
		lua_gettable(L, LUA_GLOBALSINDEX);
		push_::invoke(L, arg1);
		push_::invoke(L, arg2);
		lua_pcall(L, 2, ret_<RVal>::value, 1);
		lua_remove(L, 1);
		return pop_::invoke<RVal>(L, -1);
	}

	template<typename RVal, typename T1, typename T2, typename T3>
	RVal call(const char* name, T1 arg1, T2 arg2, T3 arg3)
	{
		lua_State *L = lua_state::L();

		lua_pushcclosure(L, _exception, 0);
		lua_pushstring(L, name);
		lua_gettable(L, LUA_GLOBALSINDEX);
		push_::invoke(L, arg1);
		push_::invoke(L, arg2);
		push_::invoke(L, arg3);
		lua_pcall(L, 3, ret_<RVal>::value, 1);
		lua_remove(L, 1);
		return pop_::invoke<RVal>(L, -1);
	}

	// Tinker Class
	template<typename T>
	struct class_ : lua_obj
	{
		// initialize
		class_(const char* name) 
		:	m_L(lua_state::L())
		{ 
			_name(name); 

			lua_pushstring(m_L, name);
			lua_newtable(m_L);

			lua_pushstring(m_L, "__name");
			lua_pushstring(m_L, name);
			lua_rawset(m_L, -3);

			lua_pushstring(m_L, "__index");
			lua_pushcclosure(m_L, get_var, 0);
			lua_rawset(m_L, -3);

			lua_pushstring(m_L, "__newindex");
			lua_pushcclosure(m_L, set_var, 0);
			lua_rawset(m_L, -3);

			lua_pushstring(m_L, "__gc");
			lua_pushcclosure(m_L, destroyer<T>::invoke, 0);
			lua_rawset(m_L, -3);

			lua_settable(m_L, LUA_GLOBALSINDEX);
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
			push_func(m_L, func);
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
			const char* name = _name();
			if(name[0])
			{
				lua_pushstring(L, name);
				lua_gettable(L, LUA_GLOBALSINDEX);
			}
			else
			{
				lua_pushnil(L);
			}
		}

		// global name
		static const char* _name(const char* name = NULL)
		{
			static char temp[256] = "";
			if(name) strcpy(temp, name);
			return temp;
		}

		lua_State* m_L;	    
	};

} // namespace lua_tinker

#endif //_LUA_TINKER_H_