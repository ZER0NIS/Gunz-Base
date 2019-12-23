#include "stdafx.h"

#include "ZAniEventHandler.h"

void ZAniEventHandler::ZAniEventHandlerCB(RAniEventInfo* pAniEventInfo)//이벤트가 발생할때 불리기 때문에 다른거 체크할 필요없고 그냥 플레이만 해주면 된다. 
{
	char* cEventType = pAniEventInfo->GetEventType();
	
	if( strcmp(cEventType , "sound") == 0)
	{
		char* cFileName = pAniEventInfo->GetFileName();
		ZGetSoundEngine()->PlaySound(cFileName, pAniEventInfo->m_vPos, false);
	}
}