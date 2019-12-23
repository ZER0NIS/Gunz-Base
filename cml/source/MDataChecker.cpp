#include "stdafx.h"
#include "MDataChecker.h"
#include <crtdbg.h>
#include "MDebug.h"
#include "MMemoryProxy.h"


//// MDataCheckNode ////
MDataCheckNode::MDataCheckNode(BYTE* pData, unsigned int nLen, MEMORYFUGITIVE_TYPE memFugitiveType)
{
	static unsigned int s_nID = 0;

	m_memFugitiveType = memFugitiveType;

	m_nID = s_nID++;

	m_pData = pData;
	m_nLen = nLen;

	UpdateChecksum();
	
	m_nLastChecksum = m_nChecksum;
}

MDataCheckNode::~MDataCheckNode()
{
	m_pData = NULL;
	m_nLen = 0;
	m_nChecksum = 0;
	m_nLastChecksum = 0;
}

bool MDataCheckNode::UpdateChecksum()
{
	m_nLastChecksum = m_nChecksum;

	m_nChecksum = 0;

	BYTE* data_ptr = NULL;

	switch (m_memFugitiveType)
	{
	case MT_MEMORYFUGITIVE_NONE :
		data_ptr = m_pData;
		break;
	case MT_MEMORYFUGITIVE_INT :
		//data_ptr = (BYTE*) &((MMemoryFugitive<int>*)m_pData)->GetData();
		break;
	case MT_MEMORYFUGITIVE_FLOAT :
		//data_ptr = (BYTE*) &((MMemoryFugitive<float>*)m_pData)->GetData();
		break;
	default:
		_ASSERT(0);
		data_ptr = m_pData;
	}

	// checksum이 너무 간단하다.. 검사 비용에 비해서 효과는; (int값 5를 뻥튀기하려면 05 00 00 00 -> 00 00 00 05면 간단히 끝. 개선필요)
	// 하지만 아이템 수치는 fugitive로 감쌌기 때문에 위치 찾기는 어려울듯
	for (unsigned int i=0; i<m_nLen; i++) {
		m_nChecksum += data_ptr[i];
	}

	if (m_nChecksum == m_nLastChecksum)
		return true;
	else
		return false;
}

//// MDataChecker ////
MDataChecker::MDataChecker()
{
	m_nTotalChecksum = 0;
	m_nLastTotalChecksum = 0;
}

MDataChecker::~MDataChecker()
{
	Clear();
}

void MDataChecker::Clear()
{
	while(m_DataCheckMap.size()) {
		MDataCheckMap::iterator i = m_DataCheckMap.begin();
		MDataCheckNode* pNode = (*i).second;
		m_DataCheckMap.erase(i);
		delete pNode;
	}
}

MDataCheckNode* MDataChecker::FindCheck(BYTE* pData)
{
	MDataCheckMap::iterator i = m_DataCheckMap.find(pData);
	if(i==m_DataCheckMap.end())
		return NULL;
	else
		return (*i).second;
}

MDataCheckNode* MDataChecker::AddCheck(BYTE* pData, unsigned int nLen, MEMORYFUGITIVE_TYPE memFugitiveType)
{
	MDataCheckNode* pNode = new MDataCheckNode(pData, nLen, memFugitiveType);
	m_DataCheckMap.insert(MDataCheckMap::value_type(pData, pNode));
	return pNode;
}

void MDataChecker::RenewCheck(BYTE* pData, unsigned int nLen)
{
	MDataCheckMap::iterator i = m_DataCheckMap.find(pData);
	if(i==m_DataCheckMap.end()) {
		_ASSERT("MDataChecker::RenewCheck() - Not existing CheckNode \n");
		return;
	}

	MDataCheckNode* pNode = (*i).second;

	if (pNode->m_pData == pData) {
		pNode->m_nLen = nLen;
		pNode->UpdateChecksum();
		pNode->Validate();
		return;
	}
}

bool MDataChecker::UpdateChecksum()
{
	m_nLastTotalChecksum = m_nTotalChecksum;

	bool bResult = true;
	m_nTotalChecksum = 0;
	for (MDataCheckMap::iterator i=m_DataCheckMap.begin(); i!=m_DataCheckMap.end(); i++) {
		MDataCheckNode* pNode = (*i).second;
		bool bResultCurrent = pNode->UpdateChecksum();
		bResult &= bResultCurrent;
		m_nTotalChecksum += pNode->GetChecksum();
		#ifdef _DEBUG
			if (bResultCurrent == false)
				mlog("MEMORYHACK: ID=%u, CurrChecksum=%u, LastChecksum=%u \n", 
					pNode->GetID(), pNode->GetChecksum(), pNode->GetLastChecksum());
		#endif
	}

	return bResult;
}

void MDataChecker::BringError()
{
	int* pError = new int;
	AddCheck((BYTE*)(pError), sizeof(int));	
	*pError = 1742;	// Leak 내버리면서 Checksum 엉망 만들기
}
