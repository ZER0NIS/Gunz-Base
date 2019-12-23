// CPP 에서 클래스 부분만 뺀 헤더파일. C 개발 환경에서 사용함. C++ 관련을 매핑처리

#ifndef _GG_AUTH_SERVER_H_
#define _GG_AUTH_SERVER_H_

#ifndef WIN32
	typedef unsigned int   DWORD;
	typedef void*          LPVOID;
#endif
								
#ifdef _EXPORT_DLL
	#define GGAUTHS_API    extern "C" __declspec(dllexport)
	#define GGAUTHS_EXPORT __declspec(dllexport)
	#define __CDECL        __cdecl
#else
	#define GGAUTHS_API extern "C"
	#define GGAUTHS_EXPORT 
	#define __CDECL 
#endif

// gameguard auth data
typedef struct _GG_AUTH_DATA
{
	DWORD dwIndex;
	DWORD dwValue1;
	DWORD dwValue2;
	DWORD dwValue3;
} GG_AUTH_DATA, *PGG_AUTH_DATA;

typedef struct _GG_AUTH_PROTOCOL *PGG_AUTH_PROTOCOL;

// ggauth.dll의 Path
GGAUTHS_API DWORD __CDECL InitGameguardAuth(char* sGGPath, DWORD dwNumActive = 50);
GGAUTHS_API void  __CDECL CleanupGameguardAuth();

// protocol dll name
GGAUTHS_API DWORD __CDECL AddAuthProtocol(char* sDllName);

// C type CSAuth2
typedef LPVOID          LPGGAUTH;

GGAUTHS_API LPGGAUTH __CDECL GGAuthInitUser();                                              // CCSAuth2()
GGAUTHS_API void     __CDECL GGAuthCloseUser(LPGGAUTH pGGAuth);                             // ~CCSAuth2()
GGAUTHS_API DWORD    __CDECL GGAuthGetQuery(LPGGAUTH pGGAuth, PGG_AUTH_DATA pAuthData);     // GetAuthQuery()
GGAUTHS_API DWORD    __CDECL GGAuthCheckAnswer(LPGGAUTH pGGAuth, PGG_AUTH_DATA pAuthData);  // CheckAuthAnswer()

#endif


