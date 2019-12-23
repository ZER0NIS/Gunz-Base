#pragma once


#include "MAsyncDBJob.h"


// 비종기 문제 때문에 사용하지 않고 있음.
//class MBMatchAsyncDBJob_UpdateEquipItem : public MAsyncJob
//{
//public :
//	void Input( const MUID& uidPlayer
//		, const DWORD dwCID
//		, const MUID& uidItem
//		, const MMatchCharItemParts Parts
//		, const DWORD dwCIID
//		, const DWORD dwItemID
//		, const MUID& uidStage )
//	{
//		m_uidPlayer = uidPlayer;
//		m_dwCID		= dwCID;
//		m_uidItem	= uidItem;
//		m_Parts		= Parts;
//		m_dwCIID	= dwCIID;
//		m_dwItemID = dwItemID;
//		m_uidStage = uidStage;
//	}
//
//
//	void Run( void* pContext );
//
//
//	const MUID&					GetPlayerUID()	{ return m_uidPlayer; }
//	const MUID&					GetItemUID()	{ return m_uidItem; }
//	const MUID&					GetStageUID()	{ return m_uidStage; }
//	const MMatchCharItemParts	GetParts()		{ return m_Parts; }
//	const bool					GetRet()		{ return m_bRet; }
//
//
//private :
//	MBMatchAsyncDBJob_UpdateEquipItem() : MAsyncJob( MASYNCJOB_UPDATEEQUIPITEM )
//	{
//		m_bRet = false;
//	}
//	
//	~MBMatchAsyncDBJob_UpdateEquipItem()
//	{
//	}
//
//
//private :
//	MUID				m_uidPlayer;
//	DWORD				m_dwCID;
//	MUID				m_uidItem;
//	MMatchCharItemParts m_Parts;
//	DWORD				m_dwCIID;
//	DWORD				m_dwItemID;
//	MUID				m_uidStage;
//
//	bool				m_bRet;
//};