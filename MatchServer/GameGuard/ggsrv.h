#ifndef _GG_AUTH_SERVER_H_
#define _GG_AUTH_SERVER_H_

#ifndef WIN32
	typedef unsigned long   DWORD;
	typedef void*          LPVOID;
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

// C type CSAuth2
typedef LPVOID          LPGGAUTH;


#ifdef LOCALE_NHNUSAA


#ifdef _EXPORT_DLL
	#define GGAUTHS_API    extern "C" __declspec(dllexport)
	#define GGAUTHS_EXPORT __declspec(dllexport)
	#define __CDECL        __cdecl
#else
	#define GGAUTHS_API extern "C"
	#define GGAUTHS_EXPORT 
	#define __CDECL 
#endif

// ggauth.dll¿« Path
GGAUTHS_API DWORD __CDECL InitGameguardAuth(char* sGGPath, DWORD dwNumActive = 50);
GGAUTHS_API void  __CDECL CleanupGameguardAuth();

// protocol dll name
GGAUTHS_API DWORD __CDECL AddAuthProtocol(char* sDllName);

// Class CSAuth2
class GGAUTHS_EXPORT CCSAuth2
{
public:
	// Constructor
	CCSAuth2();

	// Destructor
	~CCSAuth2();

protected:
	PGG_AUTH_PROTOCOL m_pProtocol;
	DWORD m_bPrtcRef;	
	DWORD m_dwUserFlag;

public:	
	GG_AUTH_DATA m_AuthQuery;
	GG_AUTH_DATA m_AuthAnswer;

	void  Init();
	void Close();
	DWORD GetAuthQuery();
	DWORD CheckAuthAnswer();	
};

GGAUTHS_API LPGGAUTH __CDECL GGAuthInitUser();                                              // CCSAuth2()
GGAUTHS_API void     __CDECL GGAuthCloseUser(LPGGAUTH pGGAuth);                             // ~CCSAuth2()
GGAUTHS_API DWORD    __CDECL GGAuthGetQuery(LPGGAUTH pGGAuth, PGG_AUTH_DATA pAuthData);     // GetAuthQuery()
GGAUTHS_API DWORD    __CDECL GGAuthCheckAnswer(LPGGAUTH pGGAuth, PGG_AUTH_DATA pAuthData);  // CheckAuthAnswer()



#else // LOCALE_NHNUSAA



// ggauth.dll¿« Path
DWORD InitGameguardAuth(char* sGGPath, DWORD dwNumActive = 50)  { return 0; }
void  CleanupGameguardAuth() { }

// protocol dll name
DWORD AddAuthProtocol(char* sDllName) { return 0; }

// Class CSAuth2
class CCSAuth2
{
public:
	CCSAuth2() {}

	// Destructor
	~CCSAuth2() {}

protected:
	PGG_AUTH_PROTOCOL m_pProtocol;
	DWORD m_bPrtcRef;	
	DWORD m_dwUserFlag;

public:	
	GG_AUTH_DATA m_AuthQuery;
	GG_AUTH_DATA m_AuthAnswer;

	void  Init() {}
	void Close() {}
	DWORD GetAuthQuery() { return 0; }
	DWORD CheckAuthAnswer() { return 0; }
};

LPGGAUTH GGAuthInitUser()  { return NULL; }
void     GGAuthCloseUser(LPGGAUTH pGGAuth)  { }
DWORD    GGAuthGetQuery(LPGGAUTH pGGAuth, PGG_AUTH_DATA pAuthData)  { return 0; }
DWORD    GGAuthCheckAnswer(LPGGAUTH pGGAuth, PGG_AUTH_DATA pAuthData)  { return 0; }

#endif

#endif


