#include "stdafx.h"
#include "ZGlobal.h"
#include "ZApplication.h"
#include "ZQuest.h"

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

ZEffectManager* ZGetEffectManager(void) {
	return ZGetGameInterface()->GetEffectManager();
}

ZScreenEffectManager* ZGetScreenEffectManager(void) {
	return ZGetGameInterface()->GetScreenEffectManager();
}