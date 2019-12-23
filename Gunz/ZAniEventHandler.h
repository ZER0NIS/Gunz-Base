//////////////////////////////////////////////////////////////////////////////
//////2007.01.31
//////ZAniEventHandler.h
//////애니메이션 이벤트가 발생했을때 해당 이벤트를 처리하는 곳
//////작성자: 홍영진
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../RealSpace2/Include/RAniEventInfo.h"


class ZAniEventHandler
{
public:
	static void ZAniEventHandlerCB(RAniEventInfo* pAniEventInfo);//이벤트가 발생할때 불리기 때문에 다른거 체크할 필요없고 그냥 플레이만 해주면 된다. 
};