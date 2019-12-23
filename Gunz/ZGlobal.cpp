#include "stdafx.h"
#include "ZGlobal.h"
#include "ZApplication.h"
#include "ZQuest.h"
//#include "SVNRevision/SVNRevision.cpp"

bool ZIsLaunchDevelop(void) { 
	return ZApplication::GetInstance()->IsLaunchDevelop(); 
}
bool ZIsLaunchTest(void) { 
	return ZApplication::GetInstance()->IsLaunchTest(); 
}

RMeshMgr* ZGetNpcMeshMgr(void) { 
	return ZApplication::GetNpcMeshMgr(); 
}

RMeshMgr* ZGetMeshMgr(void) { 
	return ZApplication::GetMeshMgr(); 
}

RMeshMgr* ZGetWeaponMeshMgr(void) { 
	return ZApplication::GetWeaponMeshMgr(); 
}

RAniEventMgr* ZGetAniEventMgr(void)
{
	return ZApplication::GetAniEventMgr();
}

ZSoundEngine* ZGetSoundEngine(void) { 
	return ZApplication::GetSoundEngine(); 
}

ZEffectManager*	ZGetEffectManager(void) { 
//	return &g_pGame->m_EffectManager; 
	return ZGetGameInterface()->GetEffectManager(); 
}

ZScreenEffectManager* ZGetScreenEffectManager(void) { 
	return ZGetGameInterface()->GetScreenEffectManager(); 
}

int ZGetSVNRevision(void)
{
	return 5170;
}
