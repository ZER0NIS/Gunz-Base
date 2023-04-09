#include "stdafx.h"
#include "MMatchActiveTrapMgr.h"
#include "MBlobArray.h"

MMatchActiveTrap::MMatchActiveTrap()
	: m_vPosActivated(0, 0, 0)
{
	m_uidOwner.SetZero();
	m_nTrapItemId = 0;

	m_nTimeThrowed = 0;
	m_nLifeTime = 0;
	m_nTimeActivated = 0;
}

void MMatchActiveTrap::AddForcedEnteredPlayer(const MUID& uid)
{
	int n = (int)m_vecUidForcedEntered.size();
	for (int i = 0; i < n; ++i)
		if (m_vecUidForcedEntered[i] == uid) return;

	m_vecUidForcedEntered.push_back(uid);
}

MMatchActiveTrapMgr::MMatchActiveTrapMgr()
{
	m_pStage = NULL;
}

MMatchActiveTrapMgr::~MMatchActiveTrapMgr()
{
	Destroy();
}

void MMatchActiveTrapMgr::Create(MMatchStage* pStage)
{
	m_pStage = pStage;
}

void MMatchActiveTrapMgr::Destroy()
{
	m_pStage = NULL;
	Clear();
}

void MMatchActiveTrapMgr::Clear()

{
	for (ItorTrap it = m_listTrap.begin(); it != m_listTrap.end(); ++it)
		delete* it;
	m_listTrap.clear();
}

void MMatchActiveTrapMgr::AddThrowedTrap(const MUID& uidOwner, int nItemId)
{
	if (!m_pStage) return;
	if (!m_pStage->GetObj(uidOwner)) return;

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemId);
	if (!pItemDesc) return;

	MMatchActiveTrap* pTrap = new MMatchActiveTrap;

	pTrap->m_nTimeThrowed = MGetMatchServer()->GetGlobalClockCount();
	pTrap->m_uidOwner = uidOwner;
	pTrap->m_nTrapItemId = nItemId;

	pTrap->m_nLifeTime = pItemDesc->m_nLifeTime.Ref();

	m_listTrap.push_back(pTrap);

	OutputDebugString("AddThrowedTrap\n");
}

void MMatchActiveTrapMgr::OnActivated(const MUID& uidOwner, int nItemId, const MVector3& vPos)
{
	if (!m_pStage) return;

	MMatchActiveTrap* pTrap;
	for (ItorTrap it = m_listTrap.begin(); it != m_listTrap.end(); ++it)
	{
		pTrap = *it;
		if (pTrap->m_uidOwner == uidOwner &&
			pTrap->m_nTrapItemId == nItemId &&
			!pTrap->IsActivated())
		{
			pTrap->m_nTimeActivated = MGetMatchServer()->GetGlobalClockCount();
			pTrap->m_vPosActivated = vPos;

			OutputDebugString("OnActivated trap\n");

			RouteTrapActivationForForcedEnterd(pTrap);
			return;
		}
	}
}

void MMatchActiveTrapMgr::Update(unsigned long nClock)
{
	MMatchActiveTrap* pTrap;
	for (ItorTrap it = m_listTrap.begin(); it != m_listTrap.end(); )
	{
		pTrap = *it;

		if (pTrap->IsActivated())
		{
			if (nClock - pTrap->m_nTimeActivated > pTrap->m_nLifeTime)
			{
				it = m_listTrap.erase(it);
				OutputDebugString("Trap deactivated\n");
				continue;
			}
		}
		else
		{
			if (nClock - pTrap->m_nTimeThrowed > MAX_TRAP_THROWING_LIFE * 1000)
			{
				it = m_listTrap.erase(it);
				OutputDebugString("Trap Removed without activation\n");
				continue;
			}
		}

		++it;
	}
}

void MMatchActiveTrapMgr::RouteAllTraps(MMatchObject* pObj)
{
	OutputDebugString("Trap RouteAllTrap to ForcedEntered\n");

	MMatchActiveTrap* pTrap;

	for (ItorTrap it = m_listTrap.begin(); it != m_listTrap.end(); ++it)
	{
		pTrap = *it;
		if (!pTrap->IsActivated())
		{
			pTrap->AddForcedEnteredPlayer(pObj->GetUID());

			OutputDebugString("Trap RESERVE To NOTIFY AddForcedEnteredPlayer\n");
		}
	}

	int num = 0;
	for (ItorTrap it = m_listTrap.begin(); it != m_listTrap.end(); ++it)
		if ((*it)->IsActivated())
			++num;

	if (num <= 0) return;

	void* pTrapArray = MMakeBlobArray(sizeof(MTD_ActivatedTrap), num);

	MTD_ActivatedTrap* pNode;
	int nIndex = 0;
	for (ItorTrap it = m_listTrap.begin(); it != m_listTrap.end(); ++it)
	{
		pTrap = *it;
		if (pTrap->IsActivated())
		{
			pNode = (MTD_ActivatedTrap*)MGetBlobArrayElement(pTrapArray, nIndex++);
			Make_MTDActivatedTrap(pNode, pTrap);
		}
		else
		{
			pTrap->AddForcedEnteredPlayer(pObj->GetUID());

			OutputDebugString("Trap RESERVE To NOTIFY AddForcedEnteredPlayer\n");
		}
	}

	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_NOTIFY_ACTIATED_TRAPITEM_LIST, MUID(0, 0));
	pCmd->AddParameter(new MCommandParameterBlob(pTrapArray, MGetBlobArraySize(pTrapArray)));
	MEraseBlobArray(pTrapArray);

	MMatchServer::GetInstance()->RouteToListener(pObj, pCmd);
}

void MMatchActiveTrapMgr::RouteTrapActivationForForcedEnterd(MMatchActiveTrap* pTrap)
{
	OutputDebugString("Notify Trap activation to ForcedEnteredPlayer\n");

	if (!pTrap || !pTrap->IsActivated()) { return; }
	if (!m_pStage) return;

	int numTarget = (int)pTrap->m_vecUidForcedEntered.size();
	if (numTarget <= 0) return;

	void* pTrapArray = MMakeBlobArray(sizeof(MTD_ActivatedTrap), 1);

	MTD_ActivatedTrap* pNode = (MTD_ActivatedTrap*)MGetBlobArrayElement(pTrapArray, 0);
	Make_MTDActivatedTrap(pNode, pTrap);

	MCommand* pCommand = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_NOTIFY_ACTIATED_TRAPITEM_LIST, MUID(0, 0));
	pCommand->AddParameter(new MCommandParameterBlob(pTrapArray, MGetBlobArraySize(pTrapArray)));

	MMatchObject* pObj;
	for (int i = 0; i < numTarget; ++i)
	{
		pObj = m_pStage->GetObj(pTrap->m_vecUidForcedEntered[i]);
		if (!pObj) continue;

		MCommand* pSendCmd = pCommand->Clone();
		MMatchServer::GetInstance()->RouteToListener(pObj, pSendCmd);
	}

	delete pCommand;
	MEraseBlobArray(pTrapArray);

	pTrap->m_vecUidForcedEntered.clear();
}