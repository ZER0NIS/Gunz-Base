#include "stdafx.h"
#include "MAsyncDBJob_InsertCharBRInfo.h"

bool MAsyncDBJob_GetCharBRInfo::Input(int nCID, int nBRID, int nBRTID)
{
	m_nCID = nCID;

	m_nBRID = nBRID;
	m_nBRTID = nBRTID;

	return true;
}

void MAsyncDBJob_GetCharBRInfo::Run(void* pContext)
{
	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	if( !pDBMgr->GetCharBRInfo(m_nCID, m_nBRID, &m_CharBRInfo) )	
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	if( m_CharBRInfo.GetBRID() == -1 )
	{
		if( !pDBMgr->InsertCharBRInfo(m_nCID, m_nBRID, m_nBRTID) )
		{
			SetResult(MASYNC_RESULT_FAILED);
			return;
		}

		m_CharBRInfo.SetBRInfo(m_nBRID, m_nBRTID, 0, 0, 0);
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}
