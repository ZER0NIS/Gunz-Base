#include "stdafx.h"
#include "ZBehavior_Idle.h"

ZBehavior_Idle::ZBehavior_Idle( ZBrain* pBrain) : ZBehaviorState( pBrain, ZBEHAVIOR_STATE_IDLE)
{
	m_pBrain = pBrain;
}

ZBehavior_Idle::~ZBehavior_Idle()
{
}


void ZBehavior_Idle::OnEnter()
{
}

void ZBehavior_Idle::OnExit()
{
}

void ZBehavior_Idle::OnRun(float fDelta)
{
	if ( !m_pBrain)
		return;
}
