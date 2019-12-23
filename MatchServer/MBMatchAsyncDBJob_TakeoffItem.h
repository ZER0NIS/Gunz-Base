#pragma once


#include "MAsyncDBJob.h"


// 비종기 문제 때문에 사용하지 않고 있음.
//class MBMatchAsyncDBJob_TakeoffItem : public MAsyncJob
//{
//public : 
//	void Input( const MUID& uidPlayer
//		, const DWORD dwCID
//		, const MMatchCharItemParts Parts
//		, const MUID& uidItem
//		, const MUID& uidStage )
//	{
//		m_uidPlayer = uidPlayer;
//		m_dwCID		= dwCID;
//		m_Parts		= Parts;
//		m_uidItem	= uidItem;
//		m_uidStage	= uidStage;
//	}
//
//	void Run( void* pContext );
//
//
//	const MUID&					GetPlayerUID()	{ return m_uidPlayer; }
//	const MUID&					GetStageUID()	{ return m_uidStage; }
//	const MMatchCharItemParts	GetParts()		{ return m_Parts; }
//	const MUID&					GetItemUID()	{ return m_uidItem; }
//	const bool					GetRet()		{ return m_bRet; }
//
//
//private :
//	MBMatchAsyncDBJob_TakeoffItem() : MAsyncJob( MASYNCJOB_TAKEOFFITEM )
//	{
//		m_bRet = false;
//	}
//
//	~MBMatchAsyncDBJob_TakeoffItem()
//	{
//	}
//
//
//private :
//	MUID				m_uidPlayer;
//	DWORD				m_dwCID;
//	MMatchCharItemParts m_Parts;
//	MUID				m_uidItem;
//	MUID				m_uidStage;
//
//    bool				m_bRet;
//};