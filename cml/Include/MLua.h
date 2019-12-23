#ifndef _MLUA_H
#define _MLUA_H

#include "RLib.h"
#include "lua_tinker.h"
using namespace lua_tinker;

typedef int (*LuaFunctionType)(struct lua_State *pLuaState);
#define LuaGlue extern "C" int


class CML2_API MLua
{
private:
protected:
	lua_State*				m_pState;
	void					(*m_pErrorHandler)(const char* pError);
public:
				MLua();
	virtual		~MLua();
	bool		Create();
	void		Destroy();
	void		SetErrorHandler(void(*pErrHandler)(const char* pError)) {m_pErrorHandler = pErrHandler;}
	lua_State*	GetContext(void)											{return m_pState;}
	bool		RunFile(const char* szFileName);
	bool		RunString(const char* szStream);
	const char* GetErrorString(void);
	bool		AddFunction(const char *szFunctionName, LuaFunctionType pFunction);
	const char* GetStringArgument(int nNum, const char* pDefault=NULL);
	double		GetNumberArgument(int nNum, double dDefault=0.0);
	void		PushString(const char* szString);
	void		PushNumber(double value);

	bool		RunCall(int nargs, int nresults);
};


#endif