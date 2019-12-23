#ifndef _MBMATCHGGSVRREF
#define _MBMATCHGGSVRREF



// #include ".\\GameGuard\\ggsrv.h"
#include ".\\GameGuard\\ggsrv25.h"
#include ".\\GameGuard\\ggerror.h"



class MBMatchGGSvrRef
{
private :
	CCSAuth2 m_ServerAuth;

public :
	const GG_AUTH_DATA& GetAuthQuery() const { return m_ServerAuth.m_AuthQuery; }
	const DWORD CreateAuthQuery() { return m_ServerAuth.GetAuthQuery(); }
	void Init() { m_ServerAuth.Init(); }
	void Close() { m_ServerAuth.Close(); }
	const DWORD CheckAuthAnswer() { return m_ServerAuth.CheckAuthAnswer(); }
	void SetAuthAnswer( const DWORD dwIndex, const DWORD dwValue1, const DWORD dwValue2, const DWORD dwValue3 )
	{
		m_ServerAuth.m_AuthAnswer.dwIndex	= dwIndex;
		m_ServerAuth.m_AuthAnswer.dwValue1	= dwValue1;
		m_ServerAuth.m_AuthAnswer.dwValue2	= dwValue2;
		m_ServerAuth.m_AuthAnswer.dwValue3	= dwValue3;
	}
};



#endif