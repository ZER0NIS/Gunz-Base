#include "stdafx.h"
#include "MCommand.h"
#include "MCommandManager.h"
#include "MDebug.h"

MCommandDesc::MCommandDesc(int nID, const char* szName, const char* szDescription, int nFlag)
{
	m_nID = nID;
	strcpy(m_szName, szName);
	strcpy(m_szDescription, szDescription);
	m_nFlag = nFlag;
}

MCommandDesc::~MCommandDesc(void)
{
	for (int i = 0; i < (int)m_ParamDescs.size(); i++) {
		delete m_ParamDescs[i];
	}
	m_ParamDescs.clear();
}

void MCommandDesc::AddParamDesc(MCommandParameterDesc* pParamDesc)
{
	m_ParamDescs.push_back(pParamDesc);
}

bool MCommandDesc::IsFlag(int nFlag) const
{
	return ((m_nFlag & nFlag) == nFlag);
}

MCommandDesc* MCommandDesc::Clone()
{
	MCommandDesc* pNewDesc = new MCommandDesc(m_nID, m_szName, m_szDescription, m_nFlag);
	for (int i = 0; i < GetParameterDescCount(); i++)
	{
		MCommandParameterDesc* pSrcParamDesc = GetParameterDesc(i);
		MCommandParameterDesc* pParamDesc = new MCommandParameterDesc(pSrcParamDesc->GetType(), (char*)pSrcParamDesc->GetDescription());
		pNewDesc->AddParamDesc(pSrcParamDesc);
	}

	return pNewDesc;
}
void MCommand::Reset(void)
{
	m_pCommandDesc = NULL;
	m_nSerialNumber = 0;
	m_dwPostTime = GetTickCount(); // Custom: Command post time
	m_nCommandType = MSGID_COMMAND;
	m_Sender.SetZero();
	m_Receiver.SetZero();
	ClearParam();
}

void MCommand::ClearParam(void)
{
	const int nParamCount = GetParameterCount();
	for (int i = 0; i < nParamCount; ++i) {
		delete m_Params[i];
	}
	m_Params.clear();
}

void MCommand::ClearParam(int i)
{
	delete m_Params[i];
	m_Params.erase(m_Params.begin() + i);
}

MCommand::MCommand(void)
{
	Reset();
}

MCommand::MCommand(const MCommandDesc* pCommandDesc, MUID Receiver, MUID Sender)
{
	Reset();
	SetID(pCommandDesc);
	m_Receiver = Receiver;
	m_Sender = Sender;
}

MCommand::MCommand(int nID, MUID Sender, MUID Receiver, MCommandManager* pCommandManager)
{
	Reset();
	SetID(nID, pCommandManager);
	m_Sender = Sender;
	m_Receiver = Receiver;
}

MCommand::~MCommand(void)
{
	ClearParam();
}

void MCommand::SetID(const MCommandDesc* pCommandDesc)
{
	m_pCommandDesc = pCommandDesc;
	m_Params.reserve(pCommandDesc->GetParameterDescCount());
}

void MCommand::SetID(int nID, MCommandManager* pCommandManager)
{
	m_pCommandDesc = pCommandManager->GetCommandDescByID(nID);
	m_Params.reserve(m_pCommandDesc->GetParameterDescCount());
}

bool MCommand::AddParameter(MCommandParameter* pParam)
{
	int nCount = (int)m_Params.size();
	int nParamDescCount = m_pCommandDesc->GetParameterDescCount();

	if (nCount >= nParamDescCount) return false;

	MCommandParameterDesc* pParamDesc = m_pCommandDesc->GetParameterDesc(nCount);
	if (pParam->GetType() != pParamDesc->GetType()) return false;

	m_Params.push_back(pParam);

	return true;
}

int MCommand::GetParameterCount(void) const
{
	return (int)m_Params.size();
}

MCommandParameter* MCommand::GetParameter(int i) const
{
	if (i < 0 || i >= (int)m_Params.size()) return NULL;

	return m_Params[i];
}

bool MCommand::GetParameter(void* pValue, int i, MCommandParameterType t, int nBufferSize) const
{
	if (0 == pValue) return false;

	MCommandParameter* pParam = GetParameter(i);
	if (pParam == NULL) return false;

	if (pParam->GetType() != t) return false;

#ifdef _DEBUG
	if (pParam->GetType() == MPT_STR && nBufferSize < 0) {
	}
#endif

	if (pParam->GetType() == MPT_STR && nBufferSize >= 0) {
		char* szParamString = *(char**)pParam->GetPointer();
		if (0 == szParamString)
		{
			ASSERT(0 && "NULL ������ ��Ʈ��");
			strcpy((char*)pValue, "\0");
			return true;
		}

		int nLength = (int)strlen(szParamString);
		if (nLength >= nBufferSize - 1) {
			strncpy((char*)pValue, szParamString, nBufferSize - 1);
			((char*)pValue)[nBufferSize - 1] = 0;
		}
		else {
			pParam->GetValue(pValue);
		}
	}
	else {
		pParam->GetValue(pValue);
	}

	return true;
}

MCommand* MCommand::Clone(void) const
{
	if (m_pCommandDesc == NULL) return NULL;
	MCommand* pClone = new MCommand(m_pCommandDesc, m_Receiver, m_Sender);
	if (0 == pClone) return NULL;
	const int nParamCount = GetParameterCount();
	for (int i = 0; i < nParamCount; ++i) {
		MCommandParameter* pParameter = GetParameter(i);
		if (pClone->AddParameter(pParameter->Clone()) == false) {
			delete pClone;
			return NULL;
		}
	}

	return pClone;
}

bool MCommand::CheckRule(void)
{
	if (m_pCommandDesc == NULL) return false;

	int nCount = GetParameterCount();
	if (nCount != m_pCommandDesc->GetParameterDescCount()) return false;

	for (int i = 0; i < nCount; ++i) {
		MCommandParameter* pParam = GetParameter(i);
		MCommandParameterDesc* pParamDesc = m_pCommandDesc->GetParameterDesc(i);
		if (pParam->GetType() != pParamDesc->GetType()) return false;

		if (pParamDesc->HasConditions())
		{
			for (int j = 0; j < pParamDesc->GetConditionCount(); j++)
			{
				MCommandParamCondition* pCondition = pParamDesc->GetCondition(j);
				if (!pCondition->Check(pParam))
				{
					mlog("Cmd Param Condition Check Error(CMID = %d)\n", m_pCommandDesc->GetID());
					return false;
				}
			}
		}
	}

	return true;
}

int MCommand::GetData(char* pData, int nSize)
{
	if (m_pCommandDesc == NULL) return 0;

	int nParamCount = GetParameterCount();

	unsigned short int nDataCount = sizeof(nDataCount);

	unsigned short int nCommandID = m_pCommandDesc->GetID();
	memcpy(pData + nDataCount, &(nCommandID), sizeof(nCommandID));
	nDataCount += sizeof(nCommandID);

	memcpy(pData + nDataCount, &(m_nSerialNumber), sizeof(m_nSerialNumber));
	nDataCount += sizeof(m_nSerialNumber);

	for (int i = 0; i < nParamCount; ++i) {
		MCommandParameter* pParam = GetParameter(i);
		nDataCount += pParam->GetData(pData + nDataCount, nSize - nDataCount);
	}

	memcpy(pData, &nDataCount, sizeof(nDataCount));

	return nDataCount;
}

bool MCommand::SetData(char* pData, MCommandManager* pCM, unsigned short nDataLen)
{
	Reset();

	unsigned short int nDataCount = 0;

	unsigned short nTotalSize = 0;
	memcpy(&nTotalSize, pData, sizeof(nTotalSize));

	if ((nDataLen != USHRT_MAX) && (nDataLen != nTotalSize)) return false;

	nDataCount += sizeof(nTotalSize);

	unsigned short int nCommandID = 0;
	memcpy(&nCommandID, pData + nDataCount, sizeof(nCommandID));
	nDataCount += sizeof(nCommandID);

	MCommandDesc* pDesc = pCM->GetCommandDescByID(nCommandID);
	if (pDesc == NULL)
	{
		return false;
	}
	SetID(pDesc);

	memcpy(&m_nSerialNumber, pData + nDataCount, sizeof(m_nSerialNumber));
	nDataCount += sizeof(m_nSerialNumber);

	int nParamCount = pDesc->GetParameterDescCount();

	for (int i = 0; i < nParamCount; ++i) {
		MCommandParameterType nParamType = pDesc->GetParameterType(i);

		MCommandParameter* pParam = NULL;
		switch (nParamType) {
		case MPT_INT:
			pParam = new MCommandParameterInt;
			break;
		case MPT_UINT:
			pParam = new MCommandParameterUInt;
			break;
		case MPT_FLOAT:
			pParam = new MCommandParameterFloat;
			break;
		case MPT_STR:
			pParam = new MCommandParameterString;
			{
				unsigned short checkSize = 0;
				memcpy(&checkSize, pData + nDataCount, sizeof(checkSize));
				if (checkSize > nDataLen || checkSize == 0)
				{
					return false;
				}
			}
			break;
		case MPT_VECTOR:
			pParam = new MCommandParameterVector;
			break;
		case MPT_POS:
			pParam = new MCommandParameterPos;
			break;
		case MPT_DIR:
			pParam = new MCommandParameterDir;
			break;
		case MPT_BOOL:
			pParam = new MCommandParameterBool;
			break;
		case MPT_COLOR:
			pParam = new MCommandParameterColor;
			break;
		case MPT_UID:
			pParam = new MCommandParameterUID;
			break;
		case MPT_BLOB:
			pParam = new MCommandParameterBlob;
			{
				unsigned int checkSize = 0;
				memcpy(&checkSize, pData + nDataCount, sizeof(checkSize));
				if (checkSize > nDataLen || checkSize == 0)
				{
					return false;
				}
			}
			break;
		case MPT_CHAR:
			pParam = new MCommandParameterChar;
			break;
		case MPT_UCHAR:
			pParam = new MCommandParameterUChar;
			break;
		case MPT_SHORT:
			pParam = new MCommandParameterShort;
			break;
		case MPT_USHORT:
			pParam = new MCommandParameterUShort;
			break;
		case MPT_INT64:
			pParam = new MCommandParameterInt64;
			break;
		case MPT_UINT64:
			pParam = new MCommandParameterUInt64;
			break;
		case MPT_SVECTOR:
			pParam = new MCommandParameterShortVector;
			break;
		default:
			return false;
			break;
		}

		nDataCount += pParam->SetData(pData + nDataCount);

		m_Params.push_back(pParam);

		if (nDataCount > nTotalSize)
		{
			return false;
		}
	}

	if (nDataCount != nTotalSize)
	{
		return false;
	}

	return true;
}

int MCommand::GetSize()
{
	if (m_pCommandDesc == NULL) return 0;

	int nSize = 0;

	nSize = sizeof(unsigned short int) + sizeof(unsigned short int) + sizeof(m_nSerialNumber);

	int nParamCount = (int)m_Params.size();

	for (int i = 0; i < nParamCount; i++)
	{
		MCommandParameter* pParam = GetParameter(i);
		nSize += (pParam->GetSize());
	}

	return nSize;
}

#define DEFAULT_COMMAND_SNCHECKER_CAPICITY	50

MCommandSNChecker::MCommandSNChecker() : m_nCapacity(DEFAULT_COMMAND_SNCHECKER_CAPICITY)
{
}

MCommandSNChecker::~MCommandSNChecker()
{
}

void MCommandSNChecker::InitCapacity(int nCapacity)
{
	m_nCapacity = nCapacity;
	m_SNQueue.clear();
	m_SNSet.clear();
}

bool MCommandSNChecker::CheckValidate(int nSerialNumber)
{
	set<int>::iterator itorSet = m_SNSet.find(nSerialNumber);
	if (itorSet != m_SNSet.end())
	{
		return false;
	}

	if ((int)m_SNQueue.size() >= m_nCapacity)
	{
		int nFirst = m_SNQueue.front();
		m_SNQueue.pop_front();

		m_SNSet.erase(nFirst);
	}

	m_SNSet.insert(set<int>::value_type(nSerialNumber));
	m_SNQueue.push_back(nSerialNumber);

	return true;
}