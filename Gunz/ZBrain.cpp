#include "stdafx.h"
#include "ZBrain.h"
#include "ZActor.h"
#include "ZGame.h"
#include "ZObject.h"
#include "ZMyCharacter.h"
#include "ZRangeWeaponHitDice.h"
#include "ZModule_Skills.h"
#include "MMath.h"
#include "ZGame.h"
#include "RNavigationNode.h"



ZBrain::ZBrain() : m_pBody(NULL), m_uidTarget( MUID( 0, 0))
{
	ResetStuckInState();
	ResetStuckInStateForWarp();
}


ZBrain::~ZBrain()
{
}


ZBrain* ZBrain::CreateBrain( MQUEST_NPC nNPCType)
{
	return new ZBrain();
}


ZObject* ZBrain::GetTarget()
{
#ifdef _DEBUG
	// 혼자서 AI 테스트할 경우
	if ( (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_QUEST) || 
		 (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_AI))
	{
		return (ZObject*)ZGetGame()->m_pMyCharacter;
	}
#endif

	if ( ZGetObjectManager())
	{
		ZObject* pObject = ZGetObjectManager()->GetObject( m_uidTarget);
		return pObject;
	}

	return NULL;
}


float ZBrain::MakePathFindingUpdateTime( char nIntelligence)
{
	MQuestNPCGlobalAIValue* pGlobalAIValue = ZGetQuest()->GetNPCCatalogue()->GetGlobalAIValue();
	float fShakingRatio = pGlobalAIValue->m_fPathFinding_ShakingRatio;
	float fTime = pGlobalAIValue->m_fPathFindingUpdateTime[ nIntelligence - 1];
	float fExtraValue = fTime * fShakingRatio;
	float fMinTime = fTime - fExtraValue;
	if ( fMinTime < 0.0f)
		fMinTime = 0.0f;
	float fMaxTime = fTime + fExtraValue;

	return RandomNumber( fMinTime, fMaxTime);
}

float ZBrain::MakeAttackUpdateTime( char nAgility)
{
	MQuestNPCGlobalAIValue* pGlobalAIValue = ZGetQuest()->GetNPCCatalogue()->GetGlobalAIValue();
	float fShakingRatio = pGlobalAIValue->m_fAttack_ShakingRatio;
	float fTime = pGlobalAIValue->m_fAttackUpdateTime[ nAgility - 1];
	float fExtraValue = fTime * fShakingRatio;
	float fMinTime = fTime - fExtraValue;
	if ( fMinTime < 0.0f)
		fMinTime = 0.0f;
	float fMaxTime = fTime + fExtraValue;

	return RandomNumber( fMinTime, fMaxTime);
}


float ZBrain::MakeDefaultAttackCoolTime()
{
	if ( !m_pBody->GetNPCInfo())
		return 0.0f;

	float fShakingRatio = 0.3f;
	float fCoolTime = m_pBody->GetNPCInfo()->fAttackCoolTime;
	float fExtraValue = fCoolTime * fShakingRatio;
	float fMinCoolTime = max( (fCoolTime - fExtraValue), 0.01f);
	float fMaxCoolTime = fCoolTime + fExtraValue;

	return RandomNumber( fMinCoolTime, fMaxCoolTime);

}


float ZBrain::MakeSpeed( float fSpeed)
{
	MQuestNPCGlobalAIValue* pGlobalAIValue = ZGetQuest()->GetNPCCatalogue()->GetGlobalAIValue();
	float fShakingRatio = pGlobalAIValue->m_fSpeed_ShakingRatio;
	float fExtraValue = fSpeed * fShakingRatio;
	float fMinSpeed = max( (fSpeed - fExtraValue), 0.0f);
	float fMaxSpeed = fSpeed + fExtraValue;

	return RandomNumber( fMinSpeed, fMaxSpeed);
}


void ZBrain::MakeNeglectUpdateTime()
{
	m_dwNeglectTimer = timeGetTime() + 5500;
}


void ZBrain::Init( ZActor* pBody)
{
	m_pBody = pBody;
	m_Behavior.Init( this);

	if ( m_pBody->GetNPCInfo())
	{
		float fDefaultPathFindingUpdateTime		= ZBrain::MakePathFindingUpdateTime( m_pBody->GetNPCInfo()->nIntelligence);
		float fAttackUpdateTime					= ZBrain::MakeAttackUpdateTime( m_pBody->GetNPCInfo()->nAgility);
		float fDefaultAttackUpdateTime			= m_pBody->GetNPCInfo()->fAttackCoolTime;

		m_PathFindingTimer.Init( fDefaultPathFindingUpdateTime);
		m_AttackTimer.Init( fAttackUpdateTime);
		m_DefaultAttackTimer.Init( fDefaultAttackUpdateTime);
	}

	// 첨 스폰되고 나서 이 시간동안은 스킬을 사용하지 않는다.
	m_dwNoSkillTimer = timeGetTime() + RandomNumber( 1000, 5000);

	// Neglect timer
	MakeNeglectUpdateTime();

	// Set distance
	m_fDistForcedIn		= DIST_FORCEDIN + RandomNumber( 1.0f, (DIST_FORCEDIN * 0.6f)) - (DIST_FORCEDIN * 0.6f / 2.0f);
	m_fDistIn			= DIST_IN       + RandomNumber( 1.0f, (DIST_IN       * 0.6f)) - (DIST_IN       * 0.6f / 2.0f);
	m_fDistOut			= DIST_OUT      + RandomNumber( 1.0f, (DIST_OUT      * 0.6f)) - (DIST_OUT      * 0.6f / 2.0f);
}


void ZBrain::Think( float fDelta)
{
	if ( m_pBody->isThinkAble())
	{
		MUID prevTarget = m_uidTarget;


		// 타겟 찾기
		bool bFind = FindTarget();


		// 타겟이 있으면...
		if ( bFind)
		{
			// 길찾기
			ProcessBuildPath( fDelta);


			// 공격
			ProcessAttack( fDelta);
		}

		// 타겟이 없으면...
		else if ( prevTarget != MUID( 0, 0))
		{
			m_pBody->Stop();
			m_pBody->m_TaskManager.Clear();

			MakeNeglectUpdateTime();
	
			m_pBody->OnNeglect( 1);
		}
	}


	// Check neglect
	DWORD dwCurrTime = timeGetTime();
	if ( !m_pBody->m_TaskManager.IsEmpty())
		MakeNeglectUpdateTime();

	else if ( dwCurrTime > m_dwNeglectTimer)
	{
		m_pBody->OnNeglect( 1);

		MakeNeglectUpdateTime();
	}
}


bool ZBrain::FindTarget()
{
	MUID uidTarget	= MUID(0,0);
	float fDist		= FLT_MAX;

	for ( ZCharacterManager::iterator itor = ZGetCharacterManager()->begin();  itor != ZGetCharacterManager()->end();  ++itor)
	{
		// 죽은 놈은 관심없다.
		ZCharacter* pCharacter = (*itor).second;
		if ( pCharacter->IsDie())
			continue;

		// 제외 목록에 들어간 캐릭터는 건너뜀
		if ( ZGetGame()->IsExceptedFromNpcTargetting( pCharacter))
			continue;

		// 거리를 구한다.
		float dist = MagnitudeSq( pCharacter->GetPosition() - m_pBody->GetPosition());


		// 더 가까운 놈이면 이놈을 타겟으로 정한다.
		if ( dist < fDist)
		{
			fDist = dist;
			uidTarget = pCharacter->GetUID();
		}
	}

	m_uidTarget = uidTarget;

	if ( uidTarget == MUID(0,0))
		return false;

	return true;
}


void ZBrain::ProcessAttack( float fDelta)
{
	bool bDefaultAttackEnabled = true;

	// Update time
	if ( m_pBody->GetNPCInfo() && (m_pBody->GetNPCInfo()->fAttackCoolTime != 0.0f))
		bDefaultAttackEnabled = m_DefaultAttackTimer.Update( fDelta);

	if ( !m_AttackTimer.Update(fDelta) && !bDefaultAttackEnabled)
		return;

	// Skip if friendly NPC
	if ( m_Behavior.IsFriendly())
		return;

	// Check attackable status
	if ( !m_pBody->IsAttackable())
		return;

	// Use default attack
	if ( bDefaultAttackEnabled && m_pBody->CanAttackMelee( GetTarget()) && !ZGetGame()->CheckWall(m_pBody, GetTarget(), true))
	{	// (실제 근접타격 판정할때 벽체크하는 함수) CheckWall로 타겟과 나 사이에 장애물이 없는지 확인- 안그러면 기둥 뒤에서 계속 헛방친다

		float fNextCoolTime = MakeDefaultAttackCoolTime();
		m_DefaultAttackTimer.Init( fNextCoolTime);

		ZTask* pNew = ZTaskManager::CreateAttackMelee( m_pBody);
		m_pBody->m_TaskManager.PushFront( pNew);

		return;
	}


	// Check skill useable
	if ( (m_pBody->GetNPCInfo()->nNPCAttackTypes & NPC_ATTACK_MAGIC) == NPC_ATTACK_NONE)
		return;

	ZTASK_ID nTaskID = m_pBody->m_TaskManager.GetCurrTaskID();
	if ( (nTaskID == ZTID_SKILL) || (nTaskID == ZTID_ROTATE_TO_DIR))
		return;

	if ( timeGetTime() < m_dwNoSkillTimer)
		return;


	// Get skill
	int nSkill;
	MUID uidTarget;
	rvector targetPosition;
	if ( GetUseableSkill( &nSkill, &uidTarget, &targetPosition)) 
	{
		// Use skill
		if ( m_pBody->CanSee( GetTarget()))
		{
			m_pBody->m_TaskManager.Clear();

			ZTask* pNew = ZTaskManager::CreateSkill( m_pBody, nSkill, uidTarget, targetPosition);
			m_pBody->m_TaskManager.Push( pNew);
		}
	}
}


bool ZBrain::GetUseableSkill( int *pnSkill, MUID *puidTarget, rvector *pTargetPosition)
{
	// Get skill module
	ZModule_Skills *pmod = (ZModule_Skills *)m_pBody->GetModule(ZMID_SKILLS);
	if ( !pmod)
		return false;

	// Set value
	if ( puidTarget)
		(*puidTarget) = MUID(0,0);

	if (pTargetPosition)
		(*pTargetPosition) = rvector(0.0f,0.0f,0.0f);


	// Check skills
	for ( int i = 0;  i < pmod->GetSkillCount();  i++)
	{
		ZSkill *pSkill = pmod->GetSkill( i);

		// Check cool time
		if ( !pSkill->IsReady())
			continue;

		// Get skill description
		ZSkillDesc *pDesc = pmod->GetSkill( i)->GetDesc();


		// 스킬의 적용 대상이 아군인 경우...
		if ( pDesc->IsAlliedTarget())
		{
			// 효과가 있는 대상중 가까이 있는 걸 찾는다.
			float fDist = DIST_OUT;
			ZObject *pAlliedTarget = NULL;


			for ( ZObjectManager::iterator itor = ZGetObjectManager()->begin();  itor != ZGetObjectManager()->end();  ++itor)
			{
				ZObject *pObject = itor->second;

				// 죽은 놈은 넘어간다
				if ( pObject->IsDie())
					continue;

				// 적이면 넘어간다
				if ( ZGetGame()->CanAttack(m_pBody,pObject))
					continue;

				// 자기 자신이면 넘어간다
				if ( pObject == m_pBody)
					continue;


				// Get distance
				float dist = MagnitudeSq( pObject->GetPosition() - m_pBody->GetPosition());
				if ( pSkill->IsUsable( pObject) && ( dist < fDist))
				{
					fDist = dist;
					pAlliedTarget = pObject;
				}
			}	

			// 만약 대상이 없으면 자기 자신한테라도 스킬을 건다.
			if ( ( pAlliedTarget == NULL) && ( pSkill->IsUsable( m_pBody)))
				pAlliedTarget = m_pBody;

			if (pAlliedTarget)
			{
				if ( pnSkill)
					*pnSkill = i;
				
				if ( puidTarget)
					*puidTarget = pAlliedTarget->GetUID();

				if ( pTargetPosition)
					*pTargetPosition = pAlliedTarget->GetCenterPos();

				return true;
			}
		}

		// 스킬의 적용 대상이 적군인 경우...
		else
		{
			ZObject* pTarget = GetTarget();
			if ( pTarget == NULL)
				continue;

			// Check useable
			if ( !pSkill->IsUsable( pTarget))
				continue;

			// Get pick info
			ZPICKINFO pickinfo;
			memset( &pickinfo, 0, sizeof( ZPICKINFO));


			// Check picking
			rvector pos, tarpos, dir;
			// 적과 나의 몸통 실린더에서 가슴 정도의 높이 지점끼리 피킹 쏴본다..
			pos = m_pBody->GetPosition() + rvector( 0, 0, m_pBody->GetCollHeight()*0.5f*0.8f);		// 가슴께로 낮춰주려고 *0.8
			tarpos = pTarget->GetPosition() + rvector( 0, 0, pTarget->GetCollHeight()*0.5f*0.8f);
			dir = tarpos - pos;
			Normalize( dir);

			const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;
			if ( ZGetGame()->Pick( m_pBody, pos, dir, &pickinfo, dwPickPassFlag))
			{
				if ( pickinfo.pObject)
				{
					if ( pnSkill)
						*pnSkill = i;

					if ( puidTarget)
						*puidTarget = pTarget->GetUID();

					if ( pTargetPosition)
						*pTargetPosition = pTarget->GetCenterPos();

					return true;
				}
			}
		}
	}

	return false;
}


void ZBrain::ProcessBuildPath( float fDelta)
{
	// Update timer
	if ( !m_PathFindingTimer.Update( fDelta))
		return;
	
	// Check status
	ZTASK_ID nTaskID = m_pBody->m_TaskManager.GetCurrTaskID();
	if ( (nTaskID == ZTID_ATTACK_MELEE) || (nTaskID == ZTID_ATTACK_RANGE) || (nTaskID == ZTID_ROTATE_TO_DIR) || (nTaskID == ZTID_SKILL))
		return;

	// 맵에 끼었다면 벗어난다
	// 워프해서 끼임을 탈출하는 건 서바이벌일때만으로 제한한다 (워프는 눈에 크게 띄고 조악한 해결법이다. 기존 퀘스트에서 없던 몹워프가 일어나 유저 불만이 있음)
	if (ZGetGameTypeManager()->IsSurvivalOnly( ZGetGame()->GetMatch()->GetMatchType()))
	{
		if (EscapeFromStuckIn(m_WayPointList))
			return;
	}

	// Get target
	ZObject* pTarget = GetTarget();
	if ( !pTarget)
	{
		m_pBody->m_TaskManager.Clear();
		m_pBody->Stop();

		return;
	}


	// 원거리 공격이거나 우호적이면 넘 가까이 다가가지 않고 바라만 본다.
	if ( ( m_Behavior.GetOffenseType() == ZOFFENSETYPE_RANGE) || m_Behavior.IsFriendly())
	{
		// 거리를 구한다.
		float dist = MagnitudeSq( pTarget->GetPosition() - m_pBody->GetPosition());

		bool bStop = false;

		// Friendly type
		if ( m_Behavior.IsFriendly())
		{
			if ( dist < m_fDistForcedIn)
				bStop = true;
		}
		// Else type
		else
		{
			// 직선 거리를 본다.
			if ( ( dist > DIST_FORCEDIN) && (dist < m_fDistIn))
			{
				// 직선 거리는 가까운데 높이가 많이 차이가 나는지 본다.
				dist = pTarget->GetPosition().z - m_pBody->GetPosition().z;

				// 높이가 넘 많이 차이 안나면 정지
				if ( (dist > -DIST_HEIGHT) && (dist < DIST_HEIGHT))
					bStop = true;
			}
		}

		// Stop
		if ( bStop)
		{
			// 볼 수 있는 위치여야지 정지가 가능하다. 만약 안보인다면 뛰어가서 근접공격을 하도록 한다
			if ( m_pBody->CanSee( pTarget) && m_pBody->CanAttackRange( pTarget))
			{
				m_pBody->Stop();
				m_pBody->m_TaskManager.Clear();

				return;
			}
		}
	}


	// Make path
	RNavigationMesh* pNavMesh = ZGetGame()->GetWorld()->GetBsp()->GetNavigationMesh();
	if ( pNavMesh == NULL)
		return;

	// Make navigation path
	rvector tarpos = pTarget->GetPosition();
	if ( !pNavMesh->BuildNavigationPath( m_pBody->GetPosition(), tarpos))
		return;

	m_WayPointList.clear();
	for ( list<rvector>::iterator itor = pNavMesh->GetWaypointList().begin();  itor != pNavMesh->GetWaypointList().end();  ++itor)
		m_WayPointList.push_back( (*itor));


	AdjustWayPointWithBound(m_WayPointList, pNavMesh);

	PushWayPointsToTask();
}

void ZBrain::PushWayPointsToTask()
{
	// Push task
	if ( m_WayPointList.empty())
		return;

	m_pBody->m_TaskManager.Clear();
	int nTotal	= (int)m_WayPointList.size();
	int cnt		= 0;
	for ( list<rvector>::iterator itor = m_WayPointList.begin();  itor != m_WayPointList.end();  ++itor)
	{
		bool bChained = !( (nTotal-1) == cnt);

		ZTask* pNew = ZTaskManager::CreateMoveToPos( m_pBody, (*itor), bChained);
		m_pBody->m_TaskManager.Push( pNew);

		++cnt;
	}
}

void ZBrain::AdjustWayPointWithBound(list<rvector>& wayPointList, RNavigationMesh* pNavMesh)
{
	// 몹이 코너의 벽모서리 앞에서, 벽에 밀착한 플레이어를 추적하면 waypoint 1개로 길찾기된다, 그러나 코너 앞이므로
	// 바운드볼륨때문에 벽모서리에 걸려서 제자리 걸음을 하게 된다. 이를 방지하기 위해, 자기 바운드 좌우측 지점으로부터
	// 목표물에 길찾기를 해보고 waypoint 1개 이상으로 나오면(즉 벽이 가로막았으면) 사이드 스텝을 먼저 해서 벽코너로부터 몸을 떨어뜨린다
	// * waypoint가 여러개 나왔을 때는 첫번째 waypoint를 가지고 테스트한다

	if (wayPointList.empty()) return;

	const rvector& targetpos = *(wayPointList.begin());

	// 내 중점~첫목표점을 이용해 내가 가고자하는 방향으로부터 바운딩실린더의 좌우측의 점을 얻어내보자
	rvector center = m_pBody->GetPosition();
	rvector dir = targetpos - center;
	dir.z = 0;
	Normalize(dir);

	dir *= m_pBody->GetCollRadius() * 0.8f;	// 자기 충돌 반지름보다 약간 작게 *0.8

	rvector side1 = center + rvector(-dir.y,  dir.x, 0);
	rvector side2 = center + rvector( dir.y, -dir.x, 0);

	// 좌우측 점에서부터 첫지점을 향해 길찾기 해본다. 2개이상이 나오면 몸통 일부가 기둥에 막혀 못움직이므로
	// 그냥 반대쪽으로 한발짝 먼저 움직여주자
	if ( pNavMesh->BuildNavigationPath( side1, targetpos) && pNavMesh->GetWaypointList().size() > 1)
		m_WayPointList.push_front(side2);
	else if ( pNavMesh->BuildNavigationPath( side2, targetpos) && pNavMesh->GetWaypointList().size() > 1)
		m_WayPointList.push_front(side1);
}

bool ZBrain::EscapeFromStuckIn(list<rvector>& wayPointList)
{
	// 길찾기 코드의 허점이 드러나는 맵 지점들이 있다.. 그런 곳에서는 몹이 이동하지를 못한다
	// 그런곳을 탈출하기 위해 땜빵을 한다. true 리턴하면 웨이포인트를 여기서 지정했다는 의미.
	DWORD currTime = timeGetTime();
	// 오랜시간 같은 곳에 멈춰있다면 아예 워프해버린다
	if (currTime - m_dwExPositionTimeForWarp > 2000)
	{
		rvector diff = m_exPositionForWarp - m_pBody->GetPosition();

		ResetStuckInStateForWarp();

		if (MagnitudeSq(diff) < 100)
		{
			OutputDebugString("NPC NEED WARP....\n");
			RNavigationMesh* pNavMesh = ZGetGame()->GetWorld()->GetBsp()->GetNavigationMesh();
			if (pNavMesh) {
				// 근방의 랜덤지점을 정한다

				// 랜덤 방향 얻기
				float angle = (rand() % (314*2)) * 0.01f;
				D3DXMATRIX matRot;
				D3DXMatrixRotationZ(&matRot, angle);

				rvector dir(200, 0, 0);	// 이동할 거리
				dir = dir * matRot;
				rvector newpos = m_pBody->GetPosition() + dir;

				// 가장 가까운 네비게이션노드의 센터로 옮긴다 (네비게이션노드가 크게 잡혀 있는 맵에선 워프가 심하게 눈에 띌수 있음..)
				RNavigationNode* pNavNode = pNavMesh->FindClosestNode(newpos);
				if (pNavNode) {
					m_pBody->SetPosition( pNavNode->CenterVertex());
					OutputDebugString("NPC WARP DONE!\n");
					return false;
				}
			}
		}
	}

	// 짧은시간 같은 곳에 멈춰있다면 앞으로 한발짝 정도 움직여서 탈출시도
	if (currTime - m_dwExPositionTime > 1000)
	{
		rvector diff = m_exPosition - m_pBody->GetPosition();

		ResetStuckInState();

		if (MagnitudeSq(diff) < 100)
		{
			wayPointList.clear();

			// 기본적으로 앞쪽으로 방향을 잡되 좌우로 랜덤하게 방향을 준다
			rvector dir = m_pBody->GetDirection();
			rmatrix matRot;
			D3DXMatrixRotationZ(&matRot, (rand()%314 - 157) * 0.01f);	// 3.14 즉 반바퀴 범위 내에서 방향을 틀게 함
			Normalize(dir);

			dir *= m_pBody->GetCollRadius() * 0.8f;
			wayPointList.push_back(m_pBody->GetPosition() + dir);

			PushWayPointsToTask();

			return true;
		}
	}

	return false;
}

void ZBrain::ResetStuckInState()
{
	rvector pos;
	if (m_pBody) pos = m_pBody->GetPosition();
	else	pos.x = pos.y = pos.z = 0;

	DWORD currTime = timeGetTime();

	m_dwExPositionTime = currTime;
	m_exPosition = pos;
}

void ZBrain::ResetStuckInStateForWarp()
{
	rvector pos;
	if (m_pBody) pos = m_pBody->GetPosition();
	else	pos.x = pos.y = pos.z = 0;

	DWORD currTime = timeGetTime();

	m_dwExPositionTimeForWarp = currTime;
	m_exPositionForWarp = pos;
}


void ZBrain::OnDamaged()
{
	// 우호적 성격일 경우에 공격 당하면 공격 성향으로 바뀐다.
	if ( m_Behavior.IsFriendly())
	{
		// Stop
		m_pBody->Stop();
		m_pBody->m_TaskManager.Clear();

		// Change friendly type
		m_Behavior.SetFriendly( false);
	}
}


void ZBrain::OnBody_AnimEnter(ZA_ANIM_STATE nAnimState)
{
}


void ZBrain::OnBody_AnimExit(ZA_ANIM_STATE nAnimState)
{
}


void ZBrain::OnBody_CollisionWall()
{
}


void ZBrain::OnBody_OnTaskFinished( ZTASK_ID nLastID)
{
	// 길찾기한 길을 다 갔을때는 새로운 길찾기를 바로 하도록 세팅한다.
	if ( (nLastID == ZTID_MOVE_TO_POS) || (nLastID == ZTID_MOVE_TO_DIR) || (nLastID == ZTID_MOVE_TO_TARGET))
	{
		if ( GetTarget())
			m_PathFindingTimer.Force();
	}

	// 이동 외의 동작이 일어났으면 얼마간 위치가 변경되지 않았을테니 당분간은 맵에 끼였는지 검사하지 않도록 한다.
	if ( nLastID==ZTID_ATTACK_MELEE || nLastID==ZTID_ATTACK_RANGE || nLastID==ZTID_DELAY || nLastID==ZTID_SKILL)
	{
		ResetStuckInState();
		ResetStuckInStateForWarp();
	}
}
