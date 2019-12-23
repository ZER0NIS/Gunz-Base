#include "stdafx.h"
#include "ZNPCInfoFromServer.h"

ZNPCInfoFromServerManager::ZNPCInfoFromServerManager() 
{
}


ZNPCInfoFromServerManager::~ZNPCInfoFromServerManager()
{
	Clear();
}


const MTD_NPCINFO* ZNPCInfoFromServerManager::GetNPCInfo( const BYTE nNPCID )
{
	ZNPCInfoFromServerManager::const_iterator itFind = find( nNPCID );
	if( end() == itFind )
	{
		return NULL;
	}

	return itFind->second;
}


void ZNPCInfoFromServerManager::Clear()
{
	ZNPCInfoFromServerManager::const_iterator	itEnd	= end();
	ZNPCInfoFromServerManager::iterator			It		= begin();

	for( ; itEnd != It; ++It )
	{
		delete It->second;
	}

	clear();
}


bool ZNPCInfoFromServerManager::CreateNPCInfo( const MTD_NPCINFO* pNPCInfo )
{
	if( NULL == pNPCInfo )
	{
		return false;
	}

	//_ASSERTE( end() == find(pNPCInfo->m_nNPCTID) && "NPC타입이 중복되면 안된다." );

	MTD_NPCINFO* pNewNPCInfo = new MTD_NPCINFO;
	if( NULL == pNewNPCInfo )
	{
		return false;
	}

	pNewNPCInfo->m_nNPCTID			= pNPCInfo->m_nNPCTID;

	pNewNPCInfo->m_nMaxHP			= pNPCInfo->m_nMaxHP;
	pNewNPCInfo->m_nMaxAP			= pNPCInfo->m_nMaxAP;
	pNewNPCInfo->m_nInt				= pNPCInfo->m_nInt;
	pNewNPCInfo->m_nAgility			= pNPCInfo->m_nAgility;
	pNewNPCInfo->m_fAngle			= pNPCInfo->m_fAngle;
	pNewNPCInfo->m_fDyingTime		= pNPCInfo->m_fDyingTime;

	pNewNPCInfo->m_fCollisonRadius	= pNPCInfo->m_fCollisonRadius;
	pNewNPCInfo->m_fCollisonHight	= pNPCInfo->m_fCollisonHight;

	pNewNPCInfo->m_nAttackType		= pNPCInfo->m_nAttackType;
	pNewNPCInfo->m_fAttackRange		= pNPCInfo->m_fAttackRange;
	pNewNPCInfo->m_nWeaponItemID	= pNPCInfo->m_nWeaponItemID;
	pNewNPCInfo->m_fDefaultSpeed	= pNPCInfo->m_fDefaultSpeed;

	insert( ZNPCInfoFromServerManager::value_type(pNewNPCInfo->m_nNPCTID, pNewNPCInfo) );

	return true;
}