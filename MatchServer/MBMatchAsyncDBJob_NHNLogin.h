#include "MAsyncDBJob.h"

#include <string>
using std::string;


struct MMatchAccountInfo;


class MBMatchAsyncDBJob_NHNLogin : public MAsyncJob
{
public :
	MBMatchAsyncDBJob_NHNLogin() : MAsyncJob( MASYNCJOB_NHNLOGIN, MUID(0, 0) )
	{
		m_bIsUseNHNUSAAuth = true;
	}

	~MBMatchAsyncDBJob_NHNLogin();

	bool Input( const MUID& uidCommUID, const string& strUserID, const string& strAuth, const int nCheckSum, 
		const bool bIsFreeLoginIP, const string strCountryCode3, const bool bIsUseNHNUSAAuth, const int nServerID );
	void Run( void* pContext );

	const MUID&			GetUID()			{ return m_CommUID; }
	const unsigned int	GetAID()			{ return m_nAID; }
	const int			GetCheckSumPak()	{ return m_nCheckSum; }
	const bool			IsFreeLoginIP()		{ return m_bIsFreeLoginIP; }
	const string&		GetCountryCode3()	{ return m_strCountryCode3; }
	const string&		GetAuthString()		{ return m_strAuth; }

	MMatchAccountInfo*			GetAccountInfo()		{ return m_pAccountInfo;		}
	MMatchAccountPenaltyInfo*	GetAccountPenaltyInfo()	{ return m_pAccountPenaltyInfo; }

	void DeleteMemory();

private :
	bool	m_bIsUseNHNUSAAuth;
	MUID	m_CommUID;
	string	m_strUserID;
	string	m_strAuth;
	int		m_nCheckSum;
	bool	m_bIsFreeLoginIP;
	string	m_strCountryCode3;
	int		m_nServerID;

	unsigned int				m_nAID;
	MMatchAccountInfo*			m_pAccountInfo;
	MMatchAccountPenaltyInfo*	m_pAccountPenaltyInfo;
};