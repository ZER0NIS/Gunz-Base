#include "stdafx.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MAgentObject.h"
#include "MDebug.h"
#include "MCommandCommunicator.h"
#include "MCommandBuilder.h"

int MMatchServer::AgentAdd(const MUID& uidComm)
{
	MAgentObject* pAgent = new MAgentObject(uidComm);

	m_AgentMap.insert(MAgentObjectMap::value_type(pAgent->GetUID(), pAgent));

	LOG(LOG_PROG, "Agent Added (UID:%d%d)", pAgent->GetUID().High, pAgent->GetUID().Low);

	return MOK;
}

int MMatchServer::AgentRemove(const MUID& uidAgent, MAgentObjectMap::iterator* pNextItor)
{
	MAgentObjectMap::iterator i = m_AgentMap.find(uidAgent);
	if (i == m_AgentMap.end()) return MERR_OBJECT_INVALID;

	MAgentObject* pAgent = (*i).second;

	LOG(LOG_PROG, "Agent Removed (UID:%d%d)", pAgent->GetUID().High, pAgent->GetUID().Low);

	// Clear up the Agent
	delete pAgent;

	MAgentObjectMap::iterator itorTemp = m_AgentMap.erase(i);
	if (pNextItor)
		*pNextItor = itorTemp;

	return MOK;
}

void MMatchServer::AgentClear()
{
	MAgentObjectMap::iterator i = m_AgentMap.begin();
	// Custom: Iterator dereference fix
	for (; i != m_AgentMap.end();)
	{
		AgentRemove(i->first, &i);
	}
}

MAgentObject* MMatchServer::GetAgent(const MUID& uidAgent)
{
	MAgentObjectMap::iterator i = m_AgentMap.find(uidAgent);
	if (i == m_AgentMap.end()) return NULL;
	return (*i).second;
}

MAgentObject* MMatchServer::GetAgentByCommUID(const MUID& uidComm)
{
	for (MAgentObjectMap::iterator i = m_AgentMap.begin(); i != m_AgentMap.end(); i++) {
		MAgentObject* pAgent = ((*i).second);
		for (list<MUID>::iterator j = pAgent->m_CommListener.begin(); j != pAgent->m_CommListener.end(); j++) {
			MUID TargetUID = *j;
			if (TargetUID == uidComm)
				return pAgent;
		}
	}
	return NULL;
}

MAgentObject* MMatchServer::FindFreeAgent()
{
	MAgentObject* pFreeAgent = NULL;
	for (MAgentObjectMap::iterator i = m_AgentMap.begin(); i != m_AgentMap.end(); i++) {
		MAgentObject* pAgent = (*i).second;
		if ((pFreeAgent == NULL) || (pFreeAgent->GetAssignCount() > pAgent->GetStageCount()))
			pFreeAgent = pAgent;
	}
	return pFreeAgent;
}

void MMatchServer::ReserveAgent(MMatchStage* pStage)
{
	MAgentObject* pFreeAgent = FindFreeAgent();
	if (pFreeAgent == NULL) {
		LOG(LOG_DEBUG, "No available Agent (Stage %d%d)", pStage->GetUID().High, pStage->GetUID().Low);
		return;
	}
	pStage->SetAgentUID(pFreeAgent->GetUID());

	MCommand* pCmd = CreateCommand(MC_AGENT_STAGE_RESERVE, pFreeAgent->GetCommListener());
	pCmd->AddParameter(new MCmdParamUID(pStage->GetUID()));
	Post(pCmd);
}

void MMatchServer::LocateAgentToClient(const MUID& uidPlayer, const MUID& uidAgent)
{
	MAgentObject* pAgent = GetAgent(uidAgent);
	if (pAgent == NULL)
		return;

	char szCharName[64];
	MMatchObject* pChar = GetObject(uidPlayer);
	sprintf(szCharName, "%s(%d%d)", (pChar ? pChar->GetAccountName() : "?"), uidPlayer.High, uidPlayer.Low);
	LOG(LOG_DEBUG, "Locate Agent : Locate Agent(%d%d) to Player %s ", uidAgent.High, uidAgent.Low, szCharName);

	static int nCount = 0;
	if (10 > nCount++)
	{
		mlog("Locate Agent : Locate Agent(%d%d) to Player %s. IP(%s), Port(%d)\n"
			, uidAgent.High
			, uidAgent.Low
			, szCharName
			, pAgent->GetIP()
			, pAgent->GetUDPPort());
	}

	MCommand* pCmd = CreateCommand(MC_AGENT_LOCATETO_CLIENT, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamUID(uidAgent));
	pCmd->AddParameter(new MCmdParamStr(pAgent->GetIP()));
	pCmd->AddParameter(new MCmdParamInt(pAgent->GetTCPPort()));
	pCmd->AddParameter(new MCmdParamInt(pAgent->GetUDPPort()));
	RouteToListener(pChar, pCmd);
}

void MMatchServer::OnRegisterAgent(const MUID& uidComm, char* szIP, int nTCPPort, int nUDPPort)
{
	// Custom: Fix agent invalid ip crash
	if (strstr(szIP, "%") != NULL)
	{
		return;
	}

	LOG(LOG_PROG, "Start agent Register (CommUID %u:%u) IP:%s, TCPPort:%d, UDPPort:%d\n ",
		uidComm.High, uidComm.Low, szIP, nTCPPort, nUDPPort);

	// Remove Old Connections ////////////////////////
	MAgentObject* pOldAgent = NULL;
	while (pOldAgent = FindFreeAgent())
	{
		AgentRemove(pOldAgent->GetUID(), NULL);
	}
	//////////////////////////////////////////////////

	int nErrCode = AgentAdd(uidComm);
	if (nErrCode != MOK)
	{
		LOG(LOG_DEBUG, MErrStr(nErrCode));
	}

	// ��Ŷ �ø��� üũ�� ���� �ʵ��� �Ѵ�.
	LockCommList();
	MCommObject* pCommObj = (MCommObject*)m_CommRefCache.GetRef(uidComm);
	if (pCommObj)
	{
		pCommObj->GetCommandBuilder()->SetCheckCommandSN(false);
	}
	UnlockCommList();

	MAgentObject* pAgent = GetAgent(uidComm);
	pAgent->AddCommListener(uidComm);
	pAgent->SetAddr(szIP, nTCPPort, nUDPPort);

	//	SetClientClockSynchronize(uidComm);
	LOG(LOG_PROG, "Agent Registered (CommUID %u:%u) IP:%s, TCPPort:%d, UDPPort:%d\n ",
		uidComm.High, uidComm.Low, szIP, nTCPPort, nUDPPort);

	/*	//// IOCP DEBUG RAONHAJE ///////////////////////////////////
		char* pszAlpha = "abcdefghijklmnopqrstuvwxyz";
		int nAlphaLen = strlen(pszAlpha);
		char szBulk[200];	ZeroMemory(szBulk, 200);
		for (int i=0; i<200; i++)
			szBulk[i] = pszAlpha[i%nAlphaLen];
		for (i=0; i<1000; i++) {
			char szMsg[512];
			sprintf(szMsg, "TEST_STRING=%d  (%s)", i, szBulk);

			MCommand* pCmd = CreateCommand(MC_AGENT_DEBUGTEST, pAgent->GetCommListener());
			pCmd->AddParameter(new MCmdParamStr(szMsg));
			Post(pCmd);
		}
		//////////////////////////////////////////////////////////// */
}

void MMatchServer::OnUnRegisterAgent(const MUID& uidComm)
{
	MAgentObject* pAgent = GetAgentByCommUID(uidComm);
	if (pAgent)
		AgentRemove(pAgent->GetUID(), NULL);

	LOG(LOG_DEBUG, "Agent Unregistered (CommUID %u:%u) Cleared", uidComm.High, uidComm.Low);
}
void MMatchServer::OnAgentStageReady(const MUID& uidCommAgent, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	MAgentObject* pAgent = GetAgentByCommUID(uidCommAgent);
	if (pAgent == NULL) return;

	pStage->SetAgentReady(true);

	LOG(LOG_DEBUG, "Agent Ready to Handle Stage(%d%d)", uidStage.High, uidStage.Low);

	/*	for (MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++) {
			MMatchObject* pObj = (*i).second;
			if (pObj->GetBridgePeer() == false)
				LocateAgentToClient(pObj->GetUID(), pAgent->GetUID());
		}*/
}

void MMatchServer::OnRequestLiveCheck(const MUID& uidComm, unsigned long nTimeStamp, unsigned long nStageCount, unsigned long nUserCount)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_AGENT_RESPONSE_LIVECHECK, uidComm);
	pCmd->AddParameter(new MCmdParamUInt(nTimeStamp));
	PostSafeQueue(pCmd);
}

void MMatchServer::OnPeerReady(const MUID& uidChar, const MUID& uidPeer)
{
	MMatchObject* pChar = GetObject(uidChar);
	if (pChar == NULL) return;

	MMatchStage* pStage = FindStage(pChar->GetStageUID());
	if (pStage == NULL) return;

	LocateAgentToClient(uidChar, pStage->GetAgentUID());

	MCommand* pCmd = CreateCommand(MC_MATCH_RESPONSE_PEER_RELAY, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamUID(uidPeer));
	RouteToListener(pChar, pCmd);

	static int nCount = 0;
	if (10 > nCount++)
	{
		mlog("MC_AGENT_PEER_READY OK!!\n");
	}
}

void MMatchServer::OnRequestRelayPeer(const MUID& uidChar, const MUID& uidPeer)
{
	MMatchObject* pChar = GetObject(uidChar);
	if (pChar == NULL) return;

	MMatchObject* pPeer = GetObject(uidPeer);
	if (pPeer == NULL) return;

	pChar->SetRelayPeer(true);
	// Custom: Logfix, added newline
	LOG(LOG_DEBUG, "%s Request relay peer on %s\n", pChar->GetName(), pPeer->GetName());

	MMatchStage* pStage = FindStage(pChar->GetStageUID());
	if (pStage == NULL) return;

	MAgentObject* pAgent = GetAgent(pStage->GetAgentUID());
	if (pAgent == NULL) {
		pAgent = FindFreeAgent();
		if (pAgent == NULL) {
			// Notify Agent not ready
			MCommand* pCmd = CreateCommand(MC_AGENT_ERROR, MUID(0, 0));
			pCmd->AddParameter(new MCmdParamInt(0));
			RouteToListener(pChar, pCmd);
			return;
		}
	}

	static int nCount = 0;
	if (10 > nCount++)
	{
		mlog("%s Request relay peer on %s\n", pChar->GetName(), pPeer->GetName());
	}

	pChar->SetAgentUID(pAgent->GetUID());
	pStage->SetAgentUID(pAgent->GetUID());
	pStage->SetAgentReady(true);

	// Send Relay order to Agent
	MCommand* pCmd = CreateCommand(MC_AGENT_RELAY_PEER, pAgent->GetCommListener());
	pCmd->AddParameter(new MCmdParamUID(uidChar));
	pCmd->AddParameter(new MCmdParamUID(uidPeer));
	pCmd->AddParameter(new MCmdParamUID(pStage->GetUID()));
	Post(pCmd);

	/*	MCommand* pCmd2 = CreateCommand(MC_AGENT_RELAY_PEER, pAgent->GetCommListener());
		pCmd2->AddParameter(new MCmdParamUID(uidPeer));
		pCmd2->AddParameter(new MCmdParamUID(uidChar));
		pCmd2->AddParameter(new MCmdParamUID(pStage->GetUID()));
		Post(pCmd2);*/

		//	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		//		MMatchObject* pObj = (MMatchObject*)(*i).second;
		//		LocateAgentToClient(pObj->GetUID(), pStage->GetAgentUID());
		//	}
}