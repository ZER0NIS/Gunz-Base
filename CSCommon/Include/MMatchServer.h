#ifndef MMATCHSERVER_H
#define MMATCHSERVER_H

#include "MMatchDBMgr.h"
#include "winsock2.h"
#include "MXml.h"
#include "MServer.h"
#include "MMatchObject.h"
#include "MAgentObject.h"
#include "MMatchChannel.h"
#include "MMatchStage.h"
#include "MMatchClan.h"
#include "MSafeUDP.h"
#include "MMatchTransDataType.h"
#include "MMatchAdmin.h"
#include "MAsyncProxy.h"
#include "MMatchGlobal.h"
#include "MMatchShutdown.h"
#include "MMatchChatRoom.h"
#include "MLadderMgr.h"
#include "MMatchQuest.h"
#include "MTypes.h"
#include "MStringRes.h"
#include "MMatchStringResManager.h"
#include "MMatchEventManager.h"
#include "MMatchGambleMachine.h"
#include "MHackingCharList.h"
#include "../MMatchObjectCommandHistory.h"
#include "../MConnectHistory.h"

#include "MMatchBRMachine.h"

#include <vector>
using namespace std;

class MMatchAuthBuilder;
class MMatchScheduleMgr;
class MNJ_DBAgentClient;
class MBMatchMonitor;
class MMatchGambleMachine;
class MMatchDuelTournamentMgr;

#define MATCHSERVER_UID		MUID(0, 2)

#define CHECKMEMORYNUMBER	888888

enum CUSTOM_IP_STATUS
{
	CIS_INVALID = 0,
	CIS_NONBLOCK,
	CIS_BLOCK,
	CIS_NON,
};

enum COUNT_CODE_STATUS
{
	CCS_INVALID = 0,
	CCS_NONBLOCK,
	CCS_BLOCK,
	CCS_NON,
};

struct ClanRejoin
{
	MMatchTeam Team;
	MUID StageUID;
	ClanRejoin(MMatchTeam team, MUID stageUID)
	{
		Team = team;
		StageUID = stageUID;
	}
};
typedef std::map<unsigned long, ClanRejoin*> ClanReDef;

class MMatchServer : public MServer {
private:
	static MMatchServer* m_pInstance;
	unsigned long int		m_nTickTime;
	inline void SetTickTime(unsigned long int nTickTime);

	unsigned long		m_HSCheckCounter;

protected:
	unsigned long		m_nItemFileChecksum;

	MUID				m_NextUseUID;
	MCriticalSection	m_csUIDGenerateLock;
	MCriticalSection	m_csTickTimeLock;

	ClanReDef			ClanRejoiner;

	DWORD				m_checkMemory1;
	MMatchObjectList	m_Objects;
	DWORD				m_checkMemory2;

	MMatchChannelMap	m_ChannelMap;
	DWORD				m_checkMemory3;

	char				m_szDefaultChannelName[CHANNELNAME_LEN];
	char				m_szDefaultChannelRuleName[CHANNELRULE_LEN];

	DWORD				m_checkMemory4;
	MMatchStageMap		m_StageMap;
	DWORD				m_checkMemory5;
	MMatchClanMap		m_ClanMap;
	DWORD				m_checkMemory6;
	MAgentObjectMap		m_AgentMap;
	DWORD				m_checkMemory7;

	DWORD				m_checkMemory8;
	MSafeUDP			m_SafeUDP;
	DWORD				m_checkMemory9;
	MMatchDBMgr			m_MatchDBMgr;
	DWORD				m_checkMemory10;
	MAsyncProxy			m_AsyncProxy;
	DWORD				m_checkMemory11;
	MMatchAdmin			m_Admin;
	DWORD				m_checkMemory12;
	MMatchShutdown		m_MatchShutdown;
	DWORD				m_checkMemory13;
	MMatchChatRoomMgr	m_ChatRoomMgr;
	DWORD				m_checkMemory14;
	MLadderMgr			m_LadderMgr;
	MMatchDuelTournamentMgr* m_pDTMgr;

	DWORD				m_checkMemory15;

	bool				m_bCreated;

	DWORD					m_checkMemory16;

	DWORD					m_checkMemory17;

	DWORD					m_checkMemory18;
	MMatchScheduleMgr* m_pScheduler;
	DWORD					m_checkMemory19;
	MMatchAuthBuilder* m_pAuthBuilder;
	DWORD					m_checkMemory20;
	MMatchQuest				m_Quest;
	DWORD					m_checkMemory21;

	MCountryFilter			m_CountryFilter;
	IPtoCountryList			m_TmpIPtoCountryList;
	BlockCountryCodeList	m_TmpBlockCountryCodeList;
	CustomIPList			m_TmpCustomIPList;
	DWORD					m_dwBlockCount;
	DWORD					m_dwNonBlockCount;

	MMatchEventManager		m_CustomEventManager;
	unsigned char			m_szMD5Value[16];
	MMatchGambleMachine		m_GambleMachine;

	MHackingChatList		m_HackingChatList;
	MMatchObjectCommandHistory	m_objectCommandHistory;
	MConnectHistory			m_connectionHistory;

	void InitItemCRC32Cache();
	void InitBuffCRC32Cache();

	const DWORD GetItemCRC32Cache(const int nItemID);
	const DWORD GetBuffCRC32Cache(const int nBuffID);

public:
	MMatchServer(void);
	virtual ~MMatchServer(void);

	static MMatchServer* GetInstance(void);

	bool Create(int nPort);
	void Destroy(void);
	virtual void Shutdown();
	virtual MUID UseUID(void);

	MMatchAuthBuilder* GetAuthBuilder() { return m_pAuthBuilder; }
#ifndef NEW_AUTH_MODULE
	void SetAuthBuilder(MMatchAuthBuilder* pBuilder) { m_pAuthBuilder = pBuilder; }
#endif

	MMatchChatRoomMgr* GetChatRoomMgr() { return &m_ChatRoomMgr; }

	virtual bool SendMonitorUDP(const DWORD dwIP, const USHORT nPort, const string& strMonitorCommand)
	{
		return true;
	}

protected:
	virtual bool OnCreate(void);
	virtual void OnDestroy(void);
	virtual void OnRegisterCommand(MCommandManager* pCommandManager);
	virtual bool OnCommand(MCommand* pCommand);
	virtual void OnRun(void);
	virtual void OnPrepareRun();

	virtual int Connect(MCommObject* pCommObj);
	virtual int OnConnected(MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp, MCommObject* pCommObj);

	virtual void OnNetPong(const MUID& CommUID, unsigned int nTimeStamp);
	virtual void OnHShieldPong(const MUID& CommUID, unsigned int nTimeStamp);
	bool CheckOnLoginPre(const MUID& CommUID, int nCmdVersion, bool& outbFreeIP, string& strCountryCode3);
	void OnMatchLogin(MUID CommUID, const char* szUserID, const char* szPassword, int nCommandVersion, unsigned long nChecksumPack, char* szEncryptMd5Value);
	void OnMatchLoginFromNetmarbleJP(const MUID& CommUID, const char* szLoginID, const char* szLoginPW, int nCmdVersion, unsigned long nChecksumPack);
	void OnMatchLoginFromDBAgent(const MUID& CommUID, const char* szLoginID, const char* szName, int nSex, bool bFreeLoginIP, unsigned long nChecksumPack);
	void OnMatchLoginFailedFromDBAgent(const MUID& CommUID, int nResult);
	void OnBridgePeer(const MUID& uidChar, DWORD dwIP, DWORD nPort);

	bool AddObjectOnMatchLogin(const MUID& uidComm,
		const MMatchAccountInfo* pSrcAccountInfo,
		const MMatchAccountPenaltyInfo* pSrcAccountPenaltyInfo,
		bool bFreeLoginIP, string strCountryCode3, unsigned long nChecksumPack);

	void LockUIDGenerate() { m_csUIDGenerateLock.Lock(); }
	void UnlockUIDGenerate() { m_csUIDGenerateLock.Unlock(); }

	int ObjectAdd(const MUID& uidComm);
	int ObjectRemove(const MUID& uid, MMatchObjectList::iterator* pNextItor);

	int MessageSay(MUID& uid, char* pszSay);

	MSafeUDP* GetSafeUDP() { return &m_SafeUDP; }
	void SendCommandByUDP(MCommand* pCommand, char* szIP, int nPort);
	void ParsePacket(char* pData, MPacketHeader* pPacketHeader, DWORD dwIP, WORD wRawPort);
	static bool UDPSocketRecvEvent(DWORD dwIP, WORD wRawPort, char* pPacket, DWORD dwSize);
	void ParseUDPPacket(char* pData, MPacketHeader* pPacketHeader, DWORD dwIP, WORD wRawPort);

	void ProcessAsyncJob();
	void OnAsyncGetAccountCharList(MAsyncJob* pJobResult);
	void OnAsyncGetAccountCharInfo(MAsyncJob* pJobResult);
	void OnAsyncGetCharInfo(MAsyncJob* pJobResult);
	void OnAsyncCreateChar(MAsyncJob* pJobResult);
	void OnAsyncDeleteChar(MAsyncJob* pJobResult);
	void OnAsyncGetFriendList(MAsyncJob* pJobInput);
	void OnAsyncGetLoginInfo(MAsyncJob* pJobInput);
	void OnAsyncWinTheClanGame(MAsyncJob* pJobInput);
	void OnAsyncUpdateCharInfoData(MAsyncJob* pJobInput);
	void OnAsyncCharFinalize(MAsyncJob* pJobInput);
	void OnAsyncInsertConnLog(MAsyncJob* pJobResult);
	void OnAsyncInsertGameLog(MAsyncJob* pJobResult);
	void OnAsyncCreateClan(MAsyncJob* pJobResult);
	void OnAsyncExpelClanMember(MAsyncJob* pJobResult);
	void OnAsyncInsertEvent(MAsyncJob* pJobResult);
	void OnAsyncUpdateIPtoCoutryList(MAsyncJob* pJobResult);
	void OnAsyncUpdateBlockCountryCodeList(MAsyncJob* pJobResult);
	void OnAsyncUpdateCustomIPList(MAsyncJob* pJobResult);
	void OnAsyncGetAccountItemList(MAsyncJob* pJobResult);
	void OnAsyncBuyQuestItem(MAsyncJob* pJobResult);

	virtual void OnProcessAsyncJob(MAsyncJob* pJob) {}

	bool InitScheduler();
	bool InitLocale();
	bool InitEvent();

	bool InitGambleMachine();

	bool IsEquipmentTypeItem(const MMatchItemDesc* pItemDesc);
public:
	void PostAsyncJob(MAsyncJob* pJob);

private:
	virtual bool InitSubTaskSchedule() { return true; }

protected:
	void DisconnectObject(const MUID& uidObject);
	void DebugTest();
protected:
	const char* GetDefaultChannelName() { return m_szDefaultChannelName; }
	void SetDefaultChannelName(const char* pszName) { strcpy(m_szDefaultChannelName, pszName); }
	const char* GetDefaultChannelRuleName() { return m_szDefaultChannelRuleName; }
	void SetDefaultChannelRuleName(const char* pszName) { strcpy(m_szDefaultChannelRuleName, pszName); }
	const MUID FindFreeChannel(const MUID& uidPlayer);

	bool ChannelAdd(const char* pszChannelName, const char* pszRuleName, MUID* pAllocUID, MCHANNEL_TYPE nType = MCHANNEL_TYPE_PRESET, int nMaxPlayers = DEFAULT_CHANNEL_MAXPLAYERS, int nLevelMin = -1, int nLevelMax = -1,
		const bool bIsTicketChannel = false, const DWORD dwTicketItemID = 0, const bool bIsUseTicket = false, const char* pszChannelNameStrResId = NULL);
	bool ChannelJoin(const MUID& uidPlayer, const MUID& uidChannel);
	bool ChannelJoin(const MUID& uidPlayer, const MCHANNEL_TYPE nChannelType, const char* pszChannelName);
	bool ChannelLeave(const MUID& uidPlayer, const MUID& uidChannel);
	bool ChannelChat(const MUID& uidPlayer, const MUID& uidChannel, char* pszChat);

	void ResponseChannelRule(const MUID& uidPlayer, const MUID& uidChannel);

	void OnRequestRecommendedChannel(const MUID& uidComm);
	void OnRequestChannelJoin(const MUID& uidPlayer, const MUID& uidChannel);
	void OnRequestChannelJoin(const MUID& uidPlayer, const MCHANNEL_TYPE nChannelType, const char* pszChannelName);
	void OnChannelChat(const MUID& uidPlayer, const MUID& uidChannel, char* pszChat);
	void OnStartChannelList(const MUID& uidPlayer, const int nChannelType);
	void OnStopChannelList(const MUID& uidPlayer);

	void OnChannelRequestPlayerList(const MUID& uidPlayer, const MUID& uidChannel, int nPage);
	void OnChannelRequestAllPlayerList(const MUID& uidPlayer, const MUID& uidChannel, unsigned long int nPlaceFilter, unsigned long int nOptions);

public:
	void ChannelResponsePlayerList(const MUID& uidPlayer, const MUID& uidChannel, int nPage);
	void ChannelResponseAllPlayerList(const MUID& uidPlayer, const MUID& uidChannel, unsigned long int nPlaceFilter, unsigned long int nOptions);

public:
	MMatchStage* FindStage(const MUID& uidStage);
protected:
	friend MMatchStage;
	friend MNJ_DBAgentClient;
	bool StageAdd(MMatchChannel* pChannel, const char* pszStageName, bool bPrivate, const char* pszStagePassword, MUID* pAllocUID, bool bIsAllowNullChannel = false);
	bool StageRemove(const MUID& uidStage, MMatchStageMap::iterator* pNextItor);
	bool StageJoin(const MUID& uidPlayer, const MUID& uidStage, bool rejoin = false, MMatchTeam Team = MMT_ALL);
	bool StageLeave(const MUID& uidPlayer);
	bool StageEnterBattle(const MUID& uidPlayer, const MUID& uidStage);
	bool StageLeaveBattle(const MUID& uidPlayer, bool bGameFinishLeaveBattle, bool bForcedLeave);
	bool StageChat(const MUID& uidPlayer, const MUID& uidStage, char* pszChat);
	bool StageTeam(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam);
	bool StagePlayerState(const MUID& uidPlayer, const MUID& uidStage, MMatchObjectStageState nStageState);
	bool StageMaster(const MUID& uidStage);

protected:
	MCommand* CreateCmdResponseStageSetting(const MUID& uidStage);
	MCommand* CreateCmdMatchResponseLoginOK(const MUID& uidComm,
		MUID& uidPlayer,
		const char* szUserID,
		MMatchUserGradeID nUGradeID,
		MMatchPremiumGradeID nPGradeID,
		const unsigned char* pbyGuidReqMsg);
	MCommand* CreateCmdMatchResponseLoginFailed(const MUID& uidComm, const int nResult);

	float GetDuelVictoryMultiflier(int nVictorty);
	float GetDuelPlayersMultiflier(int nPlayerCount);
	void CalcExpOnGameKill(MMatchStage* pStage, MMatchObject* pAttacker, MMatchObject* pVictim,
		int* poutAttackerExp, int* poutVictimExp);
	const int CalcBPonGameKill(MMatchStage* pStage, MMatchObject* pAttacker, const int nAttackerLevel, const int nVictimLevel);
	void PostGameDeadOnGameKill(MUID& uidStage, MMatchObject* pAttacker, MMatchObject* pVictim,
		int nAddedAttackerExp, int nSubedVictimExp);

	void OnStageCreate(const MUID& uidChar, char* pszStageName, bool bPrivate, char* pszStagePassword);
	void OnPrivateStageJoin(const MUID& uidPlayer, const MUID& uidStage, char* pszPassword);
	void OnStageFollow(const MUID& uidPlayer, const char* pszTargetName);
	void OnStageLeave(const MUID& uidPlayer);
	void OnStageRequestPlayerList(const MUID& uidPlayer, const MUID& uidStage);
	void OnStageEnterBattle(const MUID& uidPlayer, const MUID& uidStage);
	void OnStageLeaveBattle(const MUID& uidPlayer, bool bGameFinishLeaveBattle);
	void OnStageChat(const MUID& uidPlayer, const MUID& uidStage, char* pszChat);
	void OnRequestQuickJoin(const MUID& uidPlayer, void* pQuickJoinBlob);
	void ResponseQuickJoin(const MUID& uidPlayer, MTD_QuickJoinParam* pQuickJoinParam);
	void OnStageGo(const MUID& uidPlayer, unsigned int nRoomNo);
	void OnStageTeam(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam);
	void OnStagePlayerState(const MUID& uidPlayer, const MUID& uidStage, MMatchObjectStageState nStageState);
	void OnStageStart(const MUID& uidPlayer, const MUID& uidStage, int nCountdown);
	void OnStageRelayStart(const MUID& uidStage);
	void OnStartStageList(const MUID& uidComm);
	void OnStopStageList(const MUID& uidComm);
	void OnStageRequestStageList(const MUID& uidPlayer, const MUID& uidChannel, const int nStageCursor);
	void OnStageMap(const MUID& uidStage, char* pszMapName);

	void OnStageRelayMapElementUpdate(const MUID& uidStage, int nType, int nTurnCount);
	void OnStageRelayMapListUpdate(const MUID& uidStage, int nRelayMapType, int nRelayMapRepeatCount, void* pRelayMapListBlob);
	void OnStageRelayMapListInfo(const MUID& uidStage, const MUID& uidChar);
	void OnStageSetting(const MUID& uidPlayer, const MUID& uidStage, void* pStageBlob, int nStageCount);
	void OnRequestStageSetting(const MUID& uidComm, const MUID& uidStage);
	void OnRequestPeerList(const MUID& uidChar, const MUID& uidStage);
	void OnRequestGameInfo(const MUID& uidChar, const MUID& uidStage);
	void OnMatchLoadingComplete(const MUID& uidPlayer, int nPercent);
	void OnRequestRelayPeer(const MUID& uidChar, const MUID& uidPeer);
	void OnPeerReady(const MUID& uidChar, const MUID& uidPeer);
	void OnGameRoundState(const MUID& uidStage, int nState, int nRound);
	void OnRequestSpawn(const MUID& uidChar, const MVector& pos, const MVector& dir);
	void OnGameRequestTimeSync(const MUID& uidComm, unsigned long nLocalTimeStamp);
	void OnGameReportTimeSync(const MUID& uidComm, unsigned long nLocalTimeStamp, unsigned int nDataChecksum);
	void OnUpdateFinishedRound(const MUID& uidStage, const MUID& uidChar,
		void* pPeerInfo, void* pKillInfo);
	void OnRequestForcedEntry(const MUID& uidStage, const MUID& uidChar);
	void OnRequestSuicide(const MUID& uidPlayer);
	void OnRequestObtainWorldItem(const MUID& uidPlayer, const int nItemUID);
	void OnRequestFlagCap(const MUID& uidPlayer, const int nItemID);
	void OnRequestSpawnWorldItem(const MUID& uidPlayer, const int nItemID, const float x, const float y, const float z, float fDropDelayTime);
	void OnNotifyThrowTrapItem(const MUID& uidPlayer, const int nItemID);
	void OnNotifyActivatedTrapItem(const MUID& uidPlayer, const int nItemID, const MVector3& pos);

	void OnUserWhisper(const MUID& uidComm, char* pszSenderName, char* pszTargetName, char* pszMessage);
	void OnUserWhere(const MUID& uidComm, char* pszTargetName);
	void OnUserOption(const MUID& uidComm, unsigned long nOptionFlags);
	void OnChatRoomCreate(const MUID& uidPlayer, const char* pszChatRoomName);
	void OnChatRoomJoin(const MUID& uidComm, char* pszPlayerName, char* pszChatRoomName);
	void OnChatRoomLeave(const MUID& uidComm, char* pszPlayerName, char* pszChatRoomName);
	void OnChatRoomSelectWrite(const MUID& uidComm, const char* szChatRoomName);
	void OnChatRoomInvite(const MUID& uidComm, const char* pszTargetName);
	void OnChatRoomChat(const MUID& uidComm, const char* pszMessage);

	void OnRequestFlagState(const MUID& uidChar);

	void SaveGameLog(const MUID& uidStage);
	void SaveGamePlayerLog(MMatchObject* pObj, unsigned int nStageID);
protected:
	friend MLadderMgr;
	bool LadderJoin(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam);
	void LadderGameLaunch(MLadderGroup* pGroupA, MLadderGroup* pGroupB, int nMapID);
protected:
	void OnLadderRequestInvite(const MUID& uidPlayer, void* pGroupBlob);
	void OnLadderInviteAgree(const MUID& uidPlayer);
	void OnLadderInviteCancel(const MUID& uidPlayer);
	bool IsLadderRequestUserInRequestClanMember(const MUID& uidRequestMember
		, const MTD_LadderTeamMemberNode* pRequestMemberNode);
	void OnLadderRequestChallenge(const MUID& uidRequestMember, void* pGroupBlob, unsigned long int nOptions, unsigned long int nAntiLead);
	void OnLadderRequestCancelChallenge(const MUID& uidPlayer);

	void OnRequestProposal(const MUID& uidProposer, const int nProposalMode, const int nRequestID,
		const int nReplierCount, void* pReplierNamesBlob);
	void OnReplyAgreement(MUID& uidProposer, MUID& uidReplier, const char* szReplierName,
		const int nProposalMode, const int nRequestID, const bool bAgreement);
protected:
	void OnRequestCopyToTestServer(const MUID& uidPlayer);
	void ResponseCopyToTestServer(const MUID& uidPlayer, const int nResult);

	void OnRequestMySimpleCharInfo(const MUID& uidPlayer);
	void ResponseMySimpleCharInfo(const MUID& uidPlayer);
	void OnRequestCharInfoDetail(const MUID& uidChar, const char* szCharName);
	void ResponseCharInfoDetail(const MUID& uidChar, const char* szCharName);
	void OnRequestAccountCharInfo(const MUID& uidPlayer, int nCharNum);
	void OnRequestSelectChar(const MUID& uidPlayer, const int nCharIndex);
	void OnRequestDeleteChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName);
	bool ResponseDeleteChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName);
	void OnRequestCreateChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName,
		const unsigned int nSex, const unsigned int nHair, const unsigned int nFace,
		const unsigned int nCostume);
	bool ResponseCreateChar(const MUID& uidPlayer, const int nCharIndex, const char* szCharName,
		MMatchSex nSex, const unsigned int nHair, const unsigned int nFace,
		const unsigned int nCostume);
	void OnCharClear(const MUID& uidPlayer);
	bool CharInitialize(const MUID& uidPlayer);
	bool CharFinalize(const MUID& uidPlayer);
	bool CorrectEquipmentByLevel(MMatchObject* pPlayer, MMatchCharItemParts nPart, int nLegalItemLevelDiff = 0);
	bool RemoveExpiredCharItem(MMatchObject* pObject, MUID& uidItem);
	// Custom: Remove invalid equips
	void CheckInvalidItems(MMatchObject* pPlayer);
protected:
	void OnFriendAdd(const MUID& uidPlayer, const char* pszName);
	void OnFriendRemove(const MUID& uidPlayer, const char* pszName);
	void OnFriendList(const MUID& uidPlayer);
	void OnFriendMsg(const MUID& uidPlayer, const char* szMsg);
	void FriendList(const MUID& uidPlayer);

protected:
	int ValidateCreateClan(const char* szClanName, MMatchObject* pMasterObject, MMatchObject** ppSponsorObject);
	void UpdateCharClanInfo(MMatchObject* pObject, const int nCLID, const char* szClanName, const MMatchClanGrade nGrade);

	void OnClanRequestCreateClan(const MUID& uidPlayer, const int nRequestID, const char* szClanName, char** szSponsorNames);
	void OnClanAnswerSponsorAgreement(const int nRequestID, const MUID& uidClanMaster, char* szSponsorCharName, const bool bAnswer);
	void OnClanRequestAgreedCreateClan(const MUID& uidPlayer, const char* szClanName, char** szSponsorNames);
	void OnClanRequestCloseClan(const MUID& uidClanMaster, const char* szClanName);
	void ResponseCloseClan(const MUID& uidClanMaster, const char* szClanName);
	void OnClanRequestJoinClan(const MUID& uidClanAdmin, const char* szClanName, const char* szJoiner);

	void ResponseJoinClan(const MUID& uidClanAdmin, const char* szClanName, const char* szJoiner);
	void OnClanAnswerJoinAgreement(const MUID& uidClanAdmin, const char* szJoiner, const bool bAnswer);

	void OnClanRequestAgreedJoinClan(const MUID& uidClanAdmin, const char* szClanName, const char* szJoiner);
	void ResponseAgreedJoinClan(const MUID& uidClanAdmin, const char* szClanName, const char* szJoiner);

	void OnClanRequestLeaveClan(const MUID& uidPlayer);
	void ResponseLeaveClan(const MUID& uidPlayer);
	void OnClanRequestChangeClanGrade(const MUID& uidClanMaster, const char* szMember, int nClanGrade);
	void ResponseChangeClanGrade(const MUID& uidClanMaster, const char* szMember, int nClanGrade);
	void OnClanRequestExpelMember(const MUID& uidClanAdmin, const char* szMember);
	void ResponseExpelMember(const MUID& uidClanAdmin, const char* szMember);
	void OnClanRequestMsg(const MUID& uidSender, const char* szMsg);
	void OnClanRequestMemberList(const MUID& uidChar);
	void OnClanRequestClanInfo(const MUID& uidChar, const char* szClanName);

	void OnClanRequestEmblemURL(const MUID& uidChar, void* pEmblemURLListBlob);
public:
	MMatchClan* FindClan(const int nCLID);
	void ResponseClanMemberList(const MUID& uidChar);
public:
	int GetLadderTeamIDFromDB(const int nTeamTableIndex, const int* pnMemberCIDArray, const int nMemberCount);
	void SaveLadderTeamPointToDB(const int nTeamTableIndex, const int nWinnerTeamID, const int nLoserTeamID, const bool bIsDrawGame);
	void SaveClanPoint(MMatchClan* pWinnerClan, MMatchClan* pLoserClan, const bool bIsDrawGame,
		const int nRoundWins, const int nRoundLosses, const int nMapID, const int nGameType,
		const int nOneTeamMemberCount, list<MUID>& WinnerObjUIDs,
		const char* szWinnerMemberNames, const char* szLoserMemberNames, float fPointRatio);
	void BroadCastClanRenewVictories(const char* szWinnerClanName, const char* szLoserClanName, const int nVictories);
	void BroadCastClanInterruptVictories(const char* szWinnerClanName, const char* szLoserClanName, const int nVictories);
	void BroadCastDuelRenewVictories(const MUID& chanID, const char* szChampionName, const char* szChannelName, int nRoomNumber, const int nVictories);
	void BroadCastDuelInterruptVictories(const MUID& chanID, const char* szChampionName, const char* szInterrupterName, const int nVictories);
public:
	friend MVoteDiscuss;
	void OnVoteCallVote(const MUID& uidPlayer, const char* pszDiscuss, const char* pszArg);
	void OnVoteYes(const MUID& uidPlayer);
	void OnVoteNo(const MUID& uidPlayer);
	void VoteAbort(const MUID& uidPlayer);

	void OnAdminServerHalt(void);

protected:
	void OnAdminTerminal(const MUID& uidAdmin, const char* szText);
	void OnAdminAnnounce(const MUID& uidAdmin, const char* szChat, unsigned long int nType);
	void OnAdminRequestServerInfo(const MUID& uidAdmin);
	void OnAdminServerHalt(const MUID& uidAdmin);

	void OnAdminRequestKickPlayer(const MUID& uidAdmin, const char* szPlayer);
	void OnAdminRequestMutePlayer(const MUID& uidAdmin, const char* szPlayer, const int nPenaltyHour);
	void OnAdminRequestBlockPlayer(const MUID& uidAdmin, const char* szPlayer, const int nPenaltyHour);

	void OnAdminRequestUpdateAccountUGrade(const MUID& uidAdmin, const char* szPlayer);
	void OnAdminPingToAll(const MUID& uidAdmin);
	void OnAdminRequestSwitchLadderGame(const MUID& uidAdmin, const bool bEnabled);
	void OnAdminHide(const MUID& uidAdmin);
	void OnAdminResetAllHackingBlock(const MUID& uidAdmin);

	void OnEventChangeMaster(const MUID& uidAdmin);
	void OnEventChangePassword(const MUID& uidAdmin, const char* szPassword);
	void OnEventRequestJjang(const MUID& uidAdmin, const char* pszTargetName);
	void OnEventRemoveJjang(const MUID& uidAdmin, const char* pszTargetName);

public:
	void AdminTerminalOutput(const MUID& uidAdmin, const char* szText);
	bool OnAdminExecute(MAdminArgvInfo* pAI, char* szOut);
	void ApplyObjectTeamBonus(MMatchObject* pObject, int nAddedExp);
	void ProcessPlayerXPBP(MMatchStage* pStage, MMatchObject* pPlayer, int nAddedXP, int nAddedBP);
	void ProcessCharPlayInfo(MMatchObject* pPlayer);
	bool DistributeZItem(const MUID& uidPlayer, const unsigned long int nItemID, bool bRentItem, int nRentPeriodHour, int nItemCount);

protected:
	void OnRequestAccountItemList(const MUID& uidPlayer);
	void ResponseAccountItemList(const MUID& uidPlayer);

public:
	void OnDuelSetObserver(const MUID& uidChar);
	void OnDuelQueueInfo(const MUID& uidStage, const MTD_DuelQueueInfo& QueueInfo);
	void OnQuestSendPing(const MUID& uidStage, unsigned long int t);

protected:
	void OnResponseServerStatus(const MUID& uidSender);
	void OnRequestServerHearbeat(const MUID& uidSender);
	void OnResponseServerHeartbeat(const MUID& uidSender);
	void OnRequestConnectMatchServer(const MUID& uidSender);
	void OnResponseConnectMatchServer(const MUID& uidSender);
	void OnRequestKeeperAnnounce(const MUID& uidSender, const char* pszAnnounce);
	void OnRequestStopServerWithAnnounce(const MUID& uidSender);
	void OnResponseStopServerWithAnnounce(const MUID& uidSender);
	void OnRequestSchedule(const MUID& uidSender,
		const int nType,
		const int nYear,
		const int nMonth,
		const int nDay,
		const int nHour,
		const int nMin,
		const int nCount,
		const int nCommand,
		const char* pszAnnounce);
	void OnResponseSchedule(const MUID& uidSender,
		const int nType,
		const int nYear,
		const int nMonth,
		const int nDay,
		const int nHour,
		const int nMin,
		const int nCount,
		const int nCommand,
		const char* pszAnnounce);

protected:
	void OnRequestNPCDead(const MUID& uidSender, const MUID& uidKiller, MUID& uidNPC, MVector& pos);
	void OnQuestRequestDead(const MUID& uidVictim);
	void OnQuestTestRequestNPCSpawn(const MUID& uidPlayer, int nNPCType, int nNPCCount);
	void OnQuestTestRequestClearNPC(const MUID& uidPlayer);
	void OnQuestTestRequestSectorClear(const MUID& uidPlayer);
	void OnQuestTestRequestQuestFinish(const MUID& uidPlayer);
	void OnQuestRequestMovetoPortal(const MUID& uidPlayer);
	void OnQuestReadyToNewSector(const MUID& uidPlayer);
	void OnQuestStageMapset(const MUID& uidStage, int nMapsetID);

	void OnResponseCharQuestItemList(const MUID& uidSender);
	void OnRequestBuyQuestItem(const MUID& uidSender, const unsigned long int nItemID, const int nItemCount);
	void OnResponseBuyQuestItem(const MUID& uidSender, const unsigned long int nItemID, const int nItemCount);
	void OnRequestSellQuestItem(const MUID& uidSender, const unsigned long int nItemID, const int nCount);
	void OnResponseSellQuestItem(const MUID& uidSender, const unsigned long int nItemID, const int nCount);
	void OnRequestDropSacrificeItemOnSlot(const MUID& uidSender, const int nSlotIndex, const unsigned long int nItemID);
	void OnRequestCallbackSacrificeItem(const MUID& uidSender, const int nSlotIndex, const unsigned long int nItemID);
	void OnRequestQL(const MUID& uidSender);
	void OnRequestSacrificeSlotInfo(const MUID& uidSender);
	void OnRequestMonsterBibleInfo(const MUID& uidSender);
	void OnResponseMonsterBibleInfo(const MUID& uidSender);

	void OnQuestPong(const MUID& uidSender);
public:
	void OnRequestCharQuestItemList(const MUID& uidSender);

	int AgentAdd(const MUID& uidComm);
	int AgentRemove(const MUID& uidAgent, MAgentObjectMap::iterator* pNextItor);
	void AgentClear();

	MAgentObject* GetAgent(const MUID& uidAgent);
	MAgentObject* GetAgentByCommUID(const MUID& uidComm);
	MAgentObject* FindFreeAgent();
	void ReserveAgent(MMatchStage* pStage);

protected:
	bool CheckBridgeFault();
	void LocateAgentToClient(const MUID& uidPlayer, const MUID& uidAgent);
	void OnRegisterAgent(const MUID& uidComm, char* szIP, int nTCPPort, int nUDPPort);
	void OnUnRegisterAgent(const MUID& uidComm);
	void OnAgentStageReady(const MUID& uidCommAgent, const MUID& uidStage);
	void OnRequestLiveCheck(const MUID& uidComm, unsigned long nTimeStamp,
		unsigned long nStageCount, unsigned long nUserCount);
public:
	MMatchObject* GetObject(const MUID& uid);
	MMatchObject* GetPlayerByCommUID(const MUID& uid);
	MMatchObject* GetPlayerByName(const char* pszName);
	MMatchObject* GetPlayerByAID(unsigned long int nAID);

	MMatchChannel* FindChannel(const MUID& uidChannel);
	MMatchChannel* FindChannel(const MCHANNEL_TYPE nChannelType, const char* pszChannelName);

	void Announce(const MUID& CommUID, char* pszMsg);
	void Announce(MObject* pObj, char* pszMsg);
	void AnnounceErrorMsg(const MUID& CommUID, const int nErrorCode);
	void AnnounceErrorMsg(MObject* pObj, const int nErrorCode);
	void RouteToListener(MObject* pObject, MCommand* pCommand);
	void RouteToAllConnection(MCommand* pCommand);
	void RouteToAllClient(MCommand* pCommand);
	void RouteToChannel(const MUID& uidChannel, MCommand* pCommand);
	void RouteToChannelLobby(const MUID& uidChannel, MCommand* pCommand);
	void RouteToStage(const MUID& uidStage, MCommand* pCommand);
	void RouteToObjInStage(const MUID& uidStage, const MUID& uidTargetObj, MCommand* pCommand);
	void RouteToStageWaitRoom(const MUID& uidStage, MCommand* pCommand);
	void RouteToBattle(const MUID& uidStage, MCommand* pCommand);
	void RouteToClan(const int nCLID, MCommand* pCommand);
	void RouteResponseToListener(MObject* pObject, const int nCmdID, int nResult);

	void ResponseBridgePeer(const MUID& uidChar, int nCode);
	void ResponseRoundState(const MUID& uidStage);
	void ResponseRoundState(MMatchObject* pObj, const MUID& uidStage);
	void ResponsePeerList(const MUID& uidChar, const MUID& uidStage);
	void ResponseGameInfo(const MUID& uidChar, const MUID& uidStage);

	virtual void ResponseTakeoffItem(const MUID& uidPlayer, const MMatchCharItemParts parts) = 0;
	virtual bool CheckUserCanDistributeRewardItem(MMatchObject* pObj) = 0;
	virtual bool ResponseCharacterItemList(const MUID& uidPlayer) = 0;

	void NotifyMessage(const MUID& uidChar, int nMsgID);

	unsigned long GetChannelListChecksum() { return m_ChannelMap.GetChannelListChecksum(); }
	void ChannelList(const MUID& uidPlayer, MCHANNEL_TYPE nChannelType);

	unsigned long int GetStageListChecksum(MUID& uidChannel, int nStageCursor, int nStageCount);
	void StageList(const MUID& uidPlayer, int nStageStartIndex, bool bCacheUpdate);
	void StageLaunch(const MUID& uidStage);
	void StageRelayLaunch(const MUID& uidStage);
	void StageRelayMapBattleStart(const MUID& uidPlayer, const MUID& uidStage);
	void StageFinishGame(const MUID& uidStage);

	void StandbyClanList(const MUID& uidPlayer, int nClanListStartIndex, bool bCacheUpdate);

	unsigned long int GetGlobalClockCount(void) const;
	void SetClientClockSynchronize(const MUID& CommUID);
	static unsigned long int ConvertLocalClockToGlobalClock(unsigned long int nLocalClock, unsigned long int nLocalClockDistance);
	static unsigned long int ConvertGlobalClockToLocalClock(unsigned long int nGlobalClock, unsigned long int nLocalClockDistance);

	bool IsCreated() { return m_bCreated; }
	inline unsigned long int GetTickTime();
protected:
	int ValidateMakingName(const char* szCharName, int nMinLength, int nMaxLength);

	virtual int ValidateStageJoin(const MUID& uidPlayer, const MUID& uidStage) { return 0; }
	int ValidateChannelJoin(const MUID& uidPlayer, const MUID& uidChannel);
	int ValidateEquipItem(MMatchObject* pObj, MMatchItem* pItem, const MMatchCharItemParts parts);
	int ValidateChallengeLadderGame(MMatchObject** ppMemberObject, int nMemberCount);
	void CheckExpiredItems(MMatchObject* pObj);
	void ResponseExpiredItemIDList(MMatchObject* pObj, vector<unsigned long int>& vecExpiredItemIDList);

	bool LoadInitFile();
	bool LoadChannelPreset();
	bool InitDB();
	void UpdateServerLog();
	void UpdateServerStatusDB();

	void UpdateCharDBCachingData(MMatchObject* pObject);
	void UpdateCharItemDBCachingData(MMatchObject* pObject);

protected:
	unsigned long GetItemFileChecksum() { return m_nItemFileChecksum; }
	void SetItemFileChecksum(unsigned long nChecksum) { m_nItemFileChecksum = nChecksum; }

	bool CheckItemXML();
	bool CheckItemXMLFromDatabase();
	bool CompareMatchItem(MMatchItemDescForDatabase* pItem1, MMatchItemDesc* pItem2);

protected:
	friend bool StageKick(MMatchServer* pServer, const MUID& uidPlayer, const MUID& uidStage, char* pszChat);
	MCountryFilter& GetCountryFilter() { return m_CountryFilter; }
	bool InitCountryFilterDB();
	const CUSTOM_IP_STATUS	CheckIsValidCustomIP(const MUID& CommUID, const string& strIP, string& strCountryCode3, const bool bUseFilter);
	const COUNT_CODE_STATUS CheckIsNonBlockCountry(const MUID& CommUID, const string& strIP, string& strCountryCode3, const bool bUseFilter);

public:
	bool CheckUpdateItemXML();
	IPtoCountryList& GetTmpIPtoCountryList() { return m_TmpIPtoCountryList; }
	BlockCountryCodeList& GetTmpBlockCountryCodeList() { return m_TmpBlockCountryCodeList; }
	CustomIPList& GetTmpCustomIPList() { return m_TmpCustomIPList; }
	void SetUseCountryFilter();
	void SetAccetpInvalidIP();
	void UpdateIPtoCountryList();
	void UpdateBlockCountryCodeLsit();
	void UpdateCustomIPList();
	void ResetBlockCount() { m_dwBlockCount = 0; }
	void ResetNonBlockCount() { m_dwNonBlockCount = 0; }
	void IncreaseBlockCount() { ++m_dwBlockCount = 0; }
	void IncreaseNonBlockCount() { ++m_dwNonBlockCount = 0; }
	DWORD GetBlockCount() { return m_dwBlockCount; }
	DWORD GetNonBlockCount() { return m_dwNonBlockCount; }
	bool CheckIsValidIP(const MUID& CommUID, const string& strIP, string& strCountryCode3, const bool bUseFilter);
public:
	void CustomCheckEventObj(const DWORD dwEventID, MMatchObject* pObj, void* pContext);

	friend bool StageKick(MMatchServer* pServer, const MUID& uidPlayer, const MUID& uidStage, char* pszChat);

public:
	MLadderMgr* GetLadderMgr() { return &m_LadderMgr; }
	MMatchObjectList* GetObjects() { return &m_Objects; }
	MMatchStageMap* GetStageMap() { return &m_StageMap; }
	MMatchChannelMap* GetChannelMap() { return &m_ChannelMap; }
	MMatchClanMap* GetClanMap() { return &m_ClanMap; }
	MMatchDBMgr* GetDBMgr() { return &m_MatchDBMgr; }
	MMatchQuest* GetQuest() { return &m_Quest; }
	MMatchDuelTournamentMgr* GetDTMgr() { return m_pDTMgr; }
	int GetClientCount() { return (int)m_Objects.size(); }
	int GetAgentCount() { return (int)m_AgentMap.size(); }

	virtual void	XTrap_OnAdminReloadFileHash(const MUID& uidAdmin) {}
	virtual bool	PreCheckAddObj(const MUID& uidObj) { return true; }

	virtual ULONG	HShield_MakeGuidReqMsg(unsigned char* pbyGuidReqMsg, unsigned char* pbyGuidReqInfo) { return 0L; }
	virtual ULONG	HShield_AnalyzeGuidAckMsg(unsigned char* pbyGuidAckMsg, unsigned char* pbyGuidReqInfo, unsigned long** ppCrcInfo) { return 0L; }
	virtual ULONG   HShield_MakeReqMsg(unsigned long* pCrcInfo, unsigned char* pbyReqMsg, unsigned char* pbyReqInfo, unsigned long ulOption) { return 0L; }
	virtual ULONG   HShield_AnalyzeAckMsg(unsigned long* pCrcInfo, unsigned char* pbyAckMsg, unsigned char* pbyReqInfo) { return 0L; }

	void SendHShieldReqMsg();

public:
	void RequestGameguardAuth(const MUID& uidUser, const DWORD dwIndex, const DWORD dwValue1, const DWORD dwValue2, const DWORD dwValue3);
	void RequestFirstGameguardAuth(const MUID& uidUser, const DWORD dwIndex, const DWORD dwValue1, const DWORD dwValue2, const DWORD dwValue3);

	virtual bool IsSkeepByGameguardFirstAuth(MCommand* pCommand) { return true; }
	const MMatchGambleMachine& GetGambleMachine() const { return m_GambleMachine; }
	MMatchGambleMachine& GetGambleMachine() { return m_GambleMachine; }

	void CheckMemoryTest(int nState = 0, int nValue = 0);

	void LogObjectCommandHistory(MUID uid);

	void CheckMemoryCorruption();

	bool OnRequestSurvivalModeGroupRanking();
	bool OnRequestSurvivalModePrivateRanking(const MUID& uidStage, const MUID& uidPlayer, DWORD dwScenarioID, DWORD dwCID);
	bool OnPostSurvivalModeGameLog();

	void OnAsyncSurvivalModeGroupRanking(MAsyncJob* pJobResult);
	void OnAsyncSurvivalModePrivateRanking(MAsyncJob* pJobResult);
	void OnAsyncSurvivalModeGameLog(MAsyncJob* pJobResult);

	virtual void Log(unsigned int nLogLevel, const char* szLog);

protected:
	bool DuelTournamentJoin(const MUID& uidPlayer, const MUID& uidStage);

	void ResponseDuelTournamentCharSideRanking(MUID& uidPlayer);
	void ResponseChannelJoin(MUID uidPlayer, MUID uidChannel, int nChannelType, const char* szChannelStrResId, bool bEnableInterface);
	void ResponseDuelTournamentCharStatusInfo(MUID& uidPlayer, MCommand* pCommand);
	void ResponseDuelTournamentJoinChallenge(MUID& uidPlayer, MDUELTOURNAMENTTYPE nType);
	void ResponseDuelTournamentCancelChallenge(MUID& uidPlayer, MDUELTOURNAMENTTYPE nType);

	bool OnSyncRequest_InsertDuelTournamentGameLog(MDUELTOURNAMENTTYPE nDTType, int nMatchFactor, MDuelTournamentPickedGroup* pPickedGroup, int* nOutNumber, char* szOutTimeStamp);

	void OnAsyncResponse_GetDuelTournamentTimeStamp(MAsyncJob* pJobResult);
	void OnAsyncResponse_GetDuelTournamentCharacterInfo(MAsyncJob* pJobResult);
	void OnAsyncResponse_GetDuelTournamentPreviousCharacterInfo(MAsyncJob* pJobResult);
	void OnAsyncResponse_GetDuelTournamentSideRanking(MAsyncJob* pJobResult);
	void OnAsyncResponse_GetDuelTournamentGroupRanking(MAsyncJob* pJobResult);

	bool OnAsyncRequest_GetDuelTournamentCharacterInfo(MUID uidPlayer, DWORD dwCID);
	bool OnAsyncRequest_GetDuelTournamentPreviousCharacterInfo(MUID uidPlayer, DWORD dwCID);
	bool OnAsyncRequest_GetDuelTournamentSideRankingInfo(MUID uidPlayer, DWORD dwCID);
	bool OnAsyncRequest_GetDuelTournamentGroupRankingInfo();

	void PostCmdDuelTournamentChallenge(MUID uidPlayer, int nResult);
	void PostCmdDuelTournamentCharInfo(MUID uidPlayer, MMatchObjectDuelTournamentCharInfo* pDTCharInfo);
	void PostCmdDuelTournamentCharInfoPrevious(MUID uidPlayer, int nPrevTP, int nPrevWins, int nPrevLoses, int nPrevRanking, int nPrevFinalWins);
	void PostCmdDuelTournamentCharSideRankingInfo(MUID uidPlayer, list<DTRankingInfo*>* pSideRankingList);
	void PostCmdDuelTournamentCancelMatch(MUID uidPlayer, int nErrorCode);

	void RouteCmdDuelTournamentPrepareMatch(MDUELTOURNAMENTTYPE nType, MUID uidStage, MDuelTournamentPickedGroup* pPickedGroup);
	void RouteCmdDuelTournamentLaunchMatch(MUID uidStage);
	void RouteCmdDuelTournamentCancelMatch(MDuelTournamentPickedGroup* pPickedGroup, int nErrorCode);

public:
	void LaunchDuelTournamentMatch(MDUELTOURNAMENTTYPE nType, MDuelTournamentPickedGroup* pPickedGroup, MDUELTOURNAMENTMATCHMAKINGFACTOR matchFactor);

	void SendDuelTournamentPreviousCharInfoToPlayer(MUID uidPlayer);
	void SendDuelTournamentCharInfoToPlayer(MUID uidPlayer);
	void SendDuelTournamentServiceTimeClose(const MUID& uidPlayer);

	bool OnAsyncRequest_GetDuelTournamentTimeStamp();
	bool OnAsyncRequest_UpdateDuelTournamentCharacterInfo(MUID uidPlayer, char* szTimeStamp);
	bool OnAsyncRequest_UpdateDuelTournamentGameLog(char* szTimeStamp, int nGameNumber, MUID uidChampion);
	bool OnAsyncRequest_InsertDuelTournamentGameLogDetail(int nLogID, char* szTimeStamp, MDUELTOURNAMENTROUNDSTATE nDTRoundState, int nWinnerCID, int nLoserCID, int nGainTP, int nLoseTp, int nPlayTime);

	void RouteCmdDuelTournamentStageSetting(MUID uidStage);
	void RouteCmdDuelTournamentMTDGameInfo(const MUID& uidStage, MTD_DuelTournamentGameInfo& GameInfo);
	void RouteCmdDuelTournamentMTDNextGamePlayerInfo(const MUID& uidStage, MTD_DuelTournamentNextMatchPlayerInfo& PlayerInfo);
	void RouteCmdDuelTournamentMTDRoundResultInfo(const MUID& uidStage, MTD_DuelTournamentRoundResultInfo* RoundResultInfo);
	void RouteCmdDuelTournamentMTDMatchResultInfo(const MUID& uidStage, MTD_DuelTournamentMatchResultInfo* MatchResultInfo);

public:
protected:
	void UseSpendableItem(const MUID& uidPlayer, const MUID& uidItem);
	void OnRequestUseSpendableNormalItem(const MUID& uidPlayer, const MUID& uidItem);
protected:
	MMatchBRMachine m_BattletimeRewardMachine;

	bool InitBattletimeRewardMachine();
	void MakeBattleTimeRewardDescriptionMap(vector<MMatchBRDescription*>& vBattletimeRewardDescription,
		vector<MMatchBRItem*>& vBattletimeRewardItem,
		MMatchBRDescriptionMap& BattletimeRewardDescriptionMap);

	void OnAsyncResponse_GetBR_Description(MAsyncJob* pJobResult);
	void OnAsyncResponse_GetCharBRInfo(MAsyncJob* pJobResult);
	void OnAsyncResponse_UpdateCharBRInfo(MAsyncJob* pJobResult);
	void OnAsyncResponse_RewardCharBR(MAsyncJob* pJobResult);

public:
	MMatchBRMachine& GetBattleTimeRewardMachine() { return m_BattletimeRewardMachine; }

	void RouteCmdBattleTimeReward(const MUID& uidPlayer, MUID& uidStage, const char* pszName, const char* pszResetDesc, int nItemID, int nItemCnt, int nRentHourPeriod, int nRemainRewardCnt);

	void OnAsyncRequest_RewardCharBP(const MUID& uidPlayer, int nBRID, int nBRTID, int nRewardCount, int nBattleTime, int nKillCount, int nItemID, int nItemCnt, int nRentHourPeriod);
	void OnAsyncRequest_UpdateCharBRInfo(const MUID& uidPlayer, int nBRID, int nBRTID, int nRewardCount, int nBattleTime, int nKillCount);
};

void CopyCharInfoForTrans(MTD_CharInfo* pDest, MMatchCharInfo* pSrc, MMatchObject* pSrcObject);
void CopyCharInfoDetailForTrans(MTD_CharInfo_Detail* pDest, MMatchCharInfo* pSrcCharInfo, MMatchObject* pSrcObject);

inline MMatchServer* MGetMatchServer()
{
	return MMatchServer::GetInstance();
}

inline unsigned long int MMatchServer::GetTickTime()
{
	m_csTickTimeLock.Lock();
	unsigned long int ret = m_nTickTime;
	m_csTickTimeLock.Unlock();
	return ret;
}

inline void MMatchServer::SetTickTime(unsigned long int nTickTime)
{
	m_csTickTimeLock.Lock();
	m_nTickTime = nTickTime;
	m_csTickTimeLock.Unlock();
}

inline const char* MErrStr(const int nID)
{
	return MGetStringResManager()->GetErrorStr(nID);
}

inline void MMatchServer::LogObjectCommandHistory(MUID uid)
{
	m_objectCommandHistory.Dump(uid);
}

bool IsExpiredBlockEndTime(const SYSTEMTIME& st);

void _CheckValidPointer(void* pPointer1, void* pPointer2, void* pPointer3, int nState, int nValue);
#define CheckValidPointer(A, B)		_CheckValidPointer(m_pMessengerManager, m_pScheduler, m_pAuthBuilder, A, B);CheckMemoryTest(A, B);
#define CheckValidPointer2(A, B)	MMatchServer::GetInstance()->CheckMemoryTest(A, B);

#endif