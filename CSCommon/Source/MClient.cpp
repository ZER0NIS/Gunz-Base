#include "stdafx.h"
#include "MClient.h"
#include "MSharedCommandTable.h"
#include <stdarg.h>
#include "MErrorTable.h"
#include "Msg.h"
#include "MDebug.h"
#include "MCommandBuilder.h"
#include "MMatchUtil.h"
#include "MMatchConfig.h"

MClient* MClient::m_pInstance = NULL;

void SplitIAddress(char* szIP, int* pPort, const char* szAddress)
{
	*szIP = NULL;
	*pPort = 0;
	char* szCopyAddress = new char[strlen(szAddress) + 2];
	strcpy(szCopyAddress, szAddress);

	char seps[] = ":\n";
	char* token = strtok(szCopyAddress, seps);
	if (token != NULL) strcpy(szIP, token);
	token = strtok(NULL, seps);
	if (token != NULL) *pPort = atoi(token);

	delete[] szCopyAddress;
}

MClient::MClient()
{
	m_pInstance = this;

	m_pCommandBuilder = new MCommandBuilder(MUID(0, 0), MUID(0, 0), GetCommandManager());

	m_Server.SetInvalid();

	InitializeCriticalSection(&m_csRecvLock);

	m_ClientSocket.SetCallbackContext(this);
	m_ClientSocket.SetConnectCallback(SocketConnectEvent);
	m_ClientSocket.SetDisconnectCallback(SocketDisconnectEvent);
	m_ClientSocket.SetRecvCallback(SocketRecvEvent);
	m_ClientSocket.SetSocketErrorCallback(SocketErrorEvent);
}

MClient::~MClient()
{
	delete m_pCommandBuilder;
	m_pCommandBuilder = NULL;

	DeleteCriticalSection(&m_csRecvLock);
}

MClient* MClient::GetInstance(void)
{
	return m_pInstance;
}

MUID MClient::GetSenderUIDBySocket(SOCKET socket)
{
	if (m_ClientSocket.GetSocket() == socket)
		return m_Server;
	else
		return MUID(0, 0);
}

int MClient::OnConnected(MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp, MCommObject* pCommObj)
{
	return MERR_UNKNOWN;
}

int MClient::OnConnected(SOCKET sock, MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp)
{
	if (sock == m_ClientSocket.GetSocket()) {
		int ret = MCommandCommunicator::OnConnected(pTargetUID, pAllocUID, nTimeStamp, NULL);
		m_Server = *pTargetUID;

		MCommandBuilder* pCmdBuilder = GetCommandBuilder();

		MPacketCrypterKey key;
		MMakeSeedKey(&key, *pTargetUID, *pAllocUID, nTimeStamp);
		m_ServerPacketCrypter.InitKey(&key);
		pCmdBuilder->InitCrypt(&m_ServerPacketCrypter, false);

		return MOK;
	}
	else {
		return MERR_UNKNOWN;
	}
}

void MClient::OnRegisterCommand(MCommandManager* pCommandManager)
{
}

bool MClient::OnCommand(MCommand* pCommand)
{
	switch (pCommand->GetID()) {
	case MC_LOCAL_INFO:
		OutputLocalInfo();
		break;
	case MC_LOCAL_ECHO:
		// Custom: Exploit fix (Fake echo from different player)
		//if(pCommand->GetParameter(szMessage, 0, MPT_STR, sizeof(szMessage))==false) break;
		//OutputMessage(szMessage, MZMOM_LOCALREPLY);
		break;
	case MC_HELP:
		OutputMessage("MAIET Default Help :", MZMOM_LOCALREPLY);
		for (int i = 0; i < m_CommandManager.GetCommandDescCount(); i++) {
			MCommandDesc* pCmdDesc = m_CommandManager.GetCommandDesc(i);
			static char szMessage[1024];
			sprintf(szMessage, "%s : %s", pCmdDesc->GetName(), pCmdDesc->GetDescription());
			OutputMessage(szMessage, MZMOM_LOCALREPLY);
			char szSyntax[256];
			m_CommandManager.GetSyntax(szSyntax, pCmdDesc);
			sprintf(szMessage, "[Syntax] %s", szSyntax);
			OutputMessage(szMessage, MZMOM_LOCALREPLY);
		}
		break;
	case MC_NET_CONNECT:
	{
		char szAddress[256];
		if (!pCommand->GetParameter(szAddress, 0, MPT_STR, sizeof(szAddress)))
			break;

		char szIP[256];
		int nPort;
		SplitIAddress(szIP, &nPort, szAddress);

		SOCKET socket;
		int nReturn = Connect(&socket, szIP, nPort);
		if (nReturn != MOK) {
			OutputMessage("Can't connect to communicator", MZMOM_ERROR);
			break;
		}
	}
	break;
	case MC_NET_CONNECTTOZONESERVER:
	{
		SOCKET socket;
		int nReturn = Connect(&socket, "127.0.0.1", 6000);
		if (nReturn != MOK) {
			OutputMessage("Can't connect to communicator", MZMOM_ERROR);
			break;
		}
	}
	break;
	case MC_NET_DISCONNECT:
		// Custom: Exploit fix (Fake disconnect from different player)
		if (pCommand->GetSenderUID() != GetUID())
		{
			break;
		}
		Disconnect(m_Server);
		break;

	case MC_NET_ECHO:
		// Custom : Exploit fix(Fake echo from different player)
		break;
	default:
		return false;
	}

	return true;
}

int MClient::Connect(SOCKET* pSocket, char* szIP, int nPort)
{
	if (m_ClientSocket.Connect(pSocket, szIP, nPort))
		return MOK;
	else
		return MERR_UNKNOWN;
}

int MClient::Connect(MCommObject* pCommObj)
{
	if (m_ClientSocket.Connect(NULL, pCommObj->GetIPString(), pCommObj->GetPort()))
		return MOK;
	else
		return MERR_UNKNOWN;
}

void MClient::SendCommand(MCommand* pCommand)
{
	int nPacketSize = CalcPacketSize(pCommand);
	char* pSendBuf = new char[nPacketSize];

	int size = MakeCmdPacket(pSendBuf, nPacketSize, &m_ServerPacketCrypter, pCommand);

	if (size > 0)
	{
		if (!m_ClientSocket.Send(pSendBuf, size))
		{
			delete[] pSendBuf;
		}
	}
	else
	{
		delete[] pSendBuf;
	}
}

bool MClient::Post(MCommand* pCommand)
{
	LockRecv();
	bool bRet = MCommandCommunicator::Post(pCommand);
	UnlockRecv();
	return bRet;
}

bool MClient::Post(char* szErrMsg, int nErrMsgCount, const char* szCommand)
{
	return MCommandCommunicator::Post(szErrMsg, nErrMsgCount, szCommand);
}

MCommand* MClient::GetCommandSafe()
{
	LockRecv();
	MCommand* pCmd = MCommandCommunicator::GetCommandSafe();
	UnlockRecv();

	return pCmd;
}

void MClient::Disconnect(const MUID& uid)
{
	m_ClientSocket.Disconnect();
}

int MClient::MakeCmdPacket(char* pOutPacket, int iMaxPacketSize, MPacketCrypter* pPacketCrypter, MCommand* pCommand)
{
	MCommandMsg* pMsg = (MCommandMsg*)pOutPacket;

	int nCmdSize = iMaxPacketSize - sizeof(MPacketHeader);

	pMsg->Buffer[0] = 0;
	pMsg->nCheckSum = 0;
	int nPacketSize = 0;

	if (pCommand->m_pCommandDesc->IsFlag(MCCT_NON_ENCRYPTED))
	{
		pMsg->nMsg = MSGID_RAWCOMMAND;

		nCmdSize = pCommand->GetData(pMsg->Buffer, nCmdSize);
		pMsg->nSize = (unsigned short)(sizeof(MPacketHeader) + nCmdSize);

		nPacketSize = pMsg->nSize;
	}
	else
	{
		if (pPacketCrypter == NULL)
		{
			return 0;
		}

		pMsg->nMsg = MSGID_COMMAND;

		nCmdSize = pCommand->GetData(pMsg->Buffer, nCmdSize);
		nPacketSize = sizeof(MPacketHeader) + nCmdSize;
		pMsg->nSize = (unsigned short)(nPacketSize);

		if (!pPacketCrypter->Encrypt((char*)&pMsg->nSize, sizeof(unsigned short)))
		{
			mlog("MClient::MakeCmdPacket -> Size Encrypt error\n");
			return 0;
		}
		if (!pPacketCrypter->Encrypt(pMsg->Buffer, nCmdSize))
		{
			mlog("MClient::MakeCmdPacket -> Cmd Encrypt error\n");
			return 0;
		}
	}

	pMsg->nCheckSum = MBuildCheckSum(pMsg, nPacketSize);

	return nPacketSize;
}

bool MClient::OnSockConnect(SOCKET sock)
{
	return true;
}
bool MClient::OnSockDisconnect(SOCKET sock)
{
	return true;
}
bool MClient::OnSockRecv(SOCKET sock, char* pPacket, DWORD dwSize)
{
	MCommandBuilder* pCmdBuilder = GetCommandBuilder();
	pCmdBuilder->SetUID(m_This, GetSenderUIDBySocket(sock));
	pCmdBuilder->Read((char*)pPacket, dwSize);

	LockRecv();
	while (MCommand* pCmd = pCmdBuilder->GetCommand()) {
#ifdef _DEBUG
#endif
		Post(pCmd);
	}
	UnlockRecv();

	while (MPacketHeader* pNetCmd = pCmdBuilder->GetNetCommand()) {
		if (pNetCmd->nMsg == MSGID_REPLYCONNECT) {
			MReplyConnectMsg* pMsg = (MReplyConnectMsg*)pNetCmd;
			MUID HostUID, AllocUID;
			unsigned int nTimeStamp;

			HostUID.High = pMsg->nHostHigh;
			HostUID.Low = pMsg->nHostLow;
			AllocUID.High = pMsg->nAllocHigh;
			AllocUID.Low = pMsg->nAllocLow;
			nTimeStamp = pMsg->nTimeStamp;

			free(pNetCmd);

			LockRecv();
			OnConnected(sock, &HostUID, &AllocUID, nTimeStamp);
			UnlockRecv();
		}
	}

	return true;
}

void MClient::OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int& ErrorCode)
{
}

bool MClient::SocketRecvEvent(void* pCallbackContext, SOCKET sock, char* pPacket, DWORD dwSize)
{
	MClient* pClient = (MClient*)pCallbackContext;

	return pClient->OnSockRecv(sock, pPacket, dwSize);
}

bool MClient::SocketConnectEvent(void* pCallbackContext, SOCKET sock)
{
	MClient* pClient = (MClient*)pCallbackContext;

	return pClient->OnSockConnect(sock);
}
bool MClient::SocketDisconnectEvent(void* pCallbackContext, SOCKET sock)
{
	MClient* pClient = (MClient*)pCallbackContext;
	return pClient->OnSockDisconnect(sock);
}

void MClient::SocketErrorEvent(void* pCallbackContext, SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int& ErrorCode)
{
	MClient* pClient = (MClient*)pCallbackContext;

	pClient->OnSockError(sock, ErrorEvent, ErrorCode);
}

void MClient::OutputMessage(MZMOMType nType, const char* pFormat, ...)
{
	va_list args;
	static char temp[1024];

	va_start(args, pFormat);
	vsprintf(temp, pFormat, args);
	OutputMessage(temp, nType);
	va_end(args);
}

MRingBuffer::MRingBuffer()
{
	m_iBufSize = 0;
	m_Buf = NULL;
	m_cpBegin = NULL;
	m_cpEnd = NULL;
}
MRingBuffer::MRingBuffer(int iBufSize)
{
	m_iBufSize = 0;
	m_Buf = NULL;
	m_cpBegin = NULL;
	m_cpEnd = NULL;

	Reserve(iBufSize);
}

MRingBuffer::~MRingBuffer()
{
	if (m_Buf) delete[] m_Buf;
}

void MRingBuffer::Reserve(int iBufSize)
{
	if (m_Buf) delete[] m_Buf;

	m_Buf = new char[iBufSize];
	memset(m_Buf, 0, iBufSize);

	m_iBufSize = iBufSize;
}

bool MRingBuffer::Enqueue(char* cp, int iDataSize)
{
	return true;
}
bool MRingBuffer::Dequeue(char* cpOut, int iDataSize)
{
	return true;
}