#pragma once
#include "time.h"


//////////////////////////////////////////////////////
// Gamechu BaseUserinfo sturct
struct gcBaseUserInfo
{
	wchar_t		szUserSerialNumber[64];
	wchar_t		szNickName[64];
	__time64_t  ttTimeStamp;
	int			nPCBang;
	int			nSex;
	int			nAge;
	int			nLocation;
	int			nBirthYear;
};

typedef int (*TF_CheckCertification)( const wchar_t*,
									  const wchar_t*, 
									  const wchar_t*, 
									  gcBaseUserInfo* , 
									  DWORD, 
									  BOOL);


class GCCertificaltionHelper
{
public:
	GCCertificaltionHelper() 
	{ 
		m_hInstGCLauncherDLL   = NULL; 
		m_pfCheckCertification = NULL;
	}
	
	~GCCertificaltionHelper()
	{ 
		if(m_hInstGCLauncherDLL)
		{
			::FreeLibrary(m_hInstGCLauncherDLL); 

			m_hInstGCLauncherDLL   = NULL;
			m_pfCheckCertification = NULL;
		}
	}

protected:
	HINSTANCE			  m_hInstGCLauncherDLL;
	TF_CheckCertification m_pfCheckCertification;
	

public:

	BOOL Load_GamechuDLL(LPTSTR szFileName) /* Load DLL and Set function */ 
	{
		m_hInstGCLauncherDLL = LoadLibrary(szFileName);
		if( m_hInstGCLauncherDLL == NULL ) 
			return FALSE;		

		m_pfCheckCertification = (TF_CheckCertification)GetProcAddress(m_hInstGCLauncherDLL, "CheckCertification" );

		return TRUE;
	}

	//////////////////////////////////////////////////////
	// GetUserCertification
	// --------------------
	// szString    :
	// szServerKey : using MD5 key
	// pUserInfo   :
	//
	// return value --------------------------------------
	// return 0: success Certification
	// return other value : Error Certification 
	//          1 : md5 value error
	//         10 : TimeStamp over
	//        100 : Parsing Error
	//        101 : UserINfo Error
	//        102 : InputParam Error
	//       1000 : Unknown Error
	// ---------------------------------------------------
	int  CheckCertification( const wchar_t* szString, 
							 const wchar_t* szStatIndex,
							 const wchar_t* szServerKey,
							 gcBaseUserInfo* pUserInfo,
							 DWORD dTimeoutInterval)
	{
		if ( m_pfCheckCertification != NULL )
			return m_pfCheckCertification(szString, szStatIndex, szServerKey, pUserInfo, dTimeoutInterval, FALSE);

		return 1000;
	}
};
