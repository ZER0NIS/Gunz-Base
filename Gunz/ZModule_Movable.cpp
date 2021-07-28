#include "stdafx.h"

#include "ZModule_Movable.h"
#include "ZObjectManager.h"
#include "ZGame.h"
#include "ZGameConst.h"

// �� �̿��� ĳ���ʹ� ��ġ�� ����ȭ�ؼ� �����Ƿ� ������ �־ m_fRadius �� �������� 2���� �ٿ���

ZModule_Movable::ZModule_Movable()
	: m_Velocity(0, 0, 0), m_bForceCollRadius35(false) /*, m_fMaxSpeed(1000.f)*/
{
	// ���� �ʱⰪ ������ ���ߴ� ������ �׳� �����Ⱚ ���·� crc�� ����
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
	m_Velocity = rvector(0, 0, 0);
	m_bRestrict.Set_CheckCrc(false);
	m_fRestrictRatio.Set_CheckCrc(1.f);
	m_bHaste.Set_CheckCrc(false);
	m_fHasteRatio.Set_CheckCrc(1.f);
}

bool ZModule_Movable::Update(float fElapsed)
{
	ZObject* pThisObj = MStaticCast(ZObject, m_pContainer);
	if (!pThisObj->GetInitialized()) return true;
	if (!pThisObj->IsVisible()) return true;

	float fCurrTime = ZGetGame()->GetTime();

	// ���� ������ ���� �ð� üũ
	if (m_bRestrict.Ref() && fCurrTime - m_fRestrictTime.Ref() > m_fRestrictDuration.Ref()) {
		m_bRestrict.Set_CheckCrc(false);
		m_fRestrictRatio.Set_CheckCrc(1.f);
	}
	if (m_bHaste.Ref() && fCurrTime - m_fHasteTime.Ref() > m_fHasteDuration.Ref()) {
		m_bHaste.Set_CheckCrc(false);
		m_fHasteRatio.Set_CheckCrc(1.f);
	}

	if (m_bHaste.Ref() && fCurrTime > m_fNextHasteEffectTime)
	{
		// 1�ʸ��� ����Ʈ�� ���� ����
		ZGetEffectManager()->AddHasteEffect(pThisObj->GetPosition(), pThisObj);
		m_fNextHasteEffectTime += 1.f;
	}

	UpdatePosition(fElapsed);

	return true;
}

// TODO : �浹 ���� / ����� ���������� ��������
//#define COLLISION_DIST	70.f

bool ZModule_Movable::Move(rvector& diff)
{
	ZObject* pThisObj = MStaticCast(ZObject, m_pContainer);
	float fThisObjRadius = pThisObj->GetCollRadius();

	ZObjectManager* pcm = &ZGetGame()->m_ObjectManager;
	for (ZObjectManager::iterator itor = pcm->begin(); itor != pcm->end(); ++itor)
	{
		ZObject* pObject = (*itor).second;
		if (pObject != pThisObj && pObject->IsCollideable())
		{
			rvector pos = pObject->GetPosition();
			rvector dir = pThisObj->GetPosition() + diff - pos;
			dir.z = 0;
			float fDist = Magnitude(dir);

			float fCOLLISION_DIST = pObject->GetCollRadius() + pThisObj->GetCollRadius();

			if (fDist < fCOLLISION_DIST && fabs(pos.z - pThisObj->GetPosition().z) < pObject->GetCollHeight())	//height�� *0.5�ؾ��ϴ°� �ƴұ�?
			{
				// ���� ������ġ�� �ִ� ���.. ���ʹ������� �и�
				if (fDist < 1.f)
				{
					pos.x += 1.f;
					dir = pThisObj->GetPosition() - pos;
				}

				if (DotProduct(dir, diff) < 0)	// �� ��������� �����̸�
				{
					Normalize(dir);
					rvector newthispos = pos + dir * (fCOLLISION_DIST + 1.f);

					rvector newdiff = newthispos - pThisObj->GetPosition();
					diff.x = newdiff.x;
					diff.y = newdiff.y;
				}
			}
		}
	}

	rvector origin, targetpos;
	rplane impactplane;

	// �ּ� 120�̻��� ������ �̵��� �� �ִ� ���� ������ �÷��̾�� ������ �ϱ� �����̰�,
	// 1.7142857142857143f�� �÷��̾��� (radius / height)��
	//float fCollUpHeight = max(120.0f, pThisObj->GetCollHeight() - pThisObj->GetCollRadius() * 1.7142857142857143f);

	// �� �÷��̾� height=180 radius=35�ε� ��� radius/height=1.714�� �Ǵ°�? height-radius*1.7�̶�� ���ĵ� �̽��׸�..

	// ��ó�� ����ϸ� ���� ���ٴڿ� ������ �� �浹�˻簡 ����Ȯ�� ��찡 ���ܼ� �Ʒ��� ���� �غ��Ҵ�.
	// �÷��̾ ��� �ö󼳼� �ִ� �������� 120�̶�� �ϰ� ���� height�� ����Ͽ� �� �������� ũ�� �����ϰ� �Ͽ���.
	// CheckWall() ������ bsp/cylinder �浹 �˻���� ����Ȯ�� ����� �� ��� ���� Ȯ���� �ٰſ� ���� ��������� �ƴϴ�..
	// (height�� 0.5���� ����, ���ϳ��� �̽��׸�����, �ٸ� GetCollHeight()���ó������ ������ 0.5���Ͽ� ���� �ִ�. ������, 0.5�踦 ���ϸ� ����ũ�⺸�� 2�� ũ��ũ��.)
	float fCollUpHeight = max(120.f, 0.666667f * pThisObj->GetCollHeight() * 0.5f); // 0.666667f == 120.f/180.f

	origin = pThisObj->GetPosition() + rvector(0, 0, fCollUpHeight);
	targetpos = origin + diff;

	// ���� ���ϴ� �� üũ���� �ʴ´�.
	if (pThisObj->GetPosition().z > DIE_CRITICAL_LINE)
	{
		if (m_bForceCollRadius35)
			m_bAdjusted.Set_CheckCrc(ZGetGame()->GetWorld()->GetBsp()->CheckWall(origin, targetpos, 35, 60, RCW_CYLINDER, 0, &impactplane));
		else
			m_bAdjusted.Set_CheckCrc(ZGetGame()->GetWorld()->GetBsp()->CheckWall(origin, targetpos, fThisObjRadius, 60, RCW_CYLINDER, 0, &impactplane));

		//kimyhwan - �� CheckWall�� bsp �浹 �κп� ������ �ִ� �� ����. "radius=35 height=60" �� ��ó�� ���� �ƴϸ� �߸��� �浹������ ����(���� Ŀ�� �۾Ƶ� ��������)
		// ��ҿ� ������ �� ������ 35, 60�� �ƴϸ� ���ö� �� �ִ� ��� ��ü ���� �ö��� ���ϴ� ������ �߻��Ѵ�.
		// ���� �ڵ嵵 ���̴� 60���� �����صξ��µ� �� ���� ��� �����ߴ��� �˼�����,
		// fThisObjRadius�� ���� �÷��̾� ĳ���� �������� ���� 35�̹Ƿ� ������ ���������� ������ ��쿣 ������ ��� �ö��� ���ϰ� �ȴ�.
		// * http://www.melax.com/bsp.html <- ������ �Ǹ���/bsp �浹����� ����Ѱ� ����.. inaccuracy ������ ���� ����� �ֱ���
	}
	else
	{
		SetVelocity(0.0f, 0.0f, GetVelocity().z);
		m_bAdjusted.Set_CheckCrc(false);
	}

	diff = targetpos - origin;
	pThisObj->SetPosition(targetpos - rvector(0, 0, fCollUpHeight));

	if (m_bAdjusted.Ref())
		m_fLastAdjustedTime.Set_CheckCrc(ZGetGame()->GetTime());

	return m_bAdjusted.Ref();
}

void ZModule_Movable::UpdateGravity(float fDelta)
{
	m_Velocity.z =
		max(m_Velocity.z - GRAVITY_CONSTANT * fDelta, -MAX_FALL_SPEED);
}

void ZModule_Movable::UpdatePosition(float fDelta)
{
	ZObject* pThisObj = MStaticCast(ZObject, m_pContainer);

	rvector diff = fDelta * m_Velocity;

	bool bUp = (diff.z > 0.01f);
	bool bDownward = (diff.z < 0.01f);

	//	rvector diff2d=rvector(diff.x,diff.y,0);
	if (Magnitude(diff) > 0.01f)
		Move(diff);

	m_FloorPlane.CheckCrc();
	rvector floor = ZGetGame()->GetFloor(pThisObj->GetPosition(), &m_FloorPlane.Ref(), pThisObj->GetUID());
	m_FloorPlane.MakeCrc();
	m_fDistToFloor.Set_CheckCrc(pThisObj->GetPosition().z - floor.z);

	// �ö󰡾� �ϴµ� ���ö󰣰��
	if (bUp && diff.z <= 0.01f) {
		SetVelocity(GetVelocity().x, GetVelocity().y, 0);
	}

	if (!m_bFalling.Ref() && diff.z < -0.1f && m_fDistToFloor.Ref()>35.f) {
		m_bFalling.Set_CheckCrc(true);
		m_fFallHeight.Set_CheckCrc(pThisObj->GetPosition().z);
	}

	if ((pThisObj->GetPosition().z > DIE_CRITICAL_LINE) && (m_bFalling.Ref() && GetDistToFloor() < 1.f)) {
		m_bFalling.Set_CheckCrc(false);
		m_bLanding.Set_CheckCrc(true);
	}
	else
		m_bLanding.Set_CheckCrc(false);

	m_lastMove = diff;
}
/*todok del ���� ������ ������ ���� �����ؾ� �ϹǷ� ����� ������ ����
void ZModule_Movable::SetMoveSpeedRestrictRatio(float fRatio, float fDuration)
{
	// �� �����ų� ���� ������ ������ �����Ѵ�
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
	// �� ���� ������ ������ �����Ѵ�
	if (m_bRestrict.Ref() && fRatio > m_fRestrictRatio.Ref()) return;

	m_bRestrict.Set_CheckCrc(true);
	m_fRestrictTime.Set_CheckCrc(ZGetGame()->GetTime());
	m_fRestrictDuration.Set_CheckCrc(fDuration);
	m_fRestrictRatio.Set_CheckCrc(fRatio);
}

void ZModule_Movable::SetMoveSpeedHasteRatio(float fRatio, float fDuration, int nItemId)
{
	// �� ū ������ ������ �����Ѵ�
	if (m_bHaste.Ref() && fRatio < m_fHasteRatio.Ref()) return;

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
	// ���Ӱ� ���� ������ ������ ó���Ѵ�
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