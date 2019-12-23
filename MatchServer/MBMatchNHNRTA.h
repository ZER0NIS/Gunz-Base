#ifndef _MBMATCH_NHNRTA
#define _MBMATCH_NHNRTA


#define MAX_RTA_DELAY_TIME (1000 * 60)


class MBMatchNHNRTA
{
public :
	MBMatchNHNRTA();
	~MBMatchNHNRTA();

	bool InitRTA( const char* szServerName, const int nNHNServerMode );
	bool InitRTAReal( const char* szServerName );
	bool InitRTAAlpha( const char* szServerName );
	bool RTA( const int nPlayerCount, const DWORD dwCurTime );
	bool IsElapsed( const DWORD dwCurTime );


	static MBMatchNHNRTA& GetInstance()
	{
		static MBMatchNHNRTA rta;
		return rta;
	}

private :
	DWORD	m_dwLastRTATime;
	char	m_szServerName[ 64 ];
};


MBMatchNHNRTA& GetNHNRTA();


#endif