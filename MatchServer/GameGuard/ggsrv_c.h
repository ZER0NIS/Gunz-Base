#ifndef _GG_AUTH_SERVER_H_
#define _GG_AUTH_SERVER_H_

#ifdef _EXPORT_DLL
	#define GGAUTHS_API    extern "C" __declspec(dllexport)
	#define GGAUTHS_EXPORT __declspec(dllexport)
	#define __CDECL        __cdecl
#else
	#define GGAUTHS_API 
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

typedef struct _GG_AUTH_USER
{
	PGG_AUTH_PROTOCOL m_pProtocol;
	DWORD             m_bPrtcRef;
	DWORD             m_dwUserFlag;
	GG_AUTH_DATA      m_AuthQuery;
	GG_AUTH_DATA      m_AuthAnswer;
} GG_AUTH_USER, *PGG_AUTH_USER;

// ggauth.dll¿« Path
GGAUTHS_API DWORD __CDECL InitGameguardAuth(char* sGGPath, DWORD dwNumActive = 50);
GGAUTHS_API void  __CDECL CleanupGameguardAuth();

// protocol dll name
GGAUTHS_API DWORD __CDECL AddAuthProtocol(char* sDllName);

// C type CSAuth2
GGAUTHS_API void __CDECL GGAuthInitUser(PGG_AUTH_USER pAuthUser);                               // CCSAuth2()
GGAUTHS_API void __CDECL GGAuthCloseUser(PGG_AUTH_USER pAuthUser);                           // ~CCSAuth2()
GGAUTHS_API DWORD    __CDECL GGAuthGetQuery(PGG_AUTH_USER pAuthUser);     // GetAuthQuery()
GGAUTHS_API DWORD    __CDECL GGAuthCheckAnswer(PGG_AUTH_USER pAuthUser);  // CheckAuthAnswer()

#endif


