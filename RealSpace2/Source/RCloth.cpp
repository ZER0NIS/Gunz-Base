#include "stdafx.h"
#include "RCloth.h"
#include "MDebug.h"

RCloth::RCloth(void) :
	m_pX(0),
	m_pOldX(0),
	m_pForce(0),
	m_pConst(0),
	m_pHolds(0),
	m_pWeights(0),
	m_pNormal(0)
{
}

RCloth::~RCloth(void)
{
	SAFE_DELETE_ARRAY(m_pX);
	SAFE_DELETE_ARRAY(m_pOldX);
	SAFE_DELETE_ARRAY(m_pForce);
	SAFE_DELETE_ARRAY(m_pConst);
	SAFE_DELETE_ARRAY(m_pHolds);
	SAFE_DELETE_ARRAY(m_pWeights);
	SAFE_DELETE_ARRAY(m_pNormal);
}

void RCloth::update()
{
	accumulateForces();
	varlet();
	satisfyConstraints();
}

void RCloth::accumulateForces()
{
}

void RCloth::varlet()
{
	rvector* swapTemp;

	for (int i = 0; i < m_nCntP; ++i)
	{
		if (m_pHolds[i] != CLOTH_HOLD)
		{
			m_pOldX[i] = m_pX[i] + m_AccelationRatio * (m_pX[i] - m_pOldX[i]) + m_pForce[i] * m_fTimeStep * m_fTimeStep;
		}
	}

	swapTemp = m_pX;
	m_pX = m_pOldX;
	m_pOldX = swapTemp;

	memset(m_pForce, 0, sizeof(rvector) * m_nCntP);
}

void RCloth::satisfyConstraints()
{
	sConstraint* c;
	rvector* x1;
	rvector* x2;
	rvector delta;
	float deltaLegth;
	float diff;
	int i, j;

	for (i = 0; i < m_nCntIter; ++i)
	{
		float w1, w2;
		for (j = 0; j < m_nCntC; ++j)
		{
			c = &m_pConst[j];

			x1 = &m_pX[c->refA];
			x2 = &m_pX[c->refB];

			w1 = m_pWeights[c->refA];
			w2 = m_pWeights[c->refB];

			if (w1 == 0 && w2 == 0)
			{
				continue;
			}

			delta = *x2 - *x1;
			deltaLegth = D3DXVec3Length(&delta);
			diff = (float)((deltaLegth - c->restLength) / (deltaLegth * (w1 + w2)));

			*x1 += delta * w1 * diff;
			*x2 -= delta * w2 * diff;
		}
	}
}

void RCloth::render()
{
}

void RCloth::UpdateNormal()
{
}

void RWindGenerator::Update(DWORD time)
{
	_ASSERT(m_Time <= time);
	DWORD	ElapsedTime = m_Time - time;
	int factor = 0;

	switch (m_WindType)
	{
	case RANDOM_WIND:
		if (m_WindPower > 0)
			factor = rand() % (int)(m_WindPower);
		m_ResultWind.x = m_WindDirection.x * factor;
		m_ResultWind.y = m_WindDirection.y * factor;
		m_ResultWind.z = 0;
		break;
	case GENTLE_BREEZE_WIND:
	{
#define PEACE_TIME	1000
#define POWER_INC		1.0f
		if (m_bFlag)
		{
			if (m_fTemp2 == 0) m_fTemp2 = 1;
			if (m_fTemp1 > m_WindPower) m_fTemp2 = -1;
			m_fTemp1 += POWER_INC * m_fTemp2;
			m_Time = time;
			if (m_fTemp1 < 0)
			{
				m_bFlag = false;
				m_fTemp1 = 0;
				m_fTemp2 = 0;
			}
		}
		else if (ElapsedTime > m_DelayTime)
		{
			m_Time = time;
			m_bFlag = true;
		}
	}
	m_ResultWind = m_WindDirection * m_fTemp1;
	break;
	default:
		m_WindDirection = rvector(0, 0, 0);
		m_WindPower = 0.f;
	}
}