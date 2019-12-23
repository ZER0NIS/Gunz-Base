#include "stdafx.h"
#include "MAsyncDBJob_GetBRDescription.h"

void MAsyncDBJob_GetBattleTimeRewardDescription::Run(void* pContext)
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if ( !pDBMgr->GetBattletimeRewardList(m_vBattletimeRewardDescription) )
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	if ( !pDBMgr->GetBattletimeRewardItemList(m_vBattletimeRewardItem) )
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}


	SetResult(MASYNC_RESULT_SUCCEED);
}
