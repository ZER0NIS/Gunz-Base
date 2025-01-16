#include "stdafx.h"
#include "ZSurvival.h"
#include "ZGameClient.h"
#include "ZEnemy.h"
#include "ZActor.h"
#include "ZObjectManager.h"
#include "Physics.h"
#include "ZGlobal.h"
#include "ZMyCharacter.h"
#include "ZGame.h"
#include "ZScreenEffectManager.h"
#include "ZMapDesc.h"

#include "MMatchQuestMonsterGroup.h"
#include "MQuestConst.h"

#include "ZWorldItem.h"
#include "ZCharacter.h"
#include "ZCharacterManager.h"
#include "ZModule_QuestStatus.h"
#include "ZWorldManager.h"
#include "ZCommandTable.h"
#include "ZModule_Skills.h"
#include "ZLanguageConf.h"
#include "RNavigationMesh.h"
#include "RNavigationNode.h"

ZSurvival::ZSurvival() : m_bLoaded(false), m_bCreatedOnce(false)
{
	memset(&m_Cheet, 0, sizeof(m_Cheet));

	m_bIsQuestComplete = false;

	m_QuestCombatState = MQUEST_COMBAT_NONE;

	m_nRewardXP = 0;
	m_nRewardBP = 0;
	m_nReachedRound = 0;
	m_nPoint = 0;
}

ZSurvival::~ZSurvival()
{
	OnDestroyOnce();
}

bool ZSurvival::OnCreate()
{
	memset(&m_Cheet, 0, sizeof(m_Cheet));
	m_bIsRoundClear = false;
	ZGetQuest()->GetGameInfo()->ClearNPCKilled();
	m_fLastWeightTime = 0.0f;

	LoadNPCMeshes();
	LoadNPCSounds();

	return ZGetScreenEffectManager()->CreateQuestRes();
}

void ZSurvival::OnDestroy()
{
	ZGetNpcMeshMgr()->UnLoadAll();
	m_GameInfo.Final();

	ZGetScreenEffectManager()->DestroyQuestRes();
}

bool ZSurvival::OnCreateOnce()
{
	if (m_bCreatedOnce) return true;

	m_bCreatedOnce = true;
	return Load();
}

void ZSurvival::OnDestroyOnce()
{
	m_bCreatedOnce = false;
}

bool ZSurvival::Load()
{
	if (m_bLoaded) return true;

	string strFileDropTable(FILENAME_DROPTABLE);
	if (!m_DropTable.ReadXml(ZApplication::GetFileSystem(), strFileDropTable.c_str()))
	{
		mlog("Error while Read m_DropTable %s", strFileDropTable.c_str());
		return false;
	}

	string strFileNameZNPC(FILENAME_ZNPC_DESC);
	if (!m_NPCCatalogue.ReadXml(ZApplication::GetFileSystem(), strFileNameZNPC.c_str()))
	{
		mlog("Error while Read Item Descriptor %s", strFileNameZNPC.c_str());
		return false;
	}

	ProcessNPCDropTableMatching();

	string strFileNameSurvivalMap(FILENAME_SURVIVALMAP);
	if (!m_MapCatalogue.ReadXml(ZApplication::GetFileSystem(), strFileNameSurvivalMap.c_str()))
	{
		mlog("Error while Read Survivalmap Catalogue %s", strFileNameSurvivalMap.c_str());
		return false;
	}

	if (!ZGetAniEventMgr()->ReadXml(ZApplication::GetFileSystem(), "System/AnimationEvent.xml"))
	{
		mlog("Read Animation Event Failed");
		return false;
	}

	AniFrameInfo::m_pEventFunc = [](const RAniEventInfo& Info, rvector Pos) {
		ZGetSoundEngine()->PlaySound(Info.Filename, Pos, false);
		};

	m_bLoaded = true;
	return true;
}

void ZSurvival::Reload()
{
	m_bLoaded = false;

	m_NPCCatalogue.Clear();
	m_MapCatalogue.Clear();
	m_DropTable.Clear();

	Load();
}

void ZSurvival::OnGameCreate()
{
	if (!ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())) return;

	OnCreateOnce();
	Create();

	m_nRewardBP = 0;
	m_nRewardXP = 0;

	m_nReachedRound = 0;
	m_nPoint = 0;

	if (m_MapCatalogue.IsHacked())
	{
		ZGetGameInterface()->ShowDisconnectMsg(MERR_FIND_HACKER, 30000);
		mlog(ZMsg(MSG_HACKING_DETECTED));
		mlog("\nhacked XML.\n");
	}
}

void ZSurvival::OnGameDestroy()
{
	if (!ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())) return;

	Destroy();

	ZGetEffectManager()->EnableDraw(true);
	ZGetGame()->m_WeaponManager.EnableRender(true);
	ZGetWorldItemManager()->EnableDraw(true);
}

void ZSurvival::OnGameUpdate(float fElapsed)
{
	if (!ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())) return;

	UpdateNavMeshWeight(fElapsed);
}

void ZSurvival::UpdateNavMeshWeight(float fDelta)
{
	if ((ZGetGame()->GetTime() - m_fLastWeightTime) >= 1.0f)
	{
		RNavigationMesh* pNavMesh = ZGetGame()->GetWorld()->GetBsp()->GetNavigationMesh();
		if (pNavMesh != NULL)
		{
			pNavMesh->ClearAllNodeWeight();
			ZNPCObjectMap* pNPCObjectMap = ZGetObjectManager()->GetNPCObjectMap();
			for (ZNPCObjectMap::iterator i = pNPCObjectMap->begin(); i != pNPCObjectMap->end(); i++)
			{
				ZObject* pNPCObject = i->second;
				RNavigationNode* pNode = pNavMesh->FindClosestNode(pNPCObject->GetPosition());
				if (pNode)
				{
					float fWeight = pNode->GetWeight() + 1.0f;
					pNode->SetWeight(fWeight);
				}
			}
		}
		m_fLastWeightTime = ZGetGame()->GetTime();
	}
}

bool ZSurvival::OnCommand(MCommand* pCommand)
{
	switch (pCommand->GetID())
	{
		HANDLE_COMMAND(MC_QUEST_GAME_INFO, OnQuestGameInfo);
		HANDLE_COMMAND(MC_MATCH_RESPONSE_MONSTER_BIBLE_INFO, OnSetMonsterBibleInfo);
	}

	return false;
}

bool ZSurvival::OnGameCommand(MCommand* pCommand)
{
	switch (pCommand->GetID())
	{
		HANDLE_COMMAND(MC_QUEST_NPC_LOCAL_SPAWN, OnNPCSpawn);
		HANDLE_COMMAND(MC_QUEST_NPC_SPAWN, OnNPCSpawn);
		HANDLE_COMMAND(MC_QUEST_NPC_DEAD, OnNPCDead);
		HANDLE_COMMAND(MC_QUEST_PEER_NPC_DEAD, OnPeerNPCDead);
		HANDLE_COMMAND(MC_QUEST_ENTRUST_NPC_CONTROL, OnEntrustNPCControl);
		HANDLE_COMMAND(MC_QUEST_PEER_NPC_BASICINFO, OnPeerNPCBasicInfo);
		HANDLE_COMMAND(MC_QUEST_PEER_NPC_HPINFO, OnPeerNPCHPInfo);
		HANDLE_COMMAND(MC_QUEST_PEER_NPC_ATTACK_MELEE, OnPeerNPCAttackMelee);
		HANDLE_COMMAND(MC_QUEST_PEER_NPC_ATTACK_RANGE, OnPeerNPCAttackRange);
		HANDLE_COMMAND(MC_QUEST_PEER_NPC_SKILL_START, OnPeerNPCSkillStart);
		HANDLE_COMMAND(MC_QUEST_PEER_NPC_SKILL_EXECUTE, OnPeerNPCSkillExecute);
		HANDLE_COMMAND(MC_QUEST_PEER_NPC_BOSS_HPAP, OnPeerNPCBossHpAp);

		HANDLE_COMMAND(MC_QUEST_REFRESH_PLAYER_STATUS, OnRefreshPlayerStatus);
		HANDLE_COMMAND(MC_QUEST_NPC_ALL_CLEAR, OnClearAllNPC);
		HANDLE_COMMAND(MC_QUEST_ROUND_START, OnQuestRoundStart);
		HANDLE_COMMAND(MC_MATCH_QUEST_PLAYER_DEAD, OnQuestPlayerDead);
		HANDLE_COMMAND(MC_QUEST_COMBAT_STATE, OnQuestCombatState);
		HANDLE_COMMAND(MC_QUEST_MOVETO_PORTAL, OnMovetoPortal);
		HANDLE_COMMAND(MC_QUEST_READYTO_NEWSECTOR, OnReadyToNewSector);
		HANDLE_COMMAND(MC_QUEST_SECTOR_START, OnSectorStart);
		HANDLE_COMMAND(MC_QUEST_OBTAIN_QUESTITEM, OnObtainQuestItem);
		HANDLE_COMMAND(MC_QUEST_OBTAIN_ZITEM, OnObtainZItem);
		HANDLE_COMMAND(MC_QUEST_SECTOR_BONUS, OnSectorBonus);

		HANDLE_COMMAND(MC_QUEST_COMPLETED, OnQuestCompleted);
		HANDLE_COMMAND(MC_QUEST_FAILED, OnQuestFailed);

		HANDLE_COMMAND(MC_QUEST_PING, OnQuestPing);

		HANDLE_COMMAND(MC_QUEST_SURVIVAL_RESULT, OnSurvivalResult);
		HANDLE_COMMAND(MC_SURVIVAL_RANKINGLIST, OnSurvivalRankingList);
		HANDLE_COMMAND(MC_SURVIVAL_PRIVATERANKING, OnSurvivalPrivateRanking);

#ifdef _QUEST_ITEM
		HANDLE_COMMAND(MC_MATCH_NEW_MONSTER_INFO, OnNewMonsterInfo);
#endif
		///Custom: latejoin
	case MC_MATCH_LATEJOIN_QUEST:
	{
		MUID targetPlayer;
		int currSector;
		pCommand->GetParameter(&targetPlayer, 0, MPT_UID);
		pCommand->GetParameter(&currSector, 1, MPT_INT);

		if (targetPlayer == ZGetGame()->m_pMyCharacter->GetUID())
		{
			MoveToRealSector(currSector);
		}
	}
	break;
	}

	return false;
}

bool ZSurvival::OnNPCSpawn(MCommand* pCommand)
{
	if (ZGetGame() == NULL) return false;

	MUID uidChar, uidNPC;
	unsigned char nNPCType, nPositionIndex;

	pCommand->GetParameter(&uidChar, 0, MPT_UID);
	pCommand->GetParameter(&uidNPC, 1, MPT_UID);
	pCommand->GetParameter(&nNPCType, 2, MPT_UCHAR);
	pCommand->GetParameter(&nPositionIndex, 3, MPT_UCHAR);

	MQUEST_NPC NPCType = MQUEST_NPC(nNPCType);

	ZMapSpawnType nSpawnType = ZMST_NPC_MELEE;

	ZMapSpawnManager* pMSM = ZGetGame()->GetMapDesc()->GetSpawnManager();
	MQuestNPCInfo* pNPCInfo = GetNPCInfo(NPCType);
	if (pNPCInfo == NULL) return false;

	switch (pNPCInfo->GetSpawnType())
	{
	case MNST_MELEE: nSpawnType = ZMST_NPC_MELEE; break;
	case MNST_RANGE: nSpawnType = ZMST_NPC_RANGE; break;
	case MNST_BOSS: nSpawnType = ZMST_NPC_BOSS; break;
	default: break;
	};

	ZMapSpawnData* pSpawnData = pMSM->GetSpawnData(nSpawnType, nPositionIndex);

	if (pSpawnData == NULL)
	{
		if (nSpawnType == ZMST_NPC_BOSS)
			pSpawnData = pMSM->GetSpawnData(ZMST_NPC_MELEE, nPositionIndex);
	}

	rvector NPCPos = rvector(0, 0, 0);
	rvector NPCDir = rvector(1, 0, 0);

	if (pSpawnData)
	{
		NPCPos = pSpawnData->m_Pos;
		NPCDir = pSpawnData->m_Dir;
	}

	{
		RMesh* pNPCMesh = ZGetNpcMeshMgr()->Get(pNPCInfo->szMeshName);
		if (pNPCMesh)
		{
			if (!pNPCMesh->m_isMeshLoaded)
			{
				ZGetNpcMeshMgr()->Load(pNPCInfo->szMeshName);
				ZGetNpcMeshMgr()->ReloadAllAnimation();
			}
		}
	}

	float fTC = m_GameInfo.GetNPC_TC();

	bool bForceCollRadius35 = false;
	if (m_GameInfo.GetMapSectorID(m_GameInfo.GetCurrSectorIndex()) == 207)
		bForceCollRadius35 = true;
	ZActor* pNewActor = ZActor::CreateActor(NPCType, fTC, m_GameInfo.GetQuestLevel(), bForceCollRadius35);
	if (pNewActor)
	{
		bool bMyControl = (uidChar == ZGetGameClient()->GetPlayerUID());
		pNewActor->SetMyControl(bMyControl);
		pNewActor->SetUID(uidNPC);
		pNewActor->SetPosition(NPCPos);
		pNewActor->SetDirection(NPCDir);

		ZCharacter* pOwner = ZGetCharacterManager()->Find(uidChar);
		if (pOwner)
			pNewActor->SetOwner(pOwner->GetUserName());

		if (pNewActor->m_pVMesh) {
			D3DCOLORVALUE color;

			color.r = pNPCInfo->vColor.x;
			color.g = pNPCInfo->vColor.y;
			color.b = pNPCInfo->vColor.z;
			color.a = 1.f;

			pNewActor->m_pVMesh->SetNPCBlendColor(color);
		}

		ZGetObjectManager()->Add(pNewActor);
		ZGetEffectManager()->AddReBirthEffect(NPCPos);

		if ((pNPCInfo->nGrade == NPC_GRADE_BOSS) || (pNPCInfo->nGrade == NPC_GRADE_LEGENDARY))
		{
			m_GameInfo.GetBosses().push_back(uidNPC);

			if (pNewActor->IsMyControl()) {
				float radius = bForceCollRadius35 ? 35.f : pNewActor->GetCollRadius();
				if (true == ZGetGame()->GetWorld()->GetBsp()->CheckSolid(NPCPos, radius)) {
					OutputDebugString("보스몹 스폰지점 충돌검사 실패...\n");

					RNavigationMesh* pNavMesh = ZGetGame()->GetWorld()->GetBsp()->GetNavigationMesh();
					if (pNavMesh) {
						RNavigationNode* pNavNode = pNavMesh->FindClosestNode(NPCPos);
						if (pNavNode) {
							pNewActor->SetPosition(pNavNode->CenterVertex());
							OutputDebugString("스폰위치조정됨!\n");
						}
					}
				}
			}
		}
	}

	return true;
}

bool ZSurvival::OnNPCDead(MCommand* pCommand)
{
	MUID uidPlayer, uidNPC;

	pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
	pCommand->GetParameter(&uidNPC, 1, MPT_UID);

	ZActor* pActor = ZGetObjectManager()->GetNPCObject(uidNPC);
	if (pActor)
	{
		pActor->OnDie();
		ZGetObjectManager()->Delete(pActor);

		m_GameInfo.IncreaseNPCKilled();

		ZCharacter* pCharacter = ZGetCharacterManager()->Find(uidPlayer);
		if (pCharacter)
		{
			ZModule_QuestStatus* pMod = (ZModule_QuestStatus*)pCharacter->GetModule(ZMID_QUESTSTATUS);
			if (pMod)
			{
				pMod->AddKills();
			}
		}
	}

	return true;
}

bool ZSurvival::OnEntrustNPCControl(MCommand* pCommand)
{
	MUID uidChar, uidNPC;
	pCommand->GetParameter(&uidChar, 0, MPT_UID);
	pCommand->GetParameter(&uidNPC, 1, MPT_UID);

	ZActor* pNPC = ZGetObjectManager()->GetNPCObject(uidNPC);
	if (pNPC)
	{
		bool bMyControl = (uidChar == ZGetGameClient()->GetPlayerUID());
		pNPC->SetMyControl(bMyControl);

		ZCharacter* pOwner = ZGetCharacterManager()->Find(uidChar);
		if (pOwner)
			pNPC->SetOwner(pOwner->GetUserName());
	}

	return true;
}

bool ZSurvival::OnPeerNPCBasicInfo(MCommand* pCommand)
{
	MCommandParameter* pParam = pCommand->GetParameter(0);
	if (pParam->GetType() != MPT_BLOB) return false;

	ZACTOR_BASICINFO* ppbi = (ZACTOR_BASICINFO*)pParam->GetPointer();

	ZBasicInfo bi;
	bi.position = rvector(ppbi->posx, ppbi->posy, ppbi->posz);
	bi.velocity = rvector(ppbi->velx, ppbi->vely, ppbi->velz);
	bi.direction = 1.f / 32000.f * rvector(ppbi->dirx, ppbi->diry, ppbi->dirz);

	ZActor* pActor = ZGetObjectManager()->GetNPCObject(ppbi->uidNPC);
	if (pActor)
	{
		pActor->InputBasicInfo(&bi, ppbi->anistate);
	}

	return true;
}

bool ZSurvival::OnPeerNPCHPInfo(MCommand* pCommand)
{
	return true;
}

bool ZSurvival::OnPeerNPCBossHpAp(MCommand* pCommand)
{
	MUID uidBoss;
	float fHp, fAp;
	pCommand->GetParameter(&uidBoss, 0, MPT_UID);
	pCommand->GetParameter(&fHp, 1, MPT_FLOAT);
	pCommand->GetParameter(&fAp, 2, MPT_FLOAT);

	ZActor* pActor = ZGetObjectManager()->GetNPCObject(uidBoss);
	if (pActor)
	{
		pActor->InputBossHpAp(fHp, fAp);
	}

	return true;
}

bool ZSurvival::OnPrePeerNPCAttackMelee(MCommand* pCommand)
{
	return true;
}

bool ZSurvival::OnPeerNPCAttackMelee(MCommand* pCommand)
{
	MUID uidOwner;
	pCommand->GetParameter(&uidOwner, 0, MPT_UID);

	ZGetGame()->OnPeerShot_Melee(uidOwner, ZGetGame()->GetTime());

	return true;
}

bool ZSurvival::OnPeerNPCAttackRange(MCommand* pCommand)
{
	MUID uidOwner;
	pCommand->GetParameter(&uidOwner, 0, MPT_UID);

	MCommandParameter* pParam = pCommand->GetParameter(1);
	if (pParam->GetType() != MPT_BLOB) return false;

	ZPACKEDSHOTINFO* pinfo = (ZPACKEDSHOTINFO*)pParam->GetPointer();
	rvector pos = rvector(pinfo->posx, pinfo->posy, pinfo->posz);
	rvector to = rvector(pinfo->tox, pinfo->toy, pinfo->toz);

	ZObject* pOwner = ZGetGame()->m_ObjectManager.GetObject(uidOwner);
	MMatchItemDesc* pDesc = NULL;

	if (pOwner == NULL) return false;

	if (pOwner->GetItems())
		if (pOwner->GetItems()->GetSelectedWeapon())
			pDesc = pOwner->GetItems()->GetSelectedWeapon()->GetDesc();
	if (pDesc)
	{
		if (pDesc->m_nWeaponType.Ref() == MWT_ROCKET)
		{
			rvector dir = to - pos;
			Normalize(dir);
			ZGetGame()->m_WeaponManager.AddRocket(pos, dir, pOwner);
			ZApplication::GetSoundEngine()->PlaySEFire(pDesc, pos.x, pos.y, pos.z, false);

			return true;
		}
		else if (pDesc->m_nWeaponType.Ref() == MWT_SKILL)
		{
			rvector dir = to - pos;
			Normalize(dir);

			ZSkill skill;
			skill.Init(pDesc->m_nGadgetID.Ref(), pOwner);

			ZApplication::GetSoundEngine()->PlaySEFire(pDesc, pos.x, pos.y, pos.z, false);

			ZGetGame()->m_WeaponManager.AddMagic(&skill, pos, dir, pOwner);
			return true;
		}
	}
	else
		return false;

	ZGetGame()->OnPeerShot_Range((MMatchCharItemParts)pinfo->sel_type, uidOwner, ZGetGame()->GetTime(), pos, to);

	return true;
}

bool ZSurvival::OnRefreshPlayerStatus(MCommand* pCommand)
{
	bool bAdminHide = false;
	if (ZGetMyInfo()->IsAdminGrade())
	{
		MMatchObjCache* pCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
		if (pCache && pCache->CheckFlag(MTD_PlayerFlags_AdminHide))
			bAdminHide = true;
	}

	if (!bAdminHide)
	{
		ZGetGame()->ReleaseObserver();

		if (ZGetGame()->m_pMyCharacter->IsDie())
		{
			ZGetGame()->GetMatch()->RespawnSolo();
		}
	}

	for (ZCharacterManager::iterator i = ZGetCharacterManager()->begin(); i != ZGetCharacterManager()->end(); i++)
	{
		ZCharacter* pCharacter = i->second;
		if (!pCharacter->IsAdminHide())	pCharacter->InitStatus();
	}

	ZGetGame()->CancelSuicide();

	return true;
}

bool ZSurvival::OnClearAllNPC(MCommand* pCommand)
{
	ZGetObjectManager()->ClearNPC();

	return true;
}

bool ZSurvival::OnQuestRoundStart(MCommand* pCommand)
{
	unsigned char nRound;
	pCommand->GetParameter(&nRound, 0, MPT_UCHAR);

	ZGetScreenEffectManager()->AddRoundStart(int(nRound));

	ZGetWorldItemManager()->Reset();

	return true;
}

bool ZSurvival::OnQuestPlayerDead(MCommand* pCommand)
{
	MUID uidVictim;
	pCommand->GetParameter(&uidVictim, 0, MPT_UID);

	ZCharacter* pVictim = ZGetCharacterManager()->Find(uidVictim);

	if (pVictim)
	{
		if (pVictim != ZGetGame()->m_pMyCharacter)
		{
			pVictim->Die();
		}

		pVictim->GetStatus().CheckCrc();
		pVictim->GetStatus().Ref().AddDeaths();
		if (pVictim->GetStatus().Ref().nLife > 0)
			pVictim->GetStatus().Ref().nLife--;
		pVictim->GetStatus().MakeCrc();
	}

	return true;
}

bool ZSurvival::OnQuestGameInfo(MCommand* pCommand)
{
	MCommandParameter* pParam = pCommand->GetParameter(0);
	if (pParam->GetType() != MPT_BLOB) return false;
	void* pBlob = pParam->GetPointer();

	MTD_QuestGameInfo* pQuestGameInfo = (MTD_QuestGameInfo*)MGetBlobArrayElement(pBlob, 0);

	m_GameInfo.Init(pQuestGameInfo);

	return true;
}

bool ZSurvival::OnQuestCombatState(MCommand* pCommand)
{
	char nState;
	pCommand->GetParameter(&nState, 0, MPT_CHAR);

	MQuestCombatState nCombatState = MQuestCombatState(nState);

	m_QuestCombatState = nCombatState;

	switch (nCombatState)
	{
	case MQUEST_COMBAT_PREPARE:
	{
	}
	break;
	case MQUEST_COMBAT_PLAY:
	{
		ZGetObjectManager()->GetNPCObjectMap()->ForceInvisibleNewNpc(false);
		ZGetEffectManager()->Clear();
		ZGetEffectManager()->EnableDraw(true);
		ZGetGame()->m_WeaponManager.EnableRender(true);
		ZGetWorldItemManager()->EnableDraw(true);
	}
	break;
	case MQUEST_COMBAT_COMPLETED:
	{
		if (!m_GameInfo.IsLastSectorInSurvival())
		{
			if (ZGetGame())
			{
				rvector portal_pos;
				int nCurrSectorIndex = m_GameInfo.GetCurrSectorIndex();
				int nLinkIndex = m_GameInfo.GetMapSectorLink(nCurrSectorIndex);
				portal_pos = ZGetGame()->GetMapDesc()->GetQuestSectorLink(nLinkIndex);
				ZGetWorldItemManager()->AddQuestPortal(portal_pos);
				m_GameInfo.SetPortalPos(portal_pos);

				ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM_GAME), ZMsg(MSG_GAME_OPEN_PORTAL));

				m_tRemainedTime = timeGetTime() + PORTAL_MOVING_TIME;
				m_bIsRoundClear = true;
			}
		}
	}
	break;
	};

	return true;
}

bool ZSurvival::OnMovetoPortal(MCommand* pCommand)
{
	char nCurrSectorIndex;
	unsigned char nCurrRepeatIndex;
	MUID uidPlayer;

	pCommand->GetParameter(&nCurrSectorIndex, 0, MPT_CHAR);
	pCommand->GetParameter(&nCurrRepeatIndex, 1, MPT_UCHAR);
	pCommand->GetParameter(&uidPlayer, 2, MPT_UID);

	if (uidPlayer == ZGetGameClient()->GetPlayerUID())
	{
		m_bIsRoundClear = false;
		ZGetQuest()->GetGameInfo()->ClearNPCKilled();

		m_GameInfo.OnMovetoNewSector((int)(nCurrSectorIndex), nCurrRepeatIndex);

		ZPostQuestReadyToNewSector(ZGetGameClient()->GetPlayerUID());

		ZGetGame()->ExceptCharacterFromNpcTargetting(ZGetGameInterface()->GetMyCharacter());
	}
	else
	{
		ZCharacter* pChar = ZGetCharacterManager()->Find(uidPlayer);
		if (pChar && m_CharactersGone.find(ZGetGameClient()->GetPlayerUID()) == m_CharactersGone.end()) {
			pChar->SetVisible(false);
			ZGetEffectManager()->AddReBirthEffect(pChar->GetPosition());

			ZGetGame()->ExceptCharacterFromNpcTargetting(pChar);
		}
	}

	ZGetGame()->m_WeaponManager.DeleteWeaponHasTarget(uidPlayer);

	return true;
}

bool ZSurvival::OnReadyToNewSector(MCommand* pCommand)
{
	MUID uidPlayer;
	pCommand->GetParameter(&uidPlayer, 0, MPT_UID);

	m_CharactersGone.insert(uidPlayer);

	ZCharacter* pChar = ZGetCharacterManager()->Find(uidPlayer);

	if (ZGetCombatInterface()->GetObserver()->GetTargetCharacter() == pChar) {
		ZGetCombatInterface()->GetObserver()->ChangeToNextTarget();
	}

	if (uidPlayer == ZGetGameClient()->GetPlayerUID()) {
		MoveToNextSector();
	}
	else
	{
		ZCharacter* pChar = ZGetCharacterManager()->Find(uidPlayer);
		if (pChar && m_CharactersGone.find(ZGetGameClient()->GetPlayerUID()) != m_CharactersGone.end()) {
			int nPosIndex = ZGetCharacterManager()->GetCharacterIndex(pChar->GetUID(), false);
			if (nPosIndex < 0) nPosIndex = 0;
			else if (nPosIndex >= MAX_QUSET_PLAYER_COUNT) nPosIndex = MAX_QUSET_PLAYER_COUNT - 1;
			ZMapSpawnData* pSpawnData = ZGetWorld()->GetDesc()->GetSpawnManager()->GetSoloData(nPosIndex);
			if (pSpawnData) {
				pChar->SetPosition(pSpawnData->m_Pos);
				pChar->SetDirection(pSpawnData->m_Dir);
				ZGetEffectManager()->AddReBirthEffect(pSpawnData->m_Pos);
			}
			else
			{
			}
		}
	}

	return true;
}

bool ZSurvival::OnSectorStart(MCommand* pCommand)
{
	char nSectorIndex;
	unsigned char nRepeatIndex;
	pCommand->GetParameter(&nSectorIndex, 0, MPT_CHAR);
	pCommand->GetParameter(&nRepeatIndex, 1, MPT_UCHAR);

	m_bIsRoundClear = false;
	ZGetQuest()->GetGameInfo()->ClearNPCKilled();

	if (m_GameInfo.GetCurrSectorIndex() != nSectorIndex ||
		m_GameInfo.GetCurrRepeatIndex() != nRepeatIndex)
	{
		m_GameInfo.OnMovetoNewSector((int)nSectorIndex, nRepeatIndex);

		MoveToNextSector();
	}

	for (ZCharacterManager::iterator i = ZGetCharacterManager()->begin(); i != ZGetCharacterManager()->end(); i++)
	{
		i->second->SetVisible(true);
	}

	ZGetGame()->ClearListExceptionFromNpcTargetting();

	ZGetGame()->m_WeaponManager.Clear();

	ZGetWorldItemManager()->Reset();
	m_CharactersGone.clear();

	MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
	if (pObjCache && pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide)) {
		ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);
	}

	return true;
}

bool ZSurvival::OnQuestCompleted(MCommand* pCommand)
{
	m_bIsQuestComplete = true;

	MCommandParameter* pParam = pCommand->GetParameter(0);
	if (pParam->GetType() != MPT_BLOB) return false;

	void* pBlob = pParam->GetPointer();
	int nBlobSize = MGetBlobArrayCount(pBlob);

	for (int i = 0; i < nBlobSize; i++)
	{
		MTD_QuestReward* pQuestRewardNode = (MTD_QuestReward*)MGetBlobArrayElement(pBlob, i);
	}

	mlog("Quest Completed\n");
	return true;
}

bool ZSurvival::OnQuestFailed(MCommand* pCommand)
{
	mlog("Quest Failed\n");

#ifdef _QUEST_ITEM
	m_bIsQuestComplete = false;
#endif

	return true;
}

bool ZSurvival::OnObtainQuestItem(MCommand* pCommand)
{
	unsigned long int nQuestItemID;
	pCommand->GetParameter(&nQuestItemID, 0, MPT_UINT);

	m_GameInfo.IncreaseObtainQuestItem();

#ifdef _QUEST_ITEM
	MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc((int)nQuestItemID);
	if (pQuestItemDesc)
	{
		char szMsg[128];
		ZTransMsg(szMsg, MSG_GAME_GET_QUEST_ITEM, 1, pQuestItemDesc->m_szQuestItemName);
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM_GAME), szMsg);
	}
#endif

	return true;
}

bool ZSurvival::OnObtainZItem(MCommand* pCommand)
{
	unsigned long int nItemID;
	pCommand->GetParameter(&nItemID, 0, MPT_UINT);

	m_GameInfo.IncreaseObtainQuestItem();

#ifdef _QUEST_ITEM
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc)
	{
		char szMsg[128];
		ZTransMsg(szMsg, MSG_GAME_GET_QUEST_ITEM, 1, pItemDesc->m_pMItemName->Ref().m_szItemName);
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM_GAME), szMsg);
	}
#endif

	return true;
}

void ZSurvival::LoadNPCMeshes()
{
	if (!m_GameInfo.IsInited())
	{
		mlog("not inialized Quest Game Info\n");
		return;
	}

	for (int i = 0; i < m_GameInfo.GetNPCInfoCount(); i++)
	{
		MQUEST_NPC npc = m_GameInfo.GetNPCInfo(i);

		ZGetNpcMeshMgr()->Load(GetNPCInfo(npc)->szMeshName);
	}

	ZGetNpcMeshMgr()->ReloadAllAnimation();
}

void ZSurvival::LoadNPCSounds()
{
	if (!m_GameInfo.IsInited())
	{
		mlog("ZSurvival::LoadNPCSounds - not inialized Quest Game Info\n");
		return;
	}

	ZSoundEngine* pSE = ZApplication::GetSoundEngine();
	if (pSE == NULL) return;

	for (int i = 0; i < m_GameInfo.GetNPCInfoCount(); i++)
	{
		MQUEST_NPC npc = m_GameInfo.GetNPCInfo(i);
		if (!pSE->LoadNPCResource(npc))
		{
			mlog("failed npc sound\n");
		}
	}
}

void ZSurvival::MoveToNextSector()
{
	ZCharacter* pMyChar = ZGetGame()->m_pMyCharacter;
	pMyChar->InitStatus();

	ZGetWorldManager()->SetCurrent(m_GameInfo.GetCurrSectorIndex());
	int nPosIndex = ZGetCharacterManager()->GetCharacterIndex(pMyChar->GetUID(), false);
	if (nPosIndex < 0) nPosIndex = 0;
	ZMapSpawnData* pSpawnData = ZGetWorld()->GetDesc()->GetSpawnManager()->GetSoloData(nPosIndex);
	if (pSpawnData != NULL && pMyChar != NULL)
	{
		pMyChar->SetPosition(pSpawnData->m_Pos);
		pMyChar->SetDirection(pSpawnData->m_Dir);
		ZGetEffectManager()->AddReBirthEffect(pSpawnData->m_Pos);
	}

	for (ZCharacterManager::iterator i = ZGetCharacterManager()->begin(); i != ZGetCharacterManager()->end(); i++)
	{
		i->second->SetVisible(false);
	}
	ZGetObjectManager()->GetNPCObjectMap()->SetVisibleAll(false);
	ZGetObjectManager()->GetNPCObjectMap()->ForceInvisibleNewNpc(true);
	ZGetEffectManager()->EnableDraw(false);
	ZGetWorldItemManager()->EnableDraw(false);
	ZGetGame()->m_WeaponManager.EnableRender(false);

	ZModule_QuestStatus* pMod = (ZModule_QuestStatus*)pMyChar->GetModule(ZMID_QUESTSTATUS);
	if (pMod)
	{
		int nKills = pMod->GetKills();
		ZGetScreenEffectManager()->SetKO(nKills);
	}
}

bool ZSurvival::OnSurvivalResult(MCommand* pCmd)
{
	if (0 == pCmd)
		return false;

	int nReachedRound;
	int nPoint;

	pCmd->GetParameter(&nReachedRound, 0, MPT_INT);
	pCmd->GetParameter(&nPoint, 1, MPT_INT);

	m_nReachedRound = nReachedRound;
	m_nPoint = nPoint;

	return true;
}

bool ZSurvival::OnSurvivalRankingList(MCommand* pCmd)
{
	if (0 == pCmd) return false;

	MTextArea* pWidgetRank = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SurvivalResult_PlayerRankList");
	MTextArea* pWidgetName = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SurvivalResult_PlayerNameList");
	MTextArea* pWidgetPoint = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SurvivalResult_PlayerRankPointList");
	if (!pWidgetRank || !pWidgetName || !pWidgetPoint) return false;

	pWidgetRank->Clear();
	pWidgetName->Clear();
	pWidgetPoint->Clear();

	MCommandParameter* pParam = pCmd->GetParameter(0);
	void* pRankBlob = (MTD_SurvivalRanking*)pParam->GetPointer();
	MTD_SurvivalRanking* pRank;
	char szText[128];

	int sizeBlob = MGetBlobArrayCount(pRankBlob);
	int size = min(sizeBlob, MAX_SURVIVAL_RANKING_LIST);

	for (int i = 0; i < size; ++i)
	{
		pRank = reinterpret_cast<MTD_SurvivalRanking*>(MGetBlobArrayElement(pRankBlob, i));

		if (pRank->m_dwRank == 0) break;

		sprintf(szText, "%d", pRank->m_dwRank);
		pWidgetRank->AddText(szText, MCOLOR(0xFFFFF794));

		if (strlen(pRank->m_szCharName) != 0) pWidgetName->AddText(pRank->m_szCharName, MCOLOR(0xFFFFF794));
		else								  pWidgetName->AddText(" ", MCOLOR(0xFFFFF794));

		sprintf(szText, "%d", pRank->m_dwPoint);
		pWidgetPoint->AddText(szText, MCOLOR(0xFFFFF794));
	}
	return true;
}

bool ZSurvival::OnSurvivalPrivateRanking(MCommand* pCmd)
{
	if (0 == pCmd) return false;

	MTextArea* pWidgetRank = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SurvivalResult_MyRank");
	MTextArea* pWidgetName = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SurvivalResult_MyName");
	MTextArea* pWidgetPoint = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SurvivalResult_MyRankPoint");
	if (!pWidgetRank || !pWidgetName || !pWidgetPoint) return false;

	pWidgetRank->Clear();
	pWidgetName->Clear();
	pWidgetPoint->Clear();

	unsigned int nRank, nPoint;
	pCmd->GetParameter(&nRank, 0, MPT_UINT);
	pCmd->GetParameter(&nPoint, 1, MPT_UINT);

	char szText[128];

	if (nRank != 0)
		sprintf(szText, "%d", nRank);
	else
		sprintf(szText, "--");
	pWidgetRank->AddText(szText, MCOLOR(0xFFFFFFCC));

	sprintf(szText, "%d", nPoint);
	pWidgetPoint->AddText(szText, MCOLOR(0xFFFFFFCC));

	pWidgetName->AddText(ZGetGame()->m_pMyCharacter->GetUserName(), MCOLOR(0xFFFFFFCC));

	return true;
}

#ifdef _QUEST_ITEM
class ObtainItemListBoxItem : public MListItem
{
protected:
	MBitmap* m_pBitmap;
	char				m_szName[128];

public:
	ObtainItemListBoxItem(MBitmap* pBitmap, const char* szName)
	{
		m_pBitmap = pBitmap;
		strcpy(m_szName, szName);
	}
	virtual const char* GetString(void)
	{
		return m_szName;
	}
	virtual const char* GetString(int i)
	{
		if (i == 1)
			return m_szName;

		return NULL;
	}
	virtual MBitmap* GetBitmap(int i)
	{
		if (i == 0)
			return m_pBitmap;

		return NULL;
	}
};
void ZSurvival::GetMyObtainQuestItemList(int nRewardXP, int nRewardBP, void* pMyObtainQuestItemListBlob, void* pMyObtainZItemListBlob)
{
}

bool ZSurvival::OnNewMonsterInfo(MCommand* pCmd)
{
	char nMonIndex;

	if (0 == pCmd)
		return false;

	pCmd->GetParameter(&nMonIndex, 0, MPT_CHAR);
	return true;
}

#endif

bool ZSurvival::OnPeerNPCSkillStart(MCommand* pCommand)
{
	MUID uidOwner, uidTarget;
	int nSkill;
	rvector targetPos;
	pCommand->GetParameter(&uidOwner, 0, MPT_UID);
	pCommand->GetParameter(&nSkill, 1, MPT_INT);
	pCommand->GetParameter(&uidTarget, 2, MPT_UID);
	pCommand->GetParameter(&targetPos, 3, MPT_POS);

	ZActor* pActor = ZGetObjectManager()->GetNPCObject(uidOwner);
	if (pActor)
	{
		ZModule_Skills* pmod = (ZModule_Skills*)pActor->GetModule(ZMID_SKILLS);
		pmod->PreExcute(nSkill, uidTarget, targetPos);
	}

	return true;
}

bool ZSurvival::OnPeerNPCSkillExecute(MCommand* pCommand)
{
	MUID uidOwner, uidTarget;
	int nSkill;
	rvector targetPos;
	pCommand->GetParameter(&uidOwner, 0, MPT_UID);
	pCommand->GetParameter(&nSkill, 1, MPT_INT);
	pCommand->GetParameter(&uidTarget, 2, MPT_UID);
	pCommand->GetParameter(&targetPos, 3, MPT_POS);

	ZActor* pActor = ZGetObjectManager()->GetNPCObject(uidOwner);
	if (pActor)
	{
		ZModule_Skills* pmod = (ZModule_Skills*)pActor->GetModule(ZMID_SKILLS);
		pmod->Excute(nSkill, uidTarget, targetPos);
	}

	return true;
}

bool ZSurvival::OnSetMonsterBibleInfo(MCommand* pCmd)
{
	if (0 == pCmd)
		return false;

	MUID				uid;
	MCommandParameter* pParam;
	void* pMonBibleInfoBlob;
	MQuestMonsterBible* pMonBible;

	pCmd->GetParameter(&uid, 0, MPT_UID);

	if (uid != ZGetGameClient()->GetPlayerUID())
	{
		return false;
	}

	pParam = pCmd->GetParameter(1);
	pMonBibleInfoBlob = pParam->GetPointer();
	pMonBible = reinterpret_cast<MQuestMonsterBible*>(MGetBlobArrayElement(pMonBibleInfoBlob, 0));

	return true;
}

bool ZSurvival::OnPeerNPCDead(MCommand* pCommand)
{
	MUID uidKiller, uidNPC;

	pCommand->GetParameter(&uidKiller, 0, MPT_UID);
	pCommand->GetParameter(&uidNPC, 1, MPT_UID);

	ZActor* pActor = ZGetObjectManager()->GetNPCObject(uidNPC);
	if (pActor)
	{
		pActor->OnPeerDie(uidKiller);

		if (uidKiller == ZGetMyUID())
		{
			ZModule_QuestStatus* pMod = (ZModule_QuestStatus*)ZGetGame()->m_pMyCharacter->GetModule(ZMID_QUESTSTATUS);
			if (pMod)
			{
				ZGetScreenEffectManager()->AddKO();
			}
		}
	}

	return true;
}

bool ZSurvival::OnSectorBonus(MCommand* pCommand)
{
	MUID uidPlayer;
	unsigned long int nExpValue = 0;
	unsigned long int nAddedBP = 0;

	pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
	pCommand->GetParameter(&nExpValue, 1, MPT_UINT);
	pCommand->GetParameter(&nAddedBP, 2, MPT_UINT);

	int nAddedXP = GetExpFromTransData(nExpValue);
	int nExpPercent = GetExpPercentFromTransData(nExpValue);

	if (ZGetCharacterManager()->Find(uidPlayer) == ZGetGame()->m_pMyCharacter)
	{
		m_nRewardXP += nAddedXP;
		m_nRewardBP += nAddedBP;

		ZGetMyInfo()->SetLevelPercent(nExpPercent);
		ZGetScreenEffectManager()->SetGaugeExpFromMyInfo();
	}

	return true;
}

bool ZSurvival::OnQuestPing(MCommand* pCommand)
{
	unsigned long int TimeStamp;
	pCommand->GetParameter(&TimeStamp, 0, MPT_UINT);
	ZPostQuestPong(TimeStamp);

	return true;
}

void ZSurvival::MoveToRealSector(int realSector)
{
	ZCharacter* pMyChar = ZGetGame()->m_pMyCharacter;
	ZGetCombatInterface()->SetObserverMode(true);
	pMyChar->InitStatus();
	pMyChar->SetVisible(false);
	pMyChar->ForceDie();
	ZGetWorldManager()->SetCurrent(realSector);
}