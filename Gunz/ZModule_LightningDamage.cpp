#include "stdafx.h"
#include "ZModule_LightningDamage.h"
#include "ZGame.h"
#include "ZApplication.h"
#include "ZModule_HPAP.h"


#define DAMAGE_DELAY	1.f			// 데미지 주는 간격

ZModule_LightningDamage::ZModule_LightningDamage()
{
	m_bOnDamage = false;
}

void ZModule_LightningDamage::InitStatus()
{
	m_bOnDamage = false;
	Active(false);
}

bool ZModule_LightningDamage::Update(float fElapsed)
{
	ZObject* pObj = MDynamicCast(ZObject, m_pContainer);
	if (!pObj)
		return false;
	else
	{
		ZModule_Movable *pMovable = (ZModule_Movable*)m_pContainer->GetModule(ZMID_MOVABLE);
		if(pMovable)
			pMovable->SetVelocity(0,0,0);

		// 데미지 간격 DAMAGE_DELAY
		if(ZGetGame()->GetTime()>m_fNextDamageTime) {
			m_fNextDamageTime+=DAMAGE_DELAY;

			// 데미지 받고 있는 이펙트 죽었더라도 완전히 투명해지지 않은 상태에서는 보인다..
			if(pObj->IsDie()) {

				if( pObj->m_pVMesh->GetVisibility() < 0.5f ) {//이펙트의 Life 타임도 있으니까...
					m_bOnDamage = false;
					return false;
				}

			}
			else //살아있을떄만..
			{
				// 라이트닝 레지스트를 여기서 적용하려고 했으나 제대로 적용이 안돼 일단 주석처리한다
				// 6 * (1.f-fFR) 이부분은 zitem.xml에서 damage="1"인부분을 damage="7"로 일단 변경
				//float fFR = 0;
				//float fDamage = 6 * (1.f-fFR) + (float)m_nDamage;
				
				pObj->OnDamaged(m_pOwner, pObj->GetPosition(), ZD_COLD, MWT_NONE, m_fDamage, 0);
/*
				ZModule_HPAP *pModule = (ZModule_HPAP*)m_pContainer->GetModule(ZMID_HPAP);
				if(pModule) {
					pModule->OnDamage(m_pOwner->GetUID(),m_fDamage,0);
//					pObj->OnScream();

				}
				*/
			}
		}
	}

	if(m_fNextDamageTime-m_fBeginTime>m_fDuration) {
		m_bOnDamage = false;
		return false;
	}
	return true;
}

// 데미지를 주기 시작한다
void ZModule_LightningDamage::BeginDamage(ZObject* pOwner, int nDamage, float fDuration)
{
	m_fBeginTime = ZGetGame()->GetTime();

	if (!m_bOnDamage)
	{
		m_fNextDamageTime = m_fBeginTime+DAMAGE_DELAY;
	}

	m_bOnDamage = true;

	m_pOwner = pOwner;
	m_fDamage = nDamage;
	m_fDuration = fDuration;

	Active();
}
