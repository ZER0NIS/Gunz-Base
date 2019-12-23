#pragma once


#include "MAsyncDBJob.h"
#include "MMatchObject.h"


#include <vector>

using std::vector;


typedef vector<unsigned long int> ExpiredItemVec;


class MAsyncDBJob_GetAccountItemList : public MAsyncJob
{
public :
	MAsyncDBJob_GetAccountItemList(const MUID& uidOwner) : MAsyncJob( MASYNCJOB_GETACCOUNTITEMLIST, uidOwner ){}

	~MAsyncDBJob_GetAccountItemList(){}

	void Input( const MUID& uidPlayer, const DWORD dwAID )
	{
		m_uidPlayer	= uidPlayer;
		m_dwAID		= dwAID;
	}
	
	void Run( void* pContext );

	const MUID&			GetPlayerUID()				{ return m_uidPlayer; }
	MAccountItemNode*	GetAccountItemList()		{ return m_AccountItems; }
	const int			GetAccountItemCount()		{ return m_nItemCount; }
	ExpiredItemVec&		GetExpiredAccountItems()	{ return m_vExpiredItems; }

private :
	MUID	m_uidPlayer;
	DWORD	m_dwAID;

	MAccountItemNode	m_AccountItems[MAX_ACCOUNT_ITEM];
	int					m_nItemCount;
	ExpiredItemVec		m_vExpiredItems;
};