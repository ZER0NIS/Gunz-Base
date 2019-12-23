#include "stdafx.h"
#include "MBMatchSecurity.h"
#include "MDebug.h"
#include <mmsystem.h>



MBMatchUserSecurityMgr::MBMatchUserSecurityMgr( const MBMatchSecurityID ID, const string strName, const DWORD dwMaxElapsedTime ) 
: m_strName( strName )
{
	m_ID = ID;
	m_dwMaxElapsedTime = 0;
}


MBMatchUserSecurityMgr::~MBMatchUserSecurityMgr()
{
	Release();
}


MBMatchUserSecurityInfo* MBMatchUserSecurityMgr::GetSecurityInfo( const MUID& uidUser )
{
	iterator itFind = find( uidUser );
	if( end() == itFind )
		return 0;

	return itFind->second;
}


bool MBMatchUserSecurityMgr::AddUserSecurityInfo( const MUID uidUser, MBMatchUserSecurityInfo* pInfo )
{
	if( end() != find(uidUser) )
		return false;

	insert( UserSecurityInfoMgrPair(uidUser, pInfo) );
	return true;
}


void MBMatchUserSecurityMgr::Delete( const MUID& uidUser )
{
	MBMatchUserSecurityInfo* pSecurityInfo = GetSecurityInfo( uidUser );
	if( 0 != pSecurityInfo )
		delete pSecurityInfo;

	erase( uidUser );
}


void MBMatchUserSecurityMgr::Release()
{
	iterator itBegin, itEnd;
	itBegin = begin();
	itEnd = end();
	for( ; itBegin != itEnd; ++itBegin )
	{
		delete itBegin->second;
	}
	clear();
}


void MBMatchUserSecurityMgr::DumpZombi( const DWORD dwCurTime )
{
	mlog( "----------- Security manager : zombi dump -----------\n" );
	mlog( "Name : %s\n\n", m_strName.c_str() );
	iterator itBegin, itEnd;
	itBegin = begin();
	itEnd = end();
	for( ; itBegin != itEnd; ++itBegin )
	{
		if( m_dwMaxElapsedTime < (dwCurTime - itBegin->second->GetLastCheckTime()) )
			mlog( "elasped time : %f\n", (dwCurTime - itBegin->second->GetLastCheckTime()) / 1000.0f );
	}
	mlog( "-----------------------------------------------------\n" );
}


// ---------------------------------------------------------------------------------------------------


MBMatchSecurity::MBMatchSecurity()
{
}


MBMatchSecurity::~MBMatchSecurity()
{
	Release();
}


bool MBMatchSecurity::AddManager( const MBMatchSecurityID ID, MBMatchUserSecurityMgr* pMgr )
{
	if( end() != find(ID) )
		return false;

	if( 0 == pMgr )
		return false;

	insert( SecurityMgrMap(ID, pMgr) );

	return true;
}


MBMatchUserSecurityMgr* MBMatchSecurity::GetManager( const MBMatchSecurityID ID )
{
	iterator itFind = find( ID );
	if( end() == itFind )
		return 0;

	return itFind->second;
}


void MBMatchSecurity::Delete( const MBMatchSecurityID ID )
{
	MBMatchUserSecurityMgr* pMgr = GetManager( ID );
	if( 0 != pMgr )
		delete pMgr;

	erase( ID );
}


void MBMatchSecurity::Release()
{
	const DWORD dwCurTime = timeGetTime();

	iterator itBegin, itEnd;
	itBegin = begin();
	itEnd = end();

	for( ; itBegin != itEnd; ++itBegin )
	{
		itBegin->second->DumpZombi( dwCurTime );
		delete itBegin->second;
	}

	clear();
}