#include "stdafx.h"
#include "MMatchAgent.h"
#include "MStageAgent.h"
#include "MObject.h"
#include "Msg.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MMatchUtil.h"
#include "MCommandBuilder.h"

#ifndef MATCHSERVER_UID
	#define MATCHSERVER_UID		MUID(0, 2)	///< MatchServer의 고유 UID(불변)
#endif

#define MATCHSERVER_DEFAULT_UDP_PORT	7778
#define DEFAULT_REQUEST_UID_SIZE		10000
MMatchAgent* MMatchAgent::m_pInstance = NULL;
extern void RcpLog(const char *pFormat,...);

MMatchAgent::MMatchAgent(void)
{
	_ASSERT(m_pInstance==NULL);
	m_pInstance = this;
	SetName("AGENTSERVER");	// For Debug

	m_uidMatchServer = MUID(0,0);
	m_uidNextAlloc = MUID(0,1000);

	SetTCPPort(0);
	SetUDPPort(0);

	SetMatchServerTrying(false);
	SetMatchServerConnected(false);
	SetMatchServerTCPPort(0);

	memset(m_szIP, 0, 64);
	memset(m_szMatchServerIP, 0, 64);
	
	SetLastLiveCheckSendTime(0);
	SetLastLiveCheckRecvTime(0);

	SetFloodCheck(true);
}

MMatchAgent::~MMatchAgent(void)
{
	Destroy();
}

bool MMatchAgent::Create(int nPort)
{
	if (nPort == 0)
		nPort = GetTCPPort();

	if(MServer::Create(nPort)==false) return false;

	if(OnCreate()==false){
//		Destroy();
		LOG(LOG_PROG, "Agent Server create FAILED (Port:%d)\n", nPort);
		return false;
	}

	SetupRCPLog(RcpLog);
	m_RealCPNet.SetLogLevel(1);

	m_SafeUDP.Create(false, GetUDPPort());
	m_SafeUDP.SetCustomRecvCallback(UDPSocketRecvEvent);

	LOG(LOG_PROG, "Match Agent Created (Port:%d)\n", nPort);
///////////////////////////////////////

///////////////////////////////////////
	return true;
}

void MMatchAgent::Destroy(void)
{
	OnDestroy();

	m_SafeUDP.Destroy();
	MServer::Destroy();
}

void MMatchAgent::ConnectToMatchServer(char* pszAddr, int nPort)
{
	LOG(LOG_FILE, "MMatchAgent::ConnectToMatchServer BEGIN \n");

	if (pszAddr == NULL)
	{
		pszAddr = GetMatchServerIP();
	}
	if (nPort == 0)
	{
		nPort = GetMatchServerTCPPort();
	}

	////////////////////////////////////////////////////////////
/*	MCommand* pCmd = CreateCommand(MC_AGENT_CONNECT, MUID(0,0));
	pCmd->AddParameter(new MCmdParamStr(pszAddr));
	pCmd->AddParameter(new MCmdParamInt(nPort));
	Post(pCmd);
	char szBuf[1024];
	int nSize = pCmd->GetData(szBuf, 1024);
	char szBuf2[1024]; CopyMemory(szBuf2, szBuf, nSize);
	MCommand* pCmd2 = new MCommand();
	pCmd2->SetData(szBuf2, &m_CommandManager);*/
	////////////////////////////////////////////////////////////

	// 이미 접속중이면 종료
	if (GetMatchServerTrying() ) return;

	// TodoH - UID가 겹치는 문제는 없을까? -.,-
	MCommObject* pCommObj = NULL;
	pCommObj = new MCommObject(this);
	pCommObj->SetUID( MATCHSERVER_UID );
	pCommObj->SetAddress(pszAddr, nPort);
	m_uidMatchServer = pCommObj->GetUID();

	LOG(LOG_PROG, "Request Connect to matchserver ip(%s), port(%u).\n", pszAddr, nPort );

	int nErrCode = Connect(pCommObj);
	if (nErrCode != MOK)
	{
		LOG(LOG_PROG, "Failed to Connect to MatchServer (Err:%d)\n", nErrCode);
		SetMatchServerTrying(false);
		SetMatchServerConnected(false);
	}
	else
	{
		SetMatchServerTrying(true);
	}
	LOG(LOG_FILE, "MMatchAgent::ConnectToMatchServer END \n");
}

void MMatchAgent::DisconnectFromMatchServer()
{
	MCommand* pCmd = CreateCommand(MC_AGENT_DISCONNECT, MUID(0,0));
	Post(pCmd);
}

void MMatchAgent::RequestLiveCheck()
{
	int nCurrentClock = GetGlobalClockCount();

	MCommand* pCmd = CreateCommand(MC_MATCH_AGENT_REQUEST_LIVECHECK, GetMatchServerUID());
	pCmd->AddParameter( new MCmdParamUInt(nCurrentClock) );
	pCmd->AddParameter( new MCmdParamUInt(GetStageCount()) );
	pCmd->AddParameter( new MCmdParamUInt(GetClientCount()) );
	Post(pCmd);
	
	SetLastLiveCheckSendTime(nCurrentClock);
	SetLastLiveCheckRecvTime(nCurrentClock);
}

void MMatchAgent::OnResponseLiveCheck(unsigned long nTimeStamp)
{
	unsigned long nCurrentTime = GetGlobalClockCount();
	int nTimeDelta = nCurrentTime - nTimeStamp;

	if (nTimeDelta >= 15000)
		LOG(LOG_PROG, "LIVECHECK Delay=%d\n", nTimeDelta);

	SetLastLiveCheckRecvTime(nCurrentTime);
}

bool MMatchAgent::OnCreate(void)
{
	return true;
}
void MMatchAgent::OnDestroy(void)
{
}

void MMatchAgent::OnRegisterCommand(MCommandManager* pCommandManager)
{
	MCommandCommunicator::OnRegisterCommand(pCommandManager);
	MAddSharedCommandTable(pCommandManager, MSCT_AGENT);
	LOG(LOG_PROG, "Command registeration completed\n");
}

bool MMatchAgent::OnCommand(MCommand* pCommand)
{
	switch(pCommand->GetID()){
		case MC_LOCAL_LOGIN:		// Local
			{
				MUID uidComm, uidPlayer;
				pCommand->GetParameter(&uidComm, 0, MPT_UID);
				pCommand->GetParameter(&uidPlayer, 1, MPT_UID);
				OnLocalLogin(uidComm, uidPlayer);
				return true;
			}

		case MC_DEBUG_TEST:			// M2M
			{
				DebugTest();
				return true;
			}
		case MC_NET_ECHO:			// M2M
			{
				char szMessage[256] = "";
				if (pCommand->GetParameter(szMessage, 0, MPT_STR, sizeof(szMessage))==false) break;

				MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_ECHO), pCommand->m_Sender, m_This);
				pNew->AddParameter(new MCommandParameterString(szMessage));
				Post(pNew);
				return true;
			}
		case MC_NET_CLEAR:			// Local
			{
				MUID uid;
				MAgentClient* pObj;
				if (pCommand->GetParameter(&uid, 0, MPT_UID) == false) break;

				if (uid == GetMatchServerUID()) {
					SetMatchServerTrying(false);
					SetMatchServerConnected(false);
					SetLastLiveCheckSendTime(0);
					SetLastLiveCheckRecvTime(0);
					LOG(LOG_FILE, "Server Connection is Cleared(UID:%d%d)\n", uid.High, uid.Low);
				} 
				else if((pObj = GetPlayerByCommUID(uid)) != NULL){
					ClientRemove(pObj->GetUID(), NULL);
					LOG(LOG_PROG, "Agent Client Remove(UID:%d%d, IP:%s, Port:%d)\n", uid.High, uid.Low, pObj->GetIP(), pObj->GetPort());
				}
				else{
					LOG(LOG_PROG, "Agent Net Clear(UID:%d%d)\n", uid.High, uid.Low);
				}				

				MServer::OnNetClear(uid);	// Remove CommObject
				return true;
			}
		case MC_NET_CHECKPING:		// Local
			{
				MUID uid;
				if (pCommand->GetParameter(&uid, 0, MPT_UID)==false) break;
				MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_PING), uid, m_This);
				pNew->AddParameter(new MCommandParameterUInt(timeGetTime()));
				Post(pNew);
				return true;
			}
		case MC_NET_PING:			// M2M
			{
				unsigned int nTimeStamp;
				if (pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT)==false) break;
				MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_PONG), pCommand->m_Sender, m_This);
				pNew->AddParameter(new MCommandParameterUInt(nTimeStamp));
				Post(pNew);
				return true;
			}
		case MC_NET_PONG:			// M2M
			{
				int nTimeStamp;
				pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT);
				LOG(LOG_PROG, "Ping from (%u:%u) = %d\n", pCommand->GetSenderUID().High, pCommand->GetSenderUID().Low, timeGetTime()-nTimeStamp);
			}
			break;
		case MC_AGENT_CONNECT:		// Local
			{
				MMatchAgent* pServer = MMatchAgent::GetInstance();

				char szAddr[64] = "";
				int nPort;
				if (pCommand->GetParameter(szAddr, 0, MPT_STR, sizeof(szAddr))==false) break;
				if (pCommand->GetParameter(&nPort, 1, MPT_INT)==false) break;

				MCommObject* pCommObj = new MCommObject(pServer);
				pCommObj->SetUID( MATCHSERVER_UID );
				pCommObj->SetAddress(szAddr, nPort);
				
				m_uidMatchServer = pCommObj->GetUID();

				int nErrCode = Connect(pCommObj);
				if(nErrCode!=MOK) {
					LOG(LOG_FILE, "Failed to Connect to MatchServer (Err:%d)\n", nErrCode);
					SetMatchServerTrying(false);
					SetMatchServerConnected(false);
					Disconnect(pCommObj->GetUID());
				}

				LOG(LOG_FILE, "Try to Connect to MatchServer (Err:%d)\n", nErrCode);

				return true;
			}
		case MC_AGENT_DISCONNECT:	// Local
			{
				Disconnect(m_uidMatchServer);
				LOG(LOG_FILE, "Disconnect server - MUID(%d%d)\n", m_uidMatchServer.High, m_uidMatchServer.Low);
				return true;
			}
		case MC_MATCH_AGENT_RESPONSE_LIVECHECK:	// M2M
			{
				unsigned long nTimeStamp;
				pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT);

				OnResponseLiveCheck(nTimeStamp);
				return true;
			}
		case MC_AGENT_LOCAL_LOGIN:	// Local
			{
				MUID uidComm, uidPlayer;
				pCommand->GetParameter(&uidComm, 0, MPT_UID);
				pCommand->GetParameter(&uidPlayer, 1, MPT_UID);
				
				OnAgentLocalLogin(uidComm, uidPlayer);

				return true;
			}
		case MC_AGENT_STAGE_RESERVE:	// M2M
			{
				MUID uidStage;
				if (pCommand->GetParameter(&uidStage, 0, MPT_UID)==false) break;
				OnStageReserve(uidStage);
				return true;
			}
		case MC_AGENT_RELAY_PEER:		// M2M
			{
				MUID uidChar, uidPeer, uidStage;

				if (pCommand->GetParameter(&uidChar, 0, MPT_UID)==false) break;
				if (pCommand->GetParameter(&uidPeer, 1, MPT_UID)==false) break;
				if (pCommand->GetParameter(&uidStage, 2, MPT_UID)==false) break;

				OnRelayPeer(uidChar, uidPeer, uidStage);
				return true;
			}
		case MC_AGENT_PEER_BINDTCP:		// M2M
			{
				MUID uidChar;
				if (pCommand->GetParameter(&uidChar, 0, MPT_UID)==false) break;

				OnPeerBindTCP(pCommand->GetSenderUID(), uidChar);
				return true;
			}
		case MC_AGENT_PEER_BINDUDP:		// M2M
			{
				MUID uidChar;
				char szLocalIP[64]=""; 
				DWORD nLocalPort;
				char szIP[64]=""; 
				DWORD nPort;

				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(szLocalIP, 1, MPT_STR, sizeof(szLocalIP));
				pCommand->GetParameter(&nLocalPort, 2, MPT_UINT);
				pCommand->GetParameter(szIP, 3, MPT_STR, sizeof(szIP));
				pCommand->GetParameter(&nPort, 4, MPT_UINT);

				OnPeerBindUDP(uidChar, szLocalIP, nLocalPort, szIP, nPort);
				return true;
			}
		case MC_AGENT_PEER_UNBIND:	// M2M
			{
				MUID uidChar;
				if (pCommand->GetParameter(&uidChar, 0, MPT_UID)==false) break;

				OnPeerUnbind(pCommand->GetSenderUID(), uidChar);
				return true;
			}
		case MC_AGENT_TUNNELING_TCP:	// M2M
			{
				MUID uidSender, uidReceiver;
				if (pCommand->GetParameter(&uidSender, 0, MPT_UID)==false) break;
				if (pCommand->GetParameter(&uidReceiver, 1, MPT_UID)==false) break;
				
				MCommandParameter* pParam = pCommand->GetParameter(2);
				if (pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				if( NULL == pBlob ) break;
				int nCount = MGetBlobArrayCount(pBlob);

				OnTunnelingTCP(uidSender, uidReceiver, pBlob, nCount);
				return true;
			}
		case MC_AGENT_TUNNELING_UDP:	// P2P
			{
				MUID uidSender, uidReceiver;
				if (pCommand->GetParameter(&uidSender, 0, MPT_UID)==false) break;
				if (pCommand->GetParameter(&uidReceiver, 1, MPT_UID)==false) break;
				
				MCommandParameter* pParam = pCommand->GetParameter(2);
				if (pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				if( NULL == pBlob ) break;
				int nCount = MGetBlobArrayCount(pBlob);

				// 전체 블럭파라미터 사이즈보다 안쪽 blockArray사이즈가 클경우
				if( MGetBlobArraySize(pBlob) > ((MCommandParameterBlob*)pParam)->m_nSize )
					break;

				OnTunnelingUDP(uidSender, uidReceiver, pBlob, nCount);
				return true;
			}
		case MC_AGENT_DEBUGTEST:		// M2M
			{
				char szMsg[256]="";
				if (pCommand->GetParameter(szMsg, 0, MPT_STR, sizeof(szMsg))==false) break;

                OnDebugTest(pCommand->GetSenderUID(), szMsg);
				return true;
			}

		default:
//			_ASSERT(0);	// 아직 핸들러가 없다.
			return false;
	};
	return false;
}

void MMatchAgent::OnRun(void)
{
	unsigned long int nGlobalClock = GetGlobalClockCount();
	for(MAgentClients::iterator i=m_Clients.begin(); i!=m_Clients.end();){
		MAgentClient* pObj = (*i).second;
		pObj->Tick(nGlobalClock);

		if (pObj->CheckDestroy(nGlobalClock) == true) {
			ClientRemove(pObj->GetUID(), &i);
		} else {
			i++;
		}
	}

	for(MStageAgents::iterator iStage=m_Stages.begin(); iStage!=m_Stages.end();){
		MStageAgent* pStage = (*iStage).second;
		pStage->Tick(nGlobalClock);
		if (pStage->CheckDestroy() == true) {
			StageRemove(pStage->GetUID(), &iStage);
		}else {
			iStage++;
		}
	}
//	if ( (GetMatchServerConnected() == false) && (GetMatchServerTrying()==false) ) {
	if (GetMatchServerConnected() == false) {
		#define TIME_MATCH_CONNECT_TRYING	10000
		static unsigned long tmLastConnectTrying = nGlobalClock;
		if (nGlobalClock - tmLastConnectTrying > TIME_MATCH_CONNECT_TRYING) {

			LOG(LOG_PROG, "Reconnect to MatchServer\n");

			tmLastConnectTrying = nGlobalClock;
			ConnectToMatchServer(NULL, 0);
		}
	} else {
		if (GetLastLiveCheckRecvTime() == 0) {
			if (nGlobalClock - GetConnectedTime() > 3000)
				RequestLiveCheck();
		} else {
			// 150초
			#define TIME_MATCH_KEEPALIVE_LIMIT	150000
			int nTimeWait = (int)GetLastLiveCheckRecvTime() - (int)nGlobalClock;
			if (abs(nTimeWait) > TIME_MATCH_KEEPALIVE_LIMIT)	{
//				LOG(LOG_PROG, "KEEPALIVE TimeOut (%d)", nTimeWait);
				DisconnectFromMatchServer();
				SetLastLiveCheckRecvTime( nGlobalClock );
				return;
			}
		
			#define TIME_MATCH_KEEPALIVE_CYCLE	10000
			static unsigned long tmLastKeepAlive = nGlobalClock;
			if ( (nGlobalClock - tmLastKeepAlive > TIME_MATCH_KEEPALIVE_CYCLE) &&
				(GetLastLiveCheckRecvTime() > GetLastLiveCheckSendTime()) ) {
				tmLastKeepAlive = nGlobalClock;
				RequestLiveCheck();
			}
		}
	}

#define MINTERVAL_GARBAGE_SESSION_PING	(3 * 60 * 1000)	// 3 min
	static unsigned long tmLastGarbageSessionCleaning = nGlobalClock;
	if (nGlobalClock - tmLastGarbageSessionCleaning > MINTERVAL_GARBAGE_SESSION_PING)
	{
		tmLastGarbageSessionCleaning = nGlobalClock;

		LOG(LOG_PROG, "GARBAGE SESSION CLEANING : ClientCount=%d, StageCount=%d, SessionCount=%d\n", 
			GetClientCount(), GetStageCount(), GetCommObjCount());
	}

}

void MMatchAgent::OnInitialize()
{
	Log(LOG_PROG, "Initilization completed");
}

MUID MMatchAgent::UseUID(void)
{
	MUID uid = m_uidNextAlloc;
	m_uidNextAlloc.Increase();
	return uid;
}

int MakeCmdPacket(char* pOutPacket, int iMaxPacketSize, MPacketCrypter* pPacketCrypter, MCommand* pCommand)
{
	MCommandMsg* pMsg = (MCommandMsg*)pOutPacket;
	int nCmdSize = iMaxPacketSize-sizeof(MPacketHeader);

	pMsg->Buffer[0] = 0;
	pMsg->nCheckSum = 0;
	int nPacketSize = 0;

	if (pCommand->m_pCommandDesc->IsFlag(MCCT_NON_ENCRYPTED) == false)
	{
		_ASSERT(0);

		pMsg->nMsg = MSGID_COMMAND;

		nCmdSize = pCommand->GetData(pMsg->Buffer, nCmdSize);
		pMsg->nSize = (unsigned short)( sizeof(MPacketHeader) + nCmdSize );
		nPacketSize = pMsg->nSize;

		if (pPacketCrypter)
		{
			if (!pPacketCrypter->Encrypt((char*)&pMsg->nSize, sizeof(unsigned short)))
			{
				return 0;
			}

			if (!pPacketCrypter->Encrypt(pMsg->Buffer, nCmdSize))
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}


	}
	else
	{
		pMsg->nMsg = MSGID_RAWCOMMAND;

		nCmdSize = pCommand->GetData(pMsg->Buffer, nCmdSize);
		pMsg->nSize = (unsigned short)( sizeof(MPacketHeader) + nCmdSize );
		nPacketSize = pMsg->nSize;
	}

	pMsg->nCheckSum = MBuildCheckSum(pMsg, nPacketSize);

	return pMsg->nSize;

}

void MMatchAgent::SendCommandByUDP(MCommand* pCommand, char* szIP, int nPort)
{
	int nPacketSize = CalcPacketSize(pCommand);
	char* pSendBuf = new char[nPacketSize];

	int nSize = MakeCmdPacket(pSendBuf, nPacketSize, &m_MatchServerPacketCrypter, pCommand);

	if (nSize > 0)
	{
		m_SafeUDP.Send(szIP, nPort, pSendBuf, nSize);
	}
	else
	{
		delete [] pSendBuf;
	}

	delete pCommand;

}

bool MMatchAgent::UDPSocketRecvEvent(DWORD dwIP, WORD wRawPort, char* pPacket, DWORD dwSize)
{
	if (dwSize < sizeof(MPacketHeader)) return false;

	MPacketHeader*	pPacketHeader;
	pPacketHeader = (MPacketHeader*)pPacket;

	if ((dwSize < pPacketHeader->nSize) || 
		((pPacketHeader->nMsg != MSGID_COMMAND) && (pPacketHeader->nMsg != MSGID_RAWCOMMAND)) ) return false;

	MMatchAgent* pServer = MMatchAgent::GetInstance();
	pServer->ParseUDPPacket(&pPacket[sizeof(MPacketHeader)], pPacketHeader, dwIP, wRawPort);
	return true;
}

void MMatchAgent::ParseUDPPacket(char* pData, MPacketHeader* pPacketHeader, DWORD dwIP, WORD wRawPort)
{
	switch (pPacketHeader->nMsg)
	{
	case MSGID_RAWCOMMAND:
		{
			MCommand* pCmd = new MCommand();
			if( !pCmd->SetData(pData, &m_CommandManager) )
			{
				delete pCmd;
				return;
			}

			if (pCmd->GetID() == MC_UDP_PING)
			{
				unsigned int nTimeStamp;
				if (pCmd->GetParameter(&nTimeStamp, 0, MPT_UINT)==false) break;

				MCommand* pCommand = CreateCommand(MC_UDP_PONG, MUID(0,0));
				pCommand->AddParameter(new MCmdParamUInt( (unsigned int)inet_addr( GetIPString() ) ));
				pCommand->AddParameter(new MCmdParamUInt(nTimeStamp));

				sockaddr_in Addr;
				Addr.sin_addr.S_un.S_addr = dwIP;
				char* pszClient_IP = inet_ntoa(Addr.sin_addr);
				SendCommandByUDP(pCommand, pszClient_IP, ntohs(wRawPort) );

				delete pCmd;
			} else if (pCmd->GetID() == MC_AGENT_PEER_BINDUDP) {
				pCmd->m_Sender = MUID(0,0);
				pCmd->m_Receiver = m_This;

				sockaddr_in Addr;
				Addr.sin_addr.S_un.S_addr = dwIP;
				Addr.sin_port = wRawPort;
				char* pszIP = inet_ntoa(Addr.sin_addr);
				unsigned int nPort = ntohs(Addr.sin_port);

				MCommandParameterString* pParamIP = (MCommandParameterString*)pCmd->GetParameter(3);
				MCommandParameterUInt* pParamPort = (MCommandParameterUInt*)pCmd->GetParameter(4);
				if (pParamIP==NULL || pParamIP->GetType()!=MPT_STR)
					break;
				if (pParamPort==NULL || pParamPort->GetType()!=MPT_UINT)
					break;

				char pData[1024];
				MCommandParameterString(pszIP).GetData(pData, 1024);
				pParamIP->SetData(pData);
				MCommandParameterUInt(nPort).GetData(pData, 1024);
				pParamPort->SetData(pData);

				PostSafeQueue(pCmd);
			} else if (pCmd->GetID() == MC_AGENT_TUNNELING_UDP) {
				pCmd->m_Sender = MUID(0,0);
				pCmd->m_Receiver = m_This;

				PostSafeQueue(pCmd);
			} 
		}
		break;
	case MSGID_COMMAND:
		{
			LOG(LOG_FILE, "MMatchAgent::ParseUDPPacket - Not Used\n");
		}
		break;
	default:
		{
			LOG(LOG_FILE, "MatchClient: Parse Packet Error\n");
		}
		break;
	}
}

int MMatchAgent::ClientAdd(const MUID& uid)
{
	if (GetClient(uid))
		return MERR_UNKNOWN;

	MAgentClient* pObj = new MAgentClient(uid);
	pObj->UpdateLastRecvPacketTime( GetGlobalClockCount() );
	m_Clients.insert(MAgentClients::value_type(uid, pObj));

	// 너무 많은 로그를 남겨 주석처리함 - bird(2005/09/30)
	//LOG(LOG_PROG, "Client Added (UID:%d%d)", uid.High, uid.Low);

	return MOK;
}

int MMatchAgent::ClientRemove(const MUID& uid, MAgentClients::iterator* pNextItor)
{
	MAgentClients::iterator i = m_Clients.find(uid);
	if(i==m_Clients.end()) return MERR_OBJECT_INVALID;

	MAgentClient* pObj = (*i).second;

	if (pObj->GetStageUID() != MUID(0,0)) {
		StageLeave(pObj->GetUID(), pObj->GetStageUID());
	}

	delete pObj;

	MAgentClients::iterator itorTemp = m_Clients.erase(i);
	if (pNextItor)
		*pNextItor = itorTemp;

	return MOK;
}

MAgentClient* MMatchAgent::GetClient(const MUID& uid)
{
	MAgentClients::iterator i = m_Clients.find(uid);
	if(i==m_Clients.end()) return NULL;
	return (*i).second;
}

MAgentClient* MMatchAgent::GetPlayerByCommUID(const MUID& uid)
{
	for(MAgentClients::iterator i=m_Clients.begin(); i!=m_Clients.end(); i++){
		MAgentClient* pObj = ((*i).second);
		for (list<MUID>::iterator j=pObj->m_CommListener.begin(); j!=pObj->m_CommListener.end(); j++){
			MUID TargetUID = *j;
			if (TargetUID == uid)
				return pObj;
		}
	}
	return NULL;
}

int MMatchAgent::OnAccept(MCommObject* pCommObj)
{
	int nErrCode = MServer::OnAccept(pCommObj);

	if (nErrCode != MOK) {
		if (nErrCode == MERR_COMMUNICATOR_HAS_NOT_UID) {
			LOG(LOG_FILE, "Agent is Not registered to MatchServer\n");
			Disconnect(pCommObj->GetUID());
		}
		return nErrCode;
	}

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_AGENT_LOCAL_LOGIN), m_This, m_This);
	pNew->AddParameter(new MCommandParameterUID(pCommObj->GetUID()));
	pNew->AddParameter(new MCommandParameterUID(MUID(0,0)));
	PostSafeQueue(pNew);

	return MOK;
}

int MMatchAgent::OnConnected(MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp, MCommObject* pCommObj)
{
	if (GetMatchServerConnected() == true)
	{
		return MERR_UNKNOWN;
	}

	int nRetCode = MServer::OnConnected(pTargetUID, pAllocUID, nTimeStamp, pCommObj);
    if (nRetCode != MOK) return nRetCode;

	m_uidMatchServer = *pTargetUID;
	m_This = *pAllocUID;
	SetMatchServerConnected(true);
	SetMatchServerTrying(false);
	SetConnectedTime(GetGlobalClockCount());

	// 매치서버와의 암호키 설정
	MCommandBuilder* pCmdBuilder = pCommObj->GetCommandBuilder();
	if (pCmdBuilder)
	{
		MPacketCrypterKey key;
		MMakeSeedKey(&key, m_uidMatchServer, m_This, nTimeStamp);
		m_MatchServerPacketCrypter.InitKey(&key);
		pCommObj->GetCrypter()->InitKey(&key);
		pCmdBuilder->InitCrypt(pCommObj->GetCrypter(), false);
	}

////////////////////
//MCommand* pCmdTest = CreateCommand(MC_NET_PING, GetMatchServerUID());
//pCmdTest->AddParameter(new MCmdParamUInt(65536));
//PostSafeQueue(pCmdTest);
////////////////////

	// Register Agent to MatchServer
	MCommand* pCmd = CreateCommand(MC_MATCH_REGISTERAGENT, GetMatchServerUID());
	pCmd->AddParameter(new MCmdParamStr(GetIPString()));
	pCmd->AddParameter(new MCmdParamInt(GetTCPPort()));
	pCmd->AddParameter(new MCmdParamInt(GetUDPPort()));
	PostSafeQueue(pCmd);

	// Log(LOG_FILE, "MatchServer connected");	// Log is Not ThreadSafe

//////////////////////////////////
//MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_ECHO), GetMatchServerUID(), GetUID());
//pNew->AddParameter(new MCommandParameterString("AgentEcho!!"));
//Post(pNew);
//////////////////////////////////
	return MOK;
}

int MMatchAgent::OnDisconnect(MCommObject* pCommObj)
{
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_CLEAR), m_This, m_This);
	pNew->AddParameter(new MCommandParameterUID( pCommObj ? pCommObj->GetUID() : MUID(0,0) ));
	PostSafeQueue(pNew);

	return MOK;
}

void MMatchAgent::OnAgentLocalLogin(const MUID& uidComm, const MUID& uidPlayer)
{	
	char szIP[128] = {0, };
	int nPort = 0;
	bool bGetComm = false;

	LockCommList();
	{
		MCommObject* pCommObj = (MCommObject*)m_CommRefCache.GetRef(uidComm);
		if(pCommObj){
			strcpy(szIP, pCommObj->GetIPString());
			nPort = pCommObj->GetPort();
			bGetComm = true;
		}
	}	
	UnlockCommList();


	if( bGetComm ) {
		LOG(LOG_PROG, "Agent Local Login (UID:%d%d, IP:%s, Port:%d)\n", uidComm.High, uidComm.Low, szIP, nPort);
	}

//	MCommand* pCmd = CreateCommand(MC_AGENT_RESPONSE_LOGIN, uidComm);
//	Post(pCmd);
}

bool MMatchAgent::StageAdd(const MUID& uidStage)
{
	MStageAgent* pStage= new MStageAgent;
	pStage->Create(uidStage);
	m_Stages.Insert(uidStage, pStage);

	// 너무 많은 로그를 남겨 주석처리함. - bird(2005/09/30)
	//LOG(LOG_PROG, "Open Stage (UID:%d%d)", uidStage.High, uidStage.Low);
	return true;
}

bool MMatchAgent::StageRemove(const MUID& uidStage, MStageAgents::iterator* pNextItor)
{
	MStageAgents::iterator i = m_Stages.find(uidStage);
	if(i==m_Stages.end()) return false;
	
	MStageAgent* pStage = (*i).second;
	// 너무 많은 로그를 남겨 주석처리함. - bird(2005/09/30)
	//LOG(LOG_PROG, "Closing Stage (UID:%d%d)", uidStage.High, uidStage.Low);

	MStageAgents::iterator itorTemp = m_Stages.erase(i);
	if (pNextItor) {
		*pNextItor = itorTemp;
	}
	delete pStage;

	return true;
}

bool MMatchAgent::StageJoin(const MUID& uidPlayer, const MUID& uidStage)
{
	MAgentClient* pClient = GetClient(uidPlayer);
	if (pClient == NULL) return false;
	if (pClient->GetStageUID() != MUID(0,0) && pClient->GetStageUID() != uidStage)
		StageLeave(uidPlayer, pClient->GetStageUID());

	MStageAgent* pStage = FindStage(uidStage);
	if (pStage == NULL) {
		StageAdd(uidStage);
		pStage = FindStage(uidStage);
		if (pStage == NULL)
			return false;
	}

	// Join
	pClient->SetStageUID(uidStage);

	if (pStage->IsExistClient(uidPlayer))
		pStage->RemoveObject(uidPlayer);

	pStage->AddObject(uidPlayer, pClient);

	return true;
}

bool MMatchAgent::StageLeave(const MUID& uidPlayer, const MUID& uidStage)
{
	// mlog("StageLeave Stage(%u%u) Player(%u%u)\n", uidStage.High, uidStage.Low, uidPlayer.High, uidPlayer.Low);

	MAgentClient* pClient = GetClient(uidPlayer);
	if (pClient == NULL) return false;

	MStageAgent* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) 
	{
		MUID uidCurrent = (*i).first;
		if (uidCurrent == uidPlayer) 
			continue;

		pClient->RemovePeerRoute(uidCurrent);

		MAgentClient* pRoomMate = GetClient(uidCurrent);
		if (pRoomMate)
			pRoomMate->RemovePeerRoute(uidPlayer);
	}

	pStage->RemoveObject(uidPlayer);
	pClient->SetStageUID(MUID(0,0));
	return true;
}

MStageAgent* MMatchAgent::FindStage(const MUID& uidStage)
{
	MStageAgents::iterator i = m_Stages.find(uidStage);
	if(i==m_Stages.end()) return NULL;

	MStageAgent* pStage = (*i).second;
	return pStage;
}

void MMatchAgent::RouteToListener(MObject* pObject, MCommand* pCommand)
{
	size_t nListenerCount = pObject->m_CommListener.size();
	if (nListenerCount <= 0) {
		delete pCommand;
		return;
	} else if (nListenerCount == 1) {
		MUID TargetUID = *pObject->m_CommListener.begin();
		pCommand->m_Receiver = TargetUID;
		Post(pCommand);
	} else {
		int nCount = 0;
		for (list<MUID>::iterator itorUID=pObject->m_CommListener.begin(); itorUID!=pObject->m_CommListener.end(); itorUID++) {
			MUID TargetUID = *itorUID;

			MCommand* pSendCmd;
			if (nCount<=0)
				pSendCmd = pCommand;
			else
				pSendCmd = pCommand->Clone();
			pSendCmd->m_Receiver = TargetUID;
			Post(pSendCmd);
			nCount++;
		}
	}
}

/*
void MMatchAgent::RouteToStage(const MUID& uidStage, MCommand* pCommand)
{
	MStageAgent* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MObject* pObj = (MObject*)(*i).second;

		MCommand* pSendCmd = pCommand->Clone();
		RouteToListener(pObj, pSendCmd);
	}
	delete pCommand;
}
*/

unsigned long int MMatchAgent::GetGlobalClockCount(void)
{
	unsigned long int i = timeGetTime();
	return i;
}

unsigned long int MMatchAgent::ConvertLocalClockToGlobalClock(unsigned long int nLocalClock, unsigned long int nLocalClockDistance)
{
	return (nLocalClock + nLocalClockDistance);
}

unsigned long int MMatchAgent::ConvertGlobalClockToLocalClock(unsigned long int nGlobalClock, unsigned long int nLocalClockDistance)
{
	return (nGlobalClock - nLocalClockDistance);
}

void MMatchAgent::DebugTest()
{
///////////
//	LOG("DebugTest: Object List");
//	for(MAgentClients::iterator it=m_Clients.begin(); it!=m_Clients.end(); it++){
//		MAgentClient* pObj = (*it).second;
//		LOG("DebugTest: Obj(%d%d)", pObj->GetUID().High, pObj->GetUID().Low);
//	}
///////////
}

void MMatchAgent::OnStageReserve(const MUID& uidStage)
{
//	MStageAgent* pStage = FindStage(uidStage);
//	if (pStage == NULL)
//	StageAdd(uidStage);	없어도 잘될것이다

	MCommand* pCmd = CreateCommand(MC_AGENT_STAGE_READY, GetMatchServerUID());
	pCmd->AddParameter(new MCmdParamUID(uidStage));
	Post(pCmd);
}

void MMatchAgent::OnRelayPeer(const MUID& uidChar, const MUID& uidPeer, const MUID& uidStage)
{
	MAgentClient* pChar = GetClient(uidChar);
	if (pChar == NULL) {
		ClientAdd(uidChar);
        pChar = GetClient(uidChar);
		if (StageJoin(uidChar, uidStage) == false) return;
	}

	MAgentClient* pPeer = GetClient(uidPeer);
	if (pPeer == NULL) {
		ClientAdd(uidPeer);
		pPeer = GetClient(uidPeer);
		if (StageJoin(uidPeer, uidStage) == false) return;
	}

	pPeer->AddPeerRoute(uidChar);
	pChar->AddPeerRoute(uidPeer);

	// Reply to Matchserver
	MCommand* pCmd = CreateCommand(MC_AGENT_PEER_READY, GetMatchServerUID());
	pCmd->AddParameter(new MCmdParamUID(uidChar));
	pCmd->AddParameter(new MCmdParamUID(uidPeer));
	Post(pCmd);
}

void MMatchAgent::OnPeerBindTCP(const MUID& uidComm, const MUID& uidChar)
{
	MAgentClient* pChar = GetClient(uidChar);
	if (pChar == NULL) {
		LOG(LOG_PROG, "PeerBindTCP Error : Unknown Peer(%d%d)\n", uidChar.High, uidChar.Low);
		return;
	}

	if (pChar->IsCommListener(uidComm))
		return;

	pChar->AddCommListener(uidComm);
	pChar->SetPeerType(AGENT_PEERTYPE_TCP);

	// 너무 많은 로그를 남겨 주석처리함 - bird(2005/09/30)
	//LOG(LOG_PROG, "PeerBindTCP Communicator(%d%d) to Client(%d%d)", uidComm.High, uidComm.Low, uidChar.High, uidChar.Low);

	MCommand* pCmd = CreateCommand(MC_AGENT_ALLOW_TUNNELING_TCP, uidComm);
	Post(pCmd);
}

void MMatchAgent::OnPeerBindUDP(const MUID& uidChar, char* szLocalIP, int nLocalPort, char* szIP, int nPort)
{
	MAgentClient* pChar = GetClient(uidChar);
	if (pChar == NULL) {
		LOG(LOG_PROG, "PeerBindUDP Error : Unknown Peer(%d%d)\n", uidChar.High, uidChar.Low);
		return;
	}

	if (pChar->GetPeerType() == AGENT_PEERTYPE_IDLE) {
		pChar->SetPeerAddr(szIP, nPort);
		pChar->SetPeerType(AGENT_PEERTYPE_UDP);
	}

	// 너무 많은 로그를 남겨 주석처리함 - bird(2005/09/30)
	//LOG(LOG_PROG, "PeerBindUDP Address(%s:%d) to Client(%d%d)", szIP, nPort, uidChar.High, uidChar.Low);

	MCommand* pCmd = CreateCommand(MC_AGENT_ALLOW_TUNNELING_UDP, MUID(0,0));
	SendCommandByUDP(pCmd, szIP, nPort);
}

void MMatchAgent::OnPeerUnbind(const MUID& uidComm, const MUID& uidChar)
{
	MAgentClient* pClient = GetClient(uidChar);
	if (pClient == NULL)
		return;
	
	ClientRemove(uidChar, NULL);
}

void MMatchAgent::OnTunnelingTCP(const MUID& uidSender, const MUID& uidReceiver, void* pBlob, int nCount)
{
	MAgentClient* pClient = GetClient(uidSender);
	if (pClient == NULL)
		return;

	pClient->UpdateLastRecvPacketTime( GetGlobalClockCount() );

	RoutePeerTunnel(pClient, uidReceiver, pBlob, nCount);
}

void MMatchAgent::OnTunnelingUDP(const MUID& uidSender, const MUID& uidReceiver, void* pBlob, int nCount)
{
	MAgentClient* pClient = GetClient(uidSender);
	if (pClient == NULL)
		return;

	pClient->UpdateLastRecvPacketTime( GetGlobalClockCount() );

	RoutePeerTunnel(pClient, uidReceiver, pBlob, nCount);
}

void MMatchAgent::SendPeerTunnel(MAgentClient* pClient, MAgentClient* pTarget, void* pBlob,int nCount)
{
	_ASSERT(pClient);
	_ASSERT(pTarget);

	if (pTarget->GetPeerType() == AGENT_PEERTYPE_IDLE) {
		return;
	} else if (pTarget->GetPeerType() == AGENT_PEERTYPE_TCP) {
		if (pTarget->HasCommListener()) {
			MCommand* pCmd = CreateCommand(MC_AGENT_TUNNELING_TCP, pTarget->GetCommListener());
			pCmd->AddParameter(new MCmdParamUID(pClient->GetUID()));
			pCmd->AddParameter(new MCmdParamUID(pTarget->GetUID()));
			pCmd->AddParameter(new MCmdParamBlob(pBlob, MGetBlobArraySize(pBlob)));
			Post(pCmd);
		}
	} else if (pTarget->GetPeerType() == AGENT_PEERTYPE_UDP) {
		MCommand* pCmd = CreateCommand(MC_AGENT_TUNNELING_UDP, pTarget->GetCommListener());
		pCmd->AddParameter(new MCmdParamUID(pClient->GetUID()));
		pCmd->AddParameter(new MCmdParamUID(pTarget->GetUID()));
		pCmd->AddParameter(new MCmdParamBlob(pBlob, MGetBlobArraySize(pBlob)));
		SendCommandByUDP(pCmd, pTarget->GetIP(), pTarget->GetPort());
	}	
}

void MMatchAgent::RoutePeerTunnel(MAgentClient* pClient,const MUID& uidReceiver,void* pBlob,int nCount)
{
//	char* pPacket = (char*)MGetBlobArrayElement(pBlob, 0);
	if (uidReceiver != MUID(0,0)) {
		MAgentClient* pTargetPeer = GetClient(uidReceiver);
		if (pTargetPeer == NULL) {
			pClient->RemovePeerRoute(uidReceiver);
			return;
		}
		if (pClient->ExamPeerRoute(uidReceiver)) {
			MStageAgent* pStage = FindStage(pClient->GetStageUID());
			if (pStage && pStage->IsExistClient(uidReceiver)) {
				SendPeerTunnel(pClient, pTargetPeer, pBlob, nCount);
			} else {
				LOG(LOG_PROG, "ForceRemoteRoute Stage(%u%u):Player(%u%u) -> Stage(%u%u):Player(%u%u)\n",
					pClient->GetStageUID().High, pClient->GetStageUID().Low, pClient->GetUID().High, pClient->GetUID().Low,
					pTargetPeer->GetStageUID().High, pTargetPeer->GetStageUID().Low, pTargetPeer->GetUID().High, pTargetPeer->GetUID().Low);
				pClient->RemovePeerRoute(uidReceiver);
			}
		}
	} else {
		for (list<MUID>::iterator i=pClient->GetPeerRouteBegin(); i!=pClient->GetPeerRouteEnd(); )
		{
			MAgentClient* pTargetPeer = GetClient(*i);
			if (pTargetPeer == NULL) {
				i = pClient->RemovePeerRoute(*i);
				continue;
			}
			SendPeerTunnel(pClient, pTargetPeer, pBlob, nCount);

			++i;
		}
	}
}

void MMatchAgent::OnDebugTest(const MUID& uidComm, const char* pszMsg)
{
}

