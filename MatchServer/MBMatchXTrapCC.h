#ifndef _MBMATCH_XTRATCC
#define _MBMATCH_XTRATCC

#include "MBMatchUserSecurityInfo.h"
#include "./XTrap/Xtrap_S_Interface.h"
#include "MemPool.h"

#define XTRAP_CS4_FILE_COUNT			2		// 사용될 MapFile 의 개수 설정
#define MAX_XTRAP_ELAPSEDTIME			20000	// 반복 주기 20초

// Buffer Size
#define XTRAP_CS4_COMSIZE_PACKDATA					128
#define XTRAP_CS4_BUFSIZE_SESSION					320
//#define XTRAP_CS4_BUFSIZE_SESSIONADDON				80
//#define	XTRAP_CS4_BUFSIZE_MAP						13000

// Timeout Limit
#define XTRAP_CS4_MAXTIMEOUT						(int)600
#define XTRAP_CS4_MINTIMEOUT						(int)100



static unsigned char sXTrapCS_Buf[XTRAP_CS4_FILE_COUNT][XTRAP_CS4_BUFSIZE_MAP];	// MapFile용 버퍼 설정
bool LoadXTrapFile();
bool ReloadXTrapMapFile();

class MBMatchXTrapCC : public MBMatchUserSecurityInfo, public CMemPool<MBMatchXTrapCC>
{
private:

	// 20080904 X-trap 모듈업데이트 http://dev:8181/projects/gunz/ticket/175
	// 변경된 내용  [1] 서버 파일 버퍼의 싸이즈가 기존 45000 Byte에서 13000 Byte로 변경 되었습니다.
	//				[2] 세션 버퍼의 싸이즈가 기존 320 + 80*n Byte에서 320 Byte로 고정되었습니다.
	//unsigned char m_sServerBuf[XTRAP_CS4_BUFSIZE_MAP*XTRAP_CS4_FILE_COUNT]; // 밑에껄로 변경됨
	unsigned char m_sServerBuf[XTRAP_CS4_BUFSIZE_SESSION];
	unsigned char m_sComBuf[XTRAP_CS4_COMSIZE_PACKDATA];
	MUID m_UID;
	DWORD m_dwLastUpdateTime;
	
public:
	MBMatchXTrapCC(MUID uId, const DWORD dwCurTime);
	virtual ~MBMatchXTrapCC();

	DWORD XTrapSessionInit();
	DWORD XTrapGetHashData();
	DWORD XTrapCheckHashData(unsigned char *pComBuf);
	void SetXTrapLastUpdateTime(DWORD curTick)		{ m_dwLastUpdateTime = curTick; }
	DWORD GetXTrapLastUpdateTime()					{ return m_dwLastUpdateTime; }
	unsigned char *GetXTrapComBuf()					{ return m_sComBuf; }
};

#endif