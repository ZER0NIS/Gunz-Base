﻿#include "stdafx.h"
#include "MMatchClient.h"
#include "MErrorTable.h"
#include "MSharedCommandTable.h"
#include "Msg.h"
#include "MDebug.h"
#include "MBlobArray.h"
#include "MMatchStage.h"

MMatchClient* g_pMatchClient = NULL;
MMatchClient* GetMainMatchClient() { return g_pMatchClient; }

/////////////////////////////////////////////////////////////////////////////////////////

void MakeTCPCommandSerialNumber(MCommand* pCmd)
{
	static unsigned char nSerial = 0;
	nSerial++;
	pCmd->m_nSerialNumber = nSerial;
}

void MakeUDPCommandSerialNumber(MCommand* pCmd)
{
	static unsigned char nSerial = 0;
	nSerial++;
	pCmd->m_nSerialNumber = nSerial;
}

#define MAX_PING_TRY_COUNT		3

/////////////////////////////////////////////////////////////////////////////////////////
int MMatchPeerInfo::GetPing(unsigned int nCurrTime)
{
	if ((int)m_nLastPongTime - (int)m_nLastPingTime < 0)
	{
		int nDelay = nCurrTime - m_nLastPingTime;
		if ((nDelay >= MAX_PING) && (m_nPingTryCount >= MAX_PING_TRY_COUNT))
		{
			return MAX_PING;
		}
	}

	return m_nPing;
}

void MMatchPeerInfo::UpdatePing(unsigned int nTime, int nPing)
{
	m_nLastPongTime = nTime;
	m_nPingTryCount = 0;
	m_nPing = nPing;
}

void MMatchPeerInfo::SetLastPingTime(unsigned int nTime)
{
	if ((int)m_nLastPongTime - (int)m_nLastPingTime >= 0)
		m_nLastPingTime = nTime;

	m_nPingTryCount++;
}

unsigned int MMatchPeerInfo::GetLastPongTime()
{
	return m_nLastPongTime;
}

/////////////////////////////////////////////////////////////////////////////////////////
MMatchPeerInfoList::MMatchPeerInfoList()
{
	InitializeCriticalSection(&m_csLock);
}

MMatchPeerInfoList::~MMatchPeerInfoList()
{
	Clear();
	DeleteCriticalSection(&m_csLock);
	mlog("PeerInfoList Released\n");
}

bool MMatchPeerInfoList::Delete(MMatchPeerInfo* pPeerInfo)
{
	bool ret = false;
	Lock();
	iterator itor = find(pPeerInfo->uidChar);
	if (itor != end())
	{
		erase(itor);

		map<MUID, MMatchPeerInfo*>::iterator itorIPPortNode =
			m_IPnPortMap.find(MUID(pPeerInfo->dwIP, (unsigned long)pPeerInfo->nPort));

		if (itorIPPortNode != m_IPnPortMap.end())
		{
			m_IPnPortMap.erase(itorIPPortNode);
		}

		delete pPeerInfo; pPeerInfo = NULL;

		ret = true;
	}

	Unlock();
	return ret;
}

void MMatchPeerInfoList::Clear()
{
	Lock();

	while (empty() == false)
	{
		delete (*begin()).second;
		erase(begin());
	}

	m_IPnPortMap.clear();

	Unlock();
}

void MMatchPeerInfoList::Add(MMatchPeerInfo* pPeerInfo)
{
	Lock();
	insert(value_type(pPeerInfo->uidChar, pPeerInfo));

	MUID uidIPPort = MUID(pPeerInfo->dwIP, (unsigned long)pPeerInfo->nPort);
	m_IPnPortMap.insert(map<MUID, MMatchPeerInfo*>::value_type(uidIPPort, pPeerInfo));
	Unlock();
}

MMatchPeerInfo* MMatchPeerInfoList::Find(const MUID& uidChar)
{
	MMatchPeerInfo* pPeer = NULL;
	Lock();

	iterator itor = find(uidChar);
	if (itor != end())
	{
		pPeer = (*itor).second;
	}

	Unlock();
	return pPeer;
}

MUID MMatchPeerInfoList::FindUID(DWORD dwIP, int nPort)
{
	MUID uidRet = MUID(0, 0);

	Lock();
	map<MUID, MMatchPeerInfo*>::iterator itor = m_IPnPortMap.find(MUID(dwIP, (unsigned long)nPort));
	if (itor != m_IPnPortMap.end())
	{
		MMatchPeerInfo* pPeerInfo = (*itor).second;
		uidRet = pPeerInfo->uidChar;
	}
	Unlock();

	return uidRet;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
MMatchClient::MMatchClient()
{
	g_pMatchClient = this;
	SetServerAddr("", 6000);
	SetServerPeerPort(7777);

	m_uidAgentServer = MUID(0, 0);
	m_uidAgentClient = MUID(0, 0);

	SetAgentAddr("", 6000);
	SetAgentPeerPort(7776);

	m_szServerName[0] = 0;
	m_nServerMode = MSM_CLAN;
	m_bEnabledSurvivalMode = false;
	m_bEnabledDuelTournament = false;

	//	m_SafeUDP.Create(true, MATCHCLIENT_DEFAULT_UDP_PORT);
}

MMatchClient::~MMatchClient()
{
	m_SafeUDP.SetCustomRecvCallback(NULL);
	m_SafeUDP.Destroy();
	ClearObjCaches();
}

bool MMatchClient::Create(unsigned short nUDPPort)
{
	if (MCommandCommunicator::Create() == false) return false;

	if (m_SafeUDP.Create(true, nUDPPort, false) == false) return false;	// REUSEADDR OFF

	m_SafeUDP.SetCustomRecvCallback(UDPSocketRecvEvent);
	SetUDPTestProcess(false);

	// Agent 소켓 이벤트 연결
	m_AgentSocket.SetCallbackContext(this);
	m_AgentSocket.SetConnectCallback(SocketConnectEvent);
	m_AgentSocket.SetDisconnectCallback(SocketDisconnectEvent);
	m_AgentSocket.SetRecvCallback(SocketRecvEvent);
	m_AgentSocket.SetSocketErrorCallback(SocketErrorEvent);

	return true;
}

MUID MMatchClient::GetSenderUIDBySocket(SOCKET socket)
{
	if (m_ClientSocket.GetSocket() == socket)
		return m_Server;
	else if (m_AgentSocket.GetSocket() == socket)
		return GetAgentServerUID();
	else
		return MUID(0, 0);
}

bool MMatchClient::OnSockConnect(SOCKET sock)
{
	MClient::OnSockConnect(sock);

	SetServerAddr(m_ClientSocket.GetHost(), m_ClientSocket.GetPort());

	return true;
}
bool MMatchClient::OnSockDisconnect(SOCKET sock)
{
	MClient::OnSockDisconnect(sock);
	OutputMessage("TCP Socket disconnected.", MZMOM_LOCALREPLY);

	return true;
}
bool MMatchClient::OnSockRecv(SOCKET sock, char* pPacket, DWORD dwSize)
{
	MClient::OnSockRecv(sock, pPacket, dwSize);

	return true;
}
void MMatchClient::OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int& ErrorCode)
{
	MClient::OnSockError(sock, ErrorEvent, ErrorCode);

	if (ErrorCode == 10053)
		OutputMessage(MZMOM_LOCALREPLY, "Disconnected", ErrorCode);
	else
		OutputMessage(MZMOM_LOCALREPLY, "TCP Socket Error(Code =  %d)", ErrorCode);
}

bool MMatchClient::OnCommand(MCommand* pCommand)
{
	bool ret = MClient::OnCommand(pCommand);

	if ((pCommand->m_pCommandDesc->IsFlag(MCDT_PEER2PEER) == true))
	{
		// Peer Network 안타고 OnCommand 불린경우 CommUID를 PlayerUID로 치환
		if (pCommand->GetSenderUID() == GetUID())
		{
			pCommand->SetSenderUID(GetPlayerUID());
		}
		else
		{
			// Peer의 패킷 시리얼은 여기서 체크한다.
			MMatchPeerInfo* pPeer = FindPeer(pCommand->GetSenderUID());
			if (pPeer)
			{
				if (!pPeer->CheckCommandValidate(pCommand))
				{
					// 암호화안한 데이타는 무시
					if (pCommand->m_pCommandDesc->IsFlag(MCCT_NON_ENCRYPTED) == false)
					{
						return false;
					}
				}
			}
		}
	}

	switch (pCommand->GetID())
	{
	case MC_MATCH_RESPONSE_LOGIN:
	{
		int nResult;
		char nServerMode;
		unsigned char nUGradeID, nPGradeID;
		MUID uidPlayer;
		char szServerName[256];
		char szAccountID[MAX_USERID_STRING_LEN];
		bool bEnabledSurvivalMode;
		bool bEnabledDuelTournament;

		pCommand->GetParameter(&nResult, 0, MPT_INT);
		pCommand->GetParameter(szServerName, 1, MPT_STR, sizeof(szServerName));
		pCommand->GetParameter(&nServerMode, 2, MPT_CHAR);
		pCommand->GetParameter(szAccountID, 3, MPT_STR, MAX_USERID_STRING_LEN);
		pCommand->GetParameter(&nUGradeID, 4, MPT_UCHAR);
		pCommand->GetParameter(&nPGradeID, 5, MPT_UCHAR);
		pCommand->GetParameter(&uidPlayer, 6, MPT_UID);
		pCommand->GetParameter(&bEnabledSurvivalMode, 7, MPT_BOOL);
		pCommand->GetParameter(&bEnabledDuelTournament, 8, MPT_BOOL);
		//				pCommand->GetParameter(szRandomValue,	7, MPT_STR, sizeof(szRandomValue) );

		//				MCommandParameter* pParam1 = pCommand->GetParameter(7);
		//				if (pParam1->GetType() != MPT_BLOB)
		//				{
		//					break;
		//				}
		//				void* pBlob1 = pParam1->GetPointer();
		//				unsigned char *szRandomValue = (unsigned char*)MGetBlobArrayElement(pBlob1, 0);

		MCommandParameter* pParam = pCommand->GetParameter(9);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();
		if (NULL == pBlob)
			break;

		int nCount = MGetBlobArrayCount(pBlob);
		unsigned char* pbyGuidReqMsg = (unsigned char*)MGetBlobArrayElement(pBlob, 0);

		OnResponseMatchLogin(pCommand->GetSenderUID(), nResult, szServerName, MMatchServerMode(nServerMode),
			szAccountID, MMatchUserGradeID(nUGradeID), MMatchPremiumGradeID(nPGradeID), uidPlayer, bEnabledSurvivalMode, bEnabledDuelTournament, pbyGuidReqMsg);
	}
	break;
	case MC_MATCH_OBJECT_CACHE:
	{
		unsigned char nType;
		pCommand->GetParameter(&nType, 0, MPT_UCHAR);
		MCommandParameter* pParam = pCommand->GetParameter(1);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();
		if (NULL == pBlob)
			break;

		int nCount = MGetBlobArrayCount(pBlob);
		OnObjectCache((unsigned int)nType, pBlob, nCount);
	}
	break;
	case MC_AGENT_RESPONSE_LOGIN:
	{
		OnResponseAgentLogin();
	}
	break;
	case MC_AGENT_LOCATETO_CLIENT:
	{
		MUID uidAgent;
		char szIP[64];
		int nPort, nUDPPort;

		if (pCommand->GetParameter(&uidAgent, 0, MPT_UID) == false) break;
		if (pCommand->GetParameter(szIP, 1, MPT_STR, sizeof(szIP)) == false) break;
		if (pCommand->GetParameter(&nPort, 2, MPT_INT) == false) break;
		if (pCommand->GetParameter(&nUDPPort, 3, MPT_INT) == false) break;

		OnLocateAgentToClient(uidAgent, szIP, nPort, nUDPPort);
	}
	break;
	case MC_AGENT_TUNNELING_TCP:
	{
		MUID uidSender, uidReceiver;
		if (pCommand->GetParameter(&uidSender, 0, MPT_UID) == false) break;
		if (pCommand->GetParameter(&uidReceiver, 1, MPT_UID) == false) break;

		MCommandParameter* pParam = pCommand->GetParameter(2);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();
		if (NULL == pBlob)
			break;
		int nCount = MGetBlobArrayCount(pBlob);

		OnTunnelingTCP(uidSender, pBlob, nCount);
	}
	break;
	case MC_AGENT_TUNNELING_UDP:
	{
		MUID uidSender, uidReceiver;
		if (pCommand->GetParameter(&uidSender, 0, MPT_UID) == false) break;
		if (pCommand->GetParameter(&uidReceiver, 1, MPT_UID) == false) break;

		MCommandParameter* pParam = pCommand->GetParameter(2);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();
		if (NULL == pBlob)
			break;

		int nCount = MGetBlobArrayCount(pBlob);

		OnTunnelingUDP(uidSender, pBlob, nCount);
	}
	break;
	case MC_AGENT_ALLOW_TUNNELING_TCP:
	{
		OnAllowTunnelingTCP();
	}
	break;
	case MC_AGENT_ALLOW_TUNNELING_UDP:
	{
		OnAllowTunnelingUDP();
	}
	break;
	case MC_AGENT_ERROR:
	{
		int nError;
		if (pCommand->GetParameter(&nError, 0, MPT_INT) == false) break;

		OnAgentError(nError);
	}
	break;

	case MC_VERSION:
		OutputMessage("MAIET MatchClient Version", MZMOM_LOCALREPLY);
		break;
	case MC_NET_ENUM:
		break;
	case MC_NET_RESPONSE_INFO:
		break;
	case MC_PEER_UDPTEST:
	{
		OnUDPTest(pCommand->GetSenderUID());
	}
	break;
	case MC_PEER_UDPTEST_REPLY:
	{
		OnUDPTestReply(pCommand->GetSenderUID());
	}
	break;
	case MC_AGENT_DEBUGTEST:
	{
	}
	break;
	default:
		if (!ret)
		{
			return false;
		}
	}
	return true;
}

void MMatchClient::OnRegisterCommand(MCommandManager* pCommandManager)
{
	MClient::OnRegisterCommand(pCommandManager);
	MAddSharedCommandTable(pCommandManager, MSCT_CLIENT);
}

void MMatchClient::OutputLocalInfo(void)
{
	OutputMessage("MAIET Match Client", MZMOM_LOCALREPLY);
	OutputMessage(MZMOM_LOCALREPLY, "UID : %u:%u", m_This.High, m_This.Low);
	OutputMessage(MZMOM_LOCALREPLY, "Connected Communicator : %u:%u", m_Server.High, m_Server.Low);
}

int MMatchClient::OnConnected(SOCKET sock, MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp)
{
#ifdef _DEBUG
	SOCKADDR_IN SockAddr;	int nErrorCode = 0;
	int nAddrLen = sizeof(SOCKADDR_IN);
	if (getsockname(m_SafeUDP.GetLocalSocket(), (SOCKADDR*)&SockAddr, &nAddrLen) == SOCKET_ERROR)
		nErrorCode = WSAGetLastError();
	char* pszIP = inet_ntoa(SockAddr.sin_addr);
	unsigned int nPort = ntohs(SockAddr.sin_port);
	mlog("UDP Address = %s:%d \n", pszIP, nPort);
#endif

	if (sock == m_ClientSocket.GetSocket())
	{
		int ret = MClient::OnConnected(sock, pTargetUID, pAllocUID, nTimeStamp);

		return ret;
	}
	else if (sock == m_AgentSocket.GetSocket())
	{
		OnAgentConnected(*pTargetUID, *pAllocUID);
		return MOK;
	}
	else
	{
		return MERR_UNKNOWN;
	}
}

void MMatchClient::OnAgentConnected(const MUID& uidAgentServer, const MUID& uidAlloc)
{
	m_uidAgentServer = uidAgentServer;
	m_uidAgentClient = uidAlloc;

	SetAllowTunneling(false);

	MPacketCrypterKey key;
	MMakeSeedKey(&key, uidAgentServer, uidAlloc, 0);
	m_AgentPacketCrypter.InitKey(&key);

	//	MCommand* pCmd = CreateCommand(MC_AGENT_PEER_BIND, GetAgentServerUID());
	//	pCmd->AddParameter(new MCmdParamUID(GetPlayerUID()));
	//	Post(pCmd);
}

int MMatchClient::OnResponseMatchLogin(const MUID& uidServer, int nResult, const char* szServerName,
	const MMatchServerMode nServerMode,
	const char* szAccountID,
	const MMatchUserGradeID nUGradeID,
	const MMatchPremiumGradeID nPGradeID,
	const MUID& uidPlayer,
	bool bEnabledSurvivalMode,
	bool bEnabledDuelTournament,
	//									   unsigned char* szRandomValue,
	unsigned char* pbyGuidReqMsg)
{
	m_uidServer = uidServer;
	m_uidPlayer = uidPlayer;

	strcpy(m_szServerName, szServerName);
	m_nServerMode = nServerMode;
	m_bEnabledSurvivalMode = bEnabledSurvivalMode;
	m_bEnabledDuelTournament = bEnabledDuelTournament;

	return MOK;
}

void MMatchClient::OnObjectCache(unsigned int nType, void* pBlob, int nCount)
{
	if (nType == MATCHCACHEMODE_REPLACE) {
		for (int i = 0; i < nCount; i++) {
			MMatchObjCache* pCache = (MMatchObjCache*)MGetBlobArrayElement(pBlob, i);
			ReplaceObjCache(pCache);
		}
	}
	else {
		if (nType == MATCHCACHEMODE_UPDATE)
			ClearObjCaches();

		for (int i = 0; i < nCount; i++) {
			MMatchObjCache* pCache = (MMatchObjCache*)MGetBlobArrayElement(pBlob, i);
			if (nType == MATCHCACHEMODE_ADD || nType == MATCHCACHEMODE_UPDATE)
				UpdateObjCache(pCache);
			else if (nType == MATCHCACHEMODE_REMOVE)
				RemoveObjCache(pCache->GetUID());
		}
	}
}

void MMatchClient::CastStageBridgePeer(const MUID& uidChar, const MUID& uidStage)
{
	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_BRIDGEPEER), GetServerUID(), m_This);
	pCmd->AddParameter(new MCommandParameterUID(uidChar));
	pCmd->AddParameter(new MCommandParameterUInt(0));	// 수신측에서 IP로 치환됨
	pCmd->AddParameter(new MCommandParameterUInt(0));		// 수신측에서 Port로 치환됨

	MSafeUDP* pSafeUDP = GetSafeUDP();
	SendCommandByUDP(pCmd, GetServerIP(), GetServerPeerPort());

	delete pCmd;
}

void MMatchClient::OnUDPTest(const MUID& uidChar)
{
	MMatchPeerInfo* pPeer = FindPeer(uidChar);
	if (pPeer) {
#ifdef _DEBUG
		if ( //(strcmp("발렌타인", pPeer->CharInfo.szName)==0) ||
			(strcmp("버드", pPeer->CharInfo.szName) == 0) ||
			(strcmp("dddd", pPeer->CharInfo.szName) == 0) ||
			(strcmp("라온하제5", pPeer->CharInfo.szName) == 0) ||
			(strcmp("라온하제6", pPeer->CharInfo.szName) == 0))
		{
			return;
		}
#endif

		MCommand* pCmd = CreateCommand(MC_PEER_UDPTEST_REPLY, uidChar);
		SendCommandByUDP(pCmd, pPeer->szIP, pPeer->nPort);
		delete pCmd;
	}
}

void MMatchClient::OnUDPTestReply(const MUID& uidChar)
{
	//// UDPTEST LOG ////////////////////////////////
#ifdef _DEBUG
	char szLog[64];
	sprintf(szLog, "[%d:%d] UDP_TEST_REPLY: from (%d:%d) \n",
		GetPlayerUID().High, GetPlayerUID().Low, uidChar.High, uidChar.Low);
	mlog(szLog);
#endif
	/////////////////////////////////////////////////
	MMatchPeerInfo* pPeer = FindPeer(uidChar);
	if (pPeer) {
		pPeer->SetUDPTestResult(true);
		pPeer->StopUDPTest();
}
}

void MMatchClient::UpdateUDPTestProcess()
{
	int nProcessCount = 0;
	for (MMatchPeerInfoList::iterator i = m_Peers.begin(); i != m_Peers.end(); i++) {
		MMatchPeerInfo* pPeer = (*i).second;
		if (pPeer->GetProcess()) {
			pPeer->UseTestCount();
			if (pPeer->GetTestCount() <= 0) {
				pPeer->StopUDPTest();

				MCommand* pCmd = CreateCommand(MC_MATCH_REQUEST_PEER_RELAY, GetServerUID());
				pCmd->AddParameter(new MCmdParamUID(GetPlayerUID()));
				pCmd->AddParameter(new MCmdParamUID(pPeer->uidChar));
				Post(pCmd);
			}
			else {
				nProcessCount++;
			}
		}
	}
	if (nProcessCount <= 0)
		SetUDPTestProcess(false);
}

void MMatchClient::OnResponseAgentLogin()
{
	MCommand* pCmd = CreateCommand(MC_AGENT_PEER_BINDTCP, GetAgentServerUID());
	pCmd->AddParameter(new MCmdParamUID(GetPlayerUID()));
	Post(pCmd);

	mlog("Logged in Agent, Bind TCP \n");
}

void MMatchClient::OnLocateAgentToClient(const MUID& uidAgent, char* szIP, int nPort, int nUDPPort)
{
	SetAgentAddr(szIP, nPort);
	SetAgentPeerPort(nUDPPort);

	if (GetBridgePeerFlag() == false) {
		AgentConnect(NULL, szIP, nPort);

		// Custom: Changed connect agent string (hide ip/port)
		mlog("Connect to Agent by TCP \n");
		// #ifdef _DEBUG
				//mlog("Connect to Agent by TCP (%s:%d) \n", szIP, nPort);
		// #endif
	}
	else {
		StartAgentPeerConnect();

		// Custom: Changed connect agent string (hide ip/port)
		mlog("Connect to Agent by UDP \n");
		// #ifdef _DEBUG
				//mlog("Connect to Agent by UDP (%s:%d) \n", szIP, nPort);
		// #endif
	}
}

MCommand* MMatchClient::MakeCmdFromTunnelingBlob(const MUID& uidSender, void* pBlob, int nBlobArrayCount)
{
	if (nBlobArrayCount != 1)
	{
		mlog("MakeCmdFromTunnelingBlob: BlobArrayCount is not 1\n");
		return NULL;
	}

	char* pPacket = (char*)MGetBlobArrayElement(pBlob, 0);

	int nSize = MGetBlobArraySize(pBlob) - (sizeof(int) * 2);
	if ((nSize <= 0) || (nSize >= MAX_BLOB_SIZE))
	{
		mlog("MakeCmdFromTunnelingBlob: Blob Size Error(size = %d)\n", nSize);
		return NULL;
	}

	char* pData = new char[nSize];

	if (!m_PeerPacketCrypter.Decrypt(pPacket, nSize, pData, nSize))
	{
		delete[] pData;
		return NULL;
	}

	MCommand* pCmd = new MCommand();
	if (!pCmd->SetData(pData, &m_CommandManager))
	{
		delete[] pData;
		delete pCmd;
		return NULL;
	}

	delete[] pData;

	pCmd->m_Sender = uidSender;
	pCmd->m_Receiver = m_This;

	MMatchPeerInfo* pPeer = FindPeer(uidSender);
	if (pPeer == NULL)
	{
		delete pCmd;
		return NULL;
	}

	// Custom: Proxy patch.
	// m_pCommandDesc will never be NULL : above SetData() did it everything.
	if (pCmd->m_pCommandDesc->IsFlag(MCDT_PEER2PEER) == false)
	{
		if (pPeer != NULL)
		{
			in_addr addr;
			addr.s_addr = pPeer->dwIP;
			char* pszIP = inet_ntoa(addr);
			mlog("Allowed non-peer packet to send from UID(%u,%u) IP(%s)\n", pPeer->uidChar.High, pPeer->uidChar.Low, pszIP);
		}
		else
		{
			mlog("Allowed non-peer packet to send from an unknown peer UID(%u,%u)\n", uidSender.High, uidSender.Low);
		}
		//delete pCmd;
		//return NULL;
	}

	return pCmd;
}

void MMatchClient::OnTunnelingTCP(const MUID& uidSender, void* pBlob, int nCount)
{
	MCommand* pCmd = MakeCmdFromTunnelingBlob(uidSender, pBlob, nCount);
	if (pCmd == NULL) return;

	LockRecv();
	m_CommandManager.Post(pCmd);
	UnlockRecv();
}

void MMatchClient::OnTunnelingUDP(const MUID& uidSender, void* pBlob, int nCount)
{
	MCommand* pCmd = MakeCmdFromTunnelingBlob(uidSender, pBlob, nCount);
	if (pCmd == NULL) return;

	LockRecv();
	m_CommandManager.Post(pCmd);
	UnlockRecv();
}

void MMatchClient::OnAllowTunnelingTCP()
{
	SetAllowTunneling(true);
}

void MMatchClient::OnAllowTunnelingUDP()
{
	SetAllowTunneling(true);
	SetAgentPeerFlag(true);
}

void MMatchClient::OnAgentError(int nError)
{
}

void MMatchClient::SendCommand(MCommand* pCommand)
{
	// P2P일경우에는 UDP를 이용
	if (pCommand->m_pCommandDesc->IsFlag(MCDT_PEER2PEER) == true)
	{
		// 시리얼 입력
		MakeUDPCommandSerialNumber(pCommand);

		if (GetBridgePeerFlag() == false) {
			SendCommandByTunneling(pCommand);
		}
		else {
			if (pCommand->GetReceiverUID() == MUID(0, 0)) {	// BroadCasting
				int nTunnelingCount = 0;

				// Peer2Peer 메세지는 Sender가 플레이어이다.
				for (MMatchPeerInfoList::iterator itor = m_Peers.begin();
					itor != m_Peers.end(); ++itor)
				{
					MMatchPeerInfo* pPeerInfo = (*itor).second;
					if ((pPeerInfo->uidChar == MUID(0, 0)) ||
						(pPeerInfo->uidChar != GetPlayerUID()))
					{
						if ((pPeerInfo->GetProcess() == false) &&
							(pPeerInfo->GetUDPTestResult() == false))
							nTunnelingCount++;
						else
						{
							// minor fix
							if (pPeerInfo->szIP != 0 && pPeerInfo->nPort != 0)
								SendCommandByUDP(pCommand, pPeerInfo->szIP, pPeerInfo->nPort);
						}
					}
				}

				if (nTunnelingCount > 0) {
					SendCommandByTunneling(pCommand);
				}
			}
			else {
				MMatchPeerInfo* pPeerInfo = FindPeer(pCommand->GetReceiverUID());
				if (pPeerInfo) {
					if ((pPeerInfo->GetProcess() == false) &&
						(pPeerInfo->GetUDPTestResult() == false))
						SendCommandByTunneling(pCommand);
					else
					{
						// minor fix
						if (pPeerInfo->szIP != 0 && pPeerInfo->nPort != 0)
							SendCommandByUDP(pCommand, pPeerInfo->szIP, pPeerInfo->nPort);
					}
				}
			}
		}
	}
	else
	{
		if ((pCommand->GetReceiverUID() != MUID(0, 0)) &&
			(pCommand->GetReceiverUID() == GetAgentServerUID()))
		{
#ifdef _DEBUG
			bool bResult = SendCommandToAgent(pCommand);

			if (pCommand->GetID() == MC_AGENT_PEER_BINDTCP) {
				if (bResult)
					OutputDebugString("SendCommand(AGENT_PEER_BINDTCP) \n");
				else
					OutputDebugString("SendCommand(AGENT_PEER_FAILED) \n");
			}
#endif
		}
		else {
			MakeTCPCommandSerialNumber(pCommand);
			MClient::SendCommand(pCommand);
		}
			}
		}

bool MMatchClient::SendCommandToAgent(MCommand* pCommand)
{
	int nPacketSize = CalcPacketSize(pCommand);
	char* pSendBuf = new char[nPacketSize];

	int size = MakeCmdPacket(pSendBuf, nPacketSize, &m_AgentPacketCrypter, pCommand);

	if (size > 0)
	{
		return m_AgentSocket.Send(pSendBuf, size);
	}
	else
	{
		delete[] pSendBuf;
		return false;
	}
}

void MMatchClient::SendCommandByUDP(MCommand* pCommand, char* szIP, int nPort)
{
	int nPacketSize = CalcPacketSize(pCommand);
	char* pSendBuf = new char[nPacketSize];

	// ##중요## - MMatchServer, MMatchAgent와 UDP통신할 때에는 암호화하지 않는 Command만 전송이 가능하다.
	int nSize = MakeCmdPacket(pSendBuf, nPacketSize, &m_PeerPacketCrypter, pCommand);

	_ASSERT(nPacketSize > 0 && nPacketSize == nSize);

	if (nSize > 0)
	{
		m_SafeUDP.Send(szIP, nPort, pSendBuf, nSize);
	}
	else
	{
		_ASSERT(0);
		delete[] pSendBuf;
	}
}

bool MMatchClient::MakeTunnelingCommandBlob(MCommand* pWrappingCmd, MCommand* pSrcCmd)
{
	// Create Param : Command Blob ////
	int nCmdSize = pSrcCmd->GetSize();
	if (nCmdSize == 0)
	{
		return false;
	}

	char* pCmdData = new char[nCmdSize];
	int nSize = pSrcCmd->GetData(pCmdData, nCmdSize);
	if (nSize != nCmdSize)
	{
		delete[] pCmdData; return false;
	}

	if (!m_PeerPacketCrypter.Encrypt(pCmdData, nSize))
	{
		delete[] pCmdData; return false;
	}

	void* pBlob = MMakeBlobArray(nSize, 1);
	char* pCmdBlock = (char*)MGetBlobArrayElement(pBlob, 0);
	CopyMemory(pCmdBlock, pCmdData, nSize);

	pWrappingCmd->AddParameter(new MCmdParamBlob(pBlob, MGetBlobArraySize(pBlob)));

	MEraseBlobArray(pBlob);
	delete[] pCmdData;

	return true;
}

void MMatchClient::SendCommandByTunneling(MCommand* pCommand)
{
	if (GetAllowTunneling() == false) {
	}
	else {
		if (GetBridgePeerFlag() == false) {
			MCommand* pCmd = CreateCommand(MC_AGENT_TUNNELING_TCP, GetAgentServerUID());
			pCmd->AddParameter(new MCmdParamUID(GetPlayerUID()));
			pCmd->AddParameter(new MCmdParamUID(pCommand->GetReceiverUID()));

			// Create Param : Command Blob ////
			if (!MakeTunnelingCommandBlob(pCmd, pCommand))
			{
				delete pCmd; pCmd = NULL; return;
			}
			///////////////////////////////////
			SendCommandToAgent(pCmd);
			delete pCmd;	// PACKETQUEUE 만들때까지 delete 임시로 사용
		}
		else {
			MCommand* pCmd = CreateCommand(MC_AGENT_TUNNELING_UDP, GetAgentServerUID());
			pCmd->AddParameter(new MCmdParamUID(GetPlayerUID()));
			pCmd->AddParameter(new MCmdParamUID(pCommand->GetReceiverUID()));
			// Create Param : Command Blob ////
			if (!MakeTunnelingCommandBlob(pCmd, pCommand))
			{
				delete pCmd; pCmd = NULL; return;
			}
			///////////////////////////////////
			SendCommandByUDP(pCmd, GetAgentIP(), GetAgentPeerPort());
			delete pCmd;	// PACKETQUEUE 만들때까지 delete 임시로 사용
		}
	}
}

bool MMatchClient::UDPSocketRecvEvent(DWORD dwIP, WORD wRawPort, char* pPacket, DWORD dwSize)
{
	if (GetMainMatchClient() == NULL) return false;
	if (dwSize < sizeof(MPacketHeader)) return false;

	MPacketHeader* pPacketHeader;
	pPacketHeader = (MPacketHeader*)pPacket;
	int nPacketSize = pPacketHeader->CalcPacketSize(&GetMainMatchClient()->m_PeerPacketCrypter);

	if ((dwSize != nPacketSize) ||
		((pPacketHeader->nMsg != MSGID_COMMAND) && (pPacketHeader->nMsg != MSGID_RAWCOMMAND))) return false;

	unsigned int nPort = ntohs(wRawPort);
	GetMainMatchClient()->ParseUDPPacket(&pPacket[sizeof(MPacketHeader)], pPacketHeader, dwIP, nPort);
	return true;
}

bool MMatchClient::DeletePeer(const MUID uid)
{
	MMatchPeerInfo* pPeer = FindPeer(uid);
	if (pPeer != NULL)
	{
		return m_Peers.Delete(pPeer);
	}
	return false;
}

void MMatchClient::AddPeer(MMatchPeerInfo* pPeerInfo)
{
	m_Peers.Add(pPeerInfo);
}

MUID MMatchClient::FindPeerUID(const DWORD dwIP, const int nPort)
{
	return m_Peers.FindUID(dwIP, nPort);
}

MMatchPeerInfo* MMatchClient::FindPeer(const MUID& uidChar)
{
	return m_Peers.Find(uidChar);
}

void MMatchClient::ClearPeers()
{
	m_Peers.Clear();
}

void MMatchClient::SetUDPPort(int nPort)
{
	m_SafeUDP.Destroy();
	m_SafeUDP.Create(true, nPort);
}

string MMatchClient::GetObjName(const MUID& uid)
{
	MMatchObjCache* pCache = FindObjCache(uid);
	char szName[32];
	if (pCache && strlen(pCache->GetName()) > 2)
		sprintf(szName, "%s", pCache->GetName());
	else
		sprintf(szName, "%d%d", uid.High, uid.Low);
	string name = szName;
	return name;
}

MMatchObjCache* MMatchClient::FindObjCache(const MUID& uid)
{
	MMatchObjCacheMap::iterator i = m_ObjCacheMap.find(uid);
	if (i == m_ObjCacheMap.end()) return NULL;
	return (*i).second;
}

void MMatchClient::ReplaceObjCache(MMatchObjCache* pCache)
{
	MMatchObjCache* pScanCache = FindObjCache(pCache->GetUID());
	if (pScanCache == NULL) return;

	pScanCache->SetInfo(pCache->GetUID(), pCache->GetName(), pCache->GetClanName(), pCache->GetLevel(),
		pCache->GetUGrade(), pCache->GetPGrade()
		, pCache->GetRank(), pCache->GetKillCount(), pCache->GetDeathCount(), pCache->GetDTGrade());
	pScanCache->SetCLID(pCache->GetCLID());
	pScanCache->SetEmblemChecksum(pCache->GetEmblemChecksum());
	pScanCache->AssignCostume(pCache->GetCostume());
	pScanCache->SetFlags(pCache->GetFlags());
}

void MMatchClient::UpdateObjCache(MMatchObjCache* pCache)
{
	MMatchObjCache* pScanCache = FindObjCache(pCache->GetUID());
	if (pScanCache != NULL) return;

	MMatchObjCache* pNewCache = new MMatchObjCache;

	pNewCache->SetInfo(pCache->GetUID(), pCache->GetName(), pCache->GetClanName(), pCache->GetLevel(),
		pCache->GetUGrade(), pCache->GetPGrade()
		, pCache->GetRank(), pCache->GetKillCount(), pCache->GetDeathCount(), pCache->GetDTGrade());
	pNewCache->SetCLID(pCache->GetCLID());
	pNewCache->SetEmblemChecksum(pCache->GetEmblemChecksum());
	pNewCache->AssignCostume(pCache->GetCostume());
	pNewCache->SetFlags(pCache->GetFlags());

	_ASSERT(m_ObjCacheMap.find(pNewCache->GetUID()) == m_ObjCacheMap.end());
	m_ObjCacheMap.Insert(pNewCache->GetUID(), pNewCache);
}

void MMatchClient::RemoveObjCache(const MUID& uid)
{
	MMatchObjCacheMap::iterator i = m_ObjCacheMap.find(uid);
	if (i == m_ObjCacheMap.end()) return;

	MMatchObjCache* pCache = i->second;
	delete pCache;
	m_ObjCacheMap.erase(i); // Channel Cache
}

void MMatchClient::ClearObjCaches()
{
	while (m_ObjCacheMap.begin() != m_ObjCacheMap.end()) {
		MMatchObjCache* pCache = (*m_ObjCacheMap.begin()).second;
		delete pCache;
		m_ObjCacheMap.erase(m_ObjCacheMap.begin());
	}
}

void MMatchClient::ParseUDPPacket(char* pData, MPacketHeader* pPacketHeader, DWORD dwIP, unsigned int nPort)
{
	switch (pPacketHeader->nMsg)
	{
	case MSGID_RAWCOMMAND:
	{
		unsigned short nCheckSum = MBuildCheckSum(pPacketHeader, pPacketHeader->nSize);
		if (pPacketHeader->nCheckSum != nCheckSum) {
			static int nLogCount = 0;
			if (nLogCount++ < 100) {	// Log Flooding 방지
				mlog("MMatchClient::ParseUDPPacket() -> CHECKSUM ERROR(R=%u/C=%u)\n",
					pPacketHeader->nCheckSum, nCheckSum);
			}
			return;
		}
		else {
			MCommand* pCmd = new MCommand();
			if (!pCmd->SetData(pData, &m_CommandManager))
			{
				mlog("MMatchClient::ParseUDPPacket() -> SetData Error\n");

				delete pCmd;
				return;
			}

			MUID uidPeer = FindPeerUID(dwIP, nPort);
			if (uidPeer != MUID(0, 0))
			{
				// Custom: Proxy patch
				if (pCmd->GetID() == MC_AGENT_TUNNELING_TCP || pCmd->GetID() == MC_AGENT_TUNNELING_UDP)
				{
					mlog("COMMAND REMOVE ERROR 1\n");
					delete pCmd;
					return;
				}

				pCmd->m_Sender = uidPeer;
			}
			else {
				// TODO: 여기 수정해야함.
				sockaddr_in Addr;
				Addr.sin_addr.S_un.S_addr = dwIP;
				Addr.sin_port = nPort;
				char* pszIP = inet_ntoa(Addr.sin_addr);

				if (strcmp(pszIP, GetAgentIP()) == 0)
				{
					pCmd->m_Sender = GetAgentServerUID();
				}
				else if ((MC_RESPONSE_SERVER_LIST_INFO == pCmd->GetID()) ||
					(MC_RESPONSE_BLOCK_COUNTRY_CODE_IP == pCmd->GetID()))
				{
					/*//NXS IP EXPORT

					   if (stricmp(pszIP, "system.hypergamers.ml") != 0)
					   {
						   delete pCmd;
						   return;
					   }*/
				}
				else if (pCmd->GetID() == MC_UDP_PONG)
				{
					// 특별히 하는건 없음. Command를 넘겨주기 위해서...(밑에 커맨드를 딜리트하기때문에)
				}
				else
				{
					delete pCmd; pCmd = NULL;
					return;
				}
			}

			pCmd->m_Receiver = m_This;

			if (IsUDPCommandValidationCheck(pCmd->GetID())) {
				LockRecv();
				m_CommandManager.Post(pCmd);
				UnlockRecv();
			}
			else {
			}
		}
	}
	break;
	case MSGID_COMMAND:
	{
		int nPacketSize = pPacketHeader->CalcPacketSize(&m_PeerPacketCrypter);
		unsigned short nCheckSum = MBuildCheckSum(pPacketHeader, nPacketSize);

		if (pPacketHeader->nCheckSum != nCheckSum) {
			static int nLogCount = 0;
			if (nLogCount++ < 100) {	// Log Flooding 방지
				mlog("MMatchClient::ParseUDPPacket() -> CHECKSUM ERROR(R=%u/C=%u)\n",
					pPacketHeader->nCheckSum, nCheckSum);
			}
			return;
		}
		else {
			MCommand* pCmd = new MCommand();

			int nCmdSize = nPacketSize - sizeof(MPacketHeader);

			if (!m_PeerPacketCrypter.Decrypt(pData, nCmdSize))
			{
				mlog("MMatchClient::ParseUDPPacket() -> Decrypt Error\n");

				delete pCmd; pCmd = NULL;
				return;
			}

			if (!pCmd->SetData(pData, &m_CommandManager))
			{
				// TODO: 여기 수정해야함.
				sockaddr_in Addr;
				Addr.sin_addr.S_un.S_addr = dwIP;
				Addr.sin_port = nPort;
				char* pszIP = inet_ntoa(Addr.sin_addr);

				mlog("MMatchClient::ParseUDPPacket() -> MSGID_COMMAND SetData Error(%s:%d), size=%d\n",
					pszIP, nPort, nCmdSize);

				delete pCmd; pCmd = NULL;
				return;
			}

			MUID uidPeer = FindPeerUID(dwIP, nPort);
			if (uidPeer != MUID(0, 0))
			{
				// Custom: Proxy patch
				if (pCmd->GetID() == MC_AGENT_TUNNELING_TCP || pCmd->GetID() == MC_AGENT_TUNNELING_UDP)
				{
					mlog("COMMAND REMOVE ERROR 2\n");
					delete pCmd;
					return;
				}

				pCmd->m_Sender = uidPeer;
			}
			else {
				// Custom: LEAVE THIS HERE. CW AGENT ERROR FIX
				// Agent와는 암호화된 커맨드는 사용하지 않는다.
				// TODO: 여기 수정해야함.
				sockaddr_in Addr;
				Addr.sin_addr.S_un.S_addr = dwIP;
				Addr.sin_port = nPort;
				char* pszIP = inet_ntoa(Addr.sin_addr);

				if (strcmp(pszIP, GetAgentIP()) == 0) {
					pCmd->m_Sender = GetAgentServerUID();
				}
				else {
					delete pCmd; pCmd = NULL;
					return;
				}
			}

			pCmd->m_Receiver = m_This;

			if (IsUDPCommandValidationCheck(pCmd->GetID())) {
				LockRecv();
				m_CommandManager.Post(pCmd);
				UnlockRecv();
			}
			else {
#ifdef _DEBUG
				mlog("%s(ID:%d) is Denied Command!\n"
					, pCmd->m_pCommandDesc->GetName(), pCmd->GetID());
#endif
			}
		}
	}
	break;
	default:
	{
		Log("MatchClient: Parse Packet Error");
	}
	break;
	}
}

int MMatchClient::AgentConnect(SOCKET* pSocket, char* szIP, int nPort)
{
	if (m_AgentSocket.Connect(pSocket, szIP, nPort))
		return MOK;
	else
		return MERR_UNKNOWN;
}

void MMatchClient::AgentDisconnect()
{
	m_AgentSocket.Disconnect();
}

void MMatchClient::StartAgentPeerConnect()
{
	SetAgentPeerFlag(false);
	SetAgentPeerCount(10);
}

void MMatchClient::CastAgentPeerConnect()
{
	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_AGENT_PEER_BINDUDP), GetAgentServerUID(), m_This);
	pCmd->AddParameter(new MCommandParameterUID(GetPlayerUID()));
	pCmd->AddParameter(new MCommandParameterString("localhost"));
	pCmd->AddParameter(new MCommandParameterUInt(0));
	pCmd->AddParameter(new MCommandParameterString(""));	// 수신측에서 IP로 치환됨
	pCmd->AddParameter(new MCommandParameterUInt(0));		// 수신측에서 Port로 치환됨

	// MSafeUDP* pSafeUDP = GetSafeUDP();
	SendCommandByUDP(pCmd, GetAgentIP(), GetAgentPeerPort());

	delete pCmd;
}

void MMatchClient::StartUDPTest(const MUID& uidChar)
{
	SetUDPTestProcess(true);
	if (uidChar == GetPlayerUID()) {
		for (MMatchPeerInfoList::iterator i = m_Peers.begin(); i != m_Peers.end(); i++) {
			MMatchPeerInfo* pPeer = (*i).second;
			if (pPeer->GetUDPTestResult() == false)
				pPeer->StartUDPTest();
		}
	}
	else {
		MMatchPeerInfo* pPeer = FindPeer(uidChar);
		if ((pPeer) && (pPeer->GetUDPTestResult() == false))
			pPeer->StartUDPTest();
	}
}

void MMatchClient::InitPeerCrypt(const MUID& uidStage, unsigned int nChecksum)
{
	//mlog("Init Peer Crypt (%u,%u,%u)\n", uidStage.High, uidStage.Low, nChecksum);

	MPacketCrypterKey key;
	MMakeSeedKey(&key, MUID(3465, nChecksum), uidStage, 9578234);

	m_PeerPacketCrypter.InitKey(&key);
}