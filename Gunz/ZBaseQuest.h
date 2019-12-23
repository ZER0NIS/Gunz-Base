#ifndef _ZBASEQUEST_H
#define _ZBASEQUEST_H

#include "ZGlobal.h"
#include "MBaseQuest.h"
#include "ZQuestMap.h"
#include "ZQuestGameInfo.h"
#include "ZMyItemList.h"
#include "ZNPCInfoFromServer.h"



#include <set>

enum ZQuestCheetType
{
	ZQUEST_CHEET_GOD			= 0,		// 무적모드
	ZQUEST_CHEET_WEAKNPCS		= 1,		// 적에너지 1
	ZQUEST_CHEET_AMMO			= 2,		// 총알만땅
	ZQUEST_CHEET_MAX
};

// 퀘스트나 서바이벌의 공통 인터페이스

// 기존 퀘스트모드와 유사한 서바이벌모드를 추가하면서, 퀘스트용 커맨드나 npc코드 등을 재사용할 수 있음에도 이것들이
// ZGetQuest()를 너무 명시적으로 사용하고 있어서 서바이벌을 추가하려면 큰 규모의 리팩토링이 일어날 수 밖에 없다.
// 그러나 이미 잘 서비스 되고 있는 퀘스트 코드를 리팩토링하는 것은 버그 발생 등의 이유로 매우 부담스럽다.
// 그래서 리팩토링 대신 서바이벌용으로 퀘스트 코드를 한벌 더 복사해서 그것을 서바이벌에 맞게 수정하기로 하였음
// 많은 중복 코드가 생기겠지만 그편이 덜 위험하다고 판단함..
// 문제는,
// 클라이언트 코드의 많은 부분이 ZGetQuest()를 사용하고 있으며 마치 이것은 싱글턴이나 전역변수와 같은 부작용을 낳았다.
// 퀘스트 코드가 있는 곳은 가능한 건드리지 않으려고 하였으나 도저히 ZGetQuest()사용하는 곳을 모두 일일히 수정할 수 없어서
// ::ZGetQuest()의 리턴타입이 ZSurvival일수도 있도록 하기 위해서 기존 ZQuest위에 억지로 아래의 인터페이스를 추가하였다.
// 퀘스트 코드를 가급적 건드리지 않기 위해서 퀘스트와 서바이벌에 공통으로 사용될 수 있는 코드가 있더라도 여기로 옮기지 않아서
// 모든 함수가 순수가상함수이다.

class ZBaseQuest : public MBaseQuest
{
#ifdef _QUEST_ITEM
    virtual bool OnRewardQuest( MCommand* pCmd ) = 0;
	virtual bool OnNewMonsterInfo( MCommand* pCmd ) = 0;

	virtual void GetMyObtainQuestItemList( int nRewardXP, int nRewardBP, void* pMyObtainQuestItemListBlob, void* pMyObtainZItemListBlob ) = 0;

public :
	virtual int GetRewardXP( void) = 0;
	virtual int GetRewardBP( void) = 0;
	virtual bool IsQuestComplete( void) = 0;
	virtual bool IsRoundClear( void) = 0;
	virtual DWORD GetRemainedTime( void) = 0;

	virtual MQuestCombatState GetQuestState() = 0;

	virtual ZNPCInfoFromServerManager& GetNPCInfoFromServerMgr() = 0;

#endif

	virtual bool OnNPCSpawn(MCommand* pCommand) = 0;
	virtual bool OnNPCDead(MCommand* pCommand) = 0;
	virtual bool OnPeerNPCDead(MCommand* pCommand) = 0;
	virtual bool OnEntrustNPCControl(MCommand* pCommand) = 0;
	virtual bool OnPeerNPCBasicInfo(MCommand* pCommand) = 0;
	virtual bool OnPeerNPCHPInfo(MCommand* pCommand) = 0;
	virtual bool OnPeerNPCAttackMelee(MCommand* pCommand) = 0;
	virtual bool OnPeerNPCAttackRange(MCommand* pCommand) = 0;
	virtual bool OnPeerNPCSkillStart(MCommand* pCommand) = 0;
	virtual bool OnPeerNPCSkillExecute(MCommand* pCommand) = 0;
	virtual bool OnPeerNPCBossHpAp(MCommand* pCommand) = 0;
	virtual bool OnRefreshPlayerStatus(MCommand* pCommand) = 0;
	virtual bool OnClearAllNPC(MCommand* pCommand) = 0;
	virtual bool OnQuestRoundStart(MCommand* pCommand) = 0;
	virtual bool OnQuestPlayerDead(MCommand* pCommand) = 0;
	virtual bool OnQuestGameInfo(MCommand* pCommand) = 0;
	virtual bool OnQuestCombatState(MCommand* pCommand) = 0;
	virtual bool OnMovetoPortal(MCommand* pCommand) = 0;
	virtual bool OnReadyToNewSector(MCommand* pCommand) = 0;
	virtual bool OnSectorStart(MCommand* pCommand) = 0;
	virtual bool OnObtainQuestItem(MCommand* pCommand) = 0;
	virtual bool OnObtainZItem(MCommand* pCommand) = 0;
	virtual bool OnSectorBonus(MCommand* pCommand) = 0;
	virtual bool OnQuestCompleted(MCommand* pCommand) = 0;
	virtual bool OnQuestFailed(MCommand* pCommand) = 0;
	virtual bool OnQuestPing(MCommand* pCommand) = 0;


	//ZQuestMap			m_Map;
	virtual void LoadNPCMeshes() = 0;
	virtual void LoadNPCSounds() = 0;
	virtual void MoveToNextSector() = 0;
	virtual void UpdateNavMeshWeight(float fDelta) = 0;
protected:
	virtual bool OnCreate() = 0;
	virtual void OnDestroy() = 0;
	virtual bool OnCreateOnce() = 0;
	virtual void OnDestroyOnce() = 0;
public:
	ZBaseQuest() {}
	virtual ~ZBaseQuest() {}
public:
	virtual void OnGameCreate() = 0;
	virtual void OnGameDestroy() = 0;
	virtual void OnGameUpdate(float fElapsed) = 0;
	virtual bool OnCommand(MCommand* pCommand) = 0;				///< 게임 이외에 날라오는 커맨드 처리
	virtual bool OnGameCommand(MCommand* pCommand) = 0;			///< 게임중 날라오는 커맨드 처리

	virtual void SetCheet(ZQuestCheetType nCheetType, bool bValue) = 0;
	virtual bool GetCheet(ZQuestCheetType nCheetType) = 0;

	virtual void Reload() = 0;
	virtual bool Load() = 0;

	
	// interface
	virtual ZQuestGameInfo* GetGameInfo() = 0;

	// 상태에 상관없이 사용될수 있는 퀘스트 관련된 커맨드.
	virtual bool OnSetMonsterBibleInfo( MCommand* pCmd ) = 0;


	virtual bool OnPrePeerNPCAttackMelee(MCommand* pCommand) = 0;	// 실제로 처리하는건 한타이밍 늦다
	
};


#endif