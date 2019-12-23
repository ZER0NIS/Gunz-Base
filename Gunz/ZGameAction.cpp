#include "stdafx.h"
#include "ZGameAction.h"
#include "ZGame.h"
#include "ZGameClient.h"
#include "ZEffectManager.h"
#include "ZApplication.h"
#include "ZSoundEngine.h"
#include "ZMyCharacter.h"
#include "ZPost.h"
#include "ZModule_FireDamage.h"
#include "ZModule_ColdDamage.h"
#include "ZModule_LightningDamage.h"
#include "ZModule_PoisonDamage.h"

#define MAX_ENCHANT_DURATION	10.f

bool ZGameAction::OnCommand(MCommand* pCommand)
{
	switch (pCommand->GetID())
	{
		HANDLE_COMMAND(MC_PEER_ENCHANT_DAMAGE,		OnEnchantDamage)
		HANDLE_COMMAND(MC_PEER_REACTION,			OnReaction)
		HANDLE_COMMAND(MC_PEER_SKILL,				OnPeerSkill)
	}

	return false;
}

bool ZGameAction::OnReaction(MCommand* pCommand)
{
	float fTime;
	int nReactionID;

	pCommand->GetParameter(&fTime,			0, MPT_FLOAT);		// 시간
	pCommand->GetParameter(&nReactionID,	1, MPT_INT);

	ZCharacter *pChar=ZGetCharacterManager()->Find(pCommand->GetSenderUID());
	if(!pChar) return true;

	switch(nReactionID)
	{
		case ZR_CHARGING	: {
			//pChar->m_bCharging=true;
			pChar->m_bCharging->Set_CheckCrc(true);	//mmemory proxy
			if(!pChar->IsHero())
				pChar->SetAnimationLower(ZC_STATE_CHARGE);
			ZGetEffectManager()->AddChargingEffect(pChar);

			if ( pChar->GetProperty()->nSex == MMS_MALE)
				ZGetSoundEngine()->PlaySound( "fx2/MAL05", pChar->GetPosition());
			else
				ZGetSoundEngine()->PlaySound( "fx2/FEM05", pChar->GetPosition());
		}break;
		case ZR_CHARGED		: {
			//pChar->m_bCharged=true;	
			pChar->m_bCharged->Set_CheckCrc(true);	//mmemory proxy
			pChar->m_fChargedFreeTime.Set_CheckCrc( ZGetGame()->GetTime() + fTime);
			ZGetEffectManager()->AddChargedEffect(pChar);

			ZGetSoundEngine()->PlaySoundChargeComplete(pChar->GetSelectItemDesc(), pChar->GetPosition());
		}break;
		case ZR_BE_UPPERCUT	: {
			rvector tpos = pChar->GetPosition();
			tpos.z += 130.f;
			ZGetEffectManager()->AddSwordUppercutDamageEffect(tpos,pChar->GetUID());
			ZGetSoundEngine()->PlaySound("uppercut", tpos);
		}break;
		case ZR_DISCHARGED	: {
			//pChar->m_bCharged=false;
			pChar->m_bCharged->Set_CheckCrc(false);
		}break;
	}

	return true;
}

bool ZGameAction::OnPeerSkill(MCommand* pCommand)
{
	float fTime;
	int nSkill,sel_type;

	pCommand->GetParameter(&fTime, 0, MPT_FLOAT);
	pCommand->GetParameter(&nSkill, 1, MPT_INT);
	pCommand->GetParameter(&sel_type, 2, MPT_INT);

	ZCharacter* pOwnerCharacter = ZGetCharacterManager()->Find(pCommand->GetSenderUID());
	if (pOwnerCharacter == NULL) return true;

	switch(nSkill)	{
		// 띄우기 스킬
		case ZC_SKILL_UPPERCUT		: OnPeerSkill_Uppercut(pOwnerCharacter);break;
			// 강베기 스플래시
		case ZC_SKILL_SPLASHSHOT	: OnPeerSkill_LastShot(fTime,pOwnerCharacter);break;
			// 단검 특수공격
		case ZC_SKILL_DASH			: OnPeerSkill_Dash(pOwnerCharacter);break;
	}

	return true;
}

// 강베기 처리한다. 내가 맞았는지만 검사한다
void ZGameAction::OnPeerSkill_LastShot(float fShotTime,ZCharacter *pOwnerCharacter)	// 칼 마지막 방 스플래시
{
	//jintriple3 디버그 레지스터 핵 방어용 불리안 값. 릴리즈 모드에서 같은 if문은 code optimize과정에서 삭제해버리므로...
	bool bReturnValue;
	//jintriple3 디버그 레지스터 핵 방어 코드...
	bReturnValue = pOwnerCharacter == NULL;
	if( pOwnerCharacter == NULL )
		PROTECT_DEBUG_REGISTER( bReturnValue )
			return;
	ZItem *pItem = pOwnerCharacter->GetItems()->GetItem(MMCIP_MELEE);
	bReturnValue = !pItem;
	if(!pItem) 
		PROTECT_DEBUG_REGISTER( bReturnValue )
			return;

	MMatchItemDesc* pDesc = pItem->GetDesc();
	bReturnValue = !pDesc;
	if(!pDesc) 
		PROTECT_DEBUG_REGISTER( bReturnValue )
			return;


	// fShotTime 이 그 캐릭터의 로컬 시간이므로 내 시간으로 변환해준다
	fShotTime -= pOwnerCharacter->m_fTimeOffset;
	/*
	float fCurrentTime = g_pGame->GetTime();

	if( abs(fCurrentTime - fShotTime ) > TIME_ERROR_BETWEEN_RECIEVEDTIME_MYTIME )
	{
#ifdef _DEBUG
		mlog("!!!!강베기 핵 사용!!!!캐릭터 네임: %s      fShotTime : %f     fCurrentTime : %f \n", 
			pOwnerCharacter->GetUserName(), fShotTime - pOwnerCharacter->m_fTimeOffset , fCurrentTime);
#endif
		return;
	}
	이 부분은 핵에서 shot을 한 시간을 조작하여 보내는 것을 감지하여 핵을 막는 코드였는데 받는 쪽에서 시간 검사를 하지 말고 
	보내는 쪽에서 검사를 해서 shot을 한 시간이 해당 캐릭터의 lacal time과 맞지 않으면 아예 패킷을 보내지 않도록 바꿨다. 
	따라서 해당 코드가 필요 없게 됨. 추후 localtime을 조작할 경우를 대비해 주석처리로 남겨둠..
	*/

		// 비정상적인 발사속도를 무시한다.
	//jintriple3 디버그 레지스터 핵 방어...이건 강베기 핵 방어 전용..
	//bReturnValue = pOwnerCharacter->CheckValidShotTime(pItem->GetDescID(), fShotTime, pItem);

	if (pOwnerCharacter->CheckValidShotTime(pItem->GetDescID(), fShotTime, pItem))
	{
		PROTECT_DEBUG_REGISTER( pOwnerCharacter->m_dwIsValidTime == FOR_DEBUG_REGISTER )
		{
			pOwnerCharacter->UpdateValidShotTime(pItem->GetDescID(), fShotTime);
		}

	}
	else 
	{
		pOwnerCharacter->UpdateValidShotTime(pItem->GetDescID(), fShotTime);
		// //_ASSERT(FALSE);	//핵 방어코드...어썰트를 삽입해서 말도 안되는 속도로 이 함수가 불릴때 게임이 힘들어지도록 한다. 
		return;
	}


	/////////////////////////////////////////////////////////////////////////////////////
//#endif

	const float fRange = 300.f;			// 범위는 4미터

//	if(pOwnerCharacter->m_AniState_Lower>=ZC_STATE_LOWER_ATTACK3 && pOwnerCharacter->m_AniState_Lower<=ZC_STATE_LOWER_ATTACK5)
	{
		// fShotTime 이 그 캐릭터의 로컬 시간이므로 내 시간으로 변환해준다
//		fShotTime-=pOwnerCharacter->m_fTimeOffset;

		rvector OwnerPosition,OwnerDir;
		bReturnValue = !pOwnerCharacter->GetHistory(&OwnerPosition,&OwnerDir,fShotTime);
		if(!pOwnerCharacter->GetHistory(&OwnerPosition,&OwnerDir,fShotTime))
			PROTECT_DEBUG_REGISTER( bReturnValue )
				return;


		rvector waveCenter = OwnerPosition; // 폭발의 중심

		rvector _vdir = OwnerDir;
		_vdir.z = 0;
//		Normalize(_vdir);
//		waveCenter += _vdir * 180.f;

		ZC_ENCHANT zc_en_type = pOwnerCharacter->GetEnchantType();

		// 사운드
		ZGetSoundEngine()->PlaySoundSmash(pDesc, waveCenter, pOwnerCharacter->IsObserverTarget());

		// 바닥의 wave 이펙트
		{
			ZGetEffectManager()->AddSwordWaveEffect(waveCenter,0,pOwnerCharacter);
		}

		for (ZObjectManager::iterator itor = ZGetObjectManager()->begin();
			itor != ZGetObjectManager()->end(); ++itor)
		{
			ZObject* pTar = (*itor).second;
			//jintriple3 디버그 레지스터 해킹 방어 코드
			bReturnValue = pTar==NULL;
			if (pTar==NULL) 
				PROTECT_DEBUG_REGISTER( bReturnValue )
					continue;
			bReturnValue = pOwnerCharacter == pTar;
			if (pOwnerCharacter == pTar) 
				PROTECT_DEBUG_REGISTER(bReturnValue)
					continue;
			bReturnValue = pTar!=ZGetGame()->m_pMyCharacter && (!pTar->IsNPC() || !((ZActor*)pTar)->IsMyControl());
			if(pTar!=ZGetGame()->m_pMyCharacter &&	// 내 캐릭터나 내가 조종하는 npc 만 체크한다
				(!pTar->IsNPC() || !((ZActor*)pTar)->IsMyControl()))
				PROTECT_DEBUG_REGISTER(bReturnValue)
					continue;

			bReturnValue = !ZGetGame()->CanAttack(pOwnerCharacter, pTar);
			if(!ZGetGame()->CanAttack(pOwnerCharacter,pTar)) 
				PROTECT_DEBUG_REGISTER(bReturnValue)
					continue;
			//// 팀플레이고 같은 팀이고 팀킬 불가로 되어있으면 넘어간다
			//if(ZGetGame()->GetMatch()->IsTeamPlay() &&
			//	pOwnerCharacter->IsTeam(pTar) && !g_pGame->GetMatch()->GetTeamKillEnabled()) return;

			rvector TargetPosition,TargetDir;
			
			bReturnValue = pTar->IsDie();
			if(pTar->IsDie()) 
				PROTECT_DEBUG_REGISTER(bReturnValue)
					continue;
			// 적절한 위치를 얻어낼수 없으면 다음으로~
			bReturnValue = !pTar->GetHistory(&TargetPosition, &TargetDir, fShotTime);
			if( !pTar->GetHistory(&TargetPosition,&TargetDir,fShotTime)) 
				PROTECT_DEBUG_REGISTER(bReturnValue)
					continue;

			rvector checkPosition = TargetPosition + rvector(0,0,80);
			float fDist = Magnitude(waveCenter - checkPosition);
			//jintriple3 디버그 레지스터 해킹 방어 코드
			bReturnValue = fDist >= fRange;
			if( fDist >= fRange)
				PROTECT_DEBUG_REGISTER(bReturnValue)
					continue;

			bReturnValue = (!pTar) || (pTar == pOwnerCharacter);
			if( (!pTar) || (pTar == pOwnerCharacter) )
				PROTECT_DEBUG_REGISTER(bReturnValue)
					continue;

			// 중간에 벽이 막고 있는가?
			bReturnValue = ZGetGame()->CheckWall( pOwnerCharacter, pTar) == true;
			if( ZGetGame()->CheckWall( pOwnerCharacter,pTar ) == true)
				PROTECT_DEBUG_REGISTER(bReturnValue)
					continue;
			// 막고있으면 데미지를 안받는다
			bReturnValue = pTar->IsGuard() && DotProduct(pTar->m_Direction,OwnerDir)<0;
			if( pTar->IsGuard() && DotProduct(pTar->m_Direction,OwnerDir)<0 )
			{
				PROTECT_DEBUG_REGISTER(bReturnValue)
				{
					rvector addVel = pTar->GetPosition() - waveCenter;
					Normalize(addVel);
					addVel = 500.f*addVel;
					addVel.z = 200.f;
					pTar->AddVelocity(addVel);
					continue;
				}
			}
			//모든 조건을 통과했으면
			rvector tpos = pTar->GetPosition();

			tpos.z += 130.f;
			if( zc_en_type == ZC_ENCHANT_NONE ) 
				ZGetEffectManager()->AddSwordUppercutDamageEffect(tpos,pTar->GetUID());
			else 
				ZGetEffectManager()->AddSwordEnchantEffect(zc_en_type,pTar->GetPosition(),20);

			tpos -= pOwnerCharacter->m_Direction * 50.f;
			rvector fTarDir = pTar->GetPosition() - pOwnerCharacter->GetPosition();
			Normalize(fTarDir);

#define MAX_DMG_RANGE	50.f	// 반경이만큼 까지는 최대 데미지를 다 먹는다
#define MIN_DMG			0.3f	// 최소 기본 데미지는 이정도.

			float fDamageRange = 1.f - (1.f-MIN_DMG)*( max(fDist-MAX_DMG_RANGE,0) / (fRange-MAX_DMG_RANGE));
//							pTar->OnDamagedKatanaSplash( pOwnerCharacter, fDamageRange );
#define SPLASH_DAMAGE_RATIO	.4f		// 스플래시 데미지 관통률
#define SLASH_DAMAGE	3		// 강베기데미지 = 일반공격의 x SLASH_DAMAGE

			int damage = (int) pDesc->m_nDamage.Ref()* fDamageRange;

			// 인챈트 속성이 있을때는 1배 데미지만 먹는다. 2005.1.14
			if(zc_en_type == ZC_ENCHANT_NONE)
				damage *=  SLASH_DAMAGE;

			pTar->OnDamaged(pOwnerCharacter,pOwnerCharacter->GetPosition(),ZD_KATANA_SPLASH,MWT_KATANA,damage,SPLASH_DAMAGE_RATIO);
			pTar->OnDamagedAnimation(pOwnerCharacter,SEM_WomanSlash5);

			ZPostPeerEnchantDamage(pOwnerCharacter->GetUID(), pTar->GetUID());

/*
			if (fDist < fRange) 
			{
				if ((pTar) && (pTar != pOwnerCharacter)) 
				{
					if(g_pGame->CheckWall( pOwnerCharacter,pTar )==false) // 중간에 벽이 막고 있는가?
					{
						// 막고있으면 데미지를 안받는다
						if(pTar->IsGuard() && DotProduct(pTar->m_Direction,OwnerDir)<0 )
						{
							rvector addVel = pTar->GetPosition() - waveCenter;
							Normalize(addVel);
							addVel = 500.f*addVel;
							addVel.z = 200.f;
							pTar->AddVelocity(addVel);
						}else
						{
							rvector tpos = pTar->GetPosition();

							tpos.z += 130.f;
							if( zc_en_type == ZC_ENCHANT_NONE ) {
								ZGetEffectManager()->AddSwordUppercutDamageEffect(tpos,pTar->GetUID());
							}
							else {
								ZGetEffectManager()->AddSwordEnchantEffect(zc_en_type,pTar->GetPosition(),20);
							}
							tpos -= pOwnerCharacter->m_Direction * 50.f;
							rvector fTarDir = pTar->GetPosition() - pOwnerCharacter->GetPosition();
							Normalize(fTarDir);
#define MAX_DMG_RANGE	50.f	// 반경이만큼 까지는 최대 데미지를 다 먹는다
#define MIN_DMG			0.3f	// 최소 기본 데미지는 이정도.
							float fDamageRange = 1.f - (1.f-MIN_DMG)*( max(fDist-MAX_DMG_RANGE,0) / (fRange-MAX_DMG_RANGE));
//							pTar->OnDamagedKatanaSplash( pOwnerCharacter, fDamageRange );
#define SPLASH_DAMAGE_RATIO	.4f		// 스플래시 데미지 관통률
#define SLASH_DAMAGE	3		// 강베기데미지 = 일반공격의 x SLASH_DAMAGE
							int damage = (int) pDesc->m_nDamage * fDamageRange;
							// 인챈트 속성이 있을때는 1배 데미지만 먹는다. 2005.1.14
							if(zc_en_type == ZC_ENCHANT_NONE)
								damage *=  SLASH_DAMAGE;
							pTar->OnDamaged(pOwnerCharacter,pOwnerCharacter->GetPosition(),ZD_KATANA_SPLASH,MWT_KATANA,damage,SPLASH_DAMAGE_RATIO);
							pTar->OnDamagedAnimation(pOwnerCharacter,SEM_WomanSlash5);
							ZPostPeerEnchantDamage(pOwnerCharacter->GetUID(), pTar->GetUID());
						} // 데미지를 먹는다
					}
				}
			}*/
		}
#define KATANA_SHOCK_RANGE		1000.f			// 10미터까지 흔들린다

		float fPower= (KATANA_SHOCK_RANGE-Magnitude(ZGetGame()->m_pMyCharacter->GetPosition()+rvector(0,0,50) - OwnerPosition))/KATANA_SHOCK_RANGE;
		if(fPower>0)
			ZGetGameInterface()->GetCamera()->Shock(fPower*500.f, .5f, rvector(0.0f, 0.0f, -1.0f));

	}
	/*
	else{

#ifndef _PUBLISH

		// 이거 칼질 제대로 안한넘이다. 수상하다.
		char szTemp[256];
		sprintf(szTemp, "%s 치트 ?", pOwnerCharacter->GetProperty()->szName);
		ZChatOutput(MCOLOR(0xFFFF0000), szTemp);

		mlog("anistate %d\n",pOwnerCharacter->m_AniState_Lower);

#endif//_PUBLISH

	}
	*/
}

void ZGameAction::OnPeerSkill_Uppercut(ZCharacter *pOwnerCharacter)
{
	float fShotTime=ZGetGame()->GetTime();
	rvector OwnerPosition,OwnerDir;
	OwnerPosition = pOwnerCharacter->GetPosition();
	OwnerDir = pOwnerCharacter->m_Direction;
	OwnerDir.z=0; 
	Normalize(OwnerDir);


	if ( !pOwnerCharacter->IsNPC())
	{
		if ( pOwnerCharacter->GetProperty()->nSex == MMS_MALE)
			ZGetSoundEngine()->PlaySound( "fx2/MAL_shot_01", pOwnerCharacter->GetPosition());
		else
			ZGetSoundEngine()->PlaySound( "fx2/FEM_shot_01", pOwnerCharacter->GetPosition());
	}


	for (ZObjectManager::iterator itor = ZGetObjectManager()->begin();
		itor != ZGetObjectManager()->end(); ++itor)
	{
		ZObject* pTar = (*itor).second;
		if (pOwnerCharacter == pTar) continue;

		rvector TargetPosition,TargetDir;

		if(pTar->IsDie()) continue;
		// 적절한 위치를 얻어낼수 없으면 다음으로~
		if( !pTar->GetHistory(&TargetPosition,&TargetDir,fShotTime)) continue;

		float fDist = Magnitude(OwnerPosition + OwnerDir*10.f - TargetPosition);

		if (fDist < 200.0f) {

			if ((pTar) && (pTar != pOwnerCharacter))
			{
				bool bCheck = false;

				if (ZGetGame()->GetMatch()->IsTeamPlay())
				{
					if (IsPlayerObject(pTar)) {
						if( pOwnerCharacter->IsTeam( (ZCharacter*)pTar ) == false){
							bCheck = true;
						}
					}
					else {
						bCheck = true;
					}
				}
				else if (ZGetGame()->GetMatch()->IsQuestDrived())
				{
					if (!IsPlayerObject(pTar)) bCheck = true;
				}
				else {
					bCheck = true;
				}

				if(ZGetGame()->CheckWall(pOwnerCharacter,pTar)==true) //중간에 벽이 막고 있는가?
					bCheck = false;

				if( bCheck) {//팀이아닌경우만

					rvector fTarDir = pTar->GetPosition() - (pOwnerCharacter->GetPosition() - 50.f*OwnerDir);
					Normalize(fTarDir);
					float fDot = D3DXVec3Dot(&OwnerDir, &fTarDir);
					if (fDot>0)
					{
						int cm = ZGetGame()->SelectSlashEffectMotion(pOwnerCharacter);//남녀 칼 휘두르는 방향

						rvector tpos = pTar->GetPosition();

						tpos.z += 130.f;

						/*
						if (IsPlayerObject(pTar))
						{
							// 우선 플레이어만 이펙트가 나온다. - effect 다 정리하고 NPC도 나오게 바뀌어야 함 -bird
							ZGetEffectManager()->AddSwordUppercutDamageEffect(tpos,(ZCharacter*)pTar);
						}
						*/

						tpos -= pOwnerCharacter->m_Direction * 50.f;

						ZGetEffectManager()->AddBloodEffect( tpos , -fTarDir);
						ZGetEffectManager()->AddSlashEffect( tpos , -fTarDir , cm );

						ZGetGame()->CheckCombo(pOwnerCharacter, pTar , true);
						if (pTar == ZGetGame()->m_pMyCharacter) 
						{
							ZGetGame()->m_pMyCharacter->SetLastThrower(pOwnerCharacter->GetUID(), ZGetGame()->GetTime()+1.0f);
							ZPostReaction(ZGetGame()->GetTime(),ZR_BE_UPPERCUT);
						}
						pTar->OnBlast(OwnerDir);

						ZCharacter* pTarCharacter = (ZCharacter*)pTar;
						if ( !pTarCharacter->IsNPC())
						{
							if ( ((ZCharacter*)pTar)->GetProperty()->nSex == MMS_MALE)
								ZGetSoundEngine()->PlaySound( "fx2/MAL07", pTar->GetPosition());
							else
								ZGetSoundEngine()->PlaySound( "fx2/FEM07", pTar->GetPosition());
						}
					}
				}
			}
		}
	}
}

void ZGameAction::OnPeerSkill_Dash(ZCharacter *pOwnerCharacter)
{
	if(pOwnerCharacter->m_AniState_Lower.Ref()!=ZC_STATE_LOWER_UPPERCUT) return;

	float fShotTime=ZGetGame()->GetTime();
	rvector OwnerPosition,OwnerDir;
	OwnerPosition = pOwnerCharacter->GetPosition();
	OwnerDir = pOwnerCharacter->m_Direction;
	OwnerDir.z=0; 
	Normalize(OwnerDir);

	ZItem *pItem = pOwnerCharacter->GetItems()->GetItem(MMCIP_MELEE);
	if(!pItem) return;
	MMatchItemDesc *pDesc = pItem->GetDesc();
	if(!pDesc) { return; }

//	ZGetEffectManager()->AddSkillDashEffect(pOwnerCharacter->GetPosition(),pOwnerCharacter->m_Direction,pOwnerCharacter);

//	for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin();
//		itor != ZGetCharacterManager()->end(); ++itor)
	for (ZObjectManager::iterator itor = ZGetObjectManager()->begin();
		itor != ZGetObjectManager()->end(); ++itor)
	{
//		ZCharacter* pTar = (*itor).second;
		ZObject* pTar = (*itor).second;

		if (pOwnerCharacter == pTar) continue;

		rvector TargetPosition,TargetDir;

		if(pTar->IsDie()) continue;

		// 적절한 위치를 얻어낼수 없으면 다음으로~
		if( !pTar->GetHistory(&TargetPosition,&TargetDir,fShotTime)) continue;

		float fDist = Magnitude(OwnerPosition + OwnerDir*10.f - TargetPosition);

		if (fDist < 600.0f) {// 6m

			if ((pTar) && (pTar != pOwnerCharacter)) {

				bool bCheck = false;
/*
				if (ZGetGame()->GetMatch()->IsTeamPlay()){
					if( pOwnerCharacter->IsTeam( pTar ) == false){
						bCheck = true;
					}
				}
				else {
					bCheck = true;
				}
*/
				if (ZGetGame()->GetMatch()->IsTeamPlay()){
					if (IsPlayerObject(pTar)) {
						if( pOwnerCharacter->IsTeam( (ZCharacter*)pTar ) == false){
							bCheck = true;
						}
					}
					else {
						bCheck = true;
					}
				}
				else {
					bCheck = true;
				}

				if(ZGetGame()->CheckWall(pOwnerCharacter,pTar)==true) //중간에 벽이 막고 있는가?
					bCheck = false;

				if( bCheck) {//팀이아닌경우만
					//				if( pOwnerCharacter->IsTeam( pTar ) == false) {//팀이아닌경우만

					rvector fTarDir = pTar->GetPosition() - pOwnerCharacter->GetPosition();
					Normalize(fTarDir);

					float fDot = D3DXVec3Dot(&OwnerDir, &fTarDir);

					bool bDamage = false;

					if( fDist < 100.f) { // 1m 안쪽은 앞에만 있어도..
						if(fDot > 0.f) {
							bDamage = true;
						}
					}
					else if(fDist < 300.f) {
						if(fDot > 0.5f) {
							bDamage = true;
						}
					}
					else {// 2m ~ 6m
						if(fDot > 0.96f) {
							bDamage = true;
						}
					}

					if ( bDamage ) {

						int cm = ZGetGame()->SelectSlashEffectMotion(pOwnerCharacter);//남녀 칼 휘두르는 방향

						float add_time = 0.3f * (fDist / 600.f);
						float time = ZGetGame()->GetTime() + add_time;			// 거리에 따라서 시간을 달리하도록 수정하기..

						rvector tpos = pTar->GetPosition();

						tpos.z += 180.f;//좀더 높임

						ZGetEffectManager()->AddSwordUppercutDamageEffect(tpos,pTar->GetUID(),(DWORD)(add_time*1000));
//						ZGetEffectManager()->AddSwordUppercutDamageEffect(tpos,pTar);

						tpos -= pOwnerCharacter->m_Direction * 50.f;

//						ZGetEffectManager()->AddBloodEffect( tpos , -fTarDir);
//						ZGetEffectManager()->AddSlashEffect( tpos , -fTarDir , cm );
						// 소리도 특정 시간 뒤에
						ZGetSoundEngine()->PlaySound("uppercut", tpos );

						if (pTar == ZGetGame()->m_pMyCharacter) {
							rvector _dir = pTar->GetPosition() - pOwnerCharacter->GetPosition();
							_dir.z = 0.f;

//							m_pMyCharacter->OnDashAttacked(_dir);
							ZGetGame()->m_pMyCharacter->ReserveDashAttacked( pOwnerCharacter->GetUID(), time,_dir );
						}
						pTar->OnBlastDagger(OwnerDir,OwnerPosition);

						float fDamage = pDesc->m_nDamage.Ref() * 1.5f;// 기본 무기데미지 * 150 %
						float fRatio = pItem->GetPiercingRatio( pDesc->m_nWeaponType.Ref() , eq_parts_chest );

						if(ZGetGame()->CanAttack(pOwnerCharacter,pTar))//공격 가능한 경우만.. 퀘스트의 경우 데미지는 없다..
							pTar->OnDamagedSkill(pOwnerCharacter,pOwnerCharacter->GetPosition(),ZD_MELEE,MWT_DAGGER,fDamage,fRatio);

						ZGetGame()->CheckCombo(pOwnerCharacter, pTar,true);
					}

				}//IsTeam
			}
		}
	}
}


bool ZGameAction::OnEnchantDamage(MCommand* pCommand)
{
	MUID ownerUID;
	MUID targetUID;
	pCommand->GetParameter(&ownerUID,	0, MPT_UID);
	pCommand->GetParameter(&targetUID,	1, MPT_UID);

	ZCharacter* pOwnerCharacter = ZGetCharacterManager()->Find(ownerUID);
	ZObject* pTarget= ZGetObjectManager()->GetObject(targetUID);

	if (pOwnerCharacter == NULL || pTarget == NULL ) return true;

	MMatchItemDesc* pDesc = pOwnerCharacter->GetEnchantItemDesc();
	if(pDesc)
	{
		switch(pOwnerCharacter->GetEnchantType())
		{
			case ZC_ENCHANT_FIRE :
				ApplyFireEnchantDamage(pTarget, pOwnerCharacter, pDesc->m_nDamage.Ref(), pDesc->m_nDelay.Ref());
				break;
			case ZC_ENCHANT_COLD :
				ApplyColdEnchantDamage(pTarget, pDesc->m_nLimitSpeed.Ref(), pDesc->m_nDelay.Ref());
				break;
			case ZC_ENCHANT_POISON :
				ApplyPoisonEnchantDamage(pTarget, pOwnerCharacter, pDesc->m_nDamage.Ref(), pDesc->m_nDelay.Ref());
				break;
			case ZC_ENCHANT_LIGHTNING : 
				ApplyLightningEnchantDamage(pTarget, pOwnerCharacter, pDesc->m_nDamage.Ref(), pDesc->m_nDelay.Ref());
				break;
		};
	}

	return true;
}

bool ZGameAction::ApplyFireEnchantDamage(ZObject* pTarget, ZObject* pOwner, int nDamage, int nDuration)
{
	if (!pTarget) return false;

	ZModule_FireDamage *pMod = (ZModule_FireDamage*)pTarget->GetModule(ZMID_FIREDAMAGE);
	if(!pMod) return false;

	// 한번 걸리면 일정 시간 동안은 다시 걸리지 않게
	if (pMod->IsOnDamage() && (ZGetGame()->GetTime() - pMod->GetDamageBeginTime() < 1.f))
		return false;

	rvector soundPos = pTarget->GetPosition();
	bool bObserverTarget = pTarget->GetUID()==ZGetCombatInterface()->GetTargetUID();
	char* szSoundName = (bObserverTarget) ? "we_enfire_2d" : "we_enfire";
	ZGetSoundEngine()->PlaySound(szSoundName, soundPos);

	pMod->BeginDamage( pOwner, nDamage, nDuration * 0.001f);
	return true;
}

bool ZGameAction::ApplyColdEnchantDamage(ZObject* pTarget, int nLimitSpeed, int nDuration)
{
	if (!pTarget) return false;

	ZModule_ColdDamage *pMod = (ZModule_ColdDamage*)pTarget->GetModule(ZMID_COLDDAMAGE);
	if(!pMod) return false;

	// 한번 걸리면 일정 시간 동안은 다시 걸리지 않게
	if (pMod->IsOnDamage() && (ZGetGame()->GetTime() - pMod->GetDamageBeginTime() < 1.f))
		return false;

	rvector soundPos = pTarget->GetPosition();
	bool bObserverTarget = pTarget->GetUID()==ZGetCombatInterface()->GetTargetUID();
	char* szSoundName = (bObserverTarget) ? "we_enice_2d" : "we_enice";
	ZGetSoundEngine()->PlaySound(szSoundName, soundPos);

	if (nLimitSpeed < 0 || 100 < nLimitSpeed) return false;
	pMod->BeginDamage( (float)nLimitSpeed*0.01f, nDuration * 0.001f);
	return true;
}

bool ZGameAction::ApplyPoisonEnchantDamage(ZObject* pTarget, ZObject* pOwner, int nDamage, int nDuration)
{
	if (!pTarget) return false;

	ZModule_PoisonDamage *pMod = (ZModule_PoisonDamage*)pTarget->GetModule(ZMID_POISONDAMAGE);
	if(!pMod) return false;

	// 한번 걸리면 일정 시간 동안은 다시 걸리지 않게
	if (pMod->IsOnDamage() && (ZGetGame()->GetTime() - pMod->GetDamageBeginTime() < 1.f))
		return false;

	rvector soundPos = pTarget->GetPosition();
	bool bObserverTarget = pTarget->GetUID()==ZGetCombatInterface()->GetTargetUID();
	char* szSoundName = (bObserverTarget) ? "we_enpoison_2d" : "we_enpoison";
	ZGetSoundEngine()->PlaySound(szSoundName, soundPos);

	pMod->BeginDamage( pOwner, nDamage, nDuration * 0.001f);
	return true;
}

bool ZGameAction::ApplyLightningEnchantDamage(ZObject* pTarget, ZObject* pOwner, int nDamage, int nDuration)
{
	if (!pTarget) return false;

	ZModule_LightningDamage *pMod = (ZModule_LightningDamage*)pTarget->GetModule(ZMID_LIGHTNINGDAMAGE);
	if(!pMod) return false;

	// 한번 걸리면 일정 시간 동안은 다시 걸리지 않게
	if (pMod->IsOnDamage() && (ZGetGame()->GetTime() - pMod->GetDamageBeginTime() < 1.f))
		return false;

	rvector soundPos = pTarget->GetPosition();
	bool bObserverTarget = pTarget->GetUID()==ZGetCombatInterface()->GetTargetUID();
	char* szSoundName = (bObserverTarget) ? "we_enlight_2d" : "we_enlight";
	ZGetSoundEngine()->PlaySound(szSoundName, soundPos);

	pMod->BeginDamage( pOwner, nDamage, nDuration * 0.001f);
	return true;
}