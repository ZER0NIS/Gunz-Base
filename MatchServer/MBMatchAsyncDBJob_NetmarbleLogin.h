#pragma once

#ifndef _MBMATCHASYNCDBJOB_NETMARBLELOGIN_H
#define _MBMATCHASYNCDBJOB_NETMARBLELOGIN_H

#include "MAsyncDBJob.h"

class MBMatchAsyncDBJob_NetmarbleLogin : public MAsyncJob {
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
	int		m_nAge;
	int		m_nSex;
	int		m_nCCode;
	string	m_strCountryCode3;

	int		m_nErrCode;

protected:	// Output Result
	unsigned int					m_nAID;
	MMatchAccountInfo*				m_pAccountInfo;
	MMatchAccountPenaltyInfo*		m_pAccountPenaltyInfo;
	
	char m_szDBPassword[ MAX_USER_PASSWORD_STRING_LEN ];

public:
	MBMatchAsyncDBJob_NetmarbleLogin(const MUID& uidComm) : MAsyncJob(MASYNCJOB_NETMARBLE_KOR_LOGIN, uidComm)
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

		m_nErrCode = -1;
	}

	virtual ~MBMatchAsyncDBJob_NetmarbleLogin()	
	{
		DeleteMemory();		
	}

	bool Input(MMatchAccountInfo* pNewAccountInfo,
		MMatchAccountPenaltyInfo* pNewAccountPenaltyInfo,
		const char* szUserID, 
		const char* szUniqueID,
		const int nAge, 
		const int nSex,
		const int nCCode,
		const bool bFreeLoginIP,
		unsigned long nChecksumPack,
		const bool bCheckPremiumIP,
		const char* szIP,
		DWORD dwIP,
		const string& strCountryCode3);

	virtual void Run(void* pContext);

	// output
	unsigned int	GetAID()			{ return m_nAID; }
	const char*		GetDBPassword()		{ return m_szDBPassword; }
	const MUID&		GetCommUID()		{ return m_uidComm; }
	unsigned long	GetChecksumPack()	{ return m_nChecksumPack; }
	const string&	GetCountryCode3()	{ return m_strCountryCode3; }
	int				GetErrorCode()		{ return m_nErrCode; }

	bool IsFreeLoginIP() { return m_bFreeLoginIP; }	

	MMatchAccountInfo*			GetAccountInfo()		{ return m_pAccountInfo;		}
	MMatchAccountPenaltyInfo*	GetAccountPenaltyInfo()	{ return m_pAccountPenaltyInfo; }

	void DeleteMemory() {
		if( m_pAccountInfo != NULL ) {
			delete m_pAccountInfo;
			m_pAccountInfo = NULL;
		}

		if( m_pAccountPenaltyInfo != NULL ) {
			delete m_pAccountPenaltyInfo;
			m_pAccountPenaltyInfo = NULL;
		}
	}
};

#endif
