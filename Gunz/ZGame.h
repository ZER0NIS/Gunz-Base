#ifndef _ZGAME_H
#define _ZGAME_H

//#pragma once

#include "ZPrerequisites.h"

#include "MMatchClient.h"
#include "MDataChecker.h"

#include "RTypes.h"
#include "RBspObject.h"
#include "ZMatch.h"
#include "ZGameTimer.h"
#include "ZWater.h"
#include "ZClothEmblem.h"
#include "ZEffectManager.h"
#include "ZWeaponMgr.h"
#include "ZHelpScreen.h"
#include "ZCharacterManager.h"
#include "ZObjectManager.h"
#include "ZWorld.h"

_USING_NAMESPACE_REALSPACE2

class MZFileSystem;

class ZLoading;
class ZGameAction;
class ZSkyBox;
class ZFile;
class ZObject;
class ZCharacter;
class ZMyCharacter;
class ZMiniMap;
class ZMsgBox;
class ZInterfaceBackground;
class ZCharacterSelectView;
class ZScreenEffectManager;
class ZPlayerMenu;
class ZGameClient;
class ZMapDesc;
class ZReplayLoader;

void CheckMsgAboutChat(char * msg);

// Game Loop �����ϱ� ���� �ʱ�ȭ�� ��ũ�Ϸ� �˻翡 ����
enum ZGAME_READYSTATE {
	ZGAME_READYSTATE_INIT,
	ZGAME_READYSTATE_WAITSYNC,
	ZGAME_READYSTATE_RUN
};

// game ���� pick �Ǿ��� ���� ���.. pCharacter(ĳ����) Ȥ�� pNode(��)���߿� �ϳ��� ����� ���´�.
struct ZPICKINFO {

	// ĳ���Ͱ� ����� ���
//	ZCharacter *pCharacter;
	ZObject*	pObject;
	RPickInfo	info;

	// ���� ����ΰ��.
	bool bBspPicked;
	int nBspPicked_DebugRegister;
	RBSPPICKINFO bpi;
};

struct ZObserverCommandItem {
	float fTime;
	MCommand *pCommand;
};

class ZObserverCommandList : public list<ZObserverCommandItem*> {
public:
	~ZObserverCommandList() { Destroy(); }
	void Destroy() {
		while(!empty())
		{
			ZObserverCommandItem *pItem=*begin();
			delete pItem->pCommand;
			delete pItem;
			erase(begin());
		}
	}
};

struct UseItemCount
{
	int Itemid;
	int ItemUseCount;
};
struct ReplayInfo_UseSpendItem
{
	MUID uid;
	UseItemCount Item[5]; // 5�� �̻��� �����̸� ������(Ȯ�� ����)
};

#define TIME_ERROR_BETWEEN_RECIEVEDTIME_MYTIME 3.0f

class ZGame { 
protected:
	enum ZGAME_LASTTIME
	{
		ZLASTTIME_HPINFO		= 0,
		ZLASTTIME_BASICINFO,
		ZLASTTIME_PEERPINGINFO,
		ZLASTTIME_SYNC_REPORT,
		ZLASTTIME_MAX
	};

//	ZWorld				*m_pWorld;
	ZGameAction			*m_pGameAction;
	MDataChecker		m_DataChecker;
	ZGameTimer			m_GameTimer;
	MProtectValue<float> m_fTime;

	DWORD				m_nLastTime[ZLASTTIME_MAX];

    bool				m_bIsCreate;
	
	ZGAME_READYSTATE	m_nReadyState;

	set<const ZCharacter*>	m_setCharacterExceptFromNpcTarget;		// NPC�� Ÿ������ ���� �ʾƾ� �ϴ� ĳ����

	void OnPreDraw();
	bool OnRuleCommand(MCommand* pCommand);
	bool InRanged( ZObject* pAttacker, ZObject* pVictim);
	bool InRanged( ZObject* pAttacker, ZObject* pVictim, int &nDebugRegister/*����� �������� ��ŷ ������ ���� ����*/);

public:

	// �� ������ �����ϱ� ���� ��������� ��ġ�� ���嶧���� �ڼ��� ���� �ּ���ũ��(runtime/ShuffleCode.bat ����)
	// m_pMyCharacter�� �߿��ϴ�. ZGame �����͸� ���� �� ĳ���͸� ã�� �������� �����Ѵ�.
	/* [[SHUFFLE_LINE]] ZGame */	ZMyCharacter*		m_pMyCharacter;

	/* [[SHUFFLE_LINE]] ZGame */	ZEffectManager*		m_pEffectManager;

	/* [[SHUFFLE_LINE]] ZGame */	ZHelpScreen	m_HelpScreen;
	/* [[SHUFFLE_LINE]] ZGame */	ZCharacterManager	m_CharacterManager;

	/* [[SHUFFLE_LINE]] ZGame */	bool m_bShowWireframe;
	/* [[SHUFFLE_LINE]] ZGame */	RParticles			*m_pParticles;
	/* [[SHUFFLE_LINE]] ZGame */	RVisualMeshMgr		m_VisualMeshMgr;
	
	/* [[SHUFFLE_LINE]] ZGame */	ZWeaponMgr			m_WeaponManager;

	/* [[SHUFFLE_LINE]] ZGame */	ZObjectManager		m_ObjectManager;
	/* [[SHUFFLE_LINE]] ZGame */	int					m_render_poly_cnt;


private :
	
public:

	ZGame();
	virtual ~ZGame();

	bool Create(MZFileSystem *pfs, ZLoadingProgress *pLoading);

	bool IsCreated() { return m_bIsCreate; }

	void Draw();
	void Draw(MDrawContextR2 &dc);
	void Update(float fElapsed);
	void Destroy();

	void OnCameraUpdate(float fElapsed);
	void OnInvalidate();
	void OnRestore();

	void ParseReservedWord(char* pszDest, const char* pszSrc);

	void ShowReplayInfo( bool bShow);

	void OnExplosionGrenade(MUID uidOwner,rvector pos,float fDamage,float fRange,float fMinDamage,float fKnockBack,MMatchTeam nTeamID);
	void OnExplosionMagic(ZWeaponMagic *pWeapon, MUID uidOwner,rvector pos,float fMinDamage,float fKnockBack,MMatchTeam nTeamID,bool bSkipNpc);
	void OnExplosionMagicThrow(ZWeaponMagic *pWeapon, MUID uidOwner,rvector pos,float fMinDamage,float fKnockBack,MMatchTeam nTeamID,bool bSkipNpc, rvector from, rvector to);
	void OnExplosionMagicNonSplash(ZWeaponMagic *pWeapon, MUID uidOwner, MUID uidTarget, rvector pos, float fKnockBack);
	void OnReloadComplete(ZCharacter *pCharacter);
	
	void OnPeerShotSp(MUID& uid, float fShotTime, rvector& pos, rvector& dir, int type, MMatchCharItemParts sel_type);

	void OnChangeWeapon(MUID& uid, MMatchCharItemParts parts);

	rvector GetMyCharacterFirePosition(void);

	void CheckMyCharDead(float fElapsed);
	//jintriple3 ����� �������� �� ��� ���� checkMyChatDead�� �ΰ��� �ɰ�..
	void CheckMyCharDeadByCriticalLine();
	void CheckMyCharDeadUnchecked();

	void CheckStylishAction(ZCharacter* pCharacter);
	void CheckCombo( ZCharacter *pOwnerCharacter , ZObject *pHitObject ,bool bPlaySound);
	void UpdateCombo(bool bShot = false );
	//void AssignCommander(const MUID& uidRedCommander, const MUID& uidBlueCommander);
	//void SetGameRuleInfo(const MUID& uidRedCommander, const MUID& uidBlueCommander);

	void PostBasicInfo();
	void PostHPAPInfo();
	void PostDuelTournamentHPAPInfo();
	void PostPeerPingInfo();
	void PostSyncReport();
	void PostMyBuffInfo();
	
	int  SelectSlashEffectMotion(ZCharacter* pCharacter);
	//jintriple3 ����� �������� ��ŷ ����~!!!
	bool CheckWall(ZObject* pObj1,ZObject* pObj2, bool bCoherentToPeer=false);
	bool CheckWall(ZObject* pObj1,ZObject* pObj2, int & nDebugRegister/*�ܼ� �񱳿�*/, bool bCoherentToPeer=false);

	void InitRound();
	void AddEffectRoundState(MMATCH_ROUNDSTATE nRoundState, int nArg);

	bool CreateMyCharacter(MTD_CharInfo* pCharInfo);//���������ӽ��ּ� , MTD_CharBuffInfo* pCharBuffInfo);
	void DeleteCharacter(const MUID& uid);
	void RefreshCharacters();
	void ConfigureCharacter(const MUID& uidChar, MMatchTeam nTeam, unsigned char nPlayerFlags);

	bool OnCommand(MCommand* pCommand);
	bool OnCommand_Immidiate(MCommand* pCommand);

	// ��ȭ & ���
	void ToggleRecording();
	void StartRecording();
	void StopRecording();

	bool OnLoadReplay(ZReplayLoader* pLoader);
	bool IsReplay() { return m_bReplaying.Ref(); }
	bool IsShowReplayInfo() { return m_bShowReplayInfo; }
	void EndReplay();

	void OnReplayRun();


	// ������ 
	void OnObserverRun();
	void OnCommand_Observer(MCommand* pCommand);
	void FlushObserverCommands();
	int	GetObserverCommandListCount() { return (int)m_ObserverCommandList.size(); }

	void ReserveObserver();
	void ReleaseObserver();

	// �ܺο��� �����Ҹ��� �͵�
	ZMatch* GetMatch()				{ return &m_Match; }
	ZMapDesc* GetMapDesc()			{ return GetWorld() ? GetWorld()->GetDesc() : NULL; }
	ZWorld* GetWorld()				{ return ZGetWorldManager()->GetCurrent(); }

	ZGameTimer* GetGameTimer()		{ return &m_GameTimer; }
	unsigned long GetTickTime()		{ return m_GameTimer.GetGlobalTick(); }
	float GetTime()					{ /*mlog("����ð�: %f \n", m_fTime.GetData());*/ return m_fTime.Ref(); }

	int GetPing(MUID& uid);

	MDataChecker* GetDataChecker()	{ return &m_DataChecker; }

	rvector GetFloor(rvector pos, rplane *pimpactplane=NULL, MUID uID=MUID(0,0) );
	bool CharacterOverlapCollision(ZObject* pFloorObject, float WorldFloorHeight, float ObjectFloorHeight);

	bool Pick(ZObject *pOwnerObject,rvector &origin,rvector &dir,ZPICKINFO *pickinfo,DWORD dwPassFlag=RM_FLAG_ADDITIVE | RM_FLAG_HIDE,bool bMyChar=false);
	bool PickTo(ZObject *pOwnerObject,rvector &origin,rvector &to,ZPICKINFO *pickinfo,DWORD dwPassFlag=RM_FLAG_ADDITIVE | RM_FLAG_HIDE,bool bMyChar=false);
	bool PickHistory(ZObject *pOwnerObject,float fTime,const rvector &origin, const rvector &to,ZPICKINFO *pickinfo,DWORD dwPassFlag,bool bMyChar=false);
	bool ObjectColTest(ZObject* pOwner, rvector& origin, rvector& to, float fRadius, ZObject** poutTarget);

	char* GetSndNameFromBsp(const char* szSrcSndName, RMATERIAL* pMaterial);

	bool CheckGameReady();
	ZGAME_READYSTATE GetReadyState()			{ return m_nReadyState; }
	void SetReadyState(ZGAME_READYSTATE nState)	{ m_nReadyState = nState; }

	bool GetSpawnRequested()			{ return m_bSpawnRequested; }
	void SetSpawnRequested(bool bVal)	{ m_bSpawnRequested = bVal; }

	bool GetUserNameColor(MUID uid,MCOLOR& color,char* sp_name);
	//jintriple3 ����� �������� ��ŷ ������ ���� CanAttack()�Լ��� �̸��� �ٲ㼭 ���.
	bool CanAttack(ZObject *pAttacker, ZObject *pTarget);
	bool CanAttack_DebugRegister(ZObject *pAttacker, ZObject *pTarget);

	bool CanISeeAttacker( ZCharacter* pAtk, const rvector& vRequestPos );

	// npc AI�� Ÿ������ �����ʾƾ� �ϴ� ĳ���� ��� �����Լ�
	void ExceptCharacterFromNpcTargetting(const ZCharacter* pChar) { m_setCharacterExceptFromNpcTarget.insert(pChar); }
	void ClearListExceptionFromNpcTargetting() { m_setCharacterExceptFromNpcTarget.clear(); }
	bool IsExceptedFromNpcTargetting(const ZCharacter* pChar) { return m_setCharacterExceptFromNpcTarget.find(pChar) != m_setCharacterExceptFromNpcTarget.end(); }

protected:
	char m_szReplayFileName[_MAX_PATH];	// ���÷��� ���� �Ϸ� �޽��� ����� ����
	ZFile *m_pReplayFile;
//	FILE *m_pRecordingFile;
	MProtectValue<bool> m_bReplaying;
	bool m_bShowReplayInfo;

	bool m_bRecording;
	bool m_bReserveObserver;

	bool m_bSuicide;
	DWORD m_dwReservedSuicideTime;

	unsigned long int m_nReservedObserverTime;
	int m_t;
	ZMatch				m_Match;
	unsigned long int	m_nSpawnTime;
	bool				m_bSpawnRequested;

	ZObserverCommandList m_ObserverCommandList;		// observer �����϶� command �� �����صδ� ��
	ZObserverCommandList m_ReplayCommandList;		// replay �����϶� command �� �����صδ� ��, �� ��ȭ�Ҷ� �����ϴ� ��.

	ZObserverCommandList m_DelayedCommandList;		// �����ð��� ������ command ���̴�. ex) ����,Į��

	float m_ReplayLogTime;
	ReplayInfo_UseSpendItem m_Replay_UseItem[16];	// 16�� �̻� ���� ����(Ȯ������) map���� ����� �ʿ伺....

	void CheckKillSound(ZCharacter* pAttacker);
	
	void OnReserveObserver();
	void DrawDebugInfo();

	void OnStageEnterBattle(MCmdEnterBattleParam nParam, MTD_PeerListNode* pPeerNode);
	void OnStageLeaveBattle(const MUID& uidChar, const bool bIsRelayMap);//, const MUID& uidStage);
	void OnPeerList(const MUID& uidStage, void* pBlob, int nCount);
	void OnAddPeer(const MUID& uidChar, DWORD dwIP, const int nPort =	
		MATCHCLIENT_DEFAULT_UDP_PORT, MTD_PeerListNode* pNode = NULL);
	void OnGameRoundState(const MUID& uidStage, int nRound, int nRoundState, int nArg);

	void OnGameResponseTimeSync(unsigned int nLocalTimeStamp, unsigned int nGlobalTimeSync);
	void OnEventUpdateJjang(const MUID& uidChar, bool bJjang);

	// �������.
//	void OnPeerShot_Item(ZCharacter* pOwnerCharacter,float fShotTime, rvector& pos, rvector& dir,int type);

	void OnPeerDead(const MUID& uidAttacker, const unsigned long int nAttackerArg, 
					const MUID& uidVictim, const unsigned long int nVictimArg);
	void OnReceiveTeamBonus(const MUID& uidChar, const unsigned long int nExpArg);
	void OnPeerDie(MUID& uidVictim, MUID& uidAttacker);
	void OnPeerDieMessage(ZCharacter* pVictim, ZCharacter* pAttacker);
	void OnChangeParts(MUID& uid,int partstype,int PartsID);
	//void OnAssignCommander(const MUID& uidRedCommander, const MUID& uidBlueCommander);
	void OnAttack(MUID& uid,int type,rvector& pos);
	void OnDamage(MUID& uid,MUID& tuid,int damage);
	void OnPeerReload(MUID& uid);
	void OnPeerSpMotion(MUID& uid,int nMotionType);
	void OnPeerChangeCharacter(MUID& uid);
	void OnPeerSpawn(MUID& uid, rvector& pos, rvector& dir);

	void OnSetObserver(MUID& uid);
	

//	void OnPeerAdd();
	void OnPeerBasicInfo(MCommand *pCommand,bool bAddHistory=true,bool bUpdate=true);
	void OnPeerHPInfo(MCommand *pCommand);
	void OnPeerHPAPInfo(MCommand *pCommand);
	void OnPeerDuelTournamentHPAPInfo(MCommand *pCommand);
	void OnPeerPing(MCommand *pCommand);
	void OnPeerPong(MCommand *pCommand);
	void OnPeerOpened(MCommand *pCommand);
	void OnPeerDash(MCommand* pCommand);
	void OnPeerBuffInfo(const MUID& uidSender, void* pBlobBuffInfo);

		
	bool FilterDelayedCommand(MCommand *pCommand);
	void ProcessDelayedCommand();

	// ��ǥ�� ����
	//	void AdjustGlobalTime();
	//	void AdjustMyData();

	void OnLocalOptainSpecialWorldItem(MCommand* pCommand);
	void OnResetTeamMembers(MCommand* pCommand);

public:

	void AutoAiming();

	void OnPeerShot( const MUID& uid, float fShotTime, const rvector& pos, const rvector& to, const MMatchCharItemParts sel_type);

	void PostSpMotion(ZC_SPMOTION_TYPE type);

	// peershot�� �ʹ� ������� �и�
	void OnPeerShot_Melee(const MUID& uidOwner, float fShotTime);
	void OnPeerShot_Range(const MMatchCharItemParts sel_type, const MUID& uidOwner, float fShotTime, const rvector& pos, const rvector& to);
	//jintriple3 ����� �������� �� �� ���� OnPeerShot_Range�� �ɰ�...
	void OnPeerShot_Range_Damaged(ZObject* pOwner, float fShotTime, const rvector& pos, const rvector& to, ZPICKINFO pickinfo, DWORD dwPickPassFlag, rvector& v1, rvector& v2, ZItem *pItem, rvector& BulletMarkNormal, bool& bBulletMark, ZTargetType& nTargetType);
	void OnPeerShot_Shotgun(ZItem *pItem, ZCharacter* pOwnerCharacter, float fShotTime, const rvector& pos, const rvector& to);
	MTD_ShotInfo* OnPeerShotgun_Damaged(ZObject* pOwner, float fShotTime, const rvector& pos, rvector &dir, ZPICKINFO pickinfo, DWORD dwPickPassFlag, rvector& v1, rvector& v2, ZItem *pItem, rvector& BulletMarkNormal, bool& bBulletMark, ZTargetType& nTargetType, bool& bHitEnemy);

    void ReserveSuicide( void);
	bool IsReservedSuicide( void)		{ return m_bSuicide; }
	void CancelSuicide( void)			{ m_bSuicide = false; }
	ZObserverCommandList* GetReplayCommandList()  { return &m_ReplayCommandList;} 

	void MakeResourceCRC32( const DWORD dwKey, DWORD& out_crc32, DWORD& out_xor );

	void OnResponseUseSpendableBuffItem(MUID& uidItem, int nResult);
	//���������ӽ��ּ� 
	//void OnGetSpendableBuffItemStatus(MUID& uidChar, MTD_CharBuffInfo* pCharBuffInfo);

	void ApplyPotion(int nItemID, ZCharacter* pCharObj, float fRemainedTime);
	void OnUseTrap(int nItemID, ZCharacter* pCharObj, rvector& pos);
	void OnUseDynamite(int nItemID, ZCharacter* pCharObj, rvector& pos);

	void CheckZoneTrap(MUID uidOwner,rvector pos,MMatchItemDesc* pItemDesc, MMatchTeam nTeamID);
	void OnExplosionDynamite(MUID uidOwner, rvector pos, float fDamage, float fRange, float fKnockBack, MMatchTeam nTeamID);

public:
	bool GetUserGradeIDColor(MMatchUserGradeID gid, MCOLOR& UserNameColor, char* sp_name);

};


extern MUID g_MyChrUID;
extern float g_fFOV;

// dll-injection �� ������ ��ũ�� �ζ��̴�
#define ZGetCharacterManager()	(ZGetGame() ? &(ZGetGame()->m_CharacterManager) : NULL)
#define ZGetObjectManager()		(ZGetGame() ? &(ZGetGame()->m_ObjectManager) : NULL)

/*__forceinline bool IsMyCharacter(ZObject* pObject)
{
	return ((ZGetGame()) && ((ZObject*)ZGetGame()->m_pMyCharacter == pObject));
}*/

#define IsMyCharacter(pObject) \
( \
	ZGetGame() && \
	( \
		((ZObject*)(ZGetGame()->m_pMyCharacter))==((ZObject*)(pObject)) \
	) \
)


/*
// Damage ��꿡 �ʿ��� ����
#define DAMAGE_MELEE_HEAD	0.6f
#define DAMAGE_MELEE_CHEST	0.6f
#define DAMAGE_MELEE_HANDS	0.6f
#define DAMAGE_MELEE_LEGS	0.6f
#define DAMAGE_MELEE_FEET	0.6f

#define DAMAGE_RANGE_HEAD	0.8f
#define DAMAGE_RANGE_CHEST	0.5f
#define DAMAGE_RANGE_HANDS	0.5f
#define DAMAGE_RANGE_LEGS	0.5f
#define DAMAGE_RANGE_FEET	0.5f
*/
#define MAX_COMBO 99

#define PEERMOVE_TICK			100		// 0.1�� ���� �̵��޼����� ������ (�ʴ� 10ȸ)
#define PEERMOVE_AGENT_TICK		100		// Agent�� ���ϸ� �ʴ� 10�� �޽����� ������.


#endif