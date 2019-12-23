#ifndef _MBMATCHUSERSECURITYINFO
#define _MBMATCHUSERSECURITYINFO


#include "MUID.h"


#include <utility>
using std::pair;



class MBMatchUserSecurityInfo
{
public :
	MBMatchUserSecurityInfo(); // 기본생성자로 생성할 수 없게 하기위해 바디는 만들지 않았습니다. - by sunge
	MBMatchUserSecurityInfo( const DWORD dwCurTime ) 
	{
		m_dwLastCheckTime = dwCurTime;
	}

	virtual ~MBMatchUserSecurityInfo() {}

	const DWORD GetLastCheckTime() { return m_dwLastCheckTime; }
	
	void UpdateLastCheckTime( const DWORD dwLastCheckTime ) { m_dwLastCheckTime = dwLastCheckTime; }

protected :
	DWORD m_dwLastCheckTime;
};


typedef pair< MUID, MBMatchUserSecurityInfo* > UserSecurityInfoMgrPair;



#endif