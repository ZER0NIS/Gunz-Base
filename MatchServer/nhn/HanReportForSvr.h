#ifndef __HanReportForSvr__
#define __HanReportForSvr__

#ifdef HANREPORTFORSVR_EXPORTS
#define HANREPORT_API __declspec(dllexport)
#else
#define HANREPORT_API __declspec(dllimport)
#endif

#define SERVICE_NATION  0x000000FF
#define SERVICE_KOR		0x00000001
#define SERVICE_USA		0x00000002
#define SERVICE_JPN		0x00000003
#define SERVICE_CHN		0x00000004

#define SERVICE_TYPE    0x00000F00
#define SERVICE_ALPHA	0x00000100
#define SERVICE_REAL	0x00000200
#define SERVICE_BETA	0x00000300


HANREPORT_API int __stdcall HanReportInit(/*IN*/ char* szGameId,
										  /*IN*/ int   nServiceCode,
										  /*IN*/ unsigned int nConnCnt);

HANREPORT_API int __stdcall HanReportSend(/*IN*/ char* szReportString);

HANREPORT_API int __stdcall HanReportSend(/*IN*/ char* szSubject,
										  /*IN*/ char* szReportString);
#endif