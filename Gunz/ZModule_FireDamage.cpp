#include "stdafx.h"
#include "ZModule_FireDamage.h"
#include "ZGame.h"
#include "ZApplication.h"
#include "ZModule_HPAP.h"

#define DAMAGE_DELAY	0.1f	//1.f	// 데미지 주는 간격
#define EFFECT_DELAY	0.15f			// 이펙트 간격

int GetEffectLevel();

ZModule_FireDamage::ZModule_FireDamage()
{
	m_bOnDamage = false;
	m_pOwner = NULL;
}

void ZModule_FireDamage::InitStatus()
{
	m_bOnDamage = false;
	Active(false);
}

bool ZModule_FireDamage::Update(float fElapsed)
{
	ZObject* pObj = MDynamicCast(ZObject, m_pContainer);
	if (!pObj)
	{
	}
	else
	{
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
				// 파이어 레지스트를 여기서 적용하려고 했으나 제대로 적용이 안돼 일단 주석처리한다
				// 6 * (1.f-fFR) 이부분때문에 zitem.xml에서 damage="1"인부분을 damage="7"로 일단 변경
				//float fFR = 0;
				//float fDamage = 6 * (1.f-fFR) + (float)m_nDamage;

				// HPAP 모듈을 직접 조작하지 말고 컨테이너의 OnDamaged를 사용하도록 한다
				pObj->OnDamaged(m_pOwner, pObj->GetPosition(), ZD_FIRE, MWT_NONE, m_fDamage, 0);
				/*ZModule_HPAP *pModule = (ZModule_HPAP*)m_pContainer->GetModule(ZMID_HPAP);
				if(pModule) {
					pModule->OnDamage(m_pOwner,m_fDamage,0);
					//pObj->OnScream();
				}*/
			}
		}

		if(ZGetGame()->GetTime()>m_fNextEffectTime) {

			if(!pObj->IsDie())
			{
				int nEffectLevel = GetEffectLevel()+1;

				m_fNextEffectTime+=EFFECT_DELAY * nEffectLevel;

				ZGetEffectManager()->AddEnchantFire2( pObj );
				ZGetEffectManager()->AddEnchantFire2( pObj );
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
void ZModule_FireDamage::BeginDamage(ZObject* pOwner, int nDamage, float fDuration)
{
	m_fBeginTime = ZGetGame()->GetTime();

	m_pOwner = pOwner;
	m_fDamage = (float)nDamage*DAMAGE_DELAY;	// DAMAGE_DELAY 당 데미지가 들어갈꺼임
	m_fDuration = fDuration+0.01f;		// 0.1초 단위로 계산을 했을때 단위가 작어져 마지막 데미지 적용이 안돼는 경우가 있어 게임과 상관없는 0.01f를 더해준다.

	if (!m_bOnDamage)
	{
		m_fNextDamageTime = m_fBeginTime+DAMAGE_DELAY;
		m_fNextEffectTime = m_fBeginTime;
	}

	m_bOnDamage = true;

	Active();
}