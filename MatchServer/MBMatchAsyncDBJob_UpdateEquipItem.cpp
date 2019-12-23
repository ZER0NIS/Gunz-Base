#include "stdafx.h"
#include "MBMatchAsyncDBJob_UpdateEquipItem.h"



//void MBMatchAsyncDBJob_UpdateEquipItem::Run( void* pContext )
//{
//	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;
//
//	if( !pDBMgr->UpdateEquipedItem(m_dwCID, m_Parts, m_dwCIID, m_dwItemID, &m_bRet) )
//	{
//		SetResult( MASYNC_RESULT_FAILED );
//		return;
//	}
//
//	SetResult( MASYNC_RESULT_SUCCEED );
//}