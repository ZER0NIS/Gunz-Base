#pragma once


#include "MMatchAuth.h"
#include <string.h>

#ifndef NEW_AUTH_MODULE

class MBMatchAuthInfo : public MMatchAuthInfo {
protected:
	char	m_szUserID[MAUTHINFO_BUFLEN];
	char	m_szUniqueID[MAUTHINFO_BUFLEN];
	int		m_nAge;
	int		m_nSex;
	int		m_nCCode;

public:
	MBMatchAuthInfo()
	{
		memset(m_szUserID,		0, sizeof(MAUTHINFO_BUFLEN));
		memset(m_szUniqueID,	0, sizeof(MAUTHINFO_BUFLEN));

		m_nAge = 0;
		m_nSex = 0;
		m_nCCode = 0;
	}

	virtual ~MBMatchAuthInfo()	{}


	virtual const char* GetUserID()			{ return m_szUserID; }
	virtual const char* GetUniqueID()		{ return m_szUniqueID; }
	virtual int GetSex()					{ return m_nSex; }
	virtual int GetCCode()					{ return m_nCCode; }
	virtual int GetAge()					{ return m_nAge; }

	void SetUserID(const char* pszVal)		{ strcpy(m_szUserID, pszVal); }
	void SetUniqueID(const char* pszVal)	{ strcpy(m_szUniqueID, pszVal); }	
	void SetSex(int nSex)					{ m_nSex = nSex; }
	void SetCCode(int nCCode)				{ m_nCCode = nCCode; }
	void SetAge(int nAge)					{ m_nAge = nAge; }
};

class MBMatchAuthBuilder : public MMatchAuthBuilder {
public:
	MBMatchAuthBuilder()				{}
	virtual ~MBMatchAuthBuilder()		{}

};

#else 

#include "./Netmarble/NMAuthLib.h"	// 넷마블 제공 인증 헤더

class MBMatchNetmarbleAuthQuery
{
public:
	MUID m_commUid;
	std::string m_strAuth;
	std::string m_strData;
	std::string m_strCp;
	bool m_bFreeLoginIP;
	unsigned long m_nChecksumPack;
	bool m_bCheckPremiumIP;
	std::string m_strIP;
	DWORD m_dwIP;
	std::string m_strCountryCode3;

	MBMatchNetmarbleAuthQuery(
		const MUID& commUid, const char* szAuth, const char* szData, const char* szCp,
		bool bFreeLoginIP,
		unsigned long nChecksumPack,
		bool bCheckPremiumIP,
		const char* szIP,
		DWORD dwIP,
		const char* szCountryCode3
		)
		: m_commUid(commUid), m_strAuth(szAuth), m_strData(szData), m_strCp(szCp)
		, m_bFreeLoginIP(bFreeLoginIP)
		, m_nChecksumPack(nChecksumPack)
		, m_bCheckPremiumIP(bCheckPremiumIP)
		, m_strIP(szIP)
		, m_dwIP(dwIP)
		, m_strCountryCode3(szCountryCode3)
		{}
};

class MBMatchNetmarbleModule {

	typedef std::list<MBMatchNetmarbleAuthQuery*> ListQuery;
	typedef ListQuery::iterator ItorQuery;

	ListQuery m_queries;
	CRITICAL_SECTION m_cs;
	HANDLE m_hThread;

	CNMAuth	m_NMAuth;

	static DWORD WINAPI WorkerThread(LPVOID pJobContext);

public:

	MBMatchNetmarbleModule();
	~MBMatchNetmarbleModule();

	bool InitModule();
	void DestroyModule();

	void ErrorLog();	// GetLastError()를 통해 에러 출력
	void ErrorLog(ERROR_NMAUTH error, const char* szMsg);
	
	static MBMatchNetmarbleModule& GetInstance() 
	{
		static MBMatchNetmarbleModule module;

		return module;
	}

	void RequestAuth(const MUID& commUid, const char* szAuth, const char* szData, const char* szCp,
		bool bFreeLoginIP,
		unsigned long nChecksumPack,
		bool bCheckPremiumIP,
		const char* szIP,
		DWORD dwIP,
		const char* szCountryCode3);
private:
	MBMatchNetmarbleAuthQuery* GetNextQuery();

};

MBMatchNetmarbleModule& MGetNetmarbleModule(); 

#endif