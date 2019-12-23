#ifndef _MBMATCHSECURITY
#define _MBMATCHSECURITY


#include "MBMatchGameGuard.h"
#include "MBMatchXTrapCC.h"
#include "MBMatchUserSecurityInfo.h"
#include "MUID.h"


#include <map>
#include <string>
#include <utility>
using std::map;
using std::string;


enum MBMatchSecurityID
{
	MSI_GAMEGUARD = 1,
	MSI_XTRAP = 2,
	MSI_END,
};



class MBMatchUserSecurityMgr : public map< MUID, MBMatchUserSecurityInfo* >
{
public :
	MBMatchUserSecurityMgr( const MBMatchSecurityID ID, const string strName, const DWORD dwMaxElapsedTime );
	virtual ~MBMatchUserSecurityMgr();

	virtual MBMatchUserSecurityInfo* GetSecurityInfo( const MUID& uidUser );
	virtual bool AddUserSecurityInfo( const MUID uidUser, MBMatchUserSecurityInfo* pInfo );
	virtual void Delete( const MUID& uidUser );
	virtual void Release();
	
	const MBMatchSecurityID GetID()		{ return m_ID; }
	const string& GetName()				{ return m_strName; }

	void DumpZombi( const DWORD dwCurTime );

private :
	MBMatchUserSecurityMgr();

private :
	MBMatchSecurityID	m_ID;
	string				m_strName;
	DWORD				m_dwMaxElapsedTime;
};



typedef pair< MBMatchSecurityID, MBMatchUserSecurityMgr* > SecurityMgrMap;



class MBMatchSecurity : public map< MBMatchSecurityID, MBMatchUserSecurityMgr* >
{
public :
	friend class MBMatchServer;

	~MBMatchSecurity();


	bool AddManager( const MBMatchSecurityID ID, MBMatchUserSecurityMgr* pMgr );
	MBMatchUserSecurityMgr* GetManager( const MBMatchSecurityID ID );
	void Delete( const MBMatchSecurityID ID );
	void Release();
	
private :
	MBMatchSecurity();
	
private :
	SecurityMgrMap m_SecurityMgrMap;
};


#endif