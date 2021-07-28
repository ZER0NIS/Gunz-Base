#include "stdafx.h"
#include <winsock2.h>
#include "MCommandBuilder.h"
#include "MMatchUtil.h"

MCommandBuilder::MCommandBuilder(MUID uidSender, MUID uidReceiver, MCommandManager* pCmdMgr)
{
	m_pPacketCrypter = NULL;
	m_uidSender = uidSender;
	m_uidReceiver = uidReceiver;
	m_pCommandManager = pCmdMgr;

	m_nBufferNext = 0;
	m_bCheckCommandSN = true;

	m_CommandCountPerSec = 0;
	m_dwLastCommandMakeTime = 0;
}

MCommandBuilder::~MCommandBuilder()
{
	Clear();
}

bool MCommandBuilder::CheckBufferEmpty()
{
	if (m_nBufferNext == 0) return true;
	else return false;
}

bool MCommandBuilder::EstimateBufferToCmd()
{
	if (m_nBufferNext < sizeof(MPacketHeader))
		return false;
	MPacketHeader* pPacket = (MPacketHeader*)m_Buffer;
	if (m_nBufferNext < _CalcPacketSize(pPacket))
		return false;
	return true;
}

void MCommandBuilder::AddBuffer(char* pBuffer, int nLen)
{
	if (nLen <= 0) return;
	if ((m_nBufferNext + nLen) >= COMMAND_BUFFER_LEN) {
		return;
	}
	CopyMemory(m_Buffer + m_nBufferNext, pBuffer, nLen);
	m_nBufferNext += nLen;
}

bool MCommandBuilder::MoveBufferToFront(int nStart, int nLen)
{
	if (nStart + nLen > m_nBufferNext)
		return false;
	CopyMemory(m_Buffer, m_Buffer + nStart, nLen);
	m_nBufferNext = nLen;
	return true;
}

int MCommandBuilder::_CalcPacketSize(MPacketHeader* pPacket)
{
	return pPacket->CalcPacketSize(m_pPacketCrypter);
}

int MCommandBuilder::CalcCommandCount(char* pBuffer, int nBufferLen)
{
	unsigned int nOffset = 0;
	int nLen = nBufferLen;
	MPacketHeader* pPacket = (MPacketHeader*)(pBuffer + nOffset);
	int nCmdCount = 0;
	int nPacketSize = 0;

	while (nLen >= sizeof(MPacketHeader))
	{
		pPacket = (MPacketHeader*)(pBuffer + nOffset);

		nPacketSize = _CalcPacketSize(pPacket);
		if ((nPacketSize > nLen) || (nPacketSize <= 0)) break;

		nOffset += nPacketSize;
		nLen -= nPacketSize;
		nCmdCount++;
	}

	return nCmdCount;
}

int MCommandBuilder::MakeCommand(char* pBuffer, int nBufferLen)
{
	unsigned int nOffset = 0;
	int nLen = nBufferLen;
	MPacketHeader* pPacket = (MPacketHeader*)(pBuffer + nOffset);
	int nCmdCount = 0;
	int nPacketSize = 0;

	while (nLen >= sizeof(MPacketHeader))
	{
		int nPacketSize = _CalcPacketSize(pPacket);
		if ((nPacketSize > nLen) || (nPacketSize <= 0)) break;

		if (pPacket->nMsg == MSGID_RAWCOMMAND)
		{
			unsigned short nCheckSum = MBuildCheckSum(pPacket, nPacketSize);
			if (pPacket->nCheckSum != nCheckSum) {
				return -1;
			}
			else if (nPacketSize > MAX_PACKET_SIZE)
			{
				return -1;
			}
			else
			{
				MCommand* pCmd = new MCommand();
				int nCmdSize = nPacketSize - sizeof(MPacketHeader);
				if (pCmd->SetData(((MCommandMsg*)pPacket)->Buffer, m_pCommandManager, (unsigned short)nCmdSize))
				{
					if (m_bCheckCommandSN)
					{
						if (!m_CommandSNChecker.CheckValidate(pCmd->m_nSerialNumber))
						{
							delete pCmd; pCmd = NULL;
							return -1;
						}
					}

					// Custom: Add packet type
					pCmd->m_nCommandType = MSGID_RAWCOMMAND;

					// refuse packet if the command data is supposed to be decrypted but we received a encrypted packet of it
					if (!pCmd->m_pCommandDesc->IsFlag(MCCT_NON_ENCRYPTED))
					{
						mlog("Mismatched cmd flag (rc)\n");
						delete pCmd; pCmd = NULL;
						return -1;
					}

					pCmd->m_Sender = m_uidSender;
					pCmd->m_Receiver = m_uidReceiver;
					m_CommandList.push_back(pCmd);
				}
				else
				{
					delete pCmd; pCmd = NULL;
					return -1;
				}
			}
		}
		else if (pPacket->nMsg == MSGID_COMMAND)
		{
			unsigned short nCheckSum = MBuildCheckSum(pPacket, nPacketSize);
			if (pPacket->nCheckSum != nCheckSum) {
				return -1;
			}
			else if (nPacketSize > MAX_PACKET_SIZE)
			{
				return -1;
			}
			else
			{
				MCommand* pCmd = new MCommand();

				int nCmdSize = nPacketSize - sizeof(MPacketHeader);
				if (m_pPacketCrypter)
				{
					if (!m_pPacketCrypter->Decrypt((char*)((MCommandMsg*)pPacket)->Buffer, nCmdSize))
					{
						delete pCmd; pCmd = NULL;
						return -1;
					}
				}

				if (pCmd->SetData((char*)((MCommandMsg*)pPacket)->Buffer, m_pCommandManager, (unsigned short)nCmdSize))
				{
					if (m_bCheckCommandSN)
					{
						if (!m_CommandSNChecker.CheckValidate(pCmd->m_nSerialNumber))
						{
							delete pCmd; pCmd = NULL;
							return -1;
						}
					}

					// Custom: Add packet type
					pCmd->m_nCommandType = MSGID_COMMAND;

					// refuse packet if the command data is supposed to be decrypted but we received a encrypted packet of it
					if (pCmd->m_pCommandDesc->IsFlag(MCCT_NON_ENCRYPTED))
					{
						mlog("Mismatched cmd flag (c)\n");
						delete pCmd; pCmd = NULL;
						return -1;
					}

					pCmd->m_Sender = m_uidSender;
					pCmd->m_Receiver = m_uidReceiver;
					m_CommandList.push_back(pCmd);
				}
				else
				{
					delete pCmd; pCmd = NULL;
					return -1;
				}
			}
		}
		else if (pPacket->nMsg == MSGID_REPLYCONNECT) {
			if (nPacketSize == sizeof(MReplyConnectMsg))
			{
				MPacketHeader* pNewPacket = (MPacketHeader*)malloc(nPacketSize);
				CopyMemory(pNewPacket, pPacket, nPacketSize);
				m_NetCmdList.push_back(pNewPacket);
			}
			else
			{
				return -1;
			}
		}
		else {
			return -1;
		}

		nOffset += nPacketSize;
		nLen -= nPacketSize;
		nCmdCount++;

		pPacket = (MPacketHeader*)(pBuffer + nOffset);
	}

	return nLen;
}

void MCommandBuilder::Clear()
{
	if (!m_CommandList.empty())
	{
		for (MCommandList::iterator itorCmd = m_CommandList.begin(); itorCmd != m_CommandList.end(); ++itorCmd)
		{
			MCommand* pCmd = (*itorCmd);
			delete pCmd;
		}
		m_CommandList.clear();
	}

	if (!m_NetCmdList.empty())
	{
		for (list<MPacketHeader*>::iterator itorNetCmd = m_NetCmdList.begin(); itorNetCmd != m_NetCmdList.end(); ++itorNetCmd)
		{
			MPacketHeader* pNetCmd = (*itorNetCmd);
			free(pNetCmd);
		}
		m_NetCmdList.clear();
	}
}

bool MCommandBuilder::Read(char* pBuffer, int nBufferLen, bool bFloodCheck, bool* bFloodResult)
{
	MPacketHeader* pPacket = (MPacketHeader*)pBuffer;

	if (CheckBufferEmpty() == true) {
		if ((nBufferLen < sizeof(MPacketHeader)) || (nBufferLen < _CalcPacketSize(pPacket))) {
			AddBuffer(pBuffer, nBufferLen);
		}
		else {
			int nCommandCount = CalcCommandCount(pBuffer, nBufferLen);
			if (CheckFlooding(nCommandCount) == true && bFloodCheck)
			{
				if (bFloodResult != NULL)
					(*bFloodResult) = true;

				return false;
			}

			int nSpareData = MakeCommand(pBuffer, nBufferLen);
			if (nSpareData > 0) {
				AddBuffer(pBuffer + (nBufferLen - nSpareData), nSpareData);
			}
			else if (nSpareData < 0) return false;
		}
	}
	else {
		AddBuffer(pBuffer, nBufferLen);
		if (EstimateBufferToCmd() == true) {
			int nCommandCount = CalcCommandCount(pBuffer, nBufferLen);
			if (CheckFlooding(nCommandCount) == true && bFloodCheck)
			{
				if (bFloodResult != NULL)
					(*bFloodResult) = true;

				return false;
			}

			int nSpareData = MakeCommand(m_Buffer, m_nBufferNext);
			if (nSpareData >= 0)
				MoveBufferToFront(m_nBufferNext - nSpareData, nSpareData);
			else return false;
		}
	}

	return true;
}

MCommand* MCommandBuilder::GetCommand()
{
	MCommandList::iterator itorCmd = m_CommandList.begin();
	if (itorCmd != m_CommandList.end()) {
		MCommand* pCmd = (*itorCmd);
		m_CommandList.pop_front();
		return (pCmd);
	}
	else {
		return NULL;
	}
}

MPacketHeader* MCommandBuilder::GetNetCommand()
{
	list<MPacketHeader*>::iterator itorCmd = m_NetCmdList.begin();
	if (itorCmd != m_NetCmdList.end()) {
		MPacketHeader* pTestCmd = (*itorCmd);
		m_NetCmdList.pop_front();
		return (pTestCmd);
	}
	else {
		return NULL;
	}
}

void MCommandBuilder::SetUID(MUID uidReceiver, MUID uidSender)
{
	m_uidReceiver = uidReceiver;
	m_uidSender = uidSender;
}

void MCommandBuilder::InitCrypt(MPacketCrypter* pPacketCrypter, bool bCheckCommandSerialNumber)
{
	m_pPacketCrypter = pPacketCrypter;
	m_bCheckCommandSN = bCheckCommandSerialNumber;
}

bool MCommandBuilder::CheckFlooding(int nCommandCount)
{
	DWORD dwCurTime = GetTickCount();

	if (dwCurTime - m_dwLastCommandMakeTime < 1000)
	{
		m_CommandCountPerSec += nCommandCount;

		if (m_CommandCountPerSec > MAX_COMMAND_COUNT_FLOODING)
			return true;
	}
	else
	{
		m_CommandCountPerSec = nCommandCount;
		m_dwLastCommandMakeTime = dwCurTime;

		if (m_CommandCountPerSec > MAX_COMMAND_COUNT_FLOODING)
			return true;
	}

	return false;
}