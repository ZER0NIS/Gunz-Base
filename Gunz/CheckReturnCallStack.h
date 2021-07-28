#pragma once

#ifdef _DEBUG
#define CHECK_RETURN_CALLSTACK(FuncName)			DWORD ret_ptr_dlrjsTmwlakdy	= 0; \
													DWORD *dwI = NULL; \
													__asm {pushad} \
													__asm {mov eax, ebp} \
													__asm {add eax, 4} \
													__asm {mov eax, dword ptr ds:[eax]}	\
													__asm {mov ret_ptr_dlrjsTmwlakdy, eax}	\
													__asm {mov eax, FuncName} \
													__asm {mov dwI, eax}	\
													__asm {popad}; \
														DWORD top		= (DWORD)0x00401000 + 0x246000; \
														DWORD bottom	= (DWORD)0x00401000; \
														if( (ret_ptr_dlrjsTmwlakdy <= bottom) || (ret_ptr_dlrjsTmwlakdy >= top) ) { \
														MCommand* pC=ZNewCmd(MC_REQUEST_GIVE_ONESELF_UP); ZPostCommand(pC); }
#else
#define CHECK_RETURN_CALLSTACK(FuncName)
#endif

#ifdef _DEBUG
#define CHECK_RETURN_CALLSTACK_ESP(FuncName)		DWORD ret_ptr_dlrjsTmwlakdy	= 0; \
													DWORD *dwI = NULL; \
													__asm {pushad} \
													__asm {mov eax, esp} \
													__asm {add eax, 4} \
													__asm {mov eax, dword ptr ds:[eax]}	\
													__asm {mov ret_ptr_dlrjsTmwlakdy, eax}	\
													__asm {mov eax, FuncName} \
													__asm {mov dwI, eax}	\
													__asm {popad}; \
													DWORD top		= (DWORD)0x00401000 + 0x246000; \
													DWORD bottom	= (DWORD)0x00401000; \
													if( (ret_ptr_dlrjsTmwlakdy <= bottom) || (ret_ptr_dlrjsTmwlakdy >= top) ) { \
													MCommand* pC=ZNewCmd(MC_REQUEST_GIVE_ONESELF_UP); ZPostCommand(pC); }
#else
#define CHECK_RETURN_CALLSTACK_ESP(FuncName)
#endif
