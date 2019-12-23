#pragma once

#include "MAsyncDBJob.h"

class MBMatchAsyncDBJob_GambleQuestItem : public MAsyncJob
{

private :
	// Input
	MUID	m_uidPlayer;
	MUID	m_uidGItem;
	int m_nCID;	
	int m_nCIID;
	int m_nGRIID;
	int m_nRIID;

public :
	MBMatchAsyncDBJob_GambleQuestItem(const MUID& uidOwner) : MAsyncJob( MASYNCJOB_GAMBLE_QUESTITEM, uidOwner )
	{
	}

	~MBMatchAsyncDBJob_GambleQuestItem()
	{
	}

	void Input( const MUID& uidPlayer
		, MUID& uidGItem
		, int nCID		
		, int nCIID
		, int nGRIID
		, int nRIID)
	{
		m_uidPlayer		= uidPlayer;
		m_uidGItem		= uidGItem;
		m_nCID			= nCID;		
		m_nCIID			= nCIID;
		m_nGRIID		= nGRIID;
		m_nRIID			= nRIID;
	}

	void Run( void* pContext );

	MUID& GetPlayerUID()	{ return m_uidPlayer; }
	MUID& GetGItemUID()		{ return m_uidGItem; }
	int GetCIID()			{ return m_nCIID; }	
	int GetRIID()			{ return m_nRIID; }
};