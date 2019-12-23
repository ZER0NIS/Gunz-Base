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

#ifdef _QUEST

bool ZSurvival::OnCreate()
{
	memset(&m_Cheet, 0, sizeof(m_Cheet));
	m_bIsRoundClear = false;
	ZGetQuest()->GetGameInfo()->ClearNPCKilled();
	m_fLastWeightTime = 0.0f;

	//m_Map.Init();
	LoadNPCMeshes();
	LoadNPCSounds();

	return ZGetScreenEffectManager()->CreateQuestRes();
}

void ZSurvival::OnDestroy()
{
	//m_Map.Final();

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
#ifndef _DEBUG
	strFileDropTable += "";
#endif
	if (!m_DropTable.ReadXml(ZApplication::GetFileSystem(), strFileDropTable.c_str()))
	{
		mlog("Error while Read m_DropTable %s", strFileDropTable.c_str());
		return false;
	}

	string strFileNameZNPC(FILENAME_ZNPC_DESC);
#ifndef _DEBUG
	strFileNameZNPC += "";	
#endif	
	if (!m_NPCCatalogue.ReadXml(ZApplication::GetFileSystem(), strFileNameZNPC.c_str()))
	{
		mlog("Error while Read Item Descriptor %s", strFileNameZNPC.c_str());
		return false;
	}

	ProcessNPCDropTableMatching();

	string strFileNameSurvivalMap(FILENAME_SURVIVALMAP);
#ifndef _DEBUG
	strFileNameSurvivalMap += "";
#endif
	if (!m_MapCatalogue.ReadXml(ZApplication::GetFileSystem(), strFileNameSurvivalMap.c_str()))
	{
		mlog("Read Survivalmap Catalogue Failed");
		return false;
	}

	//여기다 애니메이션 이벤트 xml 파일 읽는 부분을 추가해주고.......
	RAniEventMgr* AniEventMgr = ZGetAniEventMgr();
	if (!AniEventMgr->ReadXml(ZApplication::GetFileSystem(), "System/AnimationEvent.xml"))
	{
		mlog("Read Animation Event Failed");
		return false;
	}
	m_bLoaded = true;
	return true;
}

void ZSurvival::Reload()
{
	m_bLoaded = false;

#ifdef _DEBUG
	ZApplication::GetSkillManager()->Destroy();
	ZApplication::GetSkillManager()->Create();
#endif

	m_NPCCatalogue.Clear();
	m_MapCatalogue.Clear();
	m_DropTable.Clear();

	RAniEventMgr* AniEventMgr = ZGetAniEventMgr();
	AniEventMgr->Destroy();

	Load();
}

void ZSurvival::OnGameCreate()
{
	if (!ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())) return;


	// 여기서 npc.xml 등 퀘스트에 필요한 파일을 읽는다. 만약 이미 이전에 읽었었다면 읽지 않는다.
	// 때문에 ZSurvival::OnDestroyOnce()는 명시적으로 호출하지 않음.
	OnCreateOnce();
	Create();

	// 라운드별 보상 XP BP 카운터를 초기화
	m_nRewardBP = 0;
	m_nRewardXP = 0;

	m_nReachedRound = 0;
	m_nPoint = 0;

	// 서바이벌맵 해킹 감지
	if (m_MapCatalogue.IsHacked())
	{
		ZGetGameInterface()->ShowDisconnectMsg( MERR_FIND_HACKER, 30000 );
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


#else

	void ZSurvival::OnGameCreate() {}
	void ZSurvival::OnGameDestroy() {} 
	void ZSurvival::OnGameUpdate(float fElapsed) {} 
	bool ZSurvival::OnCreate(){return true;}
	void ZSurvival::OnDestroy() {}
#endif

void ZSurvival::UpdateNavMeshWeight(float fDelta)
{
	// NavMesh 가중치 업데이트
	if ((ZGetGame()->GetTime() - m_fLastWeightTime) >= 1.0f)
	{
#ifdef _DEBUG
		unsigned long int nLastTime = timeGetTime();
#endif

		RNavigationMesh* pNavMesh = ZGetGame()->GetWorld()->GetBsp()->GetNavigationMesh();		
		if (pNavMesh != NULL)
		{
			pNavMesh->ClearAllNodeWeight();
			ZNPCObjectMap* pNPCObjectMap = ZGetObjectManager()->GetNPCObjectMap();
			for(ZNPCObjectMap::iterator i = pNPCObjectMap->begin();i!=pNPCObjectMap->end();i++)
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
		HANDLE_COMMAND(MC_QUEST_GAME_INFO					,OnQuestGameInfo);
		HANDLE_COMMAND(MC_MATCH_RESPONSE_MONSTER_BIBLE_INFO ,OnSetMonsterBibleInfo);
	}

	return false;
}

bool ZSurvival::OnGameCommand(MCommand* pCommand)
{
	switch (pCommand->GetID())
	{
		HANDLE_COMMAND(MC_QUEST_NPC_LOCAL_SPAWN						,OnNPCSpawn);
		HANDLE_COMMAND(MC_QUEST_NPC_SPAWN							,OnNPCSpawn);
		HANDLE_COMMAND(MC_QUEST_NPC_DEAD							,OnNPCDead);
		HANDLE_COMMAND(MC_QUEST_PEER_NPC_DEAD						,OnPeerNPCDead);
		HANDLE_COMMAND(MC_QUEST_ENTRUST_NPC_CONTROL					,OnEntrustNPCControl);
		HANDLE_COMMAND(MC_QUEST_PEER_NPC_BASICINFO					,OnPeerNPCBasicInfo);
		HANDLE_COMMAND(MC_QUEST_PEER_NPC_HPINFO						,OnPeerNPCHPInfo);
		HANDLE_COMMAND(MC_QUEST_PEER_NPC_ATTACK_MELEE				,OnPeerNPCAttackMelee);
		HANDLE_COMMAND(MC_QUEST_PEER_NPC_ATTACK_RANGE				,OnPeerNPCAttackRange);
		HANDLE_COMMAND(MC_QUEST_PEER_NPC_SKILL_START				,OnPeerNPCSkillStart);
		HANDLE_COMMAND(MC_QUEST_PEER_NPC_SKILL_EXECUTE				,OnPeerNPCSkillExecute);
		HANDLE_COMMAND(MC_QUEST_PEER_NPC_BOSS_HPAP					,OnPeerNPCBossHpAp);

		HANDLE_COMMAND(MC_QUEST_REFRESH_PLAYER_STATUS				,OnRefreshPlayerStatus);
		HANDLE_COMMAND(MC_QUEST_NPC_ALL_CLEAR						,OnClearAllNPC);
		HANDLE_COMMAND(MC_QUEST_ROUND_START							,OnQuestRoundStart);
		HANDLE_COMMAND(MC_MATCH_QUEST_PLAYER_DEAD					,OnQuestPlayerDead);
		HANDLE_COMMAND(MC_QUEST_COMBAT_STATE						,OnQuestCombatState);
		HANDLE_COMMAND(MC_QUEST_MOVETO_PORTAL						,OnMovetoPortal);
		HANDLE_COMMAND(MC_QUEST_READYTO_NEWSECTOR					,OnReadyToNewSector);
		HANDLE_COMMAND(MC_QUEST_SECTOR_START						,OnSectorStart);
		HANDLE_COMMAND(MC_QUEST_OBTAIN_QUESTITEM					,OnObtainQuestItem);
		HANDLE_COMMAND(MC_QUEST_OBTAIN_ZITEM						,OnObtainZItem);
		HANDLE_COMMAND(MC_QUEST_SECTOR_BONUS						,OnSectorBonus);

		HANDLE_COMMAND(MC_QUEST_COMPLETED							,OnQuestCompleted);
		HANDLE_COMMAND(MC_QUEST_FAILED								,OnQuestFailed);
	
		HANDLE_COMMAND(MC_QUEST_PING								,OnQuestPing);

		HANDLE_COMMAND(MC_QUEST_SURVIVAL_RESULT						,OnSurvivalResult);
		HANDLE_COMMAND(MC_SURVIVAL_RANKINGLIST						,OnSurvivalRankingList);
		HANDLE_COMMAND(MC_SURVIVAL_PRIVATERANKING					,OnSurvivalPrivateRanking);

#ifdef _QUEST_ITEM
		//HANDLE_COMMAND(MC_MATCH_USER_REWARD_QUEST					,OnRewardQuest);
		HANDLE_COMMAND(MC_MATCH_NEW_MONSTER_INFO					,OnNewMonsterInfo );
#endif
		
		
	}


	return false;
}

bool ZSurvival::OnNPCSpawn(MCommand* pCommand)
{
	if (ZGetGame() == NULL) return false;

	MUID uidChar, uidNPC;
	unsigned char nNPCType, nPositionIndex;

	pCommand->GetParameter(&uidChar,			0, MPT_UID);
	pCommand->GetParameter(&uidNPC,				1, MPT_UID);
	pCommand->GetParameter(&nNPCType,			2, MPT_UCHAR);
	pCommand->GetParameter(&nPositionIndex,		3, MPT_UCHAR);


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
	{	// 보스 스폰지점이 없는 맵을 서바이벌맵으로 사용한 경우 그냥 밀리용 지점을 사용하자
		if (nSpawnType == ZMST_NPC_BOSS)
		pSpawnData = pMSM->GetSpawnData(ZMST_NPC_MELEE, nPositionIndex);
	}

	rvector NPCPos = rvector(0,0,0);
	rvector NPCDir = rvector(1,0,0);

	if (pSpawnData)
	{
		NPCPos = pSpawnData->m_Pos;
		NPCDir = pSpawnData->m_Dir;
	}
	
	

	// 만약 리소스 로딩을 안했으면 로드 - 이럴일은 테스트빼곤 없어야한다.
//	if (ZIsLaunchDevelop())
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

	////////// 하드코딩.. 특정 맵에서 npc 충돌 반지름을 플레이어의 반지름으로 강제한다  //////
    bool bForceCollRadius35 = false;
	if (m_GameInfo.GetMapSectorID(m_GameInfo.GetCurrSectorIndex()) == 207)	// Prison_hall2
		bForceCollRadius35 = true;
	//////////////////////////////////////////////////////////////////////////////////////////

	ZActor* pNewActor = ZActor::CreateActor(NPCType, fTC, m_GameInfo.GetQuestLevel(), bForceCollRadius35);
	if (pNewActor)
	{
		bool bMyControl = (uidChar == ZGetGameClient()->GetPlayerUID());
		pNewActor->SetMyControl(bMyControl);
		pNewActor->SetUID(uidNPC);
		pNewActor->SetPosition(NPCPos);
		pNewActor->SetDirection(NPCDir);
		
		ZCharacter *pOwner = ZGetCharacterManager()->Find(uidChar);
		if(pOwner)
			pNewActor->SetOwner(pOwner->GetUserName());

		if(pNewActor->m_pVMesh) {
		
			D3DCOLORVALUE color;

			color.r = pNPCInfo->vColor.x;
			color.g = pNPCInfo->vColor.y;
			color.b = pNPCInfo->vColor.z;
			color.a = 1.f;

			pNewActor->m_pVMesh->SetNPCBlendColor(color);//색을 지정한 경우..
		}

		ZGetObjectManager()->Add(pNewActor);
		ZGetEffectManager()->AddReBirthEffect(NPCPos);

		if ((pNPCInfo->nGrade == NPC_GRADE_BOSS) || (pNPCInfo->nGrade == NPC_GRADE_LEGENDARY))
		{
			// 만약 보스급 NPC가 스폰하면 자동적으로 boss 등록
			m_GameInfo.GetBosses().push_back(uidNPC);

			// 바운딩볼륨이 큰 보스가 벽 근처에서 스폰될때 바운딩이 벽에 일부 묻힌 채로 스폰될 수가 있다
			// 이렇게 되면 간혹 벽 뒤로 이동해버리는 경우가 발생 -> 보이지 않으므로 보스를 죽일 수 없다 -> 게임 진행 막힘
			// 보스형 몬스터에 한해서 스폰할때 벽과 충돌체크를 해서 초기위치를 조정한다
			if (pNewActor->IsMyControl()) {
				float radius = bForceCollRadius35 ? 35.f : pNewActor->GetCollRadius();
				if (true == ZGetGame()->GetWorld()->GetBsp()->CheckSolid(NPCPos, radius)) {	// 맵에 충돌한다면.
					OutputDebugString("보스몹 스폰지점 충돌검사 실패...\n");

					// 가까운 네비게이션 노드의 중앙으로 옮겨버린다
					RNavigationMesh* pNavMesh = ZGetGame()->GetWorld()->GetBsp()->GetNavigationMesh();
					if (pNavMesh) {
						RNavigationNode* pNavNode = pNavMesh->FindClosestNode(NPCPos);
						if (pNavNode) {
							pNewActor->SetPosition( pNavNode->CenterVertex());
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

	pCommand->GetParameter(&uidPlayer,	0, MPT_UID);
	pCommand->GetParameter(&uidNPC,		1, MPT_UID);

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

/*
bool ZSurvival::OnQuestGroupLoad(MCommand* pCommand)
{
	if(!pCommand) return false;

	int nGroupID = 0;

	pCommand->GetParameter(&nGroupID, 0, MPT_INT);

	MNPCGroup* pGroup = MGetNPCGroupMgr()->GetGroup( nGroupID );

	ZGetNpcMeshMgr()->CheckUnUsed();// 모두 사용안하는걸로 체크하고

	if( pGroup ) {

		MNPCList::iterator it;

		for( it = pGroup->m_NpcList.begin();it!=pGroup->m_NpcList.end();++it) {

			ZGetNpcMeshMgr()->Load( (char*)(*it).c_str() );
		}
	}

	ZGetNpcMeshMgr()->ReloadAllAnimation();// 읽지 않은 에니메이션이 있다면 로딩

	ZGetNpcMeshMgr()->UnLoadChecked();// 사용안하는 모델 제거

	return true;
}

bool ZSurvival::OnQuestGroupClear(MCommand* pCommand)
{
	ZGetNpcMeshMgr()->UnLoadAll();

	return true;
}
*/

bool ZSurvival::OnEntrustNPCControl(MCommand* pCommand)
{
	MUID uidChar, uidNPC;
	pCommand->GetParameter(&uidChar,	0, MPT_UID);
	pCommand->GetParameter(&uidNPC,		1, MPT_UID);

	ZActor* pNPC = ZGetObjectManager()->GetNPCObject(uidNPC);
	if (pNPC)
	{
		// uidChar이 내플레이어 UID이면 해당 NPC는 내가 조종하는 것이다.
		bool bMyControl = (uidChar == ZGetGameClient()->GetPlayerUID());
		pNPC->SetMyControl(bMyControl);

		ZCharacter *pOwner = ZGetCharacterManager()->Find(uidChar);
		if(pOwner)
			pNPC->SetOwner(pOwner->GetUserName());
	}

	return true;
}

bool ZSurvival::OnPeerNPCBasicInfo(MCommand* pCommand)
{
	MCommandParameter* pParam = pCommand->GetParameter(0);
	if(pParam->GetType()!=MPT_BLOB) return false;

	ZACTOR_BASICINFO* ppbi= (ZACTOR_BASICINFO*)pParam->GetPointer();
	
	ZBasicInfo bi;
	bi.position = rvector(ppbi->posx,ppbi->posy,ppbi->posz);
	bi.velocity = rvector(ppbi->velx,ppbi->vely,ppbi->velz);
	bi.direction = 1.f/32000.f * rvector(ppbi->dirx,ppbi->diry,ppbi->dirz);
	
	ZActor* pActor = ZGetObjectManager()->GetNPCObject(ppbi->uidNPC);
	if (pActor)
	{
		pActor->InputBasicInfo(&bi, ppbi->anistate);
	}


/*
	// 캐릭터의 현재시간을 업데이트한다
	// 캐릭터의 현재시간 추정치
	float fCurrentLocalTime = pActor->m_fTimeOffset + GetTime();

	// 캐릭터가 보내온 시간이 내가 추정한 시간과 3초 이상 차이가 나면 내가 알고있는 시간을 고친다.
	float fTimeError = ppbi->fTime - fCurrentLocalTime;
	if(fabs(fTimeError)>3.f) {
		pCharacter->m_fTimeOffset = ppbi->fTime - GetTime();
		pCharacter->m_fAccumulatedTimeError = 0;
		pCharacter->m_nTimeErrorCount = 0;
	}else
	{
		// 차이가 3초 이내이고 일정시간 합했다가 조금(차이의 반)씩 조절한다
		pCharacter->m_fAccumulatedTimeError += fTimeError;
		pCharacter->m_nTimeErrorCount ++;
		if(pCharacter->m_nTimeErrorCount > 10) {
			pCharacter->m_fTimeOffset += .5f*pCharacter->m_fAccumulatedTimeError/10.f;
			pCharacter->m_fAccumulatedTimeError = 0;
			pCharacter->m_nTimeErrorCount = 0;
		}
	}

	// 현재시간을 마지막 데이터 받은시간으로 기록.
	pCharacter->m_fLastReceivedTime = GetTime();
*/

/*
	// 나중에 판정을 위해 histroy 에 보관한다.
	if(bAddHistory)
	{
		ZBasicInfoItem *pitem=new ZBasicInfoItem;
		CopyMemory(&pitem->info, &bi, sizeof(ZBasicInfo));

		pitem->fReceivedTime=GetTime();

		pitem->fSendTime= ppbi->fTime - pCharacter->m_fTimeOffset;	// 내 기준으로 변환
		
		pCharacter->m_BasicHistory.push_back(pitem);

		while(pCharacter->m_BasicHistory.size()>CHARACTER_HISTROY_COUNT)
		{
			delete *pCharacter->m_BasicHistory.begin();
			pCharacter->m_BasicHistory.erase(pCharacter->m_BasicHistory.begin());
		}
	}

	if(bUpdate)
	{
		// 리플레이때를 제외하고 내 캐릭터는 모션이나 속도등등을 업데이트 할 필요가 없다.
		if(!IsReplay() && pCharacter->IsHero()) return;


		((ZNetCharacter*)(pCharacter))->SetNetPosition(bi.position, bi.velocity, bi.direction);

		pCharacter->SetAnimationLower((ZC_STATE_LOWER)ppbi->lowerstate);
		pCharacter->SetAnimationUpper((ZC_STATE_UPPER)ppbi->upperstate);

		// 들고있는 무기가 다르면 바꿔준다
		if(pCharacter->GetItems()->GetSelectedWeaponParts()!=ppbi->selweapon) {
			pCharacter->ChangeWeapon((MMatchCharItemParts)ppbi->selweapon);
		}
	}
*/

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
	pCommand->GetParameter(&uidBoss,	0, MPT_UID);
	pCommand->GetParameter(&fHp,		1, MPT_FLOAT);
	pCommand->GetParameter(&fAp,		2, MPT_FLOAT);

	ZActor* pActor = ZGetObjectManager()->GetNPCObject(uidBoss);
	if (pActor)
	{
		pActor->InputBossHpAp(fHp, fAp);
	}

	return true;
}

bool ZSurvival::OnPrePeerNPCAttackMelee(MCommand* pCommand)	// 실제로 처리하는건 한타이밍 늦다
{
	// TODO: 이때 애니메이션을 시작
	return true;
}

bool ZSurvival::OnPeerNPCAttackMelee(MCommand* pCommand)
{
	MUID uidOwner;
	pCommand->GetParameter(&uidOwner,	0, MPT_UID);

	ZGetGame()->OnPeerShot_Melee(uidOwner,ZGetGame()->GetTime());

	return true;
}

bool ZSurvival::OnPeerNPCAttackRange(MCommand* pCommand)
{
	MUID uidOwner;
	pCommand->GetParameter(&uidOwner,	0, MPT_UID);

	MCommandParameter* pParam = pCommand->GetParameter(1);
	if(pParam->GetType()!=MPT_BLOB) return false;	// 문제가 있다

	ZPACKEDSHOTINFO *pinfo =(ZPACKEDSHOTINFO*)pParam->GetPointer();
	rvector pos = rvector(pinfo->posx,pinfo->posy,pinfo->posz);
	rvector to = rvector(pinfo->tox,pinfo->toy,pinfo->toz);


	// rocket 테스트로 넣어봤다.
	ZObject* pOwner = ZGetGame()->m_ObjectManager.GetObject(uidOwner);
	MMatchItemDesc* pDesc = NULL;

	if(pOwner==NULL) return false; // 보통 치트키를 쓸경우...

	if( pOwner->GetItems() )
		if( pOwner->GetItems()->GetSelectedWeapon() )
			pDesc = pOwner->GetItems()->GetSelectedWeapon()->GetDesc();
	if (pDesc)
	{
		if (pDesc->m_nWeaponType.Ref() == MWT_ROCKET)
		{
			rvector dir = to - pos;
			Normalize(dir);
			ZGetGame()->m_WeaponManager.AddRocket(pos,dir,pOwner);
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

	ZGetGame()->OnPeerShot_Range((MMatchCharItemParts)pinfo->sel_type,uidOwner,ZGetGame()->GetTime(),pos,to);
	

	return true;
}

bool ZSurvival::OnRefreshPlayerStatus(MCommand* pCommand)
{
	// 운영자 hide는 제외
	bool bAdminHide = false;
	if (ZGetMyInfo()->IsAdminGrade()) 
	{
		MMatchObjCache* pCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
		if (pCache && pCache->CheckFlag(MTD_PlayerFlags_AdminHide))
			bAdminHide = true;
	}

	if (!bAdminHide)
	{
		// 옵저버이거나 옵저버 예약상태를 푼다.
		ZGetGame()->ReleaseObserver();

		// 죽어있으면 리스폰
		if (ZGetGame()->m_pMyCharacter->IsDie())
		{
			ZGetGame()->GetMatch()->RespawnSolo();
		}
	}


	// 피와 총알을 채운다
	for(ZCharacterManager::iterator i = ZGetCharacterManager()->begin();i!=ZGetCharacterManager()->end();i++)
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
	pCommand->GetParameter(&nRound,		0, MPT_UCHAR);

	ZGetScreenEffectManager()->AddRoundStart(int(nRound));

	// 월드아이템 초기화
	ZGetWorldItemManager()->Reset();

	return true;
}

bool ZSurvival::OnQuestPlayerDead(MCommand* pCommand)
{
	MUID uidVictim;
	pCommand->GetParameter(&uidVictim,		0, MPT_UID);

	ZCharacter* pVictim = ZGetCharacterManager()->Find(uidVictim);

	if(pVictim)
	{
		if (pVictim != ZGetGame()->m_pMyCharacter)
		{
			pVictim->Die();		// 여기서 실제로 죽는다
		}

		pVictim->GetStatus().CheckCrc();
		pVictim->GetStatus().Ref().AddDeaths();
		if (pVictim->GetStatus().Ref().nLife > 0) 
			pVictim->GetStatus().Ref().nLife--;
		pVictim->GetStatus().MakeCrc();
	}


	//ZGetGame()->OnPeerDieMessage(pVictim, pAttacker);
	

	return true;
}


bool ZSurvival::OnQuestGameInfo(MCommand* pCommand)
{
	MCommandParameter* pParam = pCommand->GetParameter(0);
	if(pParam->GetType()!=MPT_BLOB) return false;
	void* pBlob = pParam->GetPointer();

	MTD_QuestGameInfo* pQuestGameInfo= (MTD_QuestGameInfo*)MGetBlobArrayElement(pBlob, 0);
	
	m_GameInfo.Init(pQuestGameInfo);


	return true;
}

bool ZSurvival::OnQuestCombatState(MCommand* pCommand)
{
	char nState;
	pCommand->GetParameter(&nState,		0, MPT_CHAR);

	MQuestCombatState nCombatState = MQuestCombatState(nState);

	m_QuestCombatState = nCombatState; // 보관..

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
			// 서바이벌이므로 마지막 섹터라도 포탈을 찍어야 한다. 그러나 마지막 반복의 막판이라면 포탈을 없게한다
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

	pCommand->GetParameter(&nCurrSectorIndex,		0, MPT_CHAR);
	pCommand->GetParameter(&nCurrRepeatIndex,		1, MPT_UCHAR);
	pCommand->GetParameter(&uidPlayer,				2, MPT_UID);

	// 포탈로 이동한 사람이 자신이면 여기서 실제로 다음 섹터로 이동
	if (uidPlayer == ZGetGameClient()->GetPlayerUID())
	{
		m_bIsRoundClear = false;
		ZGetQuest()->GetGameInfo()->ClearNPCKilled();

		// 여기서 새로운 섹터로 이동
		m_GameInfo.OnMovetoNewSector((int)(nCurrSectorIndex), nCurrRepeatIndex);

		// 나 새로운 섹터로 왔다고 메시지를 보낸다.
		ZPostQuestReadyToNewSector(ZGetGameClient()->GetPlayerUID());

		// 나를 npc 타겟팅 후보에서 뺀다
		ZGetGame()->ExceptCharacterFromNpcTargetting( ZGetGameInterface()->GetMyCharacter());
	}
	else
	{
		// 해당 플레이어 이동
		ZCharacter *pChar = ZGetCharacterManager()->Find(uidPlayer);
		if(pChar && m_CharactersGone.find(ZGetGameClient()->GetPlayerUID())==m_CharactersGone.end()) {
			// 내가 아직 이동하지 않은 경우 해당플레이어를 안보이게 만든다
			pChar->SetVisible(false);
			ZGetEffectManager()->AddReBirthEffect(pChar->GetPosition());

			// 해당 플레이어를 npc 타겟팅 후보에서 뺀다
			ZGetGame()->ExceptCharacterFromNpcTargetting(pChar);
		}
	}

	// 이 캐릭터를 타겟으로 하는 탄환이 있으면 제거한다
	ZGetGame()->m_WeaponManager.DeleteWeaponHasTarget( uidPlayer );


	return true;
}

bool ZSurvival::OnReadyToNewSector(MCommand* pCommand)
{
	MUID uidPlayer;
	pCommand->GetParameter(&uidPlayer,	0, MPT_UID);

	m_CharactersGone.insert(uidPlayer);

	ZCharacter *pChar = ZGetCharacterManager()->Find(uidPlayer);

	// 내가 옵저브 하고 있는 캐릭터가 이동하면 다른 캐릭터로 바꾼다
	if(ZGetCombatInterface()->GetObserver()->GetTargetCharacter()==pChar) {
		ZGetCombatInterface()->GetObserver()->ChangeToNextTarget();
	}

	if (uidPlayer == ZGetGameClient()->GetPlayerUID()) {
		MoveToNextSector();
	}
	else
	{
		// 해당 플레이어 이동

		ZCharacter *pChar = ZGetCharacterManager()->Find(uidPlayer);
		if(pChar && m_CharactersGone.find(ZGetGameClient()->GetPlayerUID())!=m_CharactersGone.end()) {

			// 내가 이미 이동한 경우 해당플레이어를 보이게 만든다
//			pChar->SetVisible(true);

			// 이번에 이동할 캐릭터의 위치
			int nPosIndex = ZGetCharacterManager()->GetCharacterIndex(pChar->GetUID(), false);
			if (nPosIndex < 0) nPosIndex=0;
			else if (nPosIndex >= MAX_QUSET_PLAYER_COUNT) nPosIndex = MAX_QUSET_PLAYER_COUNT-1;
			ZMapSpawnData* pSpawnData = ZGetWorld()->GetDesc()->GetSpawnManager()->GetSoloData(nPosIndex);
			if(pSpawnData) {
				pChar->SetPosition(pSpawnData->m_Pos);
				pChar->SetDirection(pSpawnData->m_Dir);
				ZGetEffectManager()->AddReBirthEffect(pSpawnData->m_Pos);
			}
			else
			{
				//_ASSERT(0);
			}
		}

	}

	return true;
}

bool ZSurvival::OnSectorStart(MCommand* pCommand)
{
	char nSectorIndex;
	unsigned char nRepeatIndex;
	pCommand->GetParameter(&nSectorIndex,	0, MPT_CHAR);
	pCommand->GetParameter(&nRepeatIndex,	1, MPT_UCHAR);

	m_bIsRoundClear = false;
	ZGetQuest()->GetGameInfo()->ClearNPCKilled();

	// 만약 섹터가 틀리면 강제로 이동한다.
	if (m_GameInfo.GetCurrSectorIndex() != nSectorIndex ||
		m_GameInfo.GetCurrRepeatIndex() != nRepeatIndex)
	{
		// 새로운 섹터로 이동
		m_GameInfo.OnMovetoNewSector((int)nSectorIndex, nRepeatIndex);
		
		MoveToNextSector();
	}

	// 모든 사람들을 보여준다.
	for(ZCharacterManager::iterator i = ZGetCharacterManager()->begin();i!=ZGetCharacterManager()->end();i++)
	{
		i->second->SetVisible(true);
	}

	// npc의 타겟팅 후보 제외 목록을 비운다
	ZGetGame()->ClearListExceptionFromNpcTargetting();

	// 전 화면에서 남아있을 수 있는 탄을 제거한다
	ZGetGame()->m_WeaponManager.Clear();

	ZGetWorldItemManager()->Reset();
	m_CharactersGone.clear();

	// admin hide 이면 다시 옵저버를 활성화
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
	if(pParam->GetType()!=MPT_BLOB) return false;

	void* pBlob = pParam->GetPointer();
	int nBlobSize = MGetBlobArrayCount(pBlob);

	for (int i = 0; i < nBlobSize; i++)
	{
		MTD_QuestReward* pQuestRewardNode = (MTD_QuestReward*)MGetBlobArrayElement(pBlob, i);


		// 여기서 보상 내용을 딴 곳에 저장해서 화면에 보여주면 됩니다. - bird
	}


	// 월드아이템을 모두 없앤다.
	//ZGetWorldItemManager()->Reset(true);

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
	pCommand->GetParameter(&nQuestItemID,	0, MPT_UINT);

	m_GameInfo.IncreaseObtainQuestItem();

#ifdef _QUEST_ITEM
	MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc((int)nQuestItemID);
	if (pQuestItemDesc)
	{
		char szMsg[ 128];
		ZTransMsg( szMsg, MSG_GAME_GET_QUEST_ITEM, 1, pQuestItemDesc->m_szQuestItemName);
		ZChatOutput( MCOLOR(ZCOLOR_CHAT_SYSTEM_GAME), szMsg);
	}
#endif


	return true;
}

bool ZSurvival::OnObtainZItem(MCommand* pCommand)
{
	unsigned long int nItemID;
	pCommand->GetParameter(&nItemID,	0, MPT_UINT);

	m_GameInfo.IncreaseObtainQuestItem();

#ifdef _QUEST_ITEM
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc)
	{
		char szMsg[ 128];
		ZTransMsg( szMsg, MSG_GAME_GET_QUEST_ITEM, 1, pItemDesc->m_pMItemName->Ref().m_szItemName);
		ZChatOutput( MCOLOR(ZCOLOR_CHAT_SYSTEM_GAME), szMsg);
	}
#endif


	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void ZSurvival::LoadNPCMeshes()
{
#ifdef _DEBUG
	// 혼자서 AI 테스트할 경우
	if ((ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_QUEST) || 
		(ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_AI))
	{
		ZGetNpcMeshMgr()->LoadAll();

		ZGetNpcMeshMgr()->ReloadAllAnimation();// 읽지 않은 에니메이션이 있다면 로딩
		return;
	}
#endif

	if (!m_GameInfo.IsInited()) 
	{
		mlog("not inialized Quest Game Info\n");
		//_ASSERT(0);
		return;
	}

	for (int i = 0; i < m_GameInfo.GetNPCInfoCount(); i++)
	{
		MQUEST_NPC npc = m_GameInfo.GetNPCInfo(i);

		ZGetNpcMeshMgr()->Load(GetNPCInfo(npc)->szMeshName);
	}

	ZGetNpcMeshMgr()->ReloadAllAnimation();// 읽지 않은 에니메이션이 있다면 로딩
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
	ZCharacter *pMyChar = ZGetGame()->m_pMyCharacter;
	pMyChar->InitStatus();

	// 새로운 월드로 이동!!
	ZGetWorldManager()->SetCurrent(m_GameInfo.GetCurrSectorIndex());
	// 이번에 이동할 캐릭터의 위치
	int nPosIndex = ZGetCharacterManager()->GetCharacterIndex(pMyChar->GetUID(), false);
	if (nPosIndex < 0) nPosIndex=0;
	ZMapSpawnData* pSpawnData = ZGetWorld()->GetDesc()->GetSpawnManager()->GetSoloData(nPosIndex);
	// 새 좌표로 이동
	if (pSpawnData!=NULL && pMyChar!=NULL)
	{
		pMyChar->SetPosition(pSpawnData->m_Pos);
		pMyChar->SetDirection(pSpawnData->m_Dir);
		ZGetEffectManager()->AddReBirthEffect(pSpawnData->m_Pos);
	}

	// 유저 캐릭터를 보여주지 않는다
	for(ZCharacterManager::iterator i = ZGetCharacterManager()->begin();i!=ZGetCharacterManager()->end();i++)
	{
		i->second->SetVisible(false);
	}
	// 현재 npc, 앞으로 생성될 npc, 이펙트, 탄환 전부 숨긴다 (다음 섹터 시작할때까지)
	ZGetObjectManager()->GetNPCObjectMap()->SetVisibleAll(false);
	ZGetObjectManager()->GetNPCObjectMap()->ForceInvisibleNewNpc(true);
	ZGetEffectManager()->EnableDraw(false);
	ZGetWorldItemManager()->EnableDraw(false);
	ZGetGame()->m_WeaponManager.EnableRender(false);

	// ko수 동기화
	ZModule_QuestStatus* pMod = (ZModule_QuestStatus*)pMyChar->GetModule(ZMID_QUESTSTATUS);
	if (pMod)
	{
		int nKills = pMod->GetKills();
		ZGetScreenEffectManager()->SetKO(nKills);
	}
}

bool ZSurvival::OnSurvivalResult(MCommand* pCmd)
{
	if( 0 == pCmd )
		return false;

	int nReachedRound;
	int nPoint;

	pCmd->GetParameter( &nReachedRound, 0, MPT_INT );
	pCmd->GetParameter( &nPoint, 1, MPT_INT );

	m_nReachedRound = nReachedRound;
	m_nPoint = nPoint;
	
	return true;
}

bool ZSurvival::OnSurvivalRankingList(MCommand* pCmd)
{
	if( 0 == pCmd ) return false;

	MTextArea* pWidgetRank = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "SurvivalResult_PlayerRankList");
	MTextArea* pWidgetName = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "SurvivalResult_PlayerNameList");
	MTextArea* pWidgetPoint = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "SurvivalResult_PlayerRankPointList");
	if (!pWidgetRank || !pWidgetName || !pWidgetPoint) return false;

	pWidgetRank->Clear();
	pWidgetName->Clear();
	pWidgetPoint->Clear();

	MCommandParameter* pParam = pCmd->GetParameter( 0 );
	void* pRankBlob = (MTD_SurvivalRanking*)pParam->GetPointer();
	MTD_SurvivalRanking* pRank;
	char szText[128];

	int sizeBlob = MGetBlobArrayCount( pRankBlob );
	int size = min(sizeBlob, MAX_SURVIVAL_RANKING_LIST);

	for (int i = 0; i < size; ++i)
	{
		pRank = reinterpret_cast< MTD_SurvivalRanking* >( MGetBlobArrayElement(pRankBlob, i) );

		if (pRank->m_dwRank == 0) break;

		sprintf(szText, "%d", pRank->m_dwRank);
		pWidgetRank->AddText( szText, MCOLOR( 0xFFFFF794));

		if (strlen(pRank->m_szCharName) != 0) pWidgetName->AddText( pRank->m_szCharName, MCOLOR( 0xFFFFF794));
		else								  pWidgetName->AddText( " ", MCOLOR( 0xFFFFF794));			

		sprintf(szText, "%d", pRank->m_dwPoint);
		pWidgetPoint->AddText( szText, MCOLOR( 0xFFFFF794));
	}
	return true;
}

bool ZSurvival::OnSurvivalPrivateRanking(MCommand* pCmd)
{
	if( 0 == pCmd ) return false;
	
	MTextArea* pWidgetRank = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "SurvivalResult_MyRank");
	MTextArea* pWidgetName = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "SurvivalResult_MyName");
	MTextArea* pWidgetPoint = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "SurvivalResult_MyRankPoint");
	if (!pWidgetRank || !pWidgetName || !pWidgetPoint) return false;

	pWidgetRank->Clear();
	pWidgetName->Clear();
	pWidgetPoint->Clear();

	unsigned int nRank, nPoint;
	pCmd->GetParameter( &nRank, 0, MPT_UINT );
	pCmd->GetParameter( &nPoint, 1, MPT_UINT );

	char szText[128];
	
	if (nRank != 0)
		sprintf(szText, "%d", nRank);
	else
		sprintf(szText, "--");
	pWidgetRank->AddText(szText, MCOLOR( 0xFFFFFFCC));

	sprintf(szText, "%d", nPoint);
	pWidgetPoint->AddText(szText, MCOLOR( 0xFFFFFFCC));

	pWidgetName->AddText(ZGetGame()->m_pMyCharacter->GetUserName(), MCOLOR(0xFFFFFFCC));

	return true;
}

#ifdef _QUEST_ITEM
/*bool ZSurvival::OnRewardQuest( MCommand* pCmd )
{
	if( 0 == pCmd )
		return false;
	
	int nXP, nBP;

	pCmd->GetParameter( &nXP, 0, MPT_INT );
	pCmd->GetParameter( &nBP, 1, MPT_INT );
	MCommandParameter* pParam1 = pCmd->GetParameter( 2 );
	MCommandParameter* pParam2 = pCmd->GetParameter( 3 );

	GetMyObtainQuestItemList( nXP, nBP, pParam1->GetPointer(), pParam2->GetPointer() );
	
	return true;
}*/

// 획득 아이템 리스트 박스 업데이트
class ObtainItemListBoxItem : public MListItem
{
protected:
	MBitmap*			m_pBitmap;
	char				m_szName[ 128];

public:
	ObtainItemListBoxItem( MBitmap* pBitmap, const char* szName)
	{
		m_pBitmap = pBitmap;
		strcpy( m_szName, szName);
	}
	virtual const char* GetString( void)
	{
		return m_szName;
	}
	virtual const char* GetString( int i)
	{
		if ( i == 1)
			return m_szName;

		return NULL;
	}
	virtual MBitmap* GetBitmap( int i)
	{
		if ( i == 0)
			return m_pBitmap;

		return NULL;
	}
};
void ZSurvival::GetMyObtainQuestItemList( int nRewardXP, int nRewardBP, void* pMyObtainQuestItemListBlob, void* pMyObtainZItemListBlob )
{
	// 받은 XP, BP를 사용하지 않고 라운드 끝날때마다 누적해둔 것을 사용
	// 또 서바이벌에서는 퀘스트아이템을 습득하지 않으므로 아래 코드는 필요없다
/*	m_nRewardXP = nRewardXP;
	m_nRewardBP = nRewardBP;

	if (( 0 == pMyObtainQuestItemListBlob ) || ( 0 == pMyObtainZItemListBlob )) return;

	int							i;
	int							nQuestItemCount;
	MTD_QuestItemNode*			pQuestItemNode;
	ZMyQuestItemNode*			pNewQuestItem;
	ZMyQuestItemMap::iterator	itQItem;
	ZMyInfo*					pMyInfo;

	pMyInfo = ZGetMyInfo();
	MListBox* pListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget( "SurvivalResult_RankingListbox");
	if ( pListBox)
		pListBox->RemoveAll();

	// 퀘스트 아이템 -----------------
	nQuestItemCount = MGetBlobArrayCount( pMyObtainQuestItemListBlob );

	for( i = 0; i < nQuestItemCount; ++i )
	{
		pQuestItemNode = reinterpret_cast< MTD_QuestItemNode* >( MGetBlobArrayElement(pMyObtainQuestItemListBlob, i) );

		// 리스트 박스 업데이트
		if ( pListBox && (pQuestItemNode->m_nCount > 0))
		{
			MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc( pQuestItemNode->m_nItemID);
			char szNum[ 10];
			sprintf( szNum, "%d", pQuestItemNode->m_nCount);
			char szMsg[ 128];
			ZTransMsg( szMsg, MSG_GAME_GET_QUEST_ITEM2, 2, pQuestItemDesc->m_szQuestItemName, szNum);
			pListBox->Add( new ObtainItemListBoxItem( ZGetGameInterface()->GetQuestItemIcon( pQuestItemNode->m_nItemID, true), szMsg));
		}


		itQItem = pMyInfo->GetItemList()->GetQuestItemMap().find( pQuestItemNode->m_nItemID );
		if( pMyInfo->GetItemList()->GetQuestItemMap().end() != itQItem )
		{
			// 케릭터의 가지고 있는 퀘스트 아이템에 추가.
			if( 0 != pQuestItemNode->m_nCount )
			{
				// 수량이 최대치를 초과하지 않아서 정상적으로 추가가 되었을 경우.
				itQItem->second->Increase( pQuestItemNode->m_nCount );
			}
			else
			{
				// 수량이 최대치를 초과하여서 추가할 개수를 0으로 보내줬을 경우.
				// 이때는 추가적으로 클라이언트가 초과된 정보와 그에따른 처리정보를 보여 줘야 함.
			}
		}
		else
		{
			// 새로 등록되는 아이템은 초기화를 해주고 등록을 해줘야 함.

			pNewQuestItem = new ZMyQuestItemNode;
			if( 0 == pNewQuestItem )
			{
				continue;
			}

			pNewQuestItem->Create( pQuestItemNode->m_nItemID, 
								   pQuestItemNode->m_nCount, 
								   GetQuestItemDescMgr().FindQItemDesc(pQuestItemNode->m_nItemID) );

			pMyInfo->GetItemList()->GetQuestItemMap().Add( pNewQuestItem->GetItemID(), pNewQuestItem );
		}
	}

	// 일반 아이템 -----------------
	int nZItemCount = MGetBlobArrayCount( pMyObtainZItemListBlob );
	for (int i = 0; i < nZItemCount; i++)
	{
		MTD_QuestZItemNode* pZItemNode = (MTD_QuestZItemNode*)( MGetBlobArrayElement(pMyObtainZItemListBlob, i) );

		// 리스트 박스 업데이트
		if ( pListBox )
		{
			MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(pZItemNode->m_nItemID);

			char szMsg[ 128];
			ZTransMsg( szMsg, MSG_GAME_GET_QUEST_ITEM2, 2, pItemDesc->m_pMItemName->GetData().m_szItemName, "1");
			pListBox->Add( new ObtainItemListBoxItem( GetItemIconBitmap( pItemDesc, true), szMsg));
		}
	}
*/
}


bool ZSurvival::OnNewMonsterInfo( MCommand* pCmd )
{
	char nMonIndex;

	if( 0 == pCmd )
		return false;

	pCmd->GetParameter( &nMonIndex, 0, MPT_CHAR );
	return true;
}

#endif

bool ZSurvival::OnPeerNPCSkillStart(MCommand* pCommand)
{
	MUID uidOwner,uidTarget;
	int nSkill;
	rvector targetPos;
	pCommand->GetParameter(&uidOwner,	0, MPT_UID);
	pCommand->GetParameter(&nSkill,		1, MPT_INT);
	pCommand->GetParameter(&uidTarget,	2, MPT_UID);
	pCommand->GetParameter(&targetPos,	3, MPT_POS);

	ZActor* pActor = ZGetObjectManager()->GetNPCObject(uidOwner);
	if (pActor)
	{
		ZModule_Skills *pmod = (ZModule_Skills *)pActor->GetModule(ZMID_SKILLS);
		pmod->PreExcute(nSkill,uidTarget,targetPos);
	}

	return true;
}

bool ZSurvival::OnPeerNPCSkillExecute(MCommand* pCommand)
{
	MUID uidOwner,uidTarget;
	int nSkill;
	rvector targetPos;
	pCommand->GetParameter(&uidOwner,	0, MPT_UID);
	pCommand->GetParameter(&nSkill,		1, MPT_INT);
	pCommand->GetParameter(&uidTarget,	2, MPT_UID);
	pCommand->GetParameter(&targetPos,	3, MPT_POS);

	ZActor* pActor = ZGetObjectManager()->GetNPCObject(uidOwner);
	if (pActor)
	{
		ZModule_Skills *pmod = (ZModule_Skills *)pActor->GetModule(ZMID_SKILLS);
		pmod->Excute(nSkill,uidTarget,targetPos);
	}

	return true;
}


bool ZSurvival::OnSetMonsterBibleInfo( MCommand* pCmd )
{
	if( 0 == pCmd )
		return false;

	MUID				uid;
	MCommandParameter*	pParam;
	void*				pMonBibleInfoBlob;
	MQuestMonsterBible*	pMonBible;

	pCmd->GetParameter( &uid, 0, MPT_UID );

	if( uid != ZGetGameClient()->GetPlayerUID() )
	{
		return false;
	}

	pParam				= pCmd->GetParameter(1);
	pMonBibleInfoBlob	= pParam->GetPointer();
	pMonBible			= reinterpret_cast< MQuestMonsterBible* >( MGetBlobArrayElement(pMonBibleInfoBlob, 0) );

	return true;
}

bool ZSurvival::OnPeerNPCDead(MCommand* pCommand)
{
	MUID uidKiller, uidNPC;

	pCommand->GetParameter(&uidKiller,	0, MPT_UID);
	pCommand->GetParameter(&uidNPC,		1, MPT_UID);

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

	pCommand->GetParameter(&uidPlayer,	0, MPT_UID);
	pCommand->GetParameter(&nExpValue,	1, MPT_UINT);
	pCommand->GetParameter(&nAddedBP,	2, MPT_UINT);


	int nAddedXP = GetExpFromTransData(nExpValue);
	int nExpPercent = GetExpPercentFromTransData(nExpValue);


	if(ZGetCharacterManager()->Find(uidPlayer) == ZGetGame()->m_pMyCharacter)
	{
		m_nRewardXP += nAddedXP;
		m_nRewardBP += nAddedBP;

		//ZGetScreenEffectManager()->AddExpEffect(nAddedXP);		// 획득 경험치 표시 없애기
		ZGetMyInfo()->SetLevelPercent(nExpPercent);
		ZGetScreenEffectManager()->SetGaugeExpFromMyInfo();
	}

	return true;
}


bool ZSurvival::OnQuestPing(MCommand* pCommand)
{
	unsigned long int TimeStamp;
	pCommand->GetParameter(&TimeStamp,	0, MPT_UINT);
	ZPostQuestPong(TimeStamp);

	return true;
}