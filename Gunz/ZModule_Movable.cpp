#include "stdafx.h"

#include "ZModule_Movable.h"
#include "ZObjectManager.h"
#include "ZGame.h"
#include "ZGameConst.h"

// 나 이외의 캐릭터는 위치를 양자화해서 보내므로 오차가 있어서 m_fRadius 를 실제보다 2정도 줄였다

ZModule_Movable::ZModule_Movable() 
			: m_Velocity(0,0,0), m_bForceCollRadius35(false) /*, m_fMaxSpeed(1000.f)*/ 
{
	// 원래 초기값 배정을 안했던 변수는 그냥 쓰레기값 상태로 crc를 생성
	m_fDistToFloor.Set_MakeCrc(0);
	m_FloorPlane.MakeCrc();

	m_bFalling.Set_MakeCrc(false);
	m_fFallHeight.MakeCrc();

	m_bLanding.Set_MakeCrc(false);
	m_bAdjusted.MakeCrc();
	m_fLastAdjustedTime.MakeCrc();

	m_bRestrict.Set_MakeCrc(false);
	m_fRestrictTime.Set_MakeCrc(0.f);
	m_fRestrictDuration.Set_MakeCrc(0.f);
	m_fRestrictRatio.Set_MakeCrc(1.f);

	m_bHaste.Set_MakeCrc(false);
	m_fHasteTime.Set_MakeCrc(0.f);
	m_fHasteDuration.Set_MakeCrc(0.f);
	m_fHasteRatio.Set_MakeCrc(1.f);

	m_nHasteItemId = 0;
	m_fNextHasteEffectTime = 0;
}


void ZModule_Movable::OnAdd()
{
	//_ASSERT(MIsDerivedFromClass(ZObject,m_pContainer));
}

void ZModule_Movable::InitStatus()
{
	m_Velocity=rvector(0,0,0);
	m_bRestrict.Set_CheckCrc(false);
	m_fRestrictRatio.Set_CheckCrc(1.f);
	m_bHaste.Set_CheckCrc(false);
	m_fHasteRatio.Set_CheckCrc(1.f);
}

bool ZModule_Movable::Update(float fElapsed)
{
	ZObject *pThisObj = MStaticCast(ZObject,m_pContainer);
	if(!pThisObj->GetInitialized()) return true;
	if(!pThisObj->IsVisible()) return true;

	float fCurrTime = ZGetGame()->GetTime();

	// 감속 가속의 종료 시간 체크
	if(m_bRestrict.Ref() && fCurrTime-m_fRestrictTime.Ref() > m_fRestrictDuration.Ref()) {
		m_bRestrict.Set_CheckCrc(false);
		m_fRestrictRatio.Set_CheckCrc(1.f);
	}
	if(m_bHaste.Ref() && fCurrTime-m_fHasteTime.Ref() > m_fHasteDuration.Ref()) {
		m_bHaste.Set_CheckCrc(false);
		m_fHasteRatio.Set_CheckCrc(1.f);
	}

	if(m_bHaste.Ref() && fCurrTime>m_fNextHasteEffectTime)
	{
		// 1초마다 이펙트를 새로 생성
		ZGetEffectManager()->AddHasteEffect( pThisObj->GetPosition(), pThisObj);
		m_fNextHasteEffectTime += 1.f;
	}

	UpdatePosition(fElapsed);

	return true;
}

// TODO : 충돌 범위 / 방법을 유동적으로 가져가자
//#define COLLISION_DIST	70.f


bool ZModule_Movable::Move(rvector &diff)
{
	ZObject *pThisObj = MStaticCast(ZObject,m_pContainer);
	float fThisObjRadius = pThisObj->GetCollRadius();

	ZObjectManager *pcm=&ZGetGame()->m_ObjectManager;
	for (ZObjectManager::iterator itor = pcm->begin(); itor != pcm->end(); ++itor)
	{
		ZObject* pObject = (*itor).second;
		if (pObject != pThisObj && pObject->IsCollideable())
		{
			rvector pos=pObject->GetPosition();
			rvector dir=pThisObj->GetPosition()+diff-pos;
			dir.z=0;
			float fDist=Magnitude(dir);

			float fCOLLISION_DIST = pObject->GetCollRadius() + pThisObj->GetCollRadius();

			if(fDist<fCOLLISION_DIST && fabs(pos.z-pThisObj->GetPosition().z) < pObject->GetCollHeight())	//height에 *0.5해야하는거 아닐까?
			{
				// 거의 같은위치에 있는 경우.. 한쪽방향으로 밀림
				if(fDist<1.f)
				{
					pos.x+=1.f;
					dir=pThisObj->GetPosition()-pos;
				}

				if(DotProduct(dir,diff)<0)	// 더 가까워지는 방향이면
				{
					Normalize(dir);
					rvector newthispos=pos+dir*(fCOLLISION_DIST+1.f);

					rvector newdiff=newthispos-pThisObj->GetPosition();
					diff.x=newdiff.x;
					diff.y=newdiff.y;

				}
			}
		}
	}

	rvector origin,targetpos;
	rplane impactplane;

	// 최소 120이상인 이유는 이동할 수 있는 곳의 각도가 플레이어와 같도록 하기 위함이고,
	// 1.7142857142857143f는 플레이어의 (radius / height)값
	//float fCollUpHeight = max(120.0f, pThisObj->GetCollHeight() - pThisObj->GetCollRadius() * 1.7142857142857143f);

	// ↑ 플레이어 height=180 radius=35인데 어떻게 radius/height=1.714가 되는가? height-radius*1.7이라는 공식도 미스테리..

	// 위처럼 계산하면 몹이 땅바닥에 박히는 등 충돌검사가 부정확한 경우가 생겨서 아래와 같이 해보았다.
	// 플레이어가 밟고 올라설수 있는 높이차가 120이라고 하고 몹의 height에 비례하여 그 높이차를 크게 적용하게 하였다.
	// CheckWall() 내부의 bsp/cylinder 충돌 검사법이 부정확한 관계로 이 방법 역시 확실한 근거에 의해 만들어진건 아니다..
	// (height에 0.5배한 것은, 또하나의 미스테리지만, 다른 GetCollHeight()사용처에서는 무조건 0.5배하여 쓰고 있다. 실제로, 0.5배를 안하면 실제크기보다 2배 크긴크다.)
	float fCollUpHeight = max(120.f, 0.666667f * pThisObj->GetCollHeight() * 0.5f); // 0.666667f == 120.f/180.f

	origin=pThisObj->GetPosition()+rvector(0,0,fCollUpHeight);
	targetpos=origin+diff;

	// 나락 이하는 맵 체크하지 않는다.
	if (pThisObj->GetPosition().z > DIE_CRITICAL_LINE)
	{
		if (m_bForceCollRadius35)
			m_bAdjusted.Set_CheckCrc( ZGetGame()->GetWorld()->GetBsp()->CheckWall(origin,targetpos,35,60,RCW_CYLINDER,0,&impactplane));
		else
			m_bAdjusted.Set_CheckCrc( ZGetGame()->GetWorld()->GetBsp()->CheckWall(origin,targetpos,fThisObjRadius,60,RCW_CYLINDER,0,&impactplane));

		//kimyhwan - 이 CheckWall은 bsp 충돌 부분에 결함이 있는 것 같다. "radius=35 height=60" 이 근처의 값이 아니면 잘못된 충돌판정이 난다(값이 커도 작아도 마찬가지)
		// 평소엔 판정이 잘 되지만 35, 60이 아니면 밟고올라설 수 있는 배경 물체 위에 올라가질 못하는 현상이 발생한다.
		// 원래 코드도 높이는 60으로 고정해두었는데 이 값을 어떻게 산출했는지 알수없고,
		// fThisObjRadius의 경우는 플레이어 캐릭터 반지름이 원래 35이므로 괜찮게 동작하지만 몬스터의 경우엔 둔턱을 밟고 올라서지 못하게 된다.
		// * http://www.melax.com/bsp.html <- 여기의 실린더/bsp 충돌방법을 사용한것 같다.. inaccuracy 문제에 대한 언급이 있긴함
	}
	else
	{
		SetVelocity(0.0f, 0.0f, GetVelocity().z);
		m_bAdjusted.Set_CheckCrc(false);
	}

	diff=targetpos-origin;
	pThisObj->SetPosition(targetpos-rvector(0,0,fCollUpHeight));

	if(m_bAdjusted.Ref())
		m_fLastAdjustedTime.Set_CheckCrc(ZGetGame()->GetTime());

	return m_bAdjusted.Ref();
}

void ZModule_Movable::UpdateGravity(float fDelta)
{
	m_Velocity.z = 
		max( m_Velocity.z - GRAVITY_CONSTANT*fDelta,-MAX_FALL_SPEED);
}

void ZModule_Movable::UpdatePosition(float fDelta)
{
	ZObject *pThisObj = MStaticCast(ZObject,m_pContainer);

	rvector diff=fDelta*m_Velocity;

	bool bUp = (diff.z>0.01f);
	bool bDownward= (diff.z<0.01f);

//	rvector diff2d=rvector(diff.x,diff.y,0);
	if(Magnitude(diff)>0.01f)
		Move(diff);

	m_FloorPlane.CheckCrc();
	rvector floor=ZGetGame()->GetFloor(pThisObj->GetPosition(), &m_FloorPlane.Ref(), pThisObj->GetUID());
	m_FloorPlane.MakeCrc();
	m_fDistToFloor.Set_CheckCrc(pThisObj->GetPosition().z-floor.z);

	// 올라가야 하는데 못올라간경우
	if(bUp && diff.z<=0.01f) {
		SetVelocity(GetVelocity().x,GetVelocity().y,0);
	}

	if(!m_bFalling.Ref() && diff.z<-0.1f && m_fDistToFloor.Ref()>35.f) {
		m_bFalling.Set_CheckCrc(true);
		m_fFallHeight.Set_CheckCrc(pThisObj->GetPosition().z);
	}

	if ((pThisObj->GetPosition().z > DIE_CRITICAL_LINE) && (m_bFalling.Ref() && GetDistToFloor()<1.f)) {
		m_bFalling.Set_CheckCrc(false);
		m_bLanding.Set_CheckCrc(true);
	}else
		m_bLanding.Set_CheckCrc(false);

	m_lastMove = diff;
}
/*todok del 같은 제한이 있으면 새로 갱신해야 하므로 변경된 버전을 제거
void ZModule_Movable::SetMoveSpeedRestrictRatio(float fRatio, float fDuration) 
{ 
	// 더 느리거나 같은 제한이 있으면 무시한다
	if(m_bRestrict.Ref() && fRatio <= m_fRestrictRatio.Ref()) 
		return;

	if(m_bRestrict.Ref() && m_fRestrictDuration.Ref() == fDuration)
		return;

	m_bRestrict.Set_CheckCrc(true);
	m_fRestrictTime.Set_CheckCrc( ZGetGame()->GetTime());
	m_fRestrictDuration.Set_CheckCrc(fDuration);
	m_fRestrictRatio.Set_CheckCrc(fRatio); 
}*/

void ZModule_Movable::SetMoveSpeedRestrictRatio(float fRatio, float fDuration) 
{ 
	// 더 느린 제한이 있으면 무시한다
	if(m_bRestrict.Ref() && fRatio>m_fRestrictRatio.Ref()) return;

	m_bRestrict.Set_CheckCrc(true);
	m_fRestrictTime.Set_CheckCrc( ZGetGame()->GetTime());
	m_fRestrictDuration.Set_CheckCrc(fDuration);
	m_fRestrictRatio.Set_CheckCrc(fRatio); 
}

void ZModule_Movable::SetMoveSpeedHasteRatio(float fRatio, float fDuration, int nItemId)
{ 
	// 더 큰 가속이 있으면 무시한다
	if(m_bHaste.Ref() && fRatio<m_fHasteRatio.Ref()) return;

	float fCurrTime = ZGetGame()->GetTime();

	m_bHaste.Set_CheckCrc(true);
	m_fHasteTime.Set_CheckCrc(fCurrTime);
	m_fHasteDuration.Set_CheckCrc(fDuration);
	m_fHasteRatio.Set_CheckCrc(fRatio); 

	m_nHasteItemId = nItemId;
	m_fNextHasteEffectTime = fCurrTime;
}

float ZModule_Movable::GetMoveSpeedRatio()
{
	// 감속과 가속 비율을 합으로 처리한다
	float fRestrict(0.f), fHaste(0.f);
	
	if (m_bRestrict.Ref())
		fRestrict = m_fRestrictRatio.Ref() - 1.f;
	if (m_bHaste.Ref())
		fHaste = m_fHasteRatio.Ref() - 1.f;

	return 1.f + fRestrict + fHaste;
}

bool ZModule_Movable::GetHasteBuffInfo(MTD_BuffInfo& out)
{
	if (!m_bHaste.Ref()) return false;
	out.nItemId = m_nHasteItemId;

	float fElapsed = ZGetGame()->GetTime() - m_fHasteTime.Ref();
	out.nRemainedTime = unsigned short(m_fHasteDuration.Ref() - fElapsed);
	return true;
}

void ZModule_Movable::ShiftFugitiveValues()
{
	m_fDistToFloor.ShiftHeapPos_CheckCrc();
	m_FloorPlane.ShiftHeapPos_CheckCrc();

	m_bFalling.ShiftHeapPos_CheckCrc();
	m_fFallHeight.ShiftHeapPos_CheckCrc();

	m_bLanding.ShiftHeapPos_CheckCrc();
	m_bAdjusted.ShiftHeapPos_CheckCrc();
	m_fLastAdjustedTime.ShiftHeapPos_CheckCrc();

	m_bRestrict.ShiftHeapPos_CheckCrc();
	m_fRestrictTime.ShiftHeapPos_CheckCrc();	
	m_fRestrictDuration.ShiftHeapPos_CheckCrc();
	m_fRestrictRatio.ShiftHeapPos_CheckCrc();

	m_bHaste.ShiftHeapPos_CheckCrc();
	m_fHasteTime.ShiftHeapPos_CheckCrc();	
	m_fHasteDuration.ShiftHeapPos_CheckCrc();
	m_fHasteRatio.ShiftHeapPos_CheckCrc();
}