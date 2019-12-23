#ifndef __HanRTA__
#define __HanRTA__

#define SERVICE_NATION  0x000000FF
#define SERVICE_KOR		0x00000001
#define SERVICE_USA		0x00000002
#define SERVICE_JPN		0x00000003
#define SERVICE_CHN		0x00000004

#define SERVICE_TYPE    0x00000F00
#define SERVICE_ALPHA	0x00000100
#define SERVICE_REAL	0x00000200
#define SERVICE_BETA	0x00000300


#ifdef LOCALE_NHNUSAA

#ifdef HANRTA_EXPORTS
	#define HANRTA_API __declspec(dllexport)
#else
	#define HANRTA_API __declspec(dllimport)
	#ifdef _DEBUG
		#pragma comment(lib, "HanRTAD.lib") 
	#else
		#pragma comment(lib, "HanRTA.lib") 
	#endif
#endif

/*
Return value Info:
0 : Ok
- : fault from  function / this system 
	-1 : general fault.
+ : fault from  auth server
	+1 : ???
*/

HANRTA_API int __stdcall HanRTAInit(/*IN*/ char* szGameId,
									/*IN*/ int   nServiceCode,
									/*IN*/unsigned int nConnCnt );

/*
Info String: like below..
"gameid=gameid&servername=1&conncount=2"
*/

HANRTA_API int __stdcall HanRTA( /*IN*/  char *szInfoString );


#else

int __stdcall HanRTAInit(/*IN*/ char* szGameId,
									/*IN*/ int   nServiceCode,
									/*IN*/unsigned int nConnCnt ) 			{ return 0; }
int __stdcall HanRTA( /*IN*/  char *szInfoString )			{ return 0; }

#endif
									
#endif