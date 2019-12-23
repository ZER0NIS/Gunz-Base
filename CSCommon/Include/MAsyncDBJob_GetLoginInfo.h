#ifndef _MASYNCDBJOB_GETLOGININFO_H
#define _MASYNCDBJOB_GETLOGININFO_H

#include "MAsyncDBJob.h"

class MAsyncDBJob_GetLoginInfo : public MAsyncJob {
protected:
	MUID			m_uidComm;
	bool			m_bFreeLoginIP;
	unsigned long	m_nChecksumPack;
	bool			m_bCheckPremiumIP;
	char			m_szIP[128];
	DWORD			m_dwIP;

protected:	// Input Argument
	char	m_szUserID[ MAX_USERID_STRING_LEN ];
	char	m_szUniqueID[1024];
	char	m_szCertificate[1024];
	char	m_szName[ USERNAME_STRING_LEN ];
	int		m_nAge;
	int		m_nSex;
	string	m_strCountryCode3;

protected:	// Output Result
	MMatchAccountInfo*			m_pAccountInfo;
	MMatchAccountPenaltyInfo*	m_pAccountPenaltyInfo;

	unsigned int		m_nAID;
	char				m_szDBPassword[ MAX_USER_PASSWORD_STRING_LEN ];
public:
	MAsyncDBJob_GetLoginInfo(const MUID& uidComm)
		: MAsyncJob(MASYNCJOB_GETLOGININFO, uidComm)
	{
		m_uidComm = uidComm;
		m_pAccountInfo = NULL;
		m_nAID = 0;
		m_szDBPassword[0] = 0;
		m_bFreeLoginIP = false;
		m_nChecksumPack = 0;
		m_bCheckPremiumIP = false;
		m_szIP[0] = 0;
		m_dwIP = 0xFFFFFFFF;
	}
	virtual ~MAsyncDBJob_GetLoginInfo()	
	{
		DeleteMemory();		
	}

	bool Input(MMatchAccountInfo* pNewAccountInfo,
				MMatchAccountPenaltyInfo* pNewAccountPenaltyInfo,
				const char* szUserID, 
				const char* szUniqueID, 
				const char* szCertificate, 
				const char* szName, 
				const int nAge, 
				const int nSex,
				const bool bFreeLoginIP,
				unsigned long nChecksumPack,
				const bool bCheckPremiumIP,
				const char* szIP,
				DWORD dwIP,
				const string& strCountryCode3);

	virtual void Run(void* pContext);

	// output	
	unsigned int GetAID() { return m_nAID; }
	const char* GetDBPassword() { return m_szDBPassword; }
	const MUID& GetCommUID() { return m_uidComm; }
	bool IsFreeLoginIP() { return m_bFreeLoginIP; }
	unsigned long GetChecksumPack() { return m_nChecksumPack; }
	const string& GetCountryCode3() { return m_strCountryCode3; }

	MMatchAccountInfo*			GetAccountInfo()		{ return m_pAccountInfo;		}
	MMatchAccountPenaltyInfo*	GetAccountPenaltyInfo()	{ return m_pAccountPenaltyInfo; }

	void DeleteMemory() 
	{
		if( m_pAccountInfo != NULL ) {
			delete m_pAccountInfo;
			m_pAccountInfo = NULL;
		}

		if( m_pAccountPenaltyInfo != 0 ) {
			delete m_pAccountPenaltyInfo;
			m_pAccountPenaltyInfo = NULL;
		}
	}
};
#endif