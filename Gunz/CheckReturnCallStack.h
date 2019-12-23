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


// 함수 진입시 ESP 체크하려는 의도로 만든 듯한 매크로 (그러나 아직 제대로 구현되어 있지 않다)
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



// mlog("callstack : 0x%X~0x%X, 0x%X\n", bottom, top, ret_ptr_dlrjsTmwlakdy ); \
// mlog("func addr : 0x%x\n", dwI ); \
// printf("callstack hack : 0x%X~0x%X, 0x%X\n", bottom, top, ret_ptr_dlrjsTmwlakdy ); \



// #define CHECK_RETURN_CALLSTACK(FuncName)			NULL;