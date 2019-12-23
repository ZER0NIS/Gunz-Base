#include "stdafx.h"
#include "MAsyncDBJob_UpdateCharItemInfoData.h"

void MAsyncDBJob_UpdateCharItemInfoData::RemoveAll()
{
	list<UpdateItem *>::iterator itBegin = m_ItemList.begin();
	for(; itBegin != m_ItemList.end(); itBegin++){
		UpdateItem *pItem = (*itBegin);
		delete pItem;
	}

	m_ItemList.clear();
}

bool MAsyncDBJob_UpdateCharItemInfoData::Input(const int nCIID, const int nChangeCnt)
{
	if( nChangeCnt == 0 ) return false;

	UpdateItem* pItem = new UpdateItem;
	
	pItem->bIsSuccess	= false;
	pItem->nCIID		= nCIID;
	pItem->nChangeCnt	= nChangeCnt;

	m_ItemList.push_back(pItem);
	return true;
}

void MAsyncDBJob_UpdateCharItemInfoData::Run(void* pContext)
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	list<UpdateItem *>::iterator itBegin = m_ItemList.begin();
	for(; itBegin != m_ItemList.end(); itBegin++)
	{
		UpdateItem *pItem = (*itBegin);

		if( pDBMgr->UpdateCharSpendItemCount(pItem->nCIID, pItem->nChangeCnt * (-1)) ) {
			pItem->bIsSuccess = true;
		}
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}
