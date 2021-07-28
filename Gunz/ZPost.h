#ifndef _ZPOST_H
#define _ZPOST_H

#include "ZPrerequisites.h"
#include "ZGameClient.h"
#include "MBlobArray.h"
#include "MMatchTransDataType.h"
#include "MMath.h"
#include "CheckReturnCallStack.h"

#pragma pack(1)

inline bool IsNaN(float x) { return x != x; }
inline bool IsNaN(const rvector& v) { return (v.x != v.x || v.y != v.y || v.z != v.z); }

struct ZPACKEDBASICINFO {
	float	fTime;
	short	posx, posy, posz;
	short	velx, vely, velz;
	short	dirx, diry, dirz;
	BYTE	upperstate;
	BYTE	lowerstate;
	BYTE	selweapon;
};

struct ZPACKEDSHOTINFO {
	float	fTime;
	short	posx, posy, posz;
	short	tox, toy, toz;
	BYTE	sel_type;
};

struct ZPACKEDDASHINFO {
	short	posx, posy, posz;
	short	dirx, diry, dirz;
	BYTE	seltype;
};

#pragma pack()

#define CLOAK_CMD_ID(CMD_ID, FACTOR)	UncloakCmdId((CMD_ID) + (FACTOR), FACTOR)
int UncloakCmdId(int cloakedCmdId, int cloakFactor);

inline void ZPostHPInfo(float fHP)
{
	ZPOSTCMD1(MC_PEER_HPINFO, MCmdParamFloat(fHP));
}

inline void ZPostHPAPInfo(float fHP, float fAP)
{
	ZPOSTCMD2(MC_PEER_HPAPINFO, MCmdParamFloat(fHP), MCmdParamFloat(fAP));
}

inline void ZPostDuelTournamentHPAPInfo(UCHAR MaxHP, UCHAR MaxAP, UCHAR HP, UCHAR AP)
{
	ZPOSTCMD4(MC_PEER_DUELTOURNAMENT_HPAPINFO, MCmdParamUChar(MaxHP), MCmdParamUChar(MaxAP), MCmdParamUChar(HP), MCmdParamUChar(AP));
}

inline void ZPostMove(rvector& vPos, rvector& vDir, rvector& vVelocity,
	ZC_STATE_UPPER upper, ZC_STATE_LOWER lower)
{
	ZPOSTCMD5(MC_PEER_MOVE, MCommandParameterPos(vPos.x, vPos.y, vPos.z), MCommandParameterVector(vDir.x, vDir.y, vDir.z), MCommandParameterVector(vVelocity.x, vVelocity.y, vVelocity.z), MCommandParameterInt(int(upper)), MCommandParameterInt(int(lower)));
}

#define ZPostSkill( nSkill, sel_type)	{ \
	ZPOSTCMD3(CLOAK_CMD_ID(MC_PEER_SKILL, 7636), MCmdParamFloat(ZGetGame()->GetTime()), MCmdParamInt(nSkill),MCommandParameterInt(sel_type)); \
}

inline void ZPostChangeWeapon(int WeaponID)
{
	ZPOSTCMD1(MC_PEER_CHANGE_WEAPON, MCmdParamInt(WeaponID));
}

#define ZPostSpMotion( type) { ZPOSTCMD1(CLOAK_CMD_ID(MC_PEER_SPMOTION, 123),MCmdParamInt(type)); }

inline void ZPostChangeParts(int PartsType, int PartsID)
{
	ZPOSTCMD2(MC_PEER_CHANGE_PARTS, MCmdParamInt(PartsType), MCmdParamInt(PartsID));
}

inline void ZPostChangeCharacter()
{
	ZPOSTCMD0(MC_PEER_CHANGECHARACTER);
}

inline void ZPostAttack(int type, rvector& vPos)
{
	ZPOSTCMD2(MC_PEER_ATTACK, MCmdParamInt(type), MCommandParameterVector(vPos.x, vPos.y, vPos.z));
}

inline void ZPostDamage(MUID ChrUID, int damage)
{
	ZPOSTCMD2(MC_PEER_DAMAGE, MCmdParamUID(ChrUID), MCmdParamInt(damage));
}

#define ZPostDie( uidAttacker) { ZPOSTCMD1(CLOAK_CMD_ID(MC_PEER_DIE, 3421), MCmdParamUID(uidAttacker)); }

#ifdef _DEBUG
inline void ZPostSpawn(rvector& vPos, rvector& vDir)
{
	ZPOSTCMD2(MC_PEER_SPAWN, MCommandParameterPos(vPos.x, vPos.y, vPos.z), MCommandParameterDir(vDir.x, vDir.y, vDir.z));
}
#endif

inline void ZPostRequestSpawn(MUID uidChar, rvector& vPos, rvector& vDir)
{
	ZPOSTCMD3(MC_MATCH_GAME_REQUEST_SPAWN, MCmdParamUID(uidChar),
		MCmdParamPos(vPos.x, vPos.y, vPos.z), MCmdParamDir(vDir.x, vDir.y, vDir.z));
}

#define ZPostDash( vPos,  vDir,   sel_type) { 	\
	ZPACKEDDASHINFO pdi;		\
	pdi.posx = Roundf(vPos.x);	\
	pdi.posy = Roundf(vPos.y);	\
	pdi.posz = Roundf(vPos.z);	\
								\
	pdi.dirx = vDir.x*32000;	\
	pdi.diry = vDir.y*32000;	\
	pdi.dirz = vDir.z*32000;	\
								\
	pdi.seltype = sel_type;		\
								\
	ZPOSTCMD1(CLOAK_CMD_ID(MC_PEER_DASH, 9964),MCommandParameterBlob(&pdi,sizeof(ZPACKEDDASHINFO)));	\
}

inline void ZPostPeerChat(char* szMsg, int nTeam = 0)
{
	char szSendMsg[256] = { 0, };

	const size_t nInputLen = strlen(szMsg);
	if (256 > nInputLen)
	{
		ZPOSTCMD2(MC_PEER_CHAT, MCmdParamInt(nTeam), MCommandParameterString(szMsg))
	}
	else
	{
		strncpy(szSendMsg, szMsg, 255);
		ZPOSTCMD2(MC_PEER_CHAT, MCmdParamInt(nTeam), MCommandParameterString(szSendMsg))
	}
}

inline void ZPostPeerChatIcon(bool bShow)
{
	ZPOSTCMD1(MC_PEER_CHAT_ICON, MCmdParamBool(bShow));
}

inline void ZPostReload()
{
	ZPOSTCMD0(MC_PEER_RELOAD);
}

inline void ZPostPeerEnchantDamage(MUID ownerUID, MUID targetUID)
{
	ZPOSTCMD2(MC_PEER_ENCHANT_DAMAGE, MCmdParamUID(ownerUID), MCmdParamUID(targetUID));
}

inline void ZPostConnect(const char* szAddr, unsigned int nPort)
{
	char szCmd[256];
	sprintf(szCmd, "%s:%u", szAddr, nPort);
	ZPOSTCMD1(MC_NET_CONNECT, MCmdParamStr(szCmd));
}

inline void ZPostDisconnect()
{
	ZPOSTCMD0(MC_NET_DISCONNECT);
}

inline void ZPostLogin(char* szUserID, char* szPassword, unsigned int nChecksumPack, char* szEncryptMD5Value)
{
	void* pBlob = MMakeBlobArray(MAX_MD5LENGH, 1);
	unsigned char* pCmdBlock = (unsigned char*)MGetBlobArrayElement(pBlob, 0);
	CopyMemory(pCmdBlock, szEncryptMD5Value, MAX_MD5LENGH);

	ZPOSTCMD5(MC_MATCH_LOGIN, MCmdParamStr(szUserID)
		, MCmdParamStr(szPassword)
		, MCmdParamInt(MCOMMAND_VERSION)
		, MCmdParamUInt(nChecksumPack)
		, MCommandParameterBlob(pBlob, MGetBlobArraySize(pBlob)));
	MEraseBlobArray(pBlob);
}

inline void ZPostChannelRequestJoin(const MUID& uidChar, const MUID& uidChannel)
{
	ZPOSTCMD2(MC_MATCH_CHANNEL_REQUEST_JOIN, MCommandParameterUID(uidChar), MCommandParameterUID(uidChannel));
}

inline void ZPostChannelRequestJoinFromChannelName(const MUID& uidChar, int nChannelType, const char* szChannelName)
{
	if (strlen(szChannelName) >= CHANNELNAME_LEN) return;
	ZPOSTCMD3(MC_MATCH_CHANNEL_REQUEST_JOIN_FROM_NAME, MCmdParamUID(uidChar), MCmdParamInt(nChannelType), MCmdParamStr(szChannelName));
}

#define ZPostChannelChat(  uidChar,   uidChannel,  szChat) { \
	ZPOSTCMD3(CLOAK_CMD_ID(MC_MATCH_CHANNEL_REQUEST_CHAT, 87252), MCommandParameterUID(uidChar), MCommandParameterUID(uidChannel), MCmdParamStr(szChat)); \
}

inline void ZPostStartChannelList(const MUID& uidChar, const int nChannelType)
{
	ZPOSTCMD2(MC_MATCH_CHANNEL_LIST_START, MCommandParameterUID(uidChar), MCommandParameterInt(nChannelType));
}

inline void ZPostStopChannelList(const MUID& uidChar)
{
	ZPOSTCMD1(MC_MATCH_CHANNEL_LIST_STOP, MCommandParameterUID(uidChar));
}

inline void ZPostStageCreate(const MUID& uidChar, char* szStageName, bool bPrivate, char* szPassword)
{
	ZPOSTCMD4(MC_MATCH_STAGE_CREATE, MCommandParameterUID(uidChar), MCmdParamStr(szStageName),
		MCmdParamBool(bPrivate), MCmdParamStr(szPassword));
}

inline void ZPostRequestStageJoin(const MUID& uidChar, const MUID& uidStage)
{
	ZPOSTCMD2(MC_MATCH_REQUEST_STAGE_JOIN, MCommandParameterUID(uidChar), MCommandParameterUID(uidStage));
}

inline void ZPostStageGo(int nRoomNo)
{
	ZPOSTCMD1(MC_MATCH_STAGE_GO, MCmdParamUInt(nRoomNo));
}

inline void ZPostRequestPrivateStageJoin(const MUID& uidChar, const MUID& uidStage, char* szPassword)
{
	ZPOSTCMD3(MC_MATCH_REQUEST_PRIVATE_STAGE_JOIN, MCommandParameterUID(uidChar), MCommandParameterUID(uidStage),
		MCmdParamStr(szPassword));
}

inline void ZPostStageLeave(const MUID& uidChar)
{
	ZPOSTCMD1(MC_MATCH_STAGE_LEAVE, MCommandParameterUID(uidChar));
}

inline void ZPostRequestStagePlayerList(const MUID& uidStage)
{
	ZPOSTCMD1(MC_MATCH_STAGE_REQUEST_PLAYERLIST, MCommandParameterUID(uidStage));
}

inline void ZPostStageFollow(const char* pszTargetName)
{
	ZPOSTCMD1(MC_MATCH_STAGE_FOLLOW, MCommandParameterString(const_cast<char*>(pszTargetName)));
}

inline void ZPostStageStart(const MUID& uidChar, const MUID& uidStage)
{
	ZPOSTCMD3(MC_MATCH_STAGE_START, MCommandParameterUID(uidChar), MCommandParameterUID(uidStage), MCommandParameterInt(3));
}

inline void ZPostStageMap(const MUID& uidStage, char* szMap)
{
	ZPOSTCMD2(MC_MATCH_STAGE_MAP, MCommandParameterUID(uidStage), MCommandParameterString(szMap));
}

inline void ZPostStageRelayMapElementUpdate(const MUID& uidStage, int nType, int nRoundCount)
{
	ZPOSTCMD3(MC_MATCH_STAGE_RELAY_MAP_ELEMENT_UPDATE, MCommandParameterUID(uidStage), MCommandParameterInt(nType), MCommandParameterInt(nRoundCount));
}

inline void ZPostStageRelayMapInfoUpdate(const MUID& uidStage, int nType, int nRoundCount, void* pBlob)
{
	ZPOSTCMD4(MC_MATCH_STAGE_RELAY_MAP_INFO_UPDATE, MCommandParameterUID(uidStage), MCommandParameterInt(nType), MCommandParameterInt(nRoundCount), MCommandParameterBlob(pBlob, MGetBlobArraySize(pBlob)));
}

inline void ZPostStageChat(const MUID& uidChar, const MUID& uidStage, char* szChat)
{
	ZPOSTCMD3(MC_MATCH_STAGE_CHAT, MCommandParameterUID(uidChar), MCommandParameterUID(uidStage), MCmdParamStr(szChat));
}

inline void ZPostRequestStageSetting(const MUID& uidStage)
{
	ZPOSTCMD1(MC_MATCH_REQUEST_STAGESETTING, MCommandParameterUID(uidStage));
}

inline void ZPostStageSetting(const MUID& uidChar, const MUID& uidStage, MSTAGE_SETTING_NODE* pSetting)
{
	void* pBlob = MMakeBlobArray(sizeof(MSTAGE_SETTING_NODE), 1);
	MSTAGE_SETTING_NODE* pBlobNode = (MSTAGE_SETTING_NODE*)MGetBlobArrayElement(pBlob, 0);
	CopyMemory(pBlobNode, pSetting, sizeof(MSTAGE_SETTING_NODE));
	ZPOSTCMD3(MC_MATCH_STAGESETTING, MCommandParameterUID(uidChar), MCommandParameterUID(uidStage), MCommandParameterBlob(pBlob, MGetBlobArraySize(pBlob)));
	MEraseBlobArray(pBlob);
}

inline void ZPostStageTeam(const MUID& uidChar, const MUID& uidStage, int nTeam)
{
	ZPOSTCMD3(MC_MATCH_STAGE_TEAM, MCommandParameterUID(uidChar), MCommandParameterUID(uidStage), MCommandParameterUInt(nTeam));
}

inline void ZPostStageState(const MUID& uidChar, const MUID& uidStage, MMatchObjectStageState nStageState)
{
	ZPOSTCMD3(MC_MATCH_STAGE_PLAYER_STATE, MCommandParameterUID(uidChar), MCommandParameterUID(uidStage), MCommandParameterInt(int(nStageState)));
}

#define ZPostShot( pos,  to,  sel_type) {	\
	ZPACKEDSHOTINFO info;					\
	info.fTime=ZGetGame()->GetTime();		\
	info.posx = pos.x;						\
	info.posy = pos.y;						\
	info.posz = pos.z;						\
	info.tox = to.x;						\
	info.toy = to.y;						\
	info.toz = to.z;						\
	info.sel_type = sel_type;				\
	ZPOSTCMD1(CLOAK_CMD_ID(MC_PEER_SHOT, 1621), MCommandParameterBlob(&info,sizeof(ZPACKEDSHOTINFO)));	\
}

#define ZPostShotMelee( pos,  nShot) {	\
	ZPOSTCMD3(CLOAK_CMD_ID(MC_PEER_SHOT_MELEE, 38274), MCommandParameterFloat(ZGetGame()->GetTime()),MCommandParameterPos(pos.x, pos.y, pos.z),MCommandParameterInt(nShot));	\
}

#define ZPostDamageCounter( Damage,  AttackerUID) {	\
ZPOSTCMD2(CLOAK_CMD_ID(MC_GUNZ_DAMAGECOUNTER, 1621), MCommandParameterInt(Damage), MCommandParameterUID(AttackerUID));	\
}

inline void ZPostNPCRangeShot(MUID uidOwner, float fShotTime, rvector& pos, rvector& to, int sel_type)
{
	ZPACKEDSHOTINFO info;
	info.fTime = fShotTime;
	info.posx = pos.x;
	info.posy = pos.y;
	info.posz = pos.z;
	info.tox = to.x;
	info.toy = to.y;
	info.toz = to.z;
	info.sel_type = sel_type;

	ZPOSTCMD2(MC_QUEST_PEER_NPC_ATTACK_RANGE, MCommandParameterUID(uidOwner), MCommandParameterBlob(&info, sizeof(ZPACKEDSHOTINFO)));
}

inline void ZPostNPCSkillStart(MUID uidOwner, int nSkill, MUID uidTarget, rvector& targetPos)
{
	ZPOSTCMD4(MC_QUEST_PEER_NPC_SKILL_START, MCommandParameterUID(uidOwner), MCommandParameterInt(nSkill), MCommandParameterUID(uidTarget), MCommandParameterPos(targetPos.x, targetPos.y, targetPos.z));
}

inline void ZPostNPCSkillExecute(MUID uidOwner, int nSkill, MUID uidTarget, rvector& targetPos)
{
	ZPOSTCMD4(MC_QUEST_PEER_NPC_SKILL_EXECUTE, MCommandParameterUID(uidOwner), MCommandParameterInt(nSkill), MCommandParameterUID(uidTarget), MCommandParameterPos(targetPos.x, targetPos.y, targetPos.z));
}

#define ZPostShotSp( vPos,  vDir,  type,  sel_type) {	\
	if (!IsNaN(vPos) && !IsNaN(vDir)) { \
		ZPOSTCMD5(CLOAK_CMD_ID(MC_PEER_SHOT_SP, 12783), MCommandParameterFloat(ZGetGame()->GetTime()), MCommandParameterPos(vPos.x, vPos.y, vPos.z), MCommandParameterVector(vDir.x, vDir.y, vDir.z), MCommandParameterInt(type), MCommandParameterInt(sel_type));	\
	} \
}

#define ZPostNotifyThrowedTrap( itemId) { \
	ZPOSTCMD1(CLOAK_CMD_ID(MC_MATCH_NOTIFY_THROW_TRAPITEM, 19191), MCommandParameterShort(itemId)); \
}

#define ZPostNotifyActivatedTrap( itemId,  pos) { \
	ZPOSTCMD2(CLOAK_CMD_ID(MC_MATCH_NOTIFY_ACTIVATED_TRAPITEM, 95737), MCommandParameterShort(itemId), MCmdParamShortVector(pos.x, pos.y, pos.z)); \
}

#define ZPostBuffInfo( pBlob) { \
	ZPOSTCMD1(MC_PEER_BUFF_INFO, MCommandParameterBlob(pBlob, MGetBlobArraySize(pBlob))); \
}

inline void ZPostReaction(float fTime, int id)
{
	ZPOSTCMD2(MC_PEER_REACTION, MCommandParameterFloat(fTime), MCommandParameterInt(id));
}

inline void ZPostLoadingComplete(const MUID& uidChar, int nPercent)
{
	ZPOSTCMD2(MC_MATCH_LOADING_COMPLETE, MCommandParameterUID(uidChar), MCmdParamInt(nPercent));
}

inline void ZPostStageEnterBattle(const MUID& uidChar, const MUID& uidStage)
{
	ZPOSTCMD2(MC_MATCH_STAGE_REQUEST_ENTERBATTLE, MCommandParameterUID(uidChar), MCommandParameterUID(uidStage));
}

#define ZPostStageLeaveBattle(  uidChar, bGameFinishLeaveBattle) { \
	ZPOSTCMD2(CLOAK_CMD_ID(MC_MATCH_STAGE_LEAVEBATTLE_TO_SERVER, 19472), MCommandParameterUID(uidChar), MCmdParamBool(bGameFinishLeaveBattle));	\
}

inline void ZPostRequestPeerList(const MUID& uidChar, const MUID& uidStage)
{
	ZPOSTCMD2(MC_MATCH_REQUEST_PEERLIST, MCommandParameterUID(uidChar), MCommandParameterUID(uidStage));
}
#define ZPostGameKill(  uidAttacker) {	\
	ZPOSTCMD1(CLOAK_CMD_ID(MC_MATCH_GAME_KILL, 27348), MCommandParameterUID(uidAttacker));	\
}

inline void ZPostRequestTimeSync(unsigned long nTimeStamp)
{
	ZPOSTCMD1(MC_MATCH_GAME_REQUEST_TIMESYNC, MCmdParamUInt(nTimeStamp));
}

#define ZPostAccountCharList(   pbyGuidAckMsg) {			\
	void* pBlob = MMakeBlobArray(sizeof(unsigned char), SIZEOF_GUIDACKMSG);		\
	unsigned char* pCmdBlock = (unsigned char*)MGetBlobArrayElement(pBlob, 0);	\
	CopyMemory(pCmdBlock, pbyGuidAckMsg, SIZEOF_GUIDACKMSG);					\
	ZPOSTCMD1(CLOAK_CMD_ID(MC_MATCH_REQUEST_ACCOUNT_CHARLIST, 57821), MCommandParameterBlob(pBlob, MGetBlobArraySize(pBlob)));	\
	MEraseBlobArray(pBlob);	\
}

inline void ZPostAccountCharInfo(int nCharNum)
{
	ZPOSTCMD1(MC_MATCH_REQUEST_ACCOUNT_CHARINFO, MCommandParameterChar((char)nCharNum));
}

#define ZPostSelectMyChar(  uidChar,   nCharIndex) {	\
	ZPOSTCMD2(CLOAK_CMD_ID(MC_MATCH_REQUEST_SELECT_CHAR, 33333), MCommandParameterUID(uidChar), MCommandParameterUInt(nCharIndex));	\
}

inline void ZPostDeleteMyChar(const MUID& uidChar, const int nCharIndex, char* szCharName)
{
	ZPOSTCMD3(MC_MATCH_REQUEST_DELETE_CHAR, MCommandParameterUID(uidChar), MCommandParameterUInt(nCharIndex),
		MCommandParameterString(szCharName));
}

inline void ZPostCreateMyChar(const MUID& uidChar, const int nCharIndex, char* szCharName,
	const int nSex, const int nHair, const int nFace, const int nCostume)
{
	ZPOSTCMD7(MC_MATCH_REQUEST_CREATE_CHAR, MCommandParameterUID(uidChar), MCommandParameterUInt(nCharIndex),
		MCommandParameterString(szCharName), MCommandParameterUInt(nSex), MCommandParameterUInt(nHair),
		MCommandParameterUInt(nFace), MCommandParameterUInt(nCostume));
}

inline void ZPostSimpleCharInfo(const MUID& uidChar)
{
	ZPOSTCMD1(MC_MATCH_REQUEST_SIMPLE_CHARINFO, MCommandParameterUID(uidChar));
}

inline void ZPostFinishedRoundInfo(const MUID& uidStage, MUID& uidChar, MTD_RoundPeerInfo* pPeerInfo,
	int nPeerInfoCount, MTD_RoundKillInfo* pKillInfo, int nKillInfoCount)
{
	void* pBlobPeerInfo = MMakeBlobArray(sizeof(MTD_RoundPeerInfo), nPeerInfoCount);
	for (int i = 0; i < nPeerInfoCount; i++)
	{
		MTD_RoundPeerInfo* pNodePeerInfo = (MTD_RoundPeerInfo*)MGetBlobArrayElement(pBlobPeerInfo, i);
		CopyMemory(pNodePeerInfo, &pPeerInfo[i], sizeof(MTD_RoundPeerInfo));
	}

	void* pBlobKillInfo = MMakeBlobArray(sizeof(MTD_RoundKillInfo), nKillInfoCount);
	for (int i = 0; i < nKillInfoCount; i++)
	{
		MTD_RoundKillInfo* pNodeKillInfo = (MTD_RoundKillInfo*)MGetBlobArrayElement(pBlobKillInfo, i);
		CopyMemory(pNodeKillInfo, &pKillInfo[i], sizeof(MTD_RoundKillInfo));
	}

	ZPOSTCMD4(MC_MATCH_ROUND_FINISHINFO, MCommandParameterUID(uidStage), MCommandParameterUID(uidChar),
		MCommandParameterBlob(pBlobPeerInfo, MGetBlobArraySize(pBlobPeerInfo)),
		MCommandParameterBlob(pBlobKillInfo, MGetBlobArraySize(pBlobKillInfo)));

	MEraseBlobArray(pBlobPeerInfo);
	MEraseBlobArray(pBlobKillInfo);
}

inline void ZPostRequestBuyItem(const MUID& uidChar, unsigned int nItemID, unsigned int nItemCnt = 1)
{
	ZPOSTCMD3(MC_MATCH_REQUEST_BUY_ITEM, MCommandParameterUID(uidChar), MCommandParameterUInt(nItemID), MCommandParameterUInt(nItemCnt));
}

inline void ZPostRequestSellItem(const MUID& uidChar, const MUID& uidItem, unsigned int nItemCnt = 1)
{
	ZPOSTCMD3(MC_MATCH_REQUEST_SELL_ITEM, MCommandParameterUID(uidChar), MCommandParameterUID(uidItem), MCommandParameterUInt(nItemCnt));
}

inline void ZPostRequestForcedEntry(const MUID& uidChar, const MUID& uidStage)
{
	ZPOSTCMD2(MC_MATCH_STAGE_REQUEST_FORCED_ENTRY, MCommandParameterUID(uidChar), MCommandParameterUID(uidStage));
}

inline void ZPostRequestShopItemList(const MUID& uidChar, const int nFirstItemIndex, const int nItemCount)
{
	ZPOSTCMD3(MC_MATCH_REQUEST_SHOP_ITEMLIST, MCommandParameterUID(uidChar),
		MCommandParameterInt(nFirstItemIndex), MCommandParameterInt(nItemCount));
}

inline void ZPostRequestCharacterItemList(const MUID& uidChar)
{
	ZPOSTCMD1(MC_MATCH_REQUEST_CHARACTER_ITEMLIST, MCommandParameterUID(uidChar));
}

inline void ZPostRequestCharacterItemListForce(const MUID& uidChar)
{
	ZPOSTCMD1(MC_MATCH_REQUEST_CHARACTER_ITEMLIST_FORCE, MCommandParameterUID(uidChar));
}

inline void ZPostRequestAccountItemList(const MUID& uidChar)
{
	ZPOSTCMD1(MC_MATCH_REQUEST_ACCOUNT_ITEMLIST, MCommandParameterUID(uidChar));
}

inline void ZPostRequestBringAccountItem(const MUID& uidChar, const int nAIID, const int nItemID, const int nItemCnt)
{
	ZPOSTCMD4(MC_MATCH_REQUEST_BRING_ACCOUNTITEM, MCommandParameterUID(uidChar), MCommandParameterInt(nAIID), MCommandParameterInt(nItemID), MCommandParameterInt(nItemCnt));
}
inline void ZPostRequestSendAccountItem(const MUID& uidChar, const MUID& uidItem, const int nItemCnt = 1)
{
	ZPOSTCMD3(MC_MATCH_REQUEST_BRING_BACK_ACCOUNTITEM, MCommandParameterUID(uidChar), MCommandParameterUID(uidItem), MCommandParameterUInt(nItemCnt));
}

inline void ZPostRequestEquipItem(const MUID& uidChar, const MUID& uidItem, const MMatchCharItemParts parts)
{
	ZPOSTCMD3(MC_MATCH_REQUEST_EQUIP_ITEM, MCommandParameterUID(uidChar),
		MCommandParameterUID(uidItem), MCommandParameterUInt(unsigned long int(parts)));
}

inline void ZPostRequestTakeoffItem(const MUID& uidChar, const MMatchCharItemParts parts)
{
	ZPOSTCMD2(MC_MATCH_REQUEST_TAKEOFF_ITEM, MCommandParameterUID(uidChar),
		MCommandParameterUInt(unsigned long int(parts)));
}

inline void ZPostRequestCharInfoDetail(const MUID& uidChar, const char* pszCharName)
{
	ZPOSTCMD2(MC_MATCH_REQUEST_CHARINFO_DETAIL, MCommandParameterUID(uidChar), MCmdParamStr(const_cast<char*>(pszCharName)));
}

inline void ZPostFriendAdd(const char* pszName)
{
	ZPOSTCMD1(MC_MATCH_FRIEND_ADD, MCmdParamStr(const_cast<char*>(pszName)));
}

inline void ZPostFriendRemove(const char* pszName)
{
	ZPOSTCMD1(MC_MATCH_FRIEND_REMOVE, MCmdParamStr(const_cast<char*>(pszName)));
}

inline void ZPostFriendList()
{
	ZPOSTCMD0(MC_MATCH_FRIEND_LIST);
}

inline void ZPostFriendMsg(const char* pszMsg)
{
	ZPOSTCMD1(MC_MATCH_FRIEND_MSG, MCmdParamStr(const_cast<char*>(pszMsg)));
}

inline void ZPostRequestSuicide(const MUID& uidChar)
{
	ZPOSTCMD1(MC_MATCH_REQUEST_SUICIDE, MCommandParameterUID(uidChar));
}

inline void ZPostAdminTerminal(const MUID& uidChar, char* szMsg)
{
	ZPOSTCMD2(MC_ADMIN_TERMINAL, MCommandParameterUID(uidChar), MCommandParameterString(szMsg));
}

inline void ZPostRequestGameInfo(const MUID& uidChar, const MUID& uidStage)
{
	ZPOSTCMD2(MC_MATCH_REQUEST_GAME_INFO, MCommandParameterUID(uidChar),
		MCommandParameterUID(uidStage));
}

inline void ZPostRequestRecommendChannel()
{
	ZPOSTCMD0(MC_MATCH_REQUEST_RECOMMANDED_CHANNEL);
}

inline void ZPostRequestStageList(const MUID& uidChar, const MUID& uidChannel, int nStageCursor)
{
	static unsigned long int st_nLastTime = 0;
	unsigned long int nNowTime = timeGetTime();
	if ((nNowTime - st_nLastTime) > 500)
	{
		ZPOSTCMD3(MC_MATCH_REQUEST_STAGE_LIST, MCommandParameterUID(uidChar),
			MCommandParameterUID(uidChannel), MCommandParameterInt(nStageCursor));

		st_nLastTime = nNowTime;
	}
}

inline void ZPostRequestChannelPlayerList(const MUID& uidChar, const MUID& uidChannel, const int nPage)
{
	ZPOSTCMD3(MC_MATCH_CHANNEL_REQUEST_PLAYER_LIST, MCommandParameterUID(uidChar),
		MCommandParameterUID(uidChannel), MCommandParameterInt(nPage));
}

inline void ZPostRequestObtainWorldItem(const MUID& uidChar, const int nItemUID)
{
	ZPOSTCMD2(MC_MATCH_REQUEST_OBTAIN_WORLDITEM, MCommandParameterUID(uidChar),
		MCommandParameterInt(nItemUID));
}

inline void ZPostRequestSpawnWorldItem(const MUID& uidChar, const int nItemID, const rvector& pos, float fDropDelayTime)
{
	ZPOSTCMD4(MC_MATCH_REQUEST_SPAWN_WORLDITEM, MCommandParameterUID(uidChar),
		MCommandParameterInt(nItemID), MCommandParameterPos(pos.x, pos.y, pos.z), MCommandParameterFloat(fDropDelayTime));
}

inline void ZPostLadderInvite(const MUID& uidChar, const int nRequestID, char* szTeamName,
	char** ppMemberCharNames, int nMemberCharNamesCount)
{
}

inline void ZPostWhisper(char* pszSenderName, char* pszTargetName, char* pszMessage)
{
	ZPOSTCMD3(MC_MATCH_USER_WHISPER, MCmdParamStr(pszSenderName),
		MCmdParamStr(pszTargetName), MCmdParamStr(pszMessage));
}

inline void ZPostWhere(char* pszTargetName)
{
	ZPOSTCMD1(MC_MATCH_USER_WHERE, MCmdParamStr(pszTargetName));
}

void ZPostUserOption();

inline void ZPostChatRoomCreate(const MUID& uidPlayer, char* pszChatRoomName)
{
	ZPOSTCMD2(MC_MATCH_CHATROOM_CREATE, MCmdParamUID(uidPlayer),
		MCmdParamStr(pszChatRoomName));
}

inline void ZPostChatRoomJoin(char* pszChatRoomName)
{
	ZPOSTCMD2(MC_MATCH_CHATROOM_JOIN, MCmdParamStr(""),
		MCmdParamStr(pszChatRoomName));
}

inline void ZPostChatRoomLeave(char* pszChatRoomName)
{
	ZPOSTCMD2(MC_MATCH_CHATROOM_LEAVE, MCmdParamStr(""),
		MCmdParamStr(pszChatRoomName));
}

inline void ZPostSelectChatRoom(char* pszChatRoomName)
{
	ZPOSTCMD1(MC_MATCH_CHATROOM_SELECT_WRITE, MCmdParamStr(pszChatRoomName));
}

inline void ZPostInviteChatRoom(char* pszPlayerName)
{
	ZPOSTCMD3(MC_MATCH_CHATROOM_INVITE, MCmdParamStr(""), MCmdParamStr(pszPlayerName),
		MCmdParamStr(""));
}

inline void ZPostChatRoomChat(char* pszChat)
{
	ZPOSTCMD3(MC_MATCH_CHATROOM_CHAT, MCmdParamStr(""), MCmdParamStr(""), MCmdParamStr(pszChat));
}

inline void ZPostRequestMySimpleCharInfo(const MUID& uidChar)
{
	ZPOSTCMD1(MC_MATCH_REQUEST_MY_SIMPLE_CHARINFO, MCmdParamUID(uidChar));
}

inline void ZPostRequestCopyToTestServer(const MUID& uidChar)
{
	return;
	ZPOSTCMD1(MC_MATCH_REQUEST_COPY_TO_TESTSERVER, MCmdParamUID(uidChar));
}

inline void ZPostRequestQuickJoin(const MUID& uidChar, MTD_QuickJoinParam* pParam)
{
	void* pBlob = MMakeBlobArray(sizeof(MTD_QuickJoinParam), 1);
	MTD_QuickJoinParam* pBlobNode = (MTD_QuickJoinParam*)MGetBlobArrayElement(pBlob, 0);
	CopyMemory(pBlobNode, pParam, sizeof(MTD_QuickJoinParam));

	ZPOSTCMD2(MC_MATCH_STAGE_REQUEST_QUICKJOIN, MCommandParameterUID(uidChar), MCommandParameterBlob(pBlob, MGetBlobArraySize(pBlob)));
	MEraseBlobArray(pBlob);
}

inline void ZPostRequestCreateClan(const MUID& uidChar, const int nRequestID, char* szClanName,
	char** ppMemberCharNames, int nMemberCharNamesCount)
{
	if (nMemberCharNamesCount != CLAN_SPONSORS_COUNT) return;
	ZPOSTCMD7(MC_MATCH_CLAN_REQUEST_CREATE_CLAN, MCmdParamUID(uidChar), MCmdParamInt(nRequestID), MCmdParamStr(szClanName),
		MCmdParamStr(ppMemberCharNames[0]), MCmdParamStr(ppMemberCharNames[1]),
		MCmdParamStr(ppMemberCharNames[2]), MCmdParamStr(ppMemberCharNames[3]));
}

inline void ZPostAnswerSponsorAgreement(const int nRequestID, const MUID& uidClanMaster, char* szSponsorCharName, const bool bAnswer)
{
	ZPOSTCMD4(MC_MATCH_CLAN_ANSWER_SPONSOR_AGREEMENT, MCmdParamInt(nRequestID), MCmdParamUID(uidClanMaster),
		MCmdParamStr(szSponsorCharName), MCmdParamBool(bAnswer));
}

inline void ZPostRequestAgreedCreateClan(const MUID& uidChar, char* szClanName, char** ppMemberCharNames, int nMemberCharNamesCount)
{
	if (nMemberCharNamesCount != CLAN_SPONSORS_COUNT) return;
	ZPOSTCMD6(MC_MATCH_CLAN_REQUEST_AGREED_CREATE_CLAN, MCmdParamUID(uidChar), MCmdParamStr(szClanName),
		MCmdParamStr(ppMemberCharNames[0]), MCmdParamStr(ppMemberCharNames[1]),
		MCmdParamStr(ppMemberCharNames[2]), MCmdParamStr(ppMemberCharNames[3]));
}

inline void ZPostRequestCloseClan(const MUID& uidChar, char* szClanName)
{
	ZPOSTCMD2(MC_MATCH_CLAN_REQUEST_CLOSE_CLAN, MCmdParamUID(uidChar), MCmdParamStr(szClanName));
}

inline void ZPostRequestJoinClan(const MUID& uidChar, const char* szClanName, const char* szJoiner)
{
	ZPOSTCMD3(MC_MATCH_CLAN_REQUEST_JOIN_CLAN, MCmdParamUID(uidChar), MCmdParamStr(szClanName), MCmdParamStr(szJoiner));
}

inline void ZPostAnswerJoinAgreement(const MUID& uidClanAdmin, const char* szJoiner, const bool bAnswer)
{
	ZPOSTCMD3(MC_MATCH_CLAN_ANSWER_JOIN_AGREEMENT, MCmdParamUID(uidClanAdmin), MCmdParamStr(szJoiner), MCmdParamBool(bAnswer));
}

inline void ZPostRequestAgreedJoinClan(const MUID& uidClanAdmin, const char* szClanName, const char* szJoiner)
{
	ZPOSTCMD3(MC_MATCH_CLAN_REQUEST_AGREED_JOIN_CLAN, MCmdParamUID(uidClanAdmin), MCmdParamStr(szClanName), MCmdParamStr(szJoiner));
}

inline void ZPostRequestLeaveClan(const MUID& uidChar)
{
	ZPOSTCMD1(MC_MATCH_CLAN_REQUEST_LEAVE_CLAN, MCmdParamUID(uidChar));
}

inline void ZPostRequestChangeClanGrade(const MUID& uidClanAdmin, const char* szMember, int nClanGrade)
{
	ZPOSTCMD3(MC_MATCH_CLAN_MASTER_REQUEST_CHANGE_GRADE, MCmdParamUID(uidClanAdmin), MCmdParamStr(szMember), MCmdParamInt(nClanGrade));
}

inline void ZPostRequestExpelClanMember(const MUID& uidClanAdmin, const char* szMember)
{
	ZPOSTCMD2(MC_MATCH_CLAN_ADMIN_REQUEST_EXPEL_MEMBER, MCmdParamUID(uidClanAdmin), MCmdParamStr(szMember));
}

inline void ZPostRequestChannelAllPlayerList(const MUID& uidChar, const MUID& uidChannel, const unsigned long int nPlaceFilter,
	const unsigned long int nOptions)
{
	ZPOSTCMD4(MC_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST, MCommandParameterUID(uidChar),
		MCommandParameterUID(uidChannel), MCommandParameterUInt(nPlaceFilter), MCommandParameterUInt(nOptions));
}

inline void ZPostClanMsg(const MUID& uidSender, const char* pszMsg)
{
	ZPOSTCMD2(MC_MATCH_CLAN_REQUEST_MSG, MCmdParamUID(uidSender), MCmdParamStr(const_cast<char*>(pszMsg)));
}

inline void ZPostRequestClanMemberList(const MUID& uidChar)
{
	ZPOSTCMD1(MC_MATCH_CLAN_REQUEST_MEMBER_LIST, MCmdParamUID(uidChar));
}

inline void ZPostRequestClanInfo(const MUID& uidChar, const char* szClanName)
{
	ZPOSTCMD2(MC_MATCH_CLAN_REQUEST_CLAN_INFO, MCmdParamUID(uidChar), MCmdParamStr(szClanName));
}

inline void ZPostRequestProposal(const MUID& uidChar, const int nProposalMode, const int nRequestID,
	char** ppReplierCharNames, const int nReplierCount)
{
	void* pBlobRepliersName = MMakeBlobArray(sizeof(MTD_ReplierNode), nReplierCount);
	for (int i = 0; i < nReplierCount; i++)
	{
		MTD_ReplierNode* pReplierNode = (MTD_ReplierNode*)MGetBlobArrayElement(pBlobRepliersName, i);
		strcpy(pReplierNode->szName, ppReplierCharNames[i]);
	}

	ZPOSTCMD5(MC_MATCH_REQUEST_PROPOSAL, MCmdParamUID(uidChar), MCmdParamInt(nProposalMode),
		MCmdParamInt(nRequestID), MCmdParamInt(nReplierCount),
		MCmdParamBlob(pBlobRepliersName, MGetBlobArraySize(pBlobRepliersName)));

	MEraseBlobArray(pBlobRepliersName);
}

inline void ZPostReplyAgreement(const MUID& uidProposer, const MUID& uidChar, char* szReplierName, int nProposalMode,
	int nRequestID, bool bAgreement)
{
	ZPOSTCMD6(MC_MATCH_REPLY_AGREEMENT, MCmdParamUID(uidProposer), MCmdParamUID(uidChar),
		MCmdParamStr(szReplierName), MCmdParamInt(nProposalMode), MCmdParamInt(nRequestID), MCmdParamBool(bAgreement));
}

inline void ZPostLadderRequestChallenge(char** ppMemberCharNames, const int nMemberCount, unsigned long int nOptions, unsigned long int nAntiLead)
{
	void* pBlobMembersName = MMakeBlobArray(sizeof(MTD_ReplierNode), nMemberCount);
	for (int i = 0; i < nMemberCount; i++)
	{
		MTD_LadderTeamMemberNode* pMemberNode = (MTD_LadderTeamMemberNode*)MGetBlobArrayElement(pBlobMembersName, i);
		strcpy(pMemberNode->szName, ppMemberCharNames[i]);
	}

	ZPOSTCMD4(MC_MATCH_LADDER_REQUEST_CHALLENGE, MCmdParamInt(nMemberCount), MCmdParamUInt(nOptions),
		MCmdParamBlob(pBlobMembersName, MGetBlobArraySize(pBlobMembersName)), MCmdParamUInt(nAntiLead));

	MEraseBlobArray(pBlobMembersName);
}

inline void ZPostLadderCancel()
{
	ZPOSTCMD0(MC_MATCH_LADDER_REQUEST_CANCEL_CHALLENGE)
}

#ifdef _DUELTOURNAMENT
inline void ZPostDuelTournamentRequestJoinGame(const MUID& uidChar, MDUELTOURNAMENTTYPE nType)
{
	ZPOSTCMD2(MC_MATCH_DUELTOURNAMENT_REQUEST_JOINGAME, MCmdParamUID(uidChar), MCmdParamInt((int)nType));
}

inline void ZPostDuelTournamentRequestCancelGame(const MUID& uidChar, MDUELTOURNAMENTTYPE nType)
{
	ZPOSTCMD2(MC_MATCH_DUELTOURNAMENT_REQUEST_CANCELGAME, MCmdParamUID(uidChar), MCmdParamInt((int)nType));
}

inline void ZPostDuelTournamentRequestSideRankingInfo(const MUID& uidChar)
{
	static char szCharName[MATCHOBJECT_NAME_LENGTH] = { -1, 0, };
	static DWORD timeLastReqeust = 0;
	DWORD currTime = timeGetTime();

	if (timeLastReqeust == 0 || currTime - timeLastReqeust > 10 * 1000 || stricmp(ZGetMyInfo()->GetCharName(), szCharName) != 0)
	{
		timeLastReqeust = currTime;
		strcpy(szCharName, ZGetMyInfo()->GetCharName());
		ZPOSTCMD1(MC_MATCH_DUELTOURNAMENT_REQUEST_SIDERANKING_INFO, MCmdParamUID(uidChar));
	}
}

inline void ZPostDuelTournamentGamePlayerStatus(const MUID& uidChar, float fAccumulationDamage, float fHP, float fAP)
{
	ZPOSTCMD4(MC_MATCH_DUELTOURNAMENT_GAME_PLAYER_STATUS, MCmdParamUID(uidChar), MCmdParamFloat(fAccumulationDamage), MCmdParamFloat(fHP), MCmdParamFloat(fAP));
}
#endif

inline void ZPostAdminPingToAll()
{
	ZPOSTCMD0(MC_ADMIN_PING_TO_ALL);
}

inline void ZPostAdminRequestKickPlayer(char* pszTargetPlayerName)
{
	ZPOSTCMD1(MC_ADMIN_REQUEST_KICK_PLAYER, MCmdParamStr(pszTargetPlayerName));
}

inline void ZPostAdminRequestMutePlayer(char* pszTargetPlayerName, int nPunishHour)
{
	ZPOSTCMD2(MC_ADMIN_REQUEST_MUTE_PLAYER, MCmdParamStr(pszTargetPlayerName), MCmdParamInt(nPunishHour));
}

inline void ZPostAdminRequestBlockPlayer(char* pszTargetPlayerName, int nPunishHour)
{
	ZPOSTCMD2(MC_ADMIN_REQUEST_BLOCK_PLAYER, MCmdParamStr(pszTargetPlayerName), MCmdParamInt(nPunishHour));
}

inline void ZPostAdminRequestSwitchLadderGame(const MUID& uidChar, bool bEnabled)
{
	ZPOSTCMD2(MC_ADMIN_REQUEST_SWITCH_LADDER_GAME, MCmdParamUID(uidChar), MCmdParamBool(bEnabled));
}

inline void ZPostAdminAnnounce(const MUID& uidChar, char* szMsg, ZAdminAnnounceType nType)
{
	ZPOSTCMD3(MC_ADMIN_ANNOUNCE, MCommandParameterUID(uidChar),
		MCommandParameterString(szMsg), MCommandParameterUInt(nType));
}

inline void ZPostAdminHalt(const MUID& uidChar)
{
	ZPOSTCMD1(MC_ADMIN_SERVER_HALT, MCommandParameterUID(uidChar));
}

inline void ZPostAdminReloadClientHash()
{
	ZPOSTCMD0(MC_ADMIN_RELOAD_CLIENT_HASH);
}

inline void ZPostAdminResetAllHackingBlock()
{
	ZPOSTCMD0(MC_ADMIN_RESET_ALL_HACKING_BLOCK);
}

inline void ZPostAdminReloadGambleItem()
{
	ZPOSTCMD0(MC_ADMIN_RELOAD_GAMBLEITEM);
}

inline void ZPostAdminDumpGambleItemLog()
{
	ZPOSTCMD0(MC_ADMIN_DUMP_GAMBLEITEM_LOG);
}

inline void ZPostAdminAssasin()
{
	ZPOSTCMD0(MC_ADMIN_ASSASIN);
}

inline void ZPostChangeMaster()
{
	ZPOSTCMD0(MC_EVENT_CHANGE_MASTER);
}

inline void ZPostChangePassword(char* pszPassword)
{
	ZPOSTCMD1(MC_EVENT_CHANGE_PASSWORD, MCmdParamStr(pszPassword));
}

inline void ZPostAdminHide()
{
	ZPOSTCMD0(MC_ADMIN_HIDE);
}

inline void ZPostAdminRequestJjang(char* pszTargetName)
{
	ZPOSTCMD1(MC_EVENT_REQUEST_JJANG, MCmdParamStr(pszTargetName));
}

inline void ZPostAdminRemoveJjang(char* pszTargetName)
{
	ZPOSTCMD1(MC_EVENT_REMOVE_JJANG, MCmdParamStr(pszTargetName));
}

inline void ZPostRequestEmblemURL(void* pBlob)
{
	ZPOSTCMD1(MC_MATCH_CLAN_REQUEST_EMBLEMURL, MCommandParameterBlob(pBlob, MGetBlobArraySize(pBlob)));
}

inline void ZPostClanEmblemReady(unsigned int nCLID, char* pszEmblemURL)
{
	ZPOSTCMD2(MC_MATCH_CLAN_LOCAL_EMBLEMREADY, MCmdParamInt(nCLID), MCmdParamStr(pszEmblemURL));
}

#define ZPostQuestRequestNPCDead(  uidKiller,   uidNPC,  vPos) {	\
	ZPOSTCMD3(CLOAK_CMD_ID(MC_QUEST_REQUEST_NPC_DEAD, 24781), MCmdParamUID(uidKiller), MCmdParamUID(uidNPC), MCmdParamShortVector(vPos.x, vPos.y, vPos.z));	\
}

inline void ZPostQuestPeerNPCDead(const MUID& uidKiller, const MUID& uidNPC)
{
	ZPOSTCMD2(MC_QUEST_PEER_NPC_DEAD, MCmdParamUID(uidKiller), MCmdParamUID(uidNPC));
}

inline void ZPostQuestGameKill()
{
	ZPOSTCMD0(MC_MATCH_QUEST_REQUEST_DEAD);
}

#ifdef _DEBUG
inline void ZPostQuestTestNPCSpawn(const int nNPCType, const int nNPCCount)
{
	ZPOSTCMD2(MC_QUEST_TEST_REQUEST_NPC_SPAWN, MCmdParamInt(nNPCType), MCmdParamInt(nNPCCount));
}
#endif

#ifdef _DEBUG
inline void ZPostQuestTestClearNPC()
{
	ZPOSTCMD0(MC_QUEST_TEST_REQUEST_CLEAR_NPC);
}
#endif

#ifdef _DEBUG
inline void ZPostQuestTestSectorClear()
{
	ZPOSTCMD0(MC_QUEST_TEST_REQUEST_SECTOR_CLEAR);
}
#endif

#ifdef _DEBUG
inline void ZPostQuestTestFinish()
{
	ZPOSTCMD0(MC_QUEST_TEST_REQUEST_FINISH);
}
#endif

inline void ZPostQuestRequestMovetoPortal(const char nCurrSectorIndex)
{
	ZPOSTCMD1(MC_QUEST_REQUEST_MOVETO_PORTAL, MCmdParamChar(nCurrSectorIndex));
}

inline void ZPostQuestReadyToNewSector(const MUID& uidPlayer)
{
	ZPOSTCMD1(MC_QUEST_READYTO_NEWSECTOR, MCmdParamUID(uidPlayer));
}

inline void ZPostQuestPong(unsigned long int nTime)
{
	ZPOSTCMD1(MC_QUEST_PONG, MCmdParamUInt(nTime));
}

#ifdef _QUEST_ITEM
inline void ZPostRequestGetCharQuestItemInfo(const MUID& uid)
{
	ZPOSTCMD1(MC_MATCH_REQUEST_CHAR_QUEST_ITEM_LIST, MCmdParamUID(uid));
}

inline void ZPostRequestBuyQuestItem(const MUID& uid, const unsigned long int nItemID, const int nCount = 1)
{
	ZPOSTCMD3(MC_MATCH_REQUEST_BUY_QUEST_ITEM, MCmdParamUID(uid), MCommandParameterInt(nItemID), MCommandParameterInt(nCount));
}

inline void ZPostRequestSellQuestItem(const MUID& uid, const unsigned long int nItemID, const int nCount = 1)
{
	ZPOSTCMD3(MC_MATCH_REQUEST_SELL_QUEST_ITEM, MCmdParamUID(uid), MCommandParameterInt(nItemID), MCommandParameterInt(nCount));
}

inline void ZPostRequestDropSacrificeItem(const MUID& uid, const int nSlotIndex, const unsigned long int nItemID)
{
	ZPOSTCMD3(MC_MATCH_REQUEST_DROP_SACRIFICE_ITEM, MCmdParamUID(uid), MCommandParameterInt(nSlotIndex), MCommandParameterInt(nItemID));
}

inline void ZPostRequestCallbackSacrificeItem(const MUID& uid, const int nSlotIndex, const unsigned long int nItemID)
{
	ZPOSTCMD3(MC_MATCH_REQUEST_CALLBACK_SACRIFICE_ITEM, MCmdParamUID(uid), MCommandParameterInt(nSlotIndex), MCommandParameterInt(nItemID));
}

inline void ZPostRequestQL(const MUID& uid)
{
	ZPOSTCMD1(MC_QUEST_REQUEST_QL, MCmdParamUID(uid));
}

inline void ZPostRequestSacrificeSlotInfo(const MUID& uid)
{
	ZPOSTCMD1(MC_MATCH_REQUEST_SLOT_INFO, MCmdParamUID(uid));
}

inline void ZPostQuestStageMapset(const MUID& uidStage, int nMapsetID)
{
	ZPOSTCMD2(MC_QUEST_STAGE_MAPSET, MCommandParameterUID(uidStage), MCommandParameterChar((char)nMapsetID));
}

inline void ZPostRequestMonsterBibleInfo(const MUID& uid)
{
	ZPOSTCMD1(MC_MATCH_REQUEST_MONSTER_BIBLE_INFO, MCmdParamUID(uid));
}

inline void ZPostResponseXTrapSeedKey(unsigned char* szComBuf)
{
	void* pBlob = MMakeBlobArray(sizeof(unsigned char), 128);
	unsigned char* pCmdBlock = (unsigned char*)MGetBlobArrayElement(pBlob, 0);
	CopyMemory(pCmdBlock, szComBuf, 128);

	ZPOSTCMD1(MC_RESPONSE_XTRAP_SEEDKEY, MCommandParameterBlob(pBlob, MGetBlobArraySize(pBlob)));

	MEraseBlobArray(pBlob);
}

inline void ZPsotResponseGameguardAuht(const DWORD dwIndex, const DWORD dwValue1, const DWORD dwValue2, const DWORD dwValue3)
{
	ZPOSTCMD4(MC_RESPONSE_GAMEGUARD_AUTH, MCmdParamUInt(dwIndex), MCmdParamUInt(dwValue1), MCmdParamUInt(dwValue2), MCmdParamUInt(dwValue3));
}

inline void ZPostResponseFirstGameguardAuth(const DWORD dwIndex, const DWORD dwValue1, const DWORD dwValue2, const DWORD dwValue3)
{
	ZPOSTCMD4(MC_RESPONSE_FIRST_GAMEGUARD_AUTH, MCmdParamUInt(dwIndex), MCmdParamUInt(dwValue1), MCmdParamUInt(dwValue2), MCmdParamUInt(dwValue3));
}

inline void ZPostRequestGamble(const MUID& uid)
{
	ZPOSTCMD1(MC_MATCH_REQUEST_GAMBLE, MCmdParamUID(uid));
}

inline void ZPostResponseResourceCRC32(DWORD dwCRC, DWORD dwXOR)
{
	if (0 == dwCRC)
	{
		return;
	}

	ZPOSTCMD2(MC_RESPONSE_RESOURCE_CRC32, MCmdParamUInt(dwCRC), MCmdParamUInt(dwXOR));
}

inline void ZPostRequestUseSpendableNormalItem(const MUID& uid)
{
	ZPOSTCMD1(MC_MATCH_REQUEST_USE_SPENDABLE_NORMAL_ITEM, MCmdParamUID(uid));
}

inline void ZPostRequestUseSpendableBuffItem(const MUID& uid)
{
	ZPOSTCMD1(MC_MATCH_REQUEST_USE_SPENDABLE_BUFF_ITEM, MCmdParamUID(uid));
}

#endif

inline void ZPostRequestCaptureFlag(const int nItemUID)
{
	ZPOSTCMD1(MC_MATCH_REQUEST_FLAG_CAP, MCommandParameterInt(nItemUID));
}

inline void ZPostHacker(const int nHackID)
{
	ZPOSTCMD1(MC_REQUEST_GIVE_ONESELF_UP, MCommandParameterInt(nHackID));
}

inline void ZPostAdminStageJoin(MUID uidStage, char* trg)
{
	ZPOSTCMD2(MC_ADMIN_STAGE_JOIN, MCmdParamUID(uidStage), MCmdParamStr(trg));
}

inline void ZPostPlayerWarsVote(int Map)
{
	ZPOSTCMD1(MC_MATCH_PLAYERWARS_VOTE, MCmdParamInt(Map));
}
inline void ZPostStageRequestRejoin()
{
	ZPOSTCMD0(MC_MATCH_STAGE_REQUEST_REJOIN);
}

#endif