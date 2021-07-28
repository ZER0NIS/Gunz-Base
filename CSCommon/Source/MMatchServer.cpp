#include "stdafx.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchItem.h"
#include "MMatchNotify.h"
#include "Msg.h"
#include "MMatchObjCache.h"
#include "MMatchStage.h"
#include "MMatchTransDataType.h"
#include "MMatchFormula.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MMatchShop.h"
#include "MDebug.h"
#include "MMatchAuth.h"
#include "MMatchStatus.h"
#include "MAsyncDBJob.h"
#include "MMatchWorldItemDesc.h"
#include "MMatchQuestMonsterGroup.h"
#include "RTypes.h"
#include "MMatchChatRoom.h"
#include "MMatchUtil.h"
#include "MLadderStatistics.h"
#include "MMatchSchedule.h"
#include <winbase.h>
#include "MMatchGameType.h"
#include "MQuestFormula.h"
#include "MQuestItem.h"
#include "MSacrificeQItemTable.h"
#include "MMatchPremiumIPCache.h"
#include "MCommandBuilder.h"
#include "MMatchLocale.h"
#include "MMatchEvent.h"
#include "MMatchEventManager.h"
#include "MMatchEventFactory.h"
#include "../../MatchServer/HSHIELD/AntiCpSvrFunc.h"
#include "MMatchDBGambleItem.h"
#include "MMatchCRC32XORCache.h"
#include "MMatchCheckLoopTime.h"
#include "MCrashDump.h"
#include "MMatchDuelTournamentMgr.h"

#include "MAsyncDBJob_FriendList.h"
#include "MAsyncDBJob_UpdateCharInfoData.h"
#include "MAsyncDBJob_GetLoginInfo.h"
#include "MAsyncDBJob_UpdateCharItemInfoData.h"

#include "MAsyncDBJob_GetBRDescription.h"
#include "MAsyncDBJob_InsertCharBRInfo.h"
#include "MAsyncDBJob_UpdateCharBRInfo.h"
#include "MAsyncDBJob_RewardCharBR.h"

#include "MMatchBuff.h"

#define DEFAULT_REQUEST_UID_SIZE		4200000000
#define DEFAULT_REQUEST_UID_SPARE_SIZE	10000
#define DEFAULT_ASYNCPROXY_THREADPOOL	6
#define MAXUSER_WEIGHT					30

#define MAX_DB_QUERY_COUNT_OUT			5

#define MATCHSERVER_DEFAULT_UDP_PORT	7777

#define FILENAME_ITEM_DESC				"XML/zitem.xml"
#define FILENAME_ITEM_DESC_LOCALE		"XML/zitem_locale.xml"
#define FILENAME_BUFF_DESC				"XML/zBuff.xml"
#define FILENAME_SHOP					"XML/shop.xml"
#define FILENAME_CHANNEL				"XML/channel.xml"
#define FILENAME_SHUTDOWN_NOTIFY		"XML/shutdown.xml"
#define FILENAME_WORLDITEM_DESC			"XML/worlditem.xml"
#define FILENAME_MONSTERGROUP_DESC		"XML/monstergroup.xml"
#define FILENAME_CHANNELRULE			"XML/channelrule.xml"

MMatchServer* MMatchServer::m_pInstance = NULL;
void RcpLog(const char* pFormat, ...)
{
	char szBuf[256];

	va_list args;

	va_start(args, pFormat);
	vsprintf(szBuf, pFormat, args);
	va_end(args);

	int nEnd = (int)strlen(szBuf) - 1;
	if ((nEnd >= 0) && (szBuf[nEnd] == '\n')) {
		szBuf[nEnd] = NULL;
		strcat(szBuf, "\n");
	}
#ifdef _DEBUG
	OutputDebugString(szBuf);
#endif
}
class MPointerChecker
{
private:
	void* m_pPointer;
	bool	m_bPrinted;
public:
	MPointerChecker() : m_bPrinted(false), m_pPointer(0) {  }
	void Init(void* pPointer) { m_pPointer = pPointer; }
	void Check(void* pPointer, int nState, int nValue)
	{
		if ((pPointer != m_pPointer) && (!m_bPrinted))
		{
			m_bPrinted = true;
			mlog("### Invalid Pointer(%x, %x) - State(%d) , Value(%d) ###\n", m_pPointer, pPointer, nState, nValue);
		}
	}
};

#define NUM_CHECKPOINTER	3
static MPointerChecker g_PointerChecker[NUM_CHECKPOINTER];

void _CheckValidPointer(void* pPointer1, void* pPointer2, void* pPointer3, int nState, int nValue)
{
	if (pPointer1 != NULL) g_PointerChecker[0].Check(pPointer1, nState, nValue);
	if (pPointer2 != NULL) g_PointerChecker[1].Check(pPointer2, nState, nValue);
	if (pPointer3 != NULL) g_PointerChecker[2].Check(pPointer3, nState, nValue);
}

void CopyCharInfoForTrans(MTD_CharInfo* pDest, MMatchCharInfo* pSrcCharInfo, MMatchObject* pSrcObject)
{
	memset(pDest, 0, sizeof(MTD_CharInfo));

	if (pSrcCharInfo)
	{
		strcpy(pDest->szName, pSrcCharInfo->m_szName);
		strcpy(pDest->szClanName, pSrcCharInfo->m_ClanInfo.m_szClanName);

		pDest->nClanGrade = pSrcCharInfo->m_ClanInfo.m_nGrade;
		pDest->nClanContPoint = pSrcCharInfo->m_ClanInfo.m_nContPoint;

		pDest->nCharNum = (char)pSrcCharInfo->m_nCharNum;
		pDest->nLevel = (unsigned short)pSrcCharInfo->m_nLevel;
		pDest->nSex = (char)pSrcCharInfo->m_nSex;
		pDest->nFace = (char)pSrcCharInfo->m_nFace;
		pDest->nHair = (char)pSrcCharInfo->m_nHair;

		pDest->nXP = pSrcCharInfo->m_nXP;
		pDest->nBP = pSrcCharInfo->m_nBP;
		pDest->fBonusRate = pSrcCharInfo->m_fBonusRate;
		pDest->nPrize = (unsigned short)pSrcCharInfo->m_nPrize;
		pDest->nHP = (unsigned short)pSrcCharInfo->m_nHP;
		pDest->nAP = (unsigned short)pSrcCharInfo->m_nAP;
		pDest->nMaxWeight = (unsigned short)pSrcCharInfo->m_nMaxWeight;
		pDest->nSafeFalls = (unsigned short)pSrcCharInfo->m_nSafeFalls;
		pDest->nFR = (unsigned short)pSrcCharInfo->m_nFR;
		pDest->nCR = (unsigned short)pSrcCharInfo->m_nCR;
		pDest->nER = (unsigned short)pSrcCharInfo->m_nER;
		pDest->nWR = (unsigned short)pSrcCharInfo->m_nWR;

		for (int i = 0; i < MMCIP_END; i++) {
			if (pSrcCharInfo->m_EquipedItem.IsEmpty(MMatchCharItemParts(i))) {
				pDest->nEquipedItemDesc[i] = 0;
				pDest->nEquipedItemCount[i] = 0;
				pDest->uidEquipedItem[i] = MUID(0, 0);
			}
			else {
				MMatchItem* pItem = pSrcCharInfo->m_EquipedItem.GetItem(MMatchCharItemParts(i));
				if (NULL == pItem) {
					pDest->nEquipedItemDesc[i] = 0;
					pDest->nEquipedItemCount[i] = 0;
					pDest->uidEquipedItem[i] = MUID(0, 0);
				}
				else {
					MMatchItemDesc* pItemDesc = pItem->GetDesc();
					if (pItemDesc) {
						pDest->nEquipedItemDesc[i] = pItemDesc->m_nID;
						pDest->nEquipedItemCount[i] = pItem->GetItemCount();
						pDest->uidEquipedItem[i] = pSrcCharInfo->m_EquipedItem.GetItem((MMatchCharItemParts)i)->GetUID();
					}
				}
			}
		}
	}

	if (pSrcObject) {
		pDest->nUGradeID = pSrcObject->GetAccountInfo()->m_nUGrade;
	}
	else {
		pDest->nUGradeID = MMUG_FREE;
	}

	pDest->nClanCLID = pSrcCharInfo->m_ClanInfo.m_nClanID;

	if (pSrcObject && pSrcObject->GetDuelTournamentCharInfo()) {
		pDest->nDTLastWeekGrade = pSrcObject->GetDuelTournamentCharInfo()->GetLastWeekGrade();
	}
	else {
		pDest->nDTLastWeekGrade = 0;
	}
}

void CopyCharInfoDetailForTrans(MTD_CharInfo_Detail* pDest, MMatchCharInfo* pSrcCharInfo, MMatchObject* pSrcObject)
{
	memset(pDest, 0, sizeof(MTD_CharInfo_Detail));

	if (pSrcCharInfo)
	{
		strcpy(pDest->szName, pSrcCharInfo->m_szName);
		strcpy(pDest->szClanName, pSrcCharInfo->m_ClanInfo.m_szClanName);
		pDest->nClanGrade = pSrcCharInfo->m_ClanInfo.m_nGrade;
		pDest->nClanContPoint = pSrcCharInfo->m_ClanInfo.m_nContPoint;

		pDest->nLevel = (unsigned short)pSrcCharInfo->m_nLevel;
		pDest->nSex = (char)pSrcCharInfo->m_nSex;
		pDest->nFace = (char)pSrcCharInfo->m_nFace;
		pDest->nHair = (char)pSrcCharInfo->m_nHair;
		pDest->nXP = pSrcCharInfo->m_nXP;
		pDest->nBP = pSrcCharInfo->m_nBP;

		pDest->nKillCount = pSrcCharInfo->m_nTotalKillCount;
		pDest->nDeathCount = pSrcCharInfo->m_nTotalDeathCount;

		unsigned long int nNowTime = MMatchServer::GetInstance()->GetTickTime();

		pDest->nConnPlayTimeSec = MGetTimeDistance(pSrcCharInfo->m_nConnTime, nNowTime) / 1000;
		pDest->nTotalPlayTimeSec = pDest->nConnPlayTimeSec + pSrcCharInfo->m_nTotalPlayTimeSec;

		for (int i = 0; i < MMCIP_END; i++) {
			if (pSrcCharInfo->m_EquipedItem.IsEmpty(MMatchCharItemParts(i))) {
				pDest->nEquipedItemDesc[i] = 0;
			}
			else {
				if (NULL == pSrcCharInfo->m_EquipedItem.GetItem(MMatchCharItemParts(i))) {
					pDest->nEquipedItemDesc[i] = 0;
				}
				else {
					pDest->nEquipedItemDesc[i] = pSrcCharInfo->m_EquipedItem.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nID;
				}
			}
		}
	}

	if (pSrcObject) {
		pDest->nUGradeID = pSrcObject->GetAccountInfo()->m_nUGrade;
	}
	else {
		pDest->nUGradeID = MMUG_FREE;
	}

	pDest->nClanCLID = pSrcCharInfo->m_ClanInfo.m_nClanID;
}
bool IsExpiredBlockEndTime(const SYSTEMTIME& st)
{
	SYSTEMTIME stLocal;
	GetLocalTime(&stLocal);

	if (st.wYear < stLocal.wYear)
		return true;
	else if (st.wYear > stLocal.wYear)
		return false;

	if (st.wMonth < stLocal.wMonth)
		return true;
	else if (st.wMonth > stLocal.wMonth)
		return false;

	if (st.wDay < stLocal.wDay)
		return true;
	else if (st.wDay > stLocal.wDay)
		return false;

	if (st.wHour < stLocal.wHour)
		return true;
	else if (st.wHour > stLocal.wHour)
		return false;

	if (st.wMinute >= stLocal.wMinute)
		return false;

	return true;
}

MMatchServer::MMatchServer(void) : m_pScheduler(0), m_pDTMgr(new MMatchDuelTournamentMgr)
{
	m_pInstance = this;
	m_nTickTime = 0;
	m_dwBlockCount = 0;
	m_dwNonBlockCount = 0;

	m_This = (MATCHSERVER_UID);

	SetName("MATCHSERVER");
	SetDefaultChannelName("PUBLIC-");

	m_bCreated = false;

	m_pAuthBuilder = NULL;

	MMatchStringResManager::MakeInstance();

	m_checkMemory1 = m_checkMemory2 = m_checkMemory3 = m_checkMemory4 = m_checkMemory5 = m_checkMemory6 =
		m_checkMemory7 = m_checkMemory8 = m_checkMemory9 = m_checkMemory10 = m_checkMemory11 = m_checkMemory12 =
		m_checkMemory13 = m_checkMemory14 = m_checkMemory15 = m_checkMemory16 = m_checkMemory17 = m_checkMemory18 =
		m_checkMemory19 = m_checkMemory20 = m_checkMemory21 = CHECKMEMORYNUMBER;

#ifdef _HSHIELD
	if (MGetServerConfig()->IsUseHShield())
		m_HSCheckCounter = 0L;
#endif

	memset(m_szMD5Value, 0, 16);
	SetFloodCheck(true);
}

static bool g_bPrintedInvalidMemory = false;

void MMatchServer::CheckMemoryTest(int nState, int nValue)
{
#define CHECK(n) if(m_checkMemory##n!=CHECKMEMORYNUMBER) { g_bPrintedInvalidMemory=true; mlog("***WARNING !! m_checkMemory" #n " is corrupted. State(%d), Value(%d)\n", nState, nValue); }

	if (g_bPrintedInvalidMemory) return;

	CHECK(1);
	CHECK(2);
	CHECK(3);
	CHECK(4);
	CHECK(5);
	CHECK(6);
	CHECK(7);
	CHECK(8);
	CHECK(9);
	CHECK(10);
	CHECK(11);
	CHECK(12);
	CHECK(13);
	CHECK(14);
	CHECK(15);
	CHECK(16);
	CHECK(17);
	CHECK(18);
	CHECK(19);
	CHECK(20);
	CHECK(21);
}

MMatchServer::~MMatchServer(void)
{
	delete m_pDTMgr;

	CheckMemoryTest();

	if (m_pAuthBuilder) {
		delete m_pAuthBuilder;
		m_pAuthBuilder = NULL;
	}

	Destroy();
}

bool MMatchServer::LoadInitFile()
{
	if (!MGetServerConfig()->Create())
	{
		LOG(LOG_PROG, "Load Config File Failed\n");
		return false;
	}

	if (!InitLocale()) {
		LOG(LOG_PROG, "Locale ���� ����.\n");
		return false;
	}

	if (MGetServerConfig()->IsResMap())
	{
		char szText[512];
		sprintf(szText, "Enable Maps: ");
		for (int i = 0; i < MMATCH_MAP_MAX; i++)
		{
			if (MGetServerConfig()->IsEnableMap(MMATCH_MAP(i)))
			{
				strcat(szText, MGetMapDescMgr()->GetMapName(i));
				strcat(szText, ", ");
			}
		}
		LOG(LOG_PROG, szText);
	}

	if (!MMatchFormula::Create())
	{
		LOG(LOG_PROG, "Open Formula Table FAILED\n");
		return false;
	}
	if (!MQuestFormula::Create())
	{
		LOG(LOG_PROG, "Open Quest Formula Table FAILED\n");
		return false;
	}

	if (!MGetMatchWorldItemDescMgr()->ReadXml(FILENAME_WORLDITEM_DESC))
	{
		Log(LOG_PROG, "Read World Item Desc Failed\n");
		return false;
	}

	if (!MGetMapDescMgr()->Initialize("XML/map.xml"))
	{
		Log(LOG_PROG, "Read Map.xml Failed\n");
		return false;
	}
#ifdef _QUEST_ITEM
	if (!GetQuestItemDescMgr().ReadXml(QUEST_ITEM_FILE_NAME))
	{
		Log(LOG_PROG, "Load quest item xml file failed.");
		return false;
	}
	if (!MSacrificeQItemTable::GetInst().ReadXML(SACRIFICE_TABLE_XML))
	{
		Log(LOG_PROG, "Load sacrifice quest item table failed.");
		return false;
	}
#endif
	if (MGetServerConfig()->GetServerMode() == MSM_CLAN)
	{
		GetLadderMgr()->Init();

#ifdef _DEBUG
#endif
	}

	if (!MGetMapsWorldItemSpawnInfo()->Read())
	{
		Log(LOG_PROG, "Read World Item Spawn Failed\n");
		return false;
	}

	if (!MGetMatchItemDescMgr()->ReadXml(FILENAME_ITEM_DESC))
	{
		Log(LOG_PROG, "Read Item Descriptor Failed\n");
		return false;
	}
	if (!MGetMatchItemDescMgr()->ReadXml(FILENAME_ITEM_DESC_LOCALE))
	{
		Log(LOG_PROG, "Read Item Descriptor Locale Failed\n");
		return false;
	}

	InitItemCRC32Cache();

	if (!GetQuest()->Create())
	{
		Log(LOG_PROG, "Read Quest Desc Failed\n");
		return false;
	}

	if (!MGetMatchShop()->Create(FILENAME_SHOP))
	{
		Log(LOG_PROG, "Read Shop Item Failed\n");
		return false;
	}
	if (!LoadChannelPreset())
	{
		Log(LOG_PROG, "Load Channel preset Failed\n");
		return false;
	}
	if (!m_MatchShutdown.LoadXML_ShutdownNotify(FILENAME_SHUTDOWN_NOTIFY))
	{
		Log(LOG_PROG, "Load Shutdown Notify Failed\n");
		return false;
	}
	if (!MGetChannelRuleMgr()->ReadXml(FILENAME_CHANNELRULE))
	{
		Log(LOG_PROG, "Load ChannelRule.xml Failed\n");
		return false;
	}

	unsigned long nItemChecksum = MGetMZFileChecksum(FILENAME_ITEM_DESC);
	SetItemFileChecksum(nItemChecksum);

	if (!InitEvent())
	{
		Log(LOG_PROG, "init event failed.\n\n");
		return false;
	}

#ifdef _DEBUG
	CheckItemXML();
	CheckUpdateItemXML();
#endif

	return true;
}

void MMatchServer::InitItemCRC32Cache()
{
}

void MMatchServer::InitBuffCRC32Cache()
{
}

bool MMatchServer::LoadChannelPreset()
{
#define MTOK_DEFAULTCHANNELNAME		"DEFAULTCHANNELNAME"
#define MTOK_DEFAULTRULENAME		"DEFAULTRULENAME"
#define MTOK_CHANNEL				"CHANNEL"
#define MTOK_CLAN_CHANNEL			"CLAN_CHANNEL"

	MXmlDocument	xmlIniData;
	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(FILENAME_CHANNEL))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, childElement;
	char szTagName[256];
	char szBuf[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		childElement = rootElement.GetChildNode(i);
		childElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, MTOK_CHANNEL))
		{
			char	szRuleName[256] = "";
			int		nMaxPlayers = 0;
			int		nLevelMin = -1;
			int		nLevelMax = -1;
			bool	bIsTicketChannel = false;
			int		nTicketItemID = -1;
			bool	bIsDuelTournament = false;
			MCHANNEL_TYPE nChannelType = MCHANNEL_TYPE_PRESET;

			childElement.GetAttribute(szBuf, "name");
			if (childElement.GetAttribute(szRuleName, "rule") == false)
				strcpy(szRuleName, GetDefaultChannelRuleName());
			childElement.GetAttribute(&nMaxPlayers, "maxplayers");
			childElement.GetAttribute(&nLevelMin, "levelmin");
			childElement.GetAttribute(&nLevelMax, "levelmax");
			childElement.GetAttribute(&bIsTicketChannel, "IsTicketChannel");
			childElement.GetAttribute(&nTicketItemID, "TicketItemID");
			childElement.GetAttribute(&bIsDuelTournament, "IsDuelTournament");

			if (bIsDuelTournament)
				nChannelType = MCHANNEL_TYPE_DUELTOURNAMENT;

			MUID uidChannel;
			ChannelAdd(MGetStringResManager()->GetStringFromXml(szBuf),
				szRuleName, &uidChannel, nChannelType, nMaxPlayers, nLevelMin, nLevelMax,
				bIsTicketChannel, static_cast<DWORD>(nTicketItemID), bIsTicketChannel, szBuf);
		}
		else if (!strcmp(szTagName, MTOK_DEFAULTCHANNELNAME))
		{
			childElement.GetAttribute(szBuf, "name");
			SetDefaultChannelName(MGetStringResManager()->GetStringFromXml(szBuf));
		}
		else if (!strcmp(szTagName, MTOK_DEFAULTRULENAME))
		{
			childElement.GetAttribute(szBuf, "name");
			SetDefaultChannelRuleName(szBuf);
		}
		else if (!strcmp(szTagName, MTOK_CLAN_CHANNEL))
		{
			bool	bIsTicketChannel = false;
			int		nTicketItemID = -1;

			childElement.GetAttribute(&bIsTicketChannel, "IsTicketChannel");
			childElement.GetAttribute(&nTicketItemID, "TicketItemID");

			GetChannelMap()->SetIsTicketClanChannel(bIsTicketChannel);
			GetChannelMap()->SetClanChannelTicketItemID(nTicketItemID);
		}
	}

	xmlIniData.Destroy();
	return true;
}

bool MMatchServer::InitDB()
{
	CString str = m_MatchDBMgr.BuildDSNString(MGetServerConfig()->GetDB_DNS(),
		MGetServerConfig()->GetDB_UserName(),
		MGetServerConfig()->GetDB_Password());

	if (m_MatchDBMgr.Connect())
	{
		LOG(LOG_PROG, "DBMS connected\n");
	}
	else
	{
		LOG(LOG_PROG, "Can't Connect To DBMS\n");
		return false;
	}

	if (MGetServerConfig()->IsUseFilter())
	{
		if (InitCountryFilterDB())
			LOG(LOG_PROG, "InitCountryFilterDB.\n");
		else
		{
			LOG(LOG_PROG, "Fail to init country filter DB.\n");
			return false;
		}
	}

	return true;
}

#include "MLadderMgr.h"
#include "MTeamGameStrategy.h"

LONG WINAPI TopLevelExceptionFilter(_EXCEPTION_POINTERS* pException)
{
	printf("Crash!\n");

	mlog("TopLevelExceptionFilter...throw...");

	char szDumpFileName[_MAX_DIR] = { 0, };
	SYSTEMTIME SystemTime;
	GetLocalTime(&SystemTime);
	sprintf(szDumpFileName, "Log/UnhandledDump_%d-%d-%d_%d-%d-%d.dmp"
		, SystemTime.wYear
		, SystemTime.wMonth
		, SystemTime.wDay
		, SystemTime.wHour
		, SystemTime.wMinute
		, SystemTime.wSecond);

	return EXCEPTION_EXECUTE_HANDLER;
}

bool MMatchServer::Create(int nPort)
{
	::SetUnhandledExceptionFilter(TopLevelExceptionFilter);

	srand(timeGetTime());

	m_NextUseUID.SetZero();
	m_NextUseUID.Increase(10);

	SetupRCPLog(RcpLog);
	m_RealCPNet.SetLogLevel(0);
	if (!LoadInitFile())	return false;
	if (!InitDB())			return false;

	if (MGetServerConfig()->IsItemConsistency() == true) {
		if (CheckItemXMLFromDatabase() == false) {
			return false;
		}
	}

	m_AsyncProxy.Create(DEFAULT_ASYNCPROXY_THREADPOOL);
	m_Admin.Create(this);

	m_MatchDBMgr.UpdateServerInfo(MGetServerConfig()->GetServerID()
		, MGetServerConfig()->GetMaxUser()
		, MGetServerConfig()->GetServerName());

	if (nPort != MGetServerConfig()->GetServerPort())
		nPort = MGetServerConfig()->GetServerPort();
	if (MServer::Create(nPort) == false) {
		return false;
	}

	MGetServerStatusSingleton()->Create(this);

	if (!InitScheduler()) {
		LOG(LOG_PROG, "Match Server Scheduler Create FAILED\n");
		return false;
	}

	MMatchAntiHack::InitClientFileList();

	if (!InitGambleMachine())
	{
		mlog("init gamble machine fail.\n");
		return false;
	}

	if (!InitBattletimeRewardMachine())
	{
		mlog("Init Battletime Reward Machine Fail.\n");
		return false;
	}

	int nUDPPort = MATCHSERVER_DEFAULT_UDP_PORT;
	if (nUDPPort != MGetServerConfig()->GetServerUDPPort())
		nUDPPort = MGetServerConfig()->GetServerUDPPort();
	if (m_SafeUDP.Create(true, nUDPPort) == false) {
		LOG(LOG_PROG, "Match Server SafeUDP Create FAILED (Port:%d)\n", MATCHSERVER_DEFAULT_UDP_PORT);
		return false;
	}

	m_SafeUDP.SetCustomRecvCallback(UDPSocketRecvEvent);

	if (OnCreate() == false) {
		LOG(LOG_PROG, "Match Server create FAILED (Port:%d)\n", nPort);
		return false;
	}

	m_bCreated = true;
	LOG(LOG_PROG, "Match Server Created (Port:%d)\n", nPort);

	g_PointerChecker[0].Init(NULL);
	g_PointerChecker[1].Init(m_pScheduler);
	g_PointerChecker[2].Init(m_pAuthBuilder);

	return true;
}

void MMatchServer::Destroy(void)
{
	m_bCreated = false;

	GetQuest()->Destroy();

	for (MMatchObjectList::iterator ObjItor = m_Objects.begin();
		ObjItor != m_Objects.end(); ++ObjItor)
	{
		MMatchObject* pObj = (*ObjItor).second;
		if (pObj)
		{
			CharFinalize(pObj->GetUID());
		}
	}

	m_ClanMap.Destroy();

	m_ChannelMap.Destroy();

	m_SafeUDP.Destroy();

	GetGambleMachine().Release();

	OnDestroy();
	m_Admin.Destroy();
	m_AsyncProxy.Destroy();
	MGetMatchShop()->Destroy();
	m_MatchDBMgr.Disconnect();

	AgentClear();

	MServer::Destroy();

	MMatchStringResManager::FreeInstance();

	LOG(LOG_PROG, "Match Server Destoryed\n");
}

void MMatchServer::Shutdown()
{
	Log(LOG_PROG, "MatchServer Shutting down...\n");
}

bool MMatchServer::OnCreate(void)
{
	if (MGetServerConfig()->IsEnabledDuelTournament()) {
		GetDTMgr()->Init();
	}

	return true;
}
void MMatchServer::OnDestroy(void)
{
	if (0 != m_pScheduler) {
		m_pScheduler->Release();
		delete m_pScheduler;
		m_pScheduler = 0;
	}

	if (MGetServerConfig()->IsEnabledDuelTournament()) {
		GetDTMgr()->Destory();
	}
}

void MMatchServer::OnRegisterCommand(MCommandManager* pCommandManager)
{
	MCommandCommunicator::OnRegisterCommand(pCommandManager);
	MAddSharedCommandTable(pCommandManager, MSCT_MATCHSERVER);
	Log(LOG_PROG, "Command registeration completed\n");
}

void MMatchServer::OnPrepareRun()
{
	MServer::OnPrepareRun();
	MGetServerStatusSingleton()->AddCmdCount(m_CommandManager.GetCommandQueueCount());
}

int MMatchServer::Connect(MCommObject* pCommObj)
{
	if (pCommObj) {
		mlog("MMatchServer::Connect - Target IP(%s), Target Port(%d)", pCommObj->GetIPString(), pCommObj->GetPort());
	}

	return MServer::Connect(pCommObj);
}

int MMatchServer::OnConnected(MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp, MCommObject* pCommObj)
{
	if (pCommObj == NULL) {
		mlog("MMatchServer::OnConnected - pTargetUID(%d%d), pAllocUID(%d%d)\n", pTargetUID->High, pTargetUID->Low, pAllocUID->High, pAllocUID->Low);
	}
	else {
		if (pCommObj->IsPassiveSocket()) {
			mlog("MMatchServer::OnConnected - pTargetUID(%d%d), pAllocUID(%d%d), IP(%s), Port(%d), Passive Socket\n"
				, pTargetUID->High, pTargetUID->Low, pAllocUID->High, pAllocUID->Low, pCommObj->GetIPString(), pCommObj->GetPort());
		}
		else {
			mlog("MMatchServer::OnConnected - pTargetUID(%d%d), pAllocUID(%d%d), IP(%s), Port(%d), Active Socket\n"
				, pTargetUID->High, pTargetUID->Low, pAllocUID->High, pAllocUID->Low, pCommObj->GetIPString(), pCommObj->GetPort());
		}
	}

	mlog("MMatchServer::OnConnected - We should not call func which is called 'MCommandCommunicator::OnConnected'\n");
	return MOK;
}

void MMatchServer::OnRun(void)
{
#ifdef _DEBUG
#endif
	MGetServerStatusSingleton()->SetRunStatus(100);
	SetTickTime(timeGetTime());

	MGetCheckLoopTimeInstance()->SetSchedulerTick();
	if (m_pScheduler)
		m_pScheduler->Update();

	MGetCheckLoopTimeInstance()->SetPremiumIPCacheTick();
	MPremiumIPCache()->Update();

	MGetServerStatusSingleton()->SetRunStatus(101);

	MGetCheckLoopTimeInstance()->SetObjectTick();

	MUID uidDisUser;
	unsigned long int nGlobalClock = GetGlobalClockCount();
	unsigned long int nHShieldClock = GetGlobalClockCount();

	for (MMatchObjectList::iterator i = m_Objects.begin(); i != m_Objects.end();)
	{
		MMatchObject* pObj = (*i).second;
		pObj->Tick(nGlobalClock);

		if (pObj->GetCharInfo()) {
			if (pObj->GetCharInfo()->GetDBCachingData()->IsRequestUpdate()) {
				UpdateCharDBCachingData(pObj);
			}
		}

		if (!pObj->m_DBJobQ.DBJobQ.empty() && !pObj->m_DBJobQ.bIsRunningAsyncJob)
		{
			MAsyncJob* pAsyncJob = pObj->m_DBJobQ.DBJobQ.front();
			pObj->m_DBJobQ.DBJobQ.pop_front();
			pObj->m_DBJobQ.nLastJobID = pAsyncJob->GetJobID();
			PostAsyncJob(pAsyncJob);
			pObj->m_DBJobQ.bIsRunningAsyncJob = true;
		}

		if (pObj->CheckDestroy(nGlobalClock))
		{
			uidDisUser = pObj->GetUID();

			Disconnect(uidDisUser);
		}
		else if (pObj->GetDisconnStatusInfo().IsSendDisconnMsg())
		{
			MCommand* pCmd = CreateCommand(MC_MATCH_DISCONNMSG, pObj->GetUID());
			pCmd->AddParameter(new MCmdParamUInt(pObj->GetDisconnStatusInfo().GetMsgID()));
			Post(pCmd);

			pObj->GetDisconnStatusInfo().SendCompleted();
		}
		else if (pObj->GetDisconnStatusInfo().IsDisconnectable(nGlobalClock))
		{
			uidDisUser = pObj->GetUID();

			Disconnect(uidDisUser);
		}
		++i;
	}

	MGetServerStatusSingleton()->SetRunStatus(102);

	MGetCheckLoopTimeInstance()->SetStageTick();
	for (MMatchStageMap::iterator iStage = m_StageMap.begin(); iStage != m_StageMap.end();)
	{
		MMatchStage* pStage = (*iStage).second;
		pStage->Tick(nGlobalClock);
		if (pStage->GetState() == STAGE_STATE_CLOSE)
		{
			StageRemove(pStage->GetUID(), &iStage);
			continue;
		}
		++iStage;
	}

	MGetServerStatusSingleton()->SetRunStatus(103);

	MGetCheckLoopTimeInstance()->SetChannelTick();
	m_ChannelMap.Update(nGlobalClock);

	MGetServerStatusSingleton()->SetRunStatus(104);

	MGetCheckLoopTimeInstance()->SetClanTick();
	m_ClanMap.Tick(nGlobalClock);

	MGetServerStatusSingleton()->SetRunStatus(105);

	MGetCheckLoopTimeInstance()->SetLadderTick();

	if (MGetServerConfig()->GetServerMode() == MSM_CLAN)
	{
		GetLadderMgr()->Tick(nGlobalClock);
	}

	if (MGetServerConfig()->IsEnabledDuelTournament() == true) {
		GetDTMgr()->Tick(nGlobalClock);
	}
	MGetServerStatusSingleton()->SetRunStatus(106);

	MGetCheckLoopTimeInstance()->SetPingTick();

#define MINTERVAL_GARBAGE_SESSION_PING	(3 * 60 * 1000)
	static unsigned long tmLastGarbageSessionCleaning = nGlobalClock;
	if (nGlobalClock - tmLastGarbageSessionCleaning > MINTERVAL_GARBAGE_SESSION_PING)
	{
		tmLastGarbageSessionCleaning = nGlobalClock;

		LOG(LOG_PROG, "GARBAGE SESSION CLEANING : m_This(%d%d), ClientCount=%d, SessionCount=%d, AgentCount=%d,  objHistoryCount=%d\n",
			m_This.High, m_This.Low, GetClientCount(), GetCommObjCount(), GetAgentCount(),
			m_objectCommandHistory.GetObjectCount());
		MCommand* pNew = CreateCommand(MC_NET_PING, MUID(0, 0));
		pNew->AddParameter(new MCmdParamUInt(GetGlobalClockCount()));
		RouteToAllConnection(pNew);
	}

	MGetServerStatusSingleton()->SetRunStatus(107);

	MGetCheckLoopTimeInstance()->SetSessionCleanTick();

#define MINTERVAL_GARBAGE_SESSION_CLEANING	10*60*1000
	for (MMatchObjectList::iterator i = m_Objects.begin(); i != m_Objects.end(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)((*i).second);
		if (pObj->GetUID() < MUID(0, 3)) continue;
		if (GetTickTime() - pObj->GetTickLastPacketRecved() >= MINTERVAL_GARBAGE_SESSION_CLEANING)
		{
			if (pObj->GetDisconnStatusInfo().GetStatus() == MMDS_CONNECTED)
			{
				LOG(LOG_PROG, "TIMEOUT CLIENT CLEANING : %s(%u%u, %s) (ClientCnt=%d, SessionCnt=%d)",
					pObj->GetName(), pObj->GetUID().High, pObj->GetUID().Low, pObj->GetIPString(), GetClientCount(), GetCommObjCount());
				pObj->GetDisconnStatusInfo().SetStatus(MMDS_DISCONN_WAIT);
				pObj->GetDisconnStatusInfo().SetMsgID(MERR_CLEANING_CLIENT);
			}
		}
	}

	MGetServerStatusSingleton()->SetRunStatus(108);

	MGetCheckLoopTimeInstance()->SetAsyncJopTick();
	ProcessAsyncJob();

	MGetServerStatusSingleton()->SetRunStatus(109);

	MGetCheckLoopTimeInstance()->SetServerLogTick();
	UpdateServerLog();

	MGetCheckLoopTimeInstance()->SetServerDBLogTick();
	UpdateServerStatusDB();

	MGetServerStatusSingleton()->SetRunStatus(110);

	MGetCheckLoopTimeInstance()->SetCustomIPListTick();
	if (3600000 < (nGlobalClock - m_CountryFilter.GetLastUpdatedTime()))
	{
		UpdateCustomIPList();
		m_CountryFilter.SetLastUpdatedTime(nGlobalClock);
	}

	MGetServerStatusSingleton()->SetRunStatus(111);

	MGetCheckLoopTimeInstance()->SetShutdownTick();
	m_MatchShutdown.OnRun(nGlobalClock);

	MGetServerStatusSingleton()->SetRunStatus(112);

	static DWORD dwLastCommandHistoryUpdate = nGlobalClock;
	int elapsed = nGlobalClock - dwLastCommandHistoryUpdate;
	if (1000 < abs(elapsed))
	{
		dwLastCommandHistoryUpdate = nGlobalClock;
		m_objectCommandHistory.Update(nGlobalClock);
		m_connectionHistory.Update(nGlobalClock);
	}

	if (GetDTMgr()->GetTimeStampChanged() && MGetServerConfig()->IsEnabledDuelTournament()) {
		static DWORD dwLastRequestGetDTCharacterInfo = nGlobalClock;

		if (nGlobalClock - dwLastRequestGetDTCharacterInfo > 1500) {
			dwLastRequestGetDTCharacterInfo = nGlobalClock;

			bool bAllSameTimeStamp = true;

			int nCount = 0;
			for (MMatchObjectList::iterator i = m_Objects.begin(); i != m_Objects.end(); i++) {
				MMatchObject* pObj = (MMatchObject*)((*i).second);
				MMatchObjectDuelTournamentCharInfo* pInfo = pObj->GetDuelTournamentCharInfo();

				if (nCount == 50) break;
				if (pInfo != NULL)
				{
					if (GetDTMgr()->IsSameTimeStamp(pInfo->GetTimeStamp()) == false) {
						bAllSameTimeStamp = false;

						if (pObj->GetDuelTournamentCharInfo()->IsJoinDuelTournament() == false) {
							LOG(LOG_PROG, "Request Character Info, Character Previous Info(%d%d)", pObj->GetUID().High, pObj->GetUID().Low);

							OnAsyncRequest_GetDuelTournamentCharacterInfo(pObj->GetUID(), pObj->GetCharInfo()->m_nCID);
							OnAsyncRequest_GetDuelTournamentPreviousCharacterInfo(pObj->GetUID(), pObj->GetCharInfo()->m_nCID);

							nCount++;
						}
					}
				}
			}

			if (bAllSameTimeStamp) { GetDTMgr()->SetTimeStampChanged(false); }
		}
	}

	if (nGlobalClock - GetBattleTimeRewardMachine().GetLastUpdateTime() > MGetServerConfig()->GetBRDescriptionRefreshInterval())
	{
		GetBattleTimeRewardMachine().SetLastUpdateTime(nGlobalClock);

		MAsyncDBJob_GetBattleTimeRewardDescription* pJob = new MAsyncDBJob_GetBattleTimeRewardDescription;
		MMatchServer::GetInstance()->PostAsyncJob(pJob);
	}
}

void MMatchServer::UpdateServerLog()
{
	if (!IsCreated()) return;

#define SERVER_LOG_TICK		(60000)

	static unsigned long int st_nElapsedTime = 0;
	static unsigned long int nLastTime = timeGetTime();
	unsigned long int nNowTime = timeGetTime();

	st_nElapsedTime += (nNowTime - nLastTime);

	if (st_nElapsedTime > SERVER_LOG_TICK)
	{
		st_nElapsedTime = 0;

		m_MatchDBMgr.InsertServerLog(MGetServerConfig()->GetServerID(),
			(int)m_Objects.size(), (int)m_StageMap.size(),
			GetBlockCount(), GetNonBlockCount());
		ResetBlockCount();
		ResetNonBlockCount();
	}

	nLastTime = nNowTime;
}

void MMatchServer::UpdateServerStatusDB()
{
	if (!IsCreated()) return;

#define SERVER_STATUS_TICK		(30000)

	static unsigned long int st_nElapsedTime = 0;
	static unsigned long int nLastTime = timeGetTime();
	unsigned long int nNowTime = timeGetTime();

	st_nElapsedTime += (nNowTime - nLastTime);

	if (st_nElapsedTime > SERVER_STATUS_TICK)
	{
		st_nElapsedTime = 0;

		int nCurPlayer = (int)m_Objects.size();
		if (nCurPlayer > MGetServerConfig()->GetMaxUser())
			nCurPlayer = MGetServerConfig()->GetMaxUser();

		bool bResult = false;
		static int st_ErrCounter = 0;

#ifdef LOCALE_KOREA
		int nNatePlayer = 0;

		for (MMatchObjectList::iterator iter = m_Objects.begin(); iter != m_Objects.end(); iter++)
		{
			MMatchObject* pObj = (MMatchObject*)iter->second;
			if (pObj->GetAccountInfo()->m_nCCode == 30) { nNatePlayer++; }
		}

		bResult = m_MatchDBMgr.UpdateServerStatus_Netmarble(MGetServerConfig()->GetServerID(), nCurPlayer, nNatePlayer);
#else
		bResult = m_MatchDBMgr.UpdateServerStatus(MGetServerConfig()->GetServerID(), nCurPlayer);
#endif

		if (bResult == false)
		{
			LOG(LOG_PROG, "[CRITICAL ERROR] DB Connection Lost.\n");

			m_MatchDBMgr.Disconnect();

			InitDB();
			st_ErrCounter++;
			if (st_ErrCounter > MAX_DB_QUERY_COUNT_OUT)
			{
				LOG(LOG_PROG, "[CRITICAL ERROR] UpdateServerStatusDB - Shutdown\n");
				Shutdown();
			}
		}
		else
		{
			st_ErrCounter = 0;
		}
	}

	nLastTime = nNowTime;
}

inline void MMatchServer::RouteToListener(MObject* pObject, MCommand* pCommand)
{
	if (pObject == NULL) return;

	size_t nListenerCount = pObject->m_CommListener.size();
	if (nListenerCount <= 0) {
		delete pCommand;
		return;
	}
	else if (nListenerCount == 1) {
		MUID TargetUID = *pObject->m_CommListener.begin();
		pCommand->m_Receiver = TargetUID;
		Post(pCommand);
	}
	else {
		int nCount = 0;
		for (list<MUID>::iterator itorUID = pObject->m_CommListener.begin(); itorUID != pObject->m_CommListener.end(); itorUID++) {
			MUID TargetUID = *itorUID;

			MCommand* pSendCmd;
			if (nCount <= 0)
				pSendCmd = pCommand;
			else
				pSendCmd = pCommand->Clone();
			pSendCmd->m_Receiver = TargetUID;
			Post(pSendCmd);
			nCount++;
		}
	}
}

void MMatchServer::RouteResponseToListener(MObject* pObject, const int nCmdID, int nResult)
{
	MCommand* pNew = CreateCommand(nCmdID, MUID(0, 0));
	pNew->AddParameter(new MCmdParamInt(nResult));
	RouteToListener(pObject, pNew);
}

struct stRouteListenerNode
{
	DWORD				nUserContext;
	MPacketCrypterKey	CryptKey;
};

void MMatchServer::RouteToAllConnection(MCommand* pCommand)
{
	queue<stRouteListenerNode*>	ListenerList;

	LockCommList();
	for (MUIDRefCache::iterator i = m_CommRefCache.begin(); i != m_CommRefCache.end(); i++) {
		MCommObject* pCommObj = (MCommObject*)((*i).second);
		if (pCommObj->GetUID() < MUID(0, 3)) continue;

		stRouteListenerNode* pNewNode = new stRouteListenerNode;
		pNewNode->nUserContext = pCommObj->GetUserContext();
		memcpy(&pNewNode->CryptKey, pCommObj->GetCrypter()->GetKey(), sizeof(MPacketCrypterKey));
		ListenerList.push(pNewNode);
	}
	UnlockCommList();

	int nCmdSize = pCommand->GetSize();

	if (nCmdSize <= 0)
	{
		while (!ListenerList.empty())
		{
			stRouteListenerNode* pNode = ListenerList.front();
			ListenerList.pop();
			delete pNode;
		}
		return;
	}

	char* pCmdData = new char[nCmdSize];
	int nSize = pCommand->GetData(pCmdData, nCmdSize);
	if (pCommand->m_pCommandDesc->IsFlag(MCCT_NON_ENCRYPTED))
	{
		while (!ListenerList.empty())
		{
			stRouteListenerNode* pNode = ListenerList.front();
			ListenerList.pop();

			SendMsgCommand(pNode->nUserContext, pCmdData, nSize, MSGID_RAWCOMMAND, NULL);

			delete pNode;
		}
	}
	else
	{
		while (!ListenerList.empty())
		{
			stRouteListenerNode* pNode = ListenerList.front();
			ListenerList.pop();

			SendMsgCommand(pNode->nUserContext, pCmdData, nSize, MSGID_COMMAND, &pNode->CryptKey);

			delete pNode;
		}
	}

	delete[] pCmdData;
	delete pCommand;
}

void MMatchServer::RouteToAllClient(MCommand* pCommand)
{
	for (MMatchObjectList::iterator i = m_Objects.begin(); i != m_Objects.end(); i++) {
		MMatchObject* pObj = (MMatchObject*)((*i).second);
		if (pObj->GetUID() < MUID(0, 3)) continue;

		MCommand* pSendCmd = pCommand->Clone();
		pSendCmd->m_Receiver = pObj->GetUID();
		Post(pSendCmd);
	}
	delete pCommand;
}

void MMatchServer::RouteToChannel(const MUID& uidChannel, MCommand* pCommand)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL)
	{
		delete pCommand;
		return;
	}

	for (MUIDRefCache::iterator i = pChannel->GetObjBegin(); i != pChannel->GetObjEnd(); i++) {
		MObject* pObj = (MObject*)(*i).second;

		MCommand* pSendCmd = pCommand->Clone();
		RouteToListener(pObj, pSendCmd);
	}
	delete pCommand;
}

void MMatchServer::RouteToChannelLobby(const MUID& uidChannel, MCommand* pCommand)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL)
	{
		delete pCommand;
		return;
	}

	for (MUIDRefCache::iterator i = pChannel->GetLobbyObjBegin(); i != pChannel->GetLobbyObjEnd(); i++)
	{
		MObject* pObj = (MObject*)(*i).second;

		MCommand* pSendCmd = pCommand->Clone();
		RouteToListener(pObj, pSendCmd);
	}
	delete pCommand;
}

void MMatchServer::RouteToStage(const MUID& uidStage, MCommand* pCommand)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL)
	{
		delete pCommand;
		return;
	}

	for (MUIDRefCache::iterator i = pStage->GetObjBegin(); i != pStage->GetObjEnd(); i++) {
		MUID uidObj = (MUID)(*i).first;
		MObject* pObj = (MObject*)GetObject(uidObj);
		if (pObj) {
			MCommand* pSendCmd = pCommand->Clone();
			RouteToListener(pObj, pSendCmd);
		}
		else {
			LOG(LOG_PROG, "WARNING(RouteToStage) : Not Existing Obj(%u:%u)\n", uidObj.High, uidObj.Low);
			i = pStage->RemoveObject(uidObj);
			LogObjectCommandHistory(uidObj);
		}
	}
	delete pCommand;
}

void MMatchServer::RouteToObjInStage(const MUID& uidStage, const MUID& uidTargetObj, MCommand* pCommand)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL)
	{
		delete pCommand;
		return;
	}

	for (MUIDRefCache::iterator i = pStage->GetObjBegin(); i != pStage->GetObjEnd(); i++) {
		MUID uidObj = (MUID)(*i).first;
		if (uidObj == uidTargetObj) {
			MObject* pObj = (MObject*)GetObject(uidObj);
			if (pObj) {
				MCommand* pSendCmd = pCommand->Clone();
				RouteToListener(pObj, pSendCmd);
			}
			else {
				LOG(LOG_PROG, "WARNING(RouteToStage) : Not Existing Obj(%u:%u)\n", uidObj.High, uidObj.Low);
				i = pStage->RemoveObject(uidObj);
				LogObjectCommandHistory(uidObj);
			}
		}
	}
	delete pCommand;
}

void MMatchServer::RouteToStageWaitRoom(const MUID& uidStage, MCommand* pCommand)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL)
	{
		delete pCommand;
		return;
	}

	for (MUIDRefCache::iterator i = pStage->GetObjBegin(); i != pStage->GetObjEnd(); i++) {
		MUID uidObj = (MUID)(*i).first;
		MMatchObject* pObj = (MMatchObject*)GetObject(uidObj);
		if (pObj) {
			if (!pObj->GetEnterBattle())
			{
				MCommand* pSendCmd = pCommand->Clone();
				RouteToListener(pObj, pSendCmd);
			}
		}
	}
	delete pCommand;
}

void MMatchServer::RouteToBattle(const MUID& uidStage, MCommand* pCommand)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL)
	{
		delete pCommand;
		return;
	}

	for (MUIDRefCache::iterator i = pStage->GetObjBegin(); i != pStage->GetObjEnd(); i++) {
		MUID uidObj = (MUID)(*i).first;
		MMatchObject* pObj = (MMatchObject*)GetObject(uidObj);
		if (pObj) {
			if (pObj->GetEnterBattle())
			{
				MCommand* pSendCmd = pCommand->Clone();
				RouteToListener(pObj, pSendCmd);
			}
		}
		else {
			LOG(LOG_PROG, "WARNING(RouteToBattle) : Not Existing Obj(%u:%u)\n", uidObj.High, uidObj.Low);
			i = pStage->RemoveObject(uidObj);
			LogObjectCommandHistory(uidObj);
		}
	}
	delete pCommand;
}

void MMatchServer::RouteToClan(const int nCLID, MCommand* pCommand)
{
	MMatchClan* pClan = FindClan(nCLID);
	if (pClan == NULL)
	{
		delete pCommand;
		return;
	}

	for (MUIDRefCache::iterator i = pClan->GetMemberBegin(); i != pClan->GetMemberEnd(); i++) {
		MObject* pObj = (MObject*)(*i).second;

		MCommand* pSendCmd = pCommand->Clone();
		RouteToListener(pObj, pSendCmd);
	}
	delete pCommand;
}

void MMatchServer::ResponseRoundState(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	MMatchRule* pRule = pStage->GetRule();
	if (pRule == NULL) return;

	DWORD dwClockPassed = GetGlobalClockCount() - pRule->GetRoundStateTimer();

	if (dwClockPassed < 0 || GetGlobalClockCount() <= pRule->GetRoundStateTimer())
		dwClockPassed = 0;

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_ROUNDSTATE, MUID(0, 0));
	pCmd->AddParameter(new MCommandParameterUID(pStage->GetUID()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundCount()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundState()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundArg()));
	pCmd->AddParameter(new MCommandParameterUInt(dwClockPassed));

	RouteToBattle(uidStage, pCmd);
}

void MMatchServer::ResponseRoundState(MMatchObject* pObj, const MUID& uidStage)
{
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	MMatchRule* pRule = pStage->GetRule();
	if (pRule == NULL) return;

	DWORD dwClockPassed = GetGlobalClockCount() - pRule->GetRoundStateTimer();

	if (dwClockPassed < 0 || GetGlobalClockCount() <= pRule->GetRoundStateTimer())
		dwClockPassed = 0;

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_ROUNDSTATE, MUID(0, 0));
	pCmd->AddParameter(new MCommandParameterUID(pStage->GetUID()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundCount()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundState()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundArg()));
	pCmd->AddParameter(new MCommandParameterUInt(dwClockPassed));

	RouteToListener(pObj, pCmd);
}

void MMatchServer::NotifyMessage(const MUID& uidChar, int nMsgID)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_NOTIFY), MUID(0, 0), m_This);
	pNew->AddParameter(new MCommandParameterUInt(nMsgID));
	RouteToListener(pObj, pNew);
}

int MMatchServer::ObjectAdd(const MUID& uidComm)
{
	MMatchObject* pObj = new MMatchObject(uidComm);
	pObj->UpdateTickLastPacketRecved();

	m_Objects.insert(MMatchObjectList::value_type(pObj->GetUID(), pObj));
	return MOK;
}

int MMatchServer::ObjectRemove(const MUID& uid, MMatchObjectList::iterator* pNextItor)
{
	MMatchObjectList::iterator i = m_Objects.find(uid);
	if (i == m_Objects.end()) return MERR_OBJECT_INVALID;

	MMatchObject* pObj = (*i).second;

	if (pObj->GetDisconnStatusInfo().IsUpdateDB())
	{
		DWORD	dwCID = 0;
		string	strChannelName;

		if (NULL != pObj->GetCharInfo())
			dwCID = pObj->GetCharInfo()->m_nCID;

		MMatchChannel* pChannel = GetChannelMap()->Find(pObj->GetChannelUID());
		if (NULL != pChannel)
			strChannelName = pChannel->GetName();

		MAsyncDBJob_SetBlockHacking* pJob = new MAsyncDBJob_SetBlockHacking(uid);

		pJob->Input(pObj->GetAccountInfo()->m_nAID
			, dwCID
			, pObj->GetDisconnStatusInfo().GetHackingType()
			, pObj->GetDisconnStatusInfo().GetBlockLevel()
			, pObj->GetDisconnStatusInfo().GetComment()
			, pObj->GetIPString()
			, pObj->GetDisconnStatusInfo().GetEndDate()
			, MGetServerConfig()->GetServerID()
			, strChannelName);

		pObj->m_DBJobQ.DBJobQ.push_back(pJob);

		pObj->GetDisconnStatusInfo().UpdateDataBaseCompleted();
	}

	if (pObj->GetChatRoomUID() != MUID(0, 0)) {
		MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
		MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoom(pObj->GetChatRoomUID());
		if (pRoom)
			pRoom->RemovePlayer(pObj->GetUID());
	}

	if (pObj->GetStageUID() != MUID(0, 0)) { StageLeaveBattle(pObj->GetUID(), true, true); }
	if (pObj->GetStageUID() != MUID(0, 0)) { StageLeave(pObj->GetUID()); }
	if (pObj->GetChannelUID() != MUID(0, 0)) { ChannelLeave(pObj->GetUID(), pObj->GetChannelUID()); }

	m_ClanMap.RemoveObject(pObj->GetUID(), pObj);

	CharFinalize(pObj->GetUID());

	if (NULL != pObj->GetCharInfo())
	{
		pObj->FreeCharInfo();
		pObj->FreeDuelTournamentInfo();
		pObj->FreeFriendInfo();
	}

	for (list<MAsyncJob*>::iterator itAJob = pObj->m_DBJobQ.DBJobQ.begin()
		; itAJob != pObj->m_DBJobQ.DBJobQ.end()
		; itAJob = pObj->m_DBJobQ.DBJobQ.begin())
	{
		MAsyncJob* pJob = *itAJob;

		pObj->m_DBJobQ.DBJobQ.pop_front();

		PostAsyncJob(pJob);
	}
	delete pObj;
	pObj = NULL;

	MMatchObjectList::iterator itorTemp = m_Objects.erase(i);
	if (pNextItor)
		*pNextItor = itorTemp;

	return MOK;
}

MMatchObject* MMatchServer::GetObject(const MUID& uid)
{
	MMatchObjectList::iterator i = m_Objects.find(uid);
	if (i == m_Objects.end()) return NULL;
	return (*i).second;
}

MMatchObject* MMatchServer::GetPlayerByCommUID(const MUID& uid)
{
	for (MMatchObjectList::iterator i = m_Objects.begin(); i != m_Objects.end(); i++) {
		MMatchObject* pObj = ((*i).second);
		for (list<MUID>::iterator j = pObj->m_CommListener.begin(); j != pObj->m_CommListener.end(); j++) {
			MUID TargetUID = *j;
			if (TargetUID == uid)
				return pObj;
		}
	}
	return NULL;
}

MMatchObject* MMatchServer::GetPlayerByName(const char* pszName)
{
	for (MMatchObjectList::iterator i = m_Objects.begin(); i != m_Objects.end(); i++) {
		MMatchObject* pObj = ((*i).second);
		if (_stricmp(pObj->GetName(), pszName) == 0)
			return pObj;
	}
	return NULL;
}

MMatchObject* MMatchServer::GetPlayerByAID(unsigned long int nAID)
{
	if (nAID == 0) return NULL;

	for (MMatchObjectList::iterator i = m_Objects.begin(); i != m_Objects.end(); i++)
	{
		MMatchObject* pObj = ((*i).second);
		if (pObj->GetAccountInfo()->m_nAID == nAID)
			return pObj;
	}
	return NULL;
}

MUID MMatchServer::UseUID(void)
{
	LockUIDGenerate();
	MUID ret = m_NextUseUID;
	m_NextUseUID.Increase();
	UnlockUIDGenerate();
	return ret;
}

void MMatchServer::SetClientClockSynchronize(const MUID& CommUID)
{
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_CLOCK_SYNCHRONIZE), CommUID, m_This);
	pNew->AddParameter(new MCommandParameterUInt(GetGlobalClockCount()));
	Post(pNew);
}

void MMatchServer::Announce(const MUID& CommUID, char* pszMsg)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_ANNOUNCE, CommUID);
	pCmd->AddParameter(new MCmdParamUInt(0));
	pCmd->AddParameter(new MCmdParamStr(pszMsg));
	Post(pCmd);
}

void MMatchServer::Announce(MObject* pObj, char* pszMsg)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_ANNOUNCE, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamUInt(0));
	pCmd->AddParameter(new MCmdParamStr(pszMsg));
	RouteToListener(pObj, pCmd);
}

void MMatchServer::AnnounceErrorMsg(const MUID& CommUID, const int nErrorCode)
{
}

void MMatchServer::AnnounceErrorMsg(MObject* pObj, const int nErrorCode)
{
}

void MMatchServer::OnBridgePeer(const MUID& uidChar, DWORD dwIP, DWORD nPort)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;
	in_addr addr;
	addr.s_addr = dwIP;
	char* pszIP = inet_ntoa(addr);

	pObj->SetPeerAddr(dwIP, pszIP, (unsigned short)nPort);
	pObj->SetBridgePeer(true);
	pObj->SetPlayerFlag(MTD_PlayerFlags_BridgePeer, true);

	ResponseBridgePeer(uidChar, 0);
}

MMatchServer* MMatchServer::GetInstance(void)
{
	return m_pInstance;
}

unsigned long int MMatchServer::GetGlobalClockCount(void) const
{
	unsigned long int i = timeGetTime();
	return i;
}

unsigned long int MMatchServer::ConvertLocalClockToGlobalClock(unsigned long int nLocalClock, unsigned long int nLocalClockDistance)
{
	return (nLocalClock + nLocalClockDistance);
}

unsigned long int MMatchServer::ConvertGlobalClockToLocalClock(unsigned long int nGlobalClock, unsigned long int nLocalClockDistance)
{
	return (nGlobalClock - nLocalClockDistance);
}

void MMatchServer::DebugTest()
{
#ifndef _DEBUG
	return;
#endif

	LOG(LOG_DEBUG, "DebugTest: Object List");
	for (MMatchObjectList::iterator it = m_Objects.begin(); it != m_Objects.end(); it++) {
		MMatchObject* pObj = (*it).second;
		LOG(LOG_DEBUG, "DebugTest: Obj(%d%d)", pObj->GetUID().High, pObj->GetUID().Low);
	}
}

void MMatchServer::SendCommandByUDP(MCommand* pCommand, char* szIP, int nPort)
{
	const int BUF_SIZE = 1024;

	char* szBuf = new char[BUF_SIZE];
	int iMaxPacketSize = BUF_SIZE;

	MPacketHeader a_PacketHeader;
	int iHeaderSize = sizeof(a_PacketHeader);
	int size = pCommand->GetData(szBuf + iHeaderSize, iMaxPacketSize - iHeaderSize);
	size += iHeaderSize;
	a_PacketHeader.nMsg = MSGID_COMMAND;
	a_PacketHeader.nSize = size;
	memcpy(szBuf, &a_PacketHeader, iHeaderSize);

	bool bRet = m_SafeUDP.Send(szIP, nPort, szBuf, size);
}

bool MMatchServer::UDPSocketRecvEvent(DWORD dwIP, WORD wRawPort, char* pPacket, DWORD dwSize)
{
	static DWORD dwMonitorUDPIP = MGetServerConfig()->GetMonitorUDPIP();

	if (dwMonitorUDPIP == dwIP)
	{
		return true;
	}

	if (dwSize < sizeof(MPacketHeader)) return false;

	MPacketHeader* pPacketHeader;
	pPacketHeader = (MPacketHeader*)pPacket;

	if ((dwSize < pPacketHeader->nSize) ||
		((pPacketHeader->nMsg != MSGID_COMMAND) && (pPacketHeader->nMsg != MSGID_RAWCOMMAND))) return false;

	MMatchServer* pServer = MMatchServer::GetInstance();
	pServer->ParseUDPPacket(&pPacket[sizeof(MPacketHeader)], pPacketHeader, dwIP, wRawPort);
	return true;
}

void MMatchServer::ParseUDPPacket(char* pData, MPacketHeader* pPacketHeader, DWORD dwIP, WORD wRawPort)
{
	switch (pPacketHeader->nMsg)
	{
	case MSGID_RAWCOMMAND:
	{
		unsigned short nCheckSum = MBuildCheckSum(pPacketHeader, pPacketHeader->nSize);
		if (pPacketHeader->nCheckSum != nCheckSum) {
			static int nLogCount = 0;
			if (nLogCount++ < 100) {
				mlog("MMatchServer::ParseUDPPacket() -> CHECKSUM ERROR(R=%u/C=%u)\n",
					pPacketHeader->nCheckSum, nCheckSum);
			}
			return;
		}
		else {
			unsigned short nTotalSize = 0;
			memcpy(&nTotalSize, pData, sizeof(nTotalSize));

			MCommand* pCmd = new MCommand();
			if (!pCmd->SetData(pData, &m_CommandManager))
			{
				delete pCmd;
				mlog("fail ParseUDPPacket.\n");
				return;
			}

			if (pCmd->GetID() == MC_MATCH_BRIDGEPEER) {
				pCmd->m_Sender = MUID(0, 0);
				pCmd->m_Receiver = m_This;

				unsigned long nPort = ntohs(wRawPort);

				MCommandParameterUInt* pParamIP = (MCommandParameterUInt*)pCmd->GetParameter(1);
				MCommandParameterUInt* pParamPort = (MCommandParameterUInt*)pCmd->GetParameter(2);
				if (pParamIP == NULL || pParamIP->GetType() != MPT_UINT)
				{
					delete pCmd;
					break;
				}
				if (pParamPort == NULL || pParamPort->GetType() != MPT_UINT)
				{
					delete pCmd;
					break;
				}

				char pData[64];
				MCommandParameterUInt(dwIP).GetData(pData, 64);
				pParamIP->SetData(pData);
				MCommandParameterUInt(nPort).GetData(pData, 64);
				pParamPort->SetData(pData);

				PostSafeQueue(pCmd);
			}
			// Custom: Handle default packets
			else if (pCmd->GetID() == MC_UDP_PING)
			{
				delete pCmd;
			}
			else

			{
				LOG(LOG_FILE, "MMatchServer::ParseUDPPacket: NOT HANDLED COMMAND(%d)", pCmd->GetID());
				delete pCmd;
			}
		}
	}
	break;
	case MSGID_COMMAND:
	{
		Log(LOG_FILE, "MMatchServer::ParseUDPPacket: Parse Packet Error");
	}
	break;
	default:
	{
		Log(LOG_FILE, "MMatchServer::ParseUDPPacket: Parse Packet Error");
	}

	break;
	}
}

void MMatchServer::ResponseBridgePeer(const MUID& uidChar, int nCode)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MCommand* pNew = CreateCommand(MC_MATCH_BRIDGEPEER_ACK, MUID(0, 0));
	pNew->AddParameter(new MCmdParamUID(uidChar));
	pNew->AddParameter(new MCmdParamInt(nCode));
	RouteToListener(pObj, pNew);
}

void MMatchServer::ResponsePeerList(const MUID& uidChar, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_PEERLIST, MUID(0, 0));
	pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));

	int nPeerCount = pStage->GetObjInBattleCount();

	void* pPeerArray = MMakeBlobArray(sizeof(MTD_PeerListNode), nPeerCount);
	int nIndex = 0;
	for (MUIDRefCache::iterator itor = pStage->GetObjBegin(); itor != pStage->GetObjEnd(); itor++) {
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
		if (pObj->GetEnterBattle() == false) continue;

		MTD_PeerListNode* pNode = (MTD_PeerListNode*)MGetBlobArrayElement(pPeerArray, nIndex++);
		memset(pNode, 0, sizeof(MTD_PeerListNode));

		pNode->uidChar = pObj->GetUID();
		pNode->dwIP = pObj->GetIP();
		pNode->nPort = pObj->GetPort();

		CopyCharInfoForTrans(&pNode->CharInfo, pObj->GetCharInfo(), pObj);
		pNode->ExtendInfo.nPlayerFlags = pObj->GetPlayerFlags();
		if (pStage->GetStageSetting()->IsTeamPlay())	pNode->ExtendInfo.nTeam = (char)pObj->GetTeam();
		else								 			pNode->ExtendInfo.nTeam = 0;
	}

	pNew->AddParameter(new MCommandParameterBlob(pPeerArray, MGetBlobArraySize(pPeerArray)));
	MEraseBlobArray(pPeerArray);

	RouteToListener(pObj, pNew);
}

bool MMatchServer::CheckBridgeFault()
{
	for (MMatchObjectList::iterator i = m_Objects.begin(); i != m_Objects.end(); i++) {
		MMatchObject* pObj = (*i).second;
		if (pObj->GetBridgePeer() == false)
			return true;
	}
	return false;
}

void MMatchServer::OnUserWhisper(const MUID& uidComm, char* pszSenderName, char* pszTargetName, char* pszMessage)
{
	if (strlen(pszSenderName) < 2) return;
	if (strlen(pszTargetName) < 2) return;

	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}
	if (pTargetObj->CheckUserOption(MBITFLAG_USEROPTION_REJECT_WHISPER) == true) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_USER_WHISPER_REJECTED);
		return;
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_USER_WHISPER, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamStr(pObj->GetName()));
	pCmd->AddParameter(new MCmdParamStr(pszTargetName));
	pCmd->AddParameter(new MCmdParamStr(pszMessage));
	RouteToListener(pTargetObj, pCmd);
}

void MMatchServer::OnUserWhere(const MUID& uidComm, char* pszTargetName)
{
	if (strlen(pszTargetName) < 2) return;

	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (!IsEnabledObject(pObj)) return;

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	if ((IsAdminGrade(pObj) == false) && (IsAdminGrade(pTargetObj) == true)) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	char szLog[256] = "";

	bool bUnknownChannel = true;

	MMatchChannel* pChannel = FindChannel(pTargetObj->GetChannelUID());
	if (pChannel) {
		if (pTargetObj->GetPlace() == MMP_LOBBY)
		{
			bUnknownChannel = false;
			sprintf(szLog, "[%s] '%s'",
				pTargetObj->GetName(),
				pChannel->GetName());
		}
		else if ((pTargetObj->GetPlace() == MMP_STAGE) || (pTargetObj->GetPlace() == MMP_BATTLE))
		{
			MMatchStage* pStage = FindStage(pTargetObj->GetStageUID());
			if (0 != pStage)
			{
				bUnknownChannel = false;
				sprintf(szLog, "[%s] '%s' , '(%d)%s'",
					pTargetObj->GetName(),
					pChannel->GetName(),
					pStage->GetIndex() + 1,
					pStage->GetName());
			}
		}
	}

	if (bUnknownChannel)
		sprintf(szLog, "%s , Unknown Channel", pTargetObj->GetName());

	Announce(pObj, szLog);
}

void MMatchServer::OnUserOption(const MUID& uidComm, unsigned long nOptionFlags)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pObj->SetUserOption(nOptionFlags);
}

void MMatchServer::OnChatRoomCreate(const MUID& uidPlayer, const char* pszChatRoomName)
{
	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoomByName(pszChatRoomName);
	if (pRoom) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHATROOM_ALREADY_EXIST);
		return;
	}

	pRoom = pChatRoomMgr->AddChatRoom(uidPlayer, pszChatRoomName);
	if (pRoom == NULL) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHATROOM_CREATE_FAILED);
		return;
	}

	if (pRoom->AddPlayer(uidPlayer) == true) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHATROOM_CREATE_SUCCEED);
	}
	else {
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHATROOM_JOIN_FAILED);
	}
}

void MMatchServer::OnChatRoomJoin(const MUID& uidComm, char* pszPlayerName, char* pszChatRoomName)
{
	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoomByName(pszChatRoomName);

	if (pRoom == NULL) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_NOT_EXIST);
		return;
	}

	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	if (pRoom->GetUserCount() > CHATROOM_MAX_ROOMMEMBER) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_USER_FULL);
		return;
	}

	if (pRoom->AddPlayer(uidComm)) {
		MCommand* pCmd = CreateCommand(MC_MATCH_CHATROOM_JOIN, MUID(0, 0));
		pCmd->AddParameter(new MCmdParamStr(pObj->GetName()));
		pCmd->AddParameter(new MCmdParamStr(pszChatRoomName));
		pRoom->RouteCommand(pCmd);
	}
	else {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_JOIN_FAILED);
	}
}

void MMatchServer::OnChatRoomLeave(const MUID& uidComm, char* pszPlayerName, char* pszChatRoomName)
{
	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoomByName(pszChatRoomName);
	if (pRoom == NULL)
		return;

	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pRoom->RemovePlayer(uidComm);

	if (!pRoom->GetUserCount())
		pChatRoomMgr->RemoveChatRoom(pRoom->GetUID());

	MCommand* pCmd = CreateCommand(MC_MATCH_CHATROOM_LEAVE, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamStr(pObj->GetName()));
	pCmd->AddParameter(new MCmdParamStr(pszChatRoomName));
	pRoom->RouteCommand(pCmd);
}

void MMatchServer::OnChatRoomSelectWrite(const MUID& uidComm, const char* pszChatRoomName)
{
	MMatchObject* pPlayer = GetObject(uidComm);
	if (pPlayer == NULL) return;

	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoomByName(pszChatRoomName);
	if (pRoom == NULL) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_NOT_EXIST);
		return;
	}

	if (!pRoom->IsFindPlayer(uidComm)) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_NOT_JOIN);
		return;
	}

	pPlayer->SetChatRoomUID(pRoom->GetUID());
}

void MMatchServer::OnChatRoomInvite(const MUID& uidComm, const char* pszTargetName)
{
	if (strlen(pszTargetName) < 2) return;

	MMatchObject* pPlayer = GetPlayerByCommUID(uidComm);
	if (pPlayer == NULL) return;

	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoom(pPlayer->GetChatRoomUID());
	if (pRoom == NULL) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_NOT_EXIST);
		return;
	}

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);

	if (pTargetObj == NULL) {
		NotifyMessage(pPlayer->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	if (pTargetObj->GetEnterBattle())
	{
		NotifyMessage(pPlayer->GetUID(), MATCHNOTIFY_USER_INVITE_REJECTED);
		return;
	}

	if (pTargetObj->CheckUserOption(MBITFLAG_USEROPTION_REJECT_INVITE) == true) {
		NotifyMessage(pPlayer->GetUID(), MATCHNOTIFY_USER_INVITE_REJECTED);
		NotifyMessage(pTargetObj->GetUID(), MATCHNOTIFY_USER_INVITE_IGNORED);
		return;
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_CHATROOM_INVITE, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamStr(pPlayer->GetName()));
	pCmd->AddParameter(new MCmdParamStr(const_cast<char*>(pszTargetName)));
	pCmd->AddParameter(new MCmdParamStr(const_cast<char*>(pRoom->GetName())));
	RouteToListener(pTargetObj, pCmd);
}

#ifdef _DEBUG
#include "CMLexicalAnalyzer.h"
bool StageFinish(MMatchServer* pServer, const MUID& uidPlayer, char* pszChat)
{
	MMatchObject* pChar = pServer->GetObject(uidPlayer);
	if (pChar == NULL)	return false;
	MMatchStage* pStage = pServer->FindStage(pChar->GetStageUID());
	if (pStage == NULL) return false;

	bool bResult = false;
	CMLexicalAnalyzer lex;
	lex.Create(pszChat);

	if (lex.GetCount() >= 1) {
		char* pszCmd = lex.GetByStr(0);
		if (pszCmd) {
			if (_stricmp(pszCmd, "/finish") == 0) {
				pStage->GetRule()->DebugTest();
				bResult = true;
			}
}
	}

	lex.Destroy();
	return bResult;
}
#endif

void MMatchServer::OnChatRoomChat(const MUID& uidComm, const char* pszMessage)
{
	MMatchObject* pPlayer = GetObject(uidComm);
	if (pPlayer == NULL) return;

	if (pPlayer->GetChatRoomUID() == MUID(0, 0)) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_NOT_USING);
		return;
	}

	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoom(pPlayer->GetChatRoomUID());
	if (pRoom == NULL) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_NOT_EXIST);
		return;
	}

	pRoom->RouteChat(pPlayer->GetUID(), const_cast<char*>(pszMessage));
}

void MMatchServer::DisconnectObject(const MUID& uidObject)
{
	MMatchObject* pObj = GetObject(uidObject);
	if (pObj == NULL) return;

	Disconnect(pObj->GetCommListener());
}

int MMatchServer::ValidateMakingName(const char* szCharName, int nMinLength, int nMaxLength)
{
	int nNameLen = (int)strlen(szCharName);
	if (nNameLen < nMinLength)
		return MERR_TOO_SHORT_NAME;

	if (nNameLen > nMaxLength)
		return MERR_TOO_LONG_NAME;

#ifdef LOCALE_KOREA
	int nCur = 0;
	while (nCur < (nNameLen - 1))
	{
		unsigned char c1 = (unsigned char)szCharName[nCur];
		unsigned char c2 = (unsigned char)szCharName[nCur + 1];

		if (c1 > 127)
		{
			if ((c1 == 0xc9) && (c2 > 127))
			{
				return MERR_WRONG_WORD_NAME;
			}
			if ((c1 == 0xfe) && (c2 > 127))
			{
				return MERR_WRONG_WORD_NAME;
			}
			if ((c1 == 0xa1) && (c2 == 0xa1))
			{
				return MERR_WRONG_WORD_NAME;
			}
			if ((c1 == 0xa4) && (c2 == 0xd4))
			{
				return MERR_WRONG_WORD_NAME;
			}

			nCur += 2;
		}
		else
		{
			nCur++;
		}
	}
#endif

	return MOK;
}

int MMatchServer::ValidateChannelJoin(const MUID& uidPlayer, const MUID& uidChannel)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return MERR_CANNOT_JOIN_CHANNEL;
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return MERR_CANNOT_JOIN_CHANNEL;

	if (!IsAdminGrade(pObj))
	{
		const DWORD dwTicketItemID = pChannel->GetTicketItemID();

		if (MGetServerConfig()->IsUseTicket() && pChannel->IsUseTicket() && pChannel->IsTicketChannel())
		{
			if (!pObj->GetCharInfo()->m_ItemList.IsHave(dwTicketItemID))
				return MERR_CANNOT_JOIN_NEED_TICKET;
		}

		if ((int)pChannel->GetObjCount() >= pChannel->GetMaxPlayers())
		{
			return MERR_CANNOT_JOIN_CHANNEL_BY_MAXPLAYERS;
		}

		if ((pChannel->GetLevelMin() > 0) && (pChannel->GetLevelMin() > pObj->GetCharInfo()->m_nLevel))
		{
			return MERR_CANNOT_JOIN_CHANNEL_BY_LEVEL;
		}
		if ((pChannel->GetLevelMax() > 0) && (pChannel->GetLevelMax() < pObj->GetCharInfo()->m_nLevel))
		{
			return MERR_CANNOT_JOIN_CHANNEL_BY_LEVEL;
		}

		if ((pChannel->GetRuleType() == MCHANNEL_RULE_NEWBIE) && (pObj->IsNewbie() == false))
		{
			return MERR_CANNOT_JOIN_CHANNEL_BY_NEWBIE;
		}
	}

	return MOK;
}

void GetCharEquipItemTotalWeight(MMatchCharInfo* pCharInfo
	, const MMatchCharItemParts parts, MMatchItem* pItem
	, int& nWeight
	, int& nMaxWeight)
{
	pCharInfo->GetTotalWeight(&nWeight, &nMaxWeight);
}

int MMatchServer::ValidateEquipItem(MMatchObject* pObj, MMatchItem* pItem, const MMatchCharItemParts parts)
{
	if (!IsEnabledObject(pObj)) return MERR_UNKNOWN;
	if (pItem == NULL) return MERR_UNKNOWN;

	if (!IsEquipableItem(pItem->GetDescID(), pObj->GetCharInfo()->m_nLevel, pObj->GetCharInfo()->m_nSex))
	{
		return MERR_LOW_LEVEL;
	}

	int nWeight = 0;
	int nMaxWeight = 0;

	MMatchEquipedItem* pEquipedItem = &pObj->GetCharInfo()->m_EquipedItem;
	GetCharEquipItemTotalWeight(pObj->GetCharInfo(), parts, pItem, nWeight, nMaxWeight);

	if (!pEquipedItem->IsEmpty(parts))
	{
		if (pEquipedItem->GetItem(parts)->GetDesc() != NULL)
		{
			nWeight -= pEquipedItem->GetItem(parts)->GetDesc()->m_nWeight.Ref();
			nMaxWeight -= pEquipedItem->GetItem(parts)->GetDesc()->m_nMaxWT.Ref();
		}
	}

	if (pItem->GetDesc() != NULL)
	{
		nWeight += pItem->GetDesc()->m_nWeight.Ref();
	}

	if (nWeight > nMaxWeight)
	{
		return MERR_TOO_HEAVY;
	}

	if ((parts == MMCIP_PRIMARY) || (parts == MMCIP_SECONDARY))
	{
		MMatchCharItemParts tarparts = MMCIP_PRIMARY;
		if (parts == MMCIP_PRIMARY) tarparts = MMCIP_SECONDARY;

		if (!pEquipedItem->IsEmpty(tarparts))
		{
			MMatchItem* pTarItem = pEquipedItem->GetItem(tarparts);
			if (pTarItem)
			{
				if (pTarItem->GetDescID() == pItem->GetDescID())
				{
					return MERR_CANNOT_EQUIP_EQUAL_ITEM;
				}
			}
		}
	}

	return MOK;
}

void MMatchServer::OnNetPong(const MUID& CommUID, unsigned int nTimeStamp)
{
	MMatchObject* pObj = GetObject(CommUID);
	if (pObj) {
		pObj->UpdateTickLastPacketRecved();
	}
}

void MMatchServer::OnHShieldPong(const MUID& CommUID, unsigned int nTimeStamp)
{
	MMatchObject* pObj = GetObject(CommUID);
	if (pObj)
		pObj->UpdateLastHShieldMsgRecved();
}

void MMatchServer::UpdateCharDBCachingData(MMatchObject* pObject)
{
	if (!IsEnabledObject(pObject)) return;

	int	nAddedXP, nAddedBP, nAddedKillCount, nAddedDeathCount, nAddedPlayTime;

	nAddedXP = pObject->GetCharInfo()->GetDBCachingData()->nAddedXP;
	nAddedBP = pObject->GetCharInfo()->GetDBCachingData()->nAddedBP;
	nAddedKillCount = pObject->GetCharInfo()->GetDBCachingData()->nAddedKillCount;
	nAddedDeathCount = pObject->GetCharInfo()->GetDBCachingData()->nAddedDeathCount;
	nAddedPlayTime = pObject->GetCharInfo()->GetDBCachingData()->nAddedPlayTime;

	if ((nAddedXP != 0) || (nAddedBP != 0) || (nAddedKillCount != 0) || (nAddedDeathCount != 0) || (nAddedPlayTime >= 0))
	{
		MAsyncDBJob_UpdateCharInfoData* pJob = new MAsyncDBJob_UpdateCharInfoData(pObject->GetUID());
		pJob->Input(pObject->GetCharInfo()->m_nCID, nAddedXP, nAddedBP, nAddedKillCount, nAddedDeathCount, nAddedPlayTime);
		pObject->m_DBJobQ.DBJobQ.push_back(pJob);
		pObject->GetCharInfo()->GetDBCachingData()->Reset();
	}
}

void MMatchServer::UpdateCharItemDBCachingData(MMatchObject* pObject)
{
	if (!IsEnabledObject(pObject)) return;

	vector<MUID> vecRemoveItemUIDList;

	MMatchItemMap::iterator itBegin, itEnd;

	itBegin = pObject->GetCharInfo()->m_ItemList.begin();
	itEnd = pObject->GetCharInfo()->m_ItemList.end();

	MAsyncDBJob_UpdateCharItemInfoData* pJob = new MAsyncDBJob_UpdateCharItemInfoData(pObject->GetUID());

	for (; itBegin != itEnd; itBegin++)
	{
		MMatchItem* pItem = (*itBegin).second;

		if (pItem->IsNeedDBUpdate())
		{
#ifdef _DEBUG
			mlog("�ش� ������(CIID:%d, Cnt:%d)�� ������Ʈ�� �ʿ��մϴ�.\n", pItem->GetCIID(), pItem->GetItemCachingData()->nAddedItemCount);
#endif
			pJob->Input(pItem->GetCIID(), pItem->GetItemCachingData()->nAddedItemCount);
			pItem->DBUpdateDone();
		}

		if (pItem->GetDesc() &&
			pItem->GetDesc()->IsSpendableItem() &&
			pItem->GetItemCount() <= 0) {
			vecRemoveItemUIDList.push_back(pItem->GetUID());
		}
}

	pObject->m_DBJobQ.DBJobQ.push_back(pJob);

	if (!vecRemoveItemUIDList.empty())
	{
		int nRemoveItemUIDListCount = (int)vecRemoveItemUIDList.size();
		for (int i = 0; i < nRemoveItemUIDListCount; i++)
		{
			MUID uidItem = vecRemoveItemUIDList[i];
			MMatchItem* pItem = pObject->GetCharInfo()->m_ItemList.GetItem(uidItem);
			if (pItem == NULL) continue;

			MMatchCharItemParts nCheckParts = MMCIP_END;
			if (pObject->GetCharInfo()->m_EquipedItem.IsEquipedItem(uidItem, nCheckParts)) {
				pObject->GetCharInfo()->m_EquipedItem.Remove(nCheckParts);

				if (FindStage(pObject->GetStageUID()))
				{
					MCommand* pCmd = CreateCommand(MC_MATCH_ROUTE_UPDATE_STAGE_EQUIP_LOOK, MUID(0, 0));
					pCmd->AddParameter(new MCmdParamUID(pObject->GetUID()));
					pCmd->AddParameter(new MCmdParamInt(nCheckParts));
					pCmd->AddParameter(new MCmdParamInt(0));
					RouteToStage(pObject->GetStageUID(), pCmd);
				}
			}

			pObject->GetCharInfo()->m_ItemList.RemoveItem(uidItem);
		}

		ResponseCharacterItemList(pObject->GetUID());
	}
	}

bool MMatchServer::CheckItemXMLFromDatabase()
{
	map<int, MMatchItemDescForDatabase*> ItemMapFromDatabase;

	if (m_MatchDBMgr.GetItemTable(ItemMapFromDatabase) == false) {
		mlog("Can't not Get Item Information From Database. FAILED\n");
		return false;
	}

	bool bResult = true;
	int nResultCount = 0;

	mlog("========= Check validation of zitem.xml =========\n");

	mlog("== 1. Check Difference List From XML To DB\n");

	for (MMatchItemDescMgr::iterator itBegin = MGetMatchItemDescMgr()->begin();
		itBegin != MGetMatchItemDescMgr()->end(); itBegin++)
	{
		MMatchItemDesc* pItem = itBegin->second;
		if (ItemMapFromDatabase.find(pItem->m_nID) == ItemMapFromDatabase.end())
		{
			mlog("     ItemID(%d) does not exist in Database\n", pItem->m_nID);
			bResult = false;
		}
	}

	mlog("== 2. Check Difference List From DB To XML\n");

	for (map<int, MMatchItemDescForDatabase*>::iterator itBegin = ItemMapFromDatabase.begin();
		itBegin != ItemMapFromDatabase.end(); itBegin++)
	{
		MMatchItemDescForDatabase* pItem = itBegin->second;
		if (MGetMatchItemDescMgr()->GetItemDesc(pItem->m_nID) == NULL)
		{
			mlog("     ItemID(%d) does not exist in XML\n", pItem->m_nID);
		}
	}

	mlog("== 3. Check Difference between XML and DB\n");

	for (map<int, MMatchItemDescForDatabase*>::iterator itBegin = ItemMapFromDatabase.begin();
		itBegin != ItemMapFromDatabase.end(); itBegin++)
	{
		MMatchItemDescForDatabase* pItem1 = itBegin->second;
		MMatchItemDesc* pItem2 = MGetMatchItemDescMgr()->GetItemDesc(pItem1->m_nID);

		if (pItem2 != NULL)
		{
			if (!CompareMatchItem(pItem1, pItem2))
			{
				bResult = false;
				nResultCount++;
			}
		}
	}

	if (bResult == false)
	{
		mlog("========= Failed =========\n");
		mlog("========= Check zitem.xml or Item table in Database =========\n\n");
	}

	for (map<int, MMatchItemDescForDatabase*>::iterator itBegin = ItemMapFromDatabase.begin();
		itBegin != ItemMapFromDatabase.end(); itBegin++)
	{
		MMatchItemDescForDatabase* pItem1 = itBegin->second;
		delete pItem1;
	}

	ItemMapFromDatabase.clear();

	return bResult;
}

bool MMatchServer::CompareMatchItem(MMatchItemDescForDatabase* pItem1, MMatchItemDesc* pItem2)
{
	bool bResult = true;
	char* szMsg[256] = { 0, };

	if (((pItem1->m_nResSex == 3 && pItem2->m_nResSex.Ref() == -1)
		|| (pItem1->m_nResSex == 1 && pItem2->m_nResSex.Ref() == 0)
		|| (pItem1->m_nResSex == 2 && pItem2->m_nResSex.Ref() == 1)) == false)
	{
		mlog("%d(%s) - ResSex does not match\n", pItem1->m_nID, pItem2->m_pMItemName->Ref().m_szItemName);
		bResult = false;
	}

	if (pItem1->m_nResLevel != pItem2->m_nResLevel.Ref()) {
		mlog("%d(%s) - ResLevel does not match(DB : %d)\n", pItem1->m_nID, pItem2->m_pMItemName->Ref().m_szItemName, pItem1->m_nResLevel);
		bResult = false;
	}

	if (pItem1->m_nSlot != pItem2->m_nSlot) {
		mlog("%d(%s) - Slot does not match(DB : %d)\n", pItem1->m_nID, pItem2->m_pMItemName->Ref().m_szItemName, (int)pItem1->m_nSlot);
		bResult = false;
	}

	if (pItem1->m_nWeight != pItem2->m_nWeight.Ref()) {
		mlog("%d(%s) - Weight does not match(DB : %d)\n", pItem1->m_nID, pItem2->m_pMItemName->Ref().m_szItemName, pItem1->m_nWeight);
		bResult = false;
	}

	if (pItem1->m_nDamage != pItem2->m_nDamage.Ref()) {
		mlog("%d(%s) - Damage does not match(DB : %d)\n", pItem1->m_nID, pItem2->m_pMItemName->Ref().m_szItemName, pItem1->m_nDamage);
		bResult = false;
	}

	if (pItem1->m_nDelay != pItem2->m_nDelay.Ref()) {
		mlog("%d(%s) - Delay does not match(DB : %d)\n", pItem1->m_nID, pItem2->m_pMItemName->Ref().m_szItemName, pItem1->m_nDelay);
		bResult = false;
	}

	if (pItem1->m_nControllability != pItem2->m_nControllability.Ref()) {
		mlog("%d(%s) - Controllability does not match(DB : %d)\n", pItem1->m_nID, pItem2->m_pMItemName->Ref().m_szItemName, pItem1->m_nControllability);
		bResult = false;
	}

	if (pItem1->m_nMagazine != pItem2->m_nMagazine.Ref()) {
		mlog("%d(%s) - Magazine does not match(DB : %d)\n", pItem1->m_nID, pItem2->m_pMItemName->Ref().m_szItemName, pItem1->m_nMagazine);
		bResult = false;
	}

	if (pItem1->m_nReloadTime != pItem2->m_nReloadTime.Ref()) {
		mlog("%d(%s) - ReloadTime does not match(DB : %d)\n", pItem1->m_nID, pItem2->m_pMItemName->Ref().m_szItemName, pItem1->m_nReloadTime);
		bResult = false;
	}

	if (pItem1->m_nMaxBullet != pItem2->m_nMaxBullet.Ref()) {
		mlog("%d(%s) - MaxBullet does not match(DB : %d)\n", pItem1->m_nID, pItem2->m_pMItemName->Ref().m_szItemName, pItem1->m_nMaxBullet);
		bResult = false;
	}

	if (pItem1->m_nHP != pItem2->m_nHP.Ref()) {
		mlog("%d(%s) - HP does not match(DB : %d)\n", pItem1->m_nID, pItem2->m_pMItemName->Ref().m_szItemName, pItem1->m_nHP);
		bResult = false;
	}

	if (pItem1->m_nAP != pItem2->m_nAP.Ref()) {
		mlog("%d(%s) - AP does not match(DB : %d)\n", pItem1->m_nID, pItem2->m_pMItemName->Ref().m_szItemName, pItem1->m_nAP);
		bResult = false;
	}

	if (pItem1->m_bIsCashItem != pItem2->m_bIsCashItem) {
		if (pItem1->m_bIsCashItem) mlog("%d(%s) - IsCashItem does not match(DB : true)\n", pItem1->m_nID, pItem2->m_pMItemName->Ref().m_szItemName);
		else						mlog("%d(%s) - IsCashItem does not match(DB : false)\n", pItem1->m_nID, pItem2->m_pMItemName->Ref().m_szItemName);
		bResult = false;
	}

	if (pItem1->m_bIsSpendableItem != pItem2->m_bIsSpendableItem) {
		if (pItem1->m_bIsSpendableItem) mlog("%d(%s) - IsSpendableItem does not match(DB : true)\n", pItem1->m_nID, pItem2->m_pMItemName->Ref().m_szItemName);
		else							 mlog("%d(%s) - IsSpendableItem does not match(DB : false)\n", pItem1->m_nID, pItem2->m_pMItemName->Ref().m_szItemName);
		bResult = false;
	}

	return bResult;
}

static bool CheckItemXML_ReadXML(map<unsigned long int, string>& ItemXmlMap, const char* szXmlFile)
{
	MXmlDocument	xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(szXmlFile))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MICTOK_ITEM))
		{
			unsigned long int id;
			int n;
			char szItemName[256];
			chrElement.GetAttribute(&n, MICTOK_ID);
			id = n;
			chrElement.GetAttribute(szItemName, MICTOK_NAME);

			if (ItemXmlMap.find(id) != ItemXmlMap.end())
			{
				char szTemp[256];
				sprintf(szTemp, "item xml ���̵� �ߺ�: %u\n", id);
				mlog(szTemp);
				return false;
			}
			ItemXmlMap.insert(map<unsigned long int, string>::value_type(id, string(szItemName)));
		}
	}

	xmlIniData.Destroy();
	return true;
}

bool MMatchServer::CheckItemXML()
{
	map<unsigned long int, string>	ItemXmlMap;

	if (!CheckItemXML_ReadXML(ItemXmlMap, FILENAME_ITEM_DESC))
		return false;
	if (!CheckItemXML_ReadXML(ItemXmlMap, FILENAME_ITEM_DESC_LOCALE))
		return false;

	DWORD dwNomsgItemCount = 0;
	FILE* fp = fopen("item.sql", "wt");
	for (map<unsigned long int, string>::iterator itor = ItemXmlMap.begin();
		itor != ItemXmlMap.end(); ++itor)
	{
		char szTemp2[256];
		unsigned long int id = (*itor).first;
		size_t pos = (*itor).second.find(":");
		if (string::npos == pos)
		{
			ASSERT(0 && "�����ڸ� ã�� ����. ��������.");
			continue;
		}

		string name = (*itor).second.c_str() + pos + 1;

		if (0 == _stricmp("nomsg", MGetStringResManager()->GetString(name)))
		{
			++dwNomsgItemCount;
		}

		sprintf(szTemp2, "INSERT INTO Item (ItemID, Name) Values (%u, '%s')\n",
			id, MGetStringResManager()->GetString(name));

		fputs(szTemp2, fp);
	}

	mlog("\n== Nomsg item count : %u ==\n\n", dwNomsgItemCount);

	fputs("\n\n--------------------------------------\n\n", fp);

	for (MMatchItemDescMgr::iterator itor = MGetMatchItemDescMgr()->begin();
		itor != MGetMatchItemDescMgr()->end(); ++itor)
	{
		MMatchItemDesc* pItemDesc = (*itor).second;

		int nIsCashItem = 0;
		int nResSex = 1, nResLevel = 0, nSlot = 0, nWeight = 0, nHP = 0, nAP = 0, nMaxWT = 0, nIsSpendableItem = 0;

		int nDamage = 0, nDelay = 0, nControl = 0, nMagazine = 0, nReloadTime = 0, nSlugOutput = 0, nMaxBullet = 0;

		if (pItemDesc->IsCashItem())	nIsCashItem = 1;
		else							nIsCashItem = 0;

		if (pItemDesc->IsSpendableItem())	nIsSpendableItem = 1;
		else								nIsSpendableItem = 0;

		switch (pItemDesc->m_nResSex.Ref())
		{
		case 0: nResSex = 1; break;
		case 1: nResSex = 2; break;
		case -1: nResSex = 3; break;
		}

		nResLevel = pItemDesc->m_nResLevel.Ref();
		nWeight = pItemDesc->m_nWeight.Ref();
		nHP = pItemDesc->m_nHP.Ref();
		nAP = pItemDesc->m_nAP.Ref();
		nMaxWT = pItemDesc->m_nMaxWT.Ref();

		switch (pItemDesc->m_nSlot)
		{
		case MMIST_MELEE:		nSlot = 1; break;
		case MMIST_RANGE:		nSlot = 2; break;
		case MMIST_CUSTOM:		nSlot = 3; break;
		case MMIST_HEAD:		nSlot = 4; break;
		case MMIST_CHEST:		nSlot = 5; break;
		case MMIST_HANDS:		nSlot = 6; break;
		case MMIST_LEGS:		nSlot = 7; break;
		case MMIST_FEET:		nSlot = 8; break;
		case MMIST_FINGER:		nSlot = 9; break;
		case MMIST_EXTRA:		nSlot = 10; break;
		case MMIST_AVATAR:		nSlot = 11; break;
		case MMIST_COMMUNITY:	nSlot = 12; break;
		case MMIST_LONGBUFF:	nSlot = 13; break;
		}

		nDamage = pItemDesc->m_nDamage.Ref();
		nDelay = pItemDesc->m_nDelay.Ref();
		nControl = pItemDesc->m_nControllability.Ref();
		nMagazine = pItemDesc->m_nMagazine.Ref();
		nReloadTime = pItemDesc->m_nReloadTime.Ref();
		if (pItemDesc->m_bSlugOutput) nSlugOutput = 1;
		nMaxBullet = pItemDesc->m_nMaxBullet.Ref();

		fprintf(fp, "UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=%d, Delay=%d, Controllability=%d, Magazine=%d, ReloadTime=%d, SlugOutput=%d, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=%d, LimitSpeed=%d, IsCashItem=%d, \n",
			nDamage, nDelay, nControl, nMagazine, nReloadTime, nSlugOutput, nMaxBullet,
			pItemDesc->m_nLimitSpeed.Ref(), nIsCashItem);

		fprintf(fp, "ResSex=%d, ResLevel=%d, Slot=%d, Weight=%d, HP=%d, AP=%d, MAXWT=%d, IsSpendableItem=%d, \n",
			nResSex, nResLevel, nSlot, nWeight, nHP, nAP, nMaxWT, nIsSpendableItem);

		fprintf(fp, "Description='%s', \n", pItemDesc->m_szDesc);
		fprintf(fp, "Name='%s' \n", pItemDesc->m_pMItemName->Ref().m_szItemName);

		fprintf(fp, "WHERE ItemID = %u\n\n", pItemDesc->m_nID);
	}

	fclose(fp);

	fp = fopen("shop_allitemtest.xml", "wt");
	if (fp)
	{
		fprintf(fp, "<?xml version=\"1.0\"?>\n");
		fprintf(fp, "<XML id=\"shop\">\n");

		int id;
		for (MMatchItemDescMgr::iterator itor = MGetMatchItemDescMgr()->begin(); itor != MGetMatchItemDescMgr()->end(); ++itor)
		{
			id = itor->first;

			if (id == 51 ||
				id == 10001 ||
				(300000 <= id && id <= 399999) ||
				(401000 <= id && id <= 499999) ||
				id == 700001 ||
				(80000 <= id && id <= 100000) ||
				id == 800001
				)
				continue;

			fprintf(fp, "\t<SELL itemid=\"%d\" />\n", id);
		}
		for (MQuestItemDescManager::iterator itor = GetQuestItemDescMgr().begin(); itor != GetQuestItemDescMgr().end(); ++itor)
		{
			id = itor->first;

			if (id == 210001) continue;

			fprintf(fp, "\t<SELL itemid=\"%d\" />\n", id);
		}
		fprintf(fp, "</XML>\n");

		fclose(fp);
	}

	return true;
}

struct ix
{
	string id;
	string name;
	string desc;
};

bool MMatchServer::CheckUpdateItemXML()
{
	map< string, ix > imName;
	map< string, ix > imDesc;

	MXmlDocument	xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile("strings.xml"))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, "STR"))
		{
			ix tix;
			char szID[256] = { 0, };
			char szInfo[512] = { 0, };

			chrElement.GetAttribute(szID, "id");
			chrElement.GetContents(szInfo);

			if (0 == strncmp("ZITEM_NAME_", szID, 11))
			{
				if (imName.end() != imName.find(szID))
				{
					ASSERT("�ߺ�");
					continue;
				}

				tix.id = szID;
				tix.name = szInfo;

				imName.insert(map<string, ix>::value_type(szID, tix));
			}
			else if (0 == strncmp("ZITEM_DESC_", szID, 11))
			{
				if (imDesc.end() != imDesc.find(szID))
				{
					ASSERT("�ߺ�");
					continue;
				}

				tix.id = szID;
				tix.desc = szInfo;

				imDesc.insert(map<string, ix>::value_type(szID, tix));
			}
			else
			{
			}
		}
	}

	int ic, dc;
	ic = static_cast<int>(imName.size());
	dc = static_cast<int>(imDesc.size());

	xmlIniData.Destroy();

	map< string, ix >::iterator it, end;
	it = imName.begin();
	end = imName.end();
	FILE* fpName = fopen("name.sql", "w");
	for (; it != end; ++it)
	{
		char szID[128];
		string a = it->second.name;
		strcpy(szID, it->second.id.c_str() + 11);

		unsigned int nID = static_cast<unsigned long>(atol(szID));
		int k = 0;

		fprintf(fpName, "UPDATE Item SET Name = '%s' WHERE ItemID = %d\n",
			it->second.name.c_str(), nID);
	}
	fclose(fpName);

	it = imDesc.begin();
	end = imDesc.end();
	FILE* fpDesc = fopen("desc.sql", "w");
	for (; it != end; ++it)
	{
		char szID[128];
		string a = it->second.name;
		strcpy(szID, it->second.id.c_str() + 11);

		unsigned int nID = static_cast<unsigned long>(atol(szID));
		int k = 0;

		char szQ[1024] = { 0, };
		fprintf(fpDesc, "UPDATE Item SET Description = '%s' WHERE ItemID = %d\n",
			it->second.desc.c_str(), nID);
	}
	fclose(fpDesc);

	return true;
}

unsigned long int MMatchServer::GetStageListChecksum(MUID& uidChannel, int nStageCursor, int nStageCount)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return 0;

	unsigned long int nStageListChecksum = 0;
	int nRealStageCount = 0;

	for (int i = nStageCursor; i < pChannel->GetMaxPlayers(); i++)
	{
		if (nRealStageCount >= nStageCount) break;

		if (pChannel->IsEmptyStage(i)) continue;
		MMatchStage* pStage = pChannel->GetStage(i);
		if ((pStage == NULL) || (pStage->GetState() == STAGE_STATE_CLOSE)) continue;

		nStageListChecksum += pStage->GetChecksum();

		nRealStageCount++;
	}

	return nStageListChecksum;
}

void MMatchServer::BroadCastClanRenewVictories(const char* szWinnerClanName, const char* szLoserClanName, const int nVictories)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_BROADCAST_CLAN_RENEW_VICTORIES, MUID(0, 0));
	pCmd->AddParameter(new MCommandParameterString(szWinnerClanName));
	pCmd->AddParameter(new MCommandParameterString(szLoserClanName));
	pCmd->AddParameter(new MCommandParameterInt(nVictories));

	RouteToAllClient(pCmd);
}

void MMatchServer::BroadCastClanInterruptVictories(const char* szWinnerClanName, const char* szLoserClanName, const int nVictories)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_BROADCAST_CLAN_INTERRUPT_VICTORIES, MUID(0, 0));
	pCmd->AddParameter(new MCommandParameterString(szWinnerClanName));
	pCmd->AddParameter(new MCommandParameterString(szLoserClanName));
	pCmd->AddParameter(new MCommandParameterInt(nVictories));

	RouteToAllClient(pCmd);
}

void MMatchServer::BroadCastDuelRenewVictories(const MUID& chanID, const char* szChampionName, const char* szChannelName, int nRoomNumber, const int nVictories)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_BROADCAST_DUEL_RENEW_VICTORIES, MUID(0, 0));
	pCmd->AddParameter(new MCommandParameterString(szChampionName));
	pCmd->AddParameter(new MCommandParameterString(szChannelName));
	pCmd->AddParameter(new MCommandParameterInt(nRoomNumber));
	pCmd->AddParameter(new MCommandParameterInt(nVictories));

	RouteToChannel(chanID, pCmd);
}

void MMatchServer::BroadCastDuelInterruptVictories(const MUID& chanID, const char* szChampionName, const char* szInterrupterName, const int nVictories)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_BROADCAST_DUEL_INTERRUPT_VICTORIES, MUID(0, 0));
	pCmd->AddParameter(new MCommandParameterString(szChampionName));
	pCmd->AddParameter(new MCommandParameterString(szInterrupterName));
	pCmd->AddParameter(new MCommandParameterInt(nVictories));

	RouteToChannel(chanID, pCmd);
}

bool MMatchServer::InitScheduler()
{
	m_pScheduler = new MMatchScheduleMgr(this);
	if (0 == m_pScheduler)
		return false;

	if (!m_pScheduler->Init()) {
		delete m_pScheduler;
		m_pScheduler = 0;
		return false;
	}

	m_pScheduler->SetUpdateTerm(10);

	if (!InitSubTaskSchedule()) {
		delete m_pScheduler;
		m_pScheduler = 0;
		return false;
	}

	return true;
}

bool MMatchServer::InitLocale()
{
	if (MGetServerConfig()->IsComplete())
	{
		MGetLocale()->Init(GetCountryID(MGetServerConfig()->GetLanguage().c_str()));
	}
	else
	{
		ASSERT(0 && "'MMatchConfig' is must be completed befor init 'MMatchLocale'.");
		return false;
	}

	MGetStringResManager()->Init("", MGetLocale()->GetCountry());

	return true;
}

bool MMatchServer::InitCountryFilterDB()
{
	IPtoCountryList icl;
	BlockCountryCodeList bccl;
	CustomIPList cil;

	if (!GetDBMgr()->GetBlockCountryCodeList(bccl))
	{
		ASSERT(0 && "Fail to init BlockCoutryCodeList.\n");
		mlog("Fail to init BlockCountryCodeList.\n");
		return false;
	}

	if (!GetDBMgr()->GetCustomIPList(cil))
	{
		ASSERT(0 && "Fail to init CustomIPList.\n");
		mlog("Fail to init CustomIPList.\n");
		return false;
	}

	if (!GetCountryFilter().Create(bccl, icl, cil))
	{
		ASSERT(0 && "Fail to create country filter.\n");
		mlog("Fail to create country filter.\n");
		return false;
	}

	return true;
}

void MMatchServer::SetUseCountryFilter()
{
	MCommand* pCmd = CreateCommand(MC_LOCAL_UPDATE_USE_COUNTRY_FILTER, GetUID());
	if (0 != pCmd)
		Post(pCmd);
}

void MMatchServer::UpdateIPtoCountryList()
{
	MCommand* pCmd = CreateCommand(MC_LOCAL_GET_DB_IP_TO_COUNTRY, GetUID());
	if (0 != pCmd)
		Post(pCmd);
}

void MMatchServer::UpdateBlockCountryCodeLsit()
{
	MCommand* pCmd = CreateCommand(MC_LOCAL_GET_DB_BLOCK_COUNTRY_CODE, GetUID());
	if (0 != pCmd)
		Post(pCmd);
}

void MMatchServer::UpdateCustomIPList()
{
	MCommand* pCmd = CreateCommand(MC_LOCAL_GET_DB_CUSTOM_IP, GetUID());
	if (0 != pCmd)
		Post(pCmd);
}

void MMatchServer::SetAccetpInvalidIP()
{
	MCommand* pCmd = CreateCommand(MC_LOCAL_UPDATE_ACCEPT_INVALID_IP, GetUID());
	if (0 != pCmd)
		Post(pCmd);
}

bool MMatchServer::CheckIsValidIP(const MUID& CommUID, const string& strIP, string& strCountryCode3, const bool bUseFilter)
{
	switch (CheckIsValidCustomIP(CommUID, strIP, strCountryCode3, bUseFilter))
	{
	case CIS_INVALID:
	{
		switch (CheckIsNonBlockCountry(CommUID, strIP, strCountryCode3, bUseFilter))
		{
		case CCS_NONBLOCK:
		{
			return true;
		}
		break;

		case CCS_BLOCK:
		{
			return false;
		}
		break;

		case CCS_INVALID:
		{
			return MGetServerConfig()->IsAcceptInvalidIP();
		}
		break;

		default:
		{
			ASSERT(0);
		}
		break;
		}
	}
	break;

	case CIS_NONBLOCK:
	{
		return true;
	}
	break;

	case CIS_BLOCK:
	{
		return false;
	}
	break;

	default:
	{
		ASSERT(0);
	}
	break;
	}

	return false;
}

const CUSTOM_IP_STATUS MMatchServer::CheckIsValidCustomIP(const MUID& CommUID, const string& strIP, string& strCountryCode3, const bool bUseFilter)
{
	string strComment;
	bool bIsBlock = false;

	if (!GetCountryFilter().GetCustomIP(strIP, bIsBlock, strCountryCode3, strComment))
		return CIS_INVALID;

	if (bUseFilter && bIsBlock)
	{
		MCommand* pCmd = CreateCommand(MC_RESPONSE_BLOCK_COUNTRYCODE, CommUID);
		if (0 != pCmd)
		{
			pCmd->AddParameter(new MCommandParameterString(strComment.c_str()));
			Post(pCmd);
		}

		if (3 != strCountryCode3.length())
			strCountryCode3 = "er_";
		return CIS_BLOCK;
	}

	return CIS_NONBLOCK;
}

const COUNT_CODE_STATUS MMatchServer::CheckIsNonBlockCountry(const MUID& CommUID, const string& strIP, string& strCountryCode3, const bool bUseFilter)
{
	if (!bUseFilter)
		return CCS_NONBLOCK;

	if (GetCountryFilter().GetIPCountryCode(strIP, strCountryCode3))
	{
		string strRoutingURL;

		if (GetCountryFilter().IsNotBlockCode(strCountryCode3, strRoutingURL))
			return CCS_NONBLOCK;
		else
		{
			MCommand* pCmd = CreateCommand(MC_RESPONSE_BLOCK_COUNTRYCODE, CommUID);
			if (0 != pCmd)
			{
				pCmd->AddParameter(new MCommandParameterString(strRoutingURL.c_str()));
				Post(pCmd);
			}
			return CCS_BLOCK;
		}
	}
	else
	{
		DWORD dwIPFrom = 0;
		DWORD dwIPTo = 0;

		if (GetDBMgr()->GetIPContryCode(strIP, dwIPFrom, dwIPTo, strCountryCode3))
		{
			if (!GetCountryFilter().AddIPtoCountry(dwIPFrom, dwIPTo, strCountryCode3))
			{
				mlog("MMatchServer::CheckIsNonBlockCountry - add new IPtoCountry(f:%u, t%u, c:%s) fail.\n",
					dwIPFrom, dwIPTo, strCountryCode3.c_str());
			}

			string strRoutingURL;
			if (GetCountryFilter().IsNotBlockCode(strCountryCode3, strRoutingURL))
				return CCS_NONBLOCK;
		}
		else
		{
			strCountryCode3 = "N/S";
			return CCS_INVALID;
		}

		MCommand* pCmd = CreateCommand(MC_RESPONSE_BLOCK_COUNTRYCODE, CommUID);
		if (0 != pCmd)
		{
			pCmd->AddParameter(new MCommandParameterString(strCountryCode3.c_str()));
			Post(pCmd);
		}

		return CCS_BLOCK;
	}

	return CCS_BLOCK;
}

bool MMatchServer::InitEvent()
{
	if (!MMatchEventDescManager::GetInstance().LoadEventXML(EVENT_XML_FILE_NAME))
	{
		ASSERT(0 && "fail to Load Event.xml");
		mlog("MMatchServer::InitEvent - fail to Load %s\n",
			EVENT_XML_FILE_NAME);
		return false;
	}

	if (!MMatchEventFactoryManager::GetInstance().LoadEventListXML(EVENT_LIST_XML_FILE_NAME))
	{
		ASSERT(0 && "fail to load EventList.xml");
		mlog("MMatchServer::InitEvent - fail to Load %s\n",
			EVENT_LIST_XML_FILE_NAME);
		return false;
	}

	MMatchEventFactoryManager::GetInstance().SetUsableState(MGetServerConfig()->IsUseEvent());

	EventPtrVec EvnPtrVec;
	if (!MMatchEventFactoryManager::GetInstance().GetEventList(MMatchEvent::GAME_TYPE_ALL, ET_CUSTOM_EVENT, EvnPtrVec))
	{
		ASSERT(0 && "�̺�Ʈ ����Ʈ ���� ����.\n");
		mlog("MMatchServer::InitEvent - ����Ʈ ���� ����.\n");
		MMatchEventManager::ClearEventPtrVec(EvnPtrVec);
		return false;
	}
	m_CustomEventManager.ChangeEventList(EvnPtrVec);

	return true;
}

void MMatchServer::CustomCheckEventObj(const DWORD dwEventID, MMatchObject* pObj, void* pContext)
{
	m_CustomEventManager.CustomCheckEventObj(dwEventID, pObj, pContext);
}

void MMatchServer::SendHShieldReqMsg()
{
	MCommand* pCommand = CreateCommand(MC_HSHIELD_PING, MUID(0, 0));
	pCommand->AddParameter(new MCmdParamUInt(GetGlobalClockCount()));

	unsigned long HSOption = ANTICPSVR_CHECK_GAME_MEMORY;
	m_HSCheckCounter++;
	if (m_HSCheckCounter == 3)
	{
		m_HSCheckCounter = 0;
		HSOption = ANTICPSVR_CHECK_ALL;
	}

	for (MMatchObjectList::iterator i = m_Objects.begin(); i != m_Objects.end(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)((*i).second);

		if (pObj->GetUID() < MUID(0, 3) || !pObj->GetHShieldMsgRecved()) continue;

		unsigned char* pbyReqMsg = pObj->GetHShieldInfo()->m_pbyReqMsg;
		unsigned char* pbyReqInfo = pObj->GetHShieldInfo()->m_pbyReqInfo;

		memset(pbyReqMsg, 0, sizeof(pbyReqMsg));
		memset(pbyReqInfo, 0, sizeof(pbyReqInfo));

		if (!pObj->GetHShieldInfo()->m_bGuidAckPass)
		{
			break;
		}

		DWORD dwRet = MGetMatchServer()->HShield_MakeReqMsg(pObj->GetHShieldInfo()->m_pCRCInfo, pbyReqMsg, pbyReqInfo, HSOption);

		if (dwRet != ERROR_SUCCESS)
			LOG(LOG_FILE, "@MakeReqMsg - %s Making Req Msg Failed. (Error code = 0x%x, CrcInfo Address = 0x%x)", pObj->GetAccountName(), dwRet, pObj->GetHShieldInfo()->m_pCRCInfo);

		void* pBlob = MMakeBlobArray(sizeof(unsigned char), SIZEOF_REQMSG);
		unsigned char* pCmdBlock = (unsigned char*)MGetBlobArrayElement(pBlob, 0);
		CopyMemory(pCmdBlock, pbyReqMsg, SIZEOF_REQMSG);

		pCommand->AddParameter(new MCmdParamBlob(pBlob, MGetBlobArraySize(pBlob)));

		MCommand* pSendCmd = pCommand->Clone();
		pSendCmd->m_Receiver = pObj->GetUID();
		Post(pSendCmd);

		pObj->SetHShieldMsgRecved(false);
		MEraseBlobArray(pBlob);
		pCommand->ClearParam(1);
	}

	delete pCommand;
}

bool MMatchServer::IsEquipmentTypeItem(const MMatchItemDesc* pItemDesc)
{
	if (0 == pItemDesc) return false;

	if (MMIT_TICKET == pItemDesc->m_nType.Ref())
		return false;

	return true;
}

void MMatchServer::RequestGameguardAuth(const MUID& uidUser, const DWORD dwIndex, const DWORD dwValue1, const DWORD dwValue2, const DWORD dwValue3)
{
	MCommand* pCmd = CreateCommand(MC_REQUEST_GAMEGUARD_AUTH, uidUser);
	if (0 == pCmd)
		return;

	pCmd->AddParameter(new MCmdParamUInt(dwIndex));
	pCmd->AddParameter(new MCmdParamUInt(dwValue1));
	pCmd->AddParameter(new MCmdParamUInt(dwValue2));
	pCmd->AddParameter(new MCmdParamUInt(dwValue3));

	PostSafeQueue(pCmd);

#ifdef _DEBUG
	mlog("GAMEGUARD : send to client : index(%u), v1(%u), v2(%u), v3(%u)\n",
		dwIndex, dwValue1, dwValue2, dwValue3);
#endif
}

void MMatchServer::RequestFirstGameguardAuth(const MUID& uidUser, const DWORD dwIndex, const DWORD dwValue1, const DWORD dwValue2, const DWORD dwValue3)
{
	MCommand* pCmd = this->CreateCommand(MC_REQUEST_FIRST_GAMEGUARD_AUTH, uidUser);
	if (0 == pCmd)
		return;

	pCmd->AddParameter(new MCmdParamUInt(dwIndex));
	pCmd->AddParameter(new MCmdParamUInt(dwValue1));
	pCmd->AddParameter(new MCmdParamUInt(dwValue2));
	pCmd->AddParameter(new MCmdParamUInt(dwValue3));

	PostSafeQueue(pCmd);

#ifdef _DEBUG
	mlog("GAMEGUARD : first send to client : index(%u), v1(%u), v2(%u), v3(%u)\n",
		dwIndex, dwValue1, dwValue2, dwValue3);
#endif
}

bool MMatchServer::InitGambleMachine()
{
	vector<MMatchGambleItem*>			vGambleItemList;
	vector<MMatchGambleRewardItem*>		vGambleRewardItemList;

	if (!GetDBMgr()->GetGambleItemList(vGambleItemList))				return false;
	if (!GetDBMgr()->GetGambleRewardItemList(vGambleRewardItemList))	return false;

	if (!GetGambleMachine().CreateGambleItemListWithGambleRewardList(vGambleItemList, vGambleRewardItemList))
		return false;

	GetGambleMachine().SetLastUpdateTime(GetGlobalClockCount());

	vector<DWORD> GambleItemIndexVec;
	GetGambleMachine().GetItemVectorByCheckedItemTime(GambleItemIndexVec, GetGlobalClockCount());

	mlog("\nStart shop GItem list log.\n");
	mlog("GambleItemCount : %d.\n", int(GambleItemIndexVec.size()));

	for (int i = 0; i < int(GambleItemIndexVec.size()); ++i)
	{
		const MMatchGambleItem* pGItem = GetGambleMachine().GetGambleItemByIndex(GambleItemIndexVec[i]);
		if (NULL != pGItem)
		{
			mlog("Shop GItem : %d\n", pGItem->GetGambleItemID());
		}
		else
		{
			mlog("Null GItem.\n");
		}
	}
	mlog("End shop GItem list log.\n\n");
	GetGambleMachine().WriteGambleItemInfoToLog();

	return true;
}

bool MMatchServer::InitBattletimeRewardMachine()
{
	vector<MMatchBRDescription*>	vBattletimeRewardDescription;
	vector<MMatchBRItem*>			vBattletimeRewardItem;

	if (!GetDBMgr()->GetBattletimeRewardList(vBattletimeRewardDescription)) return false;
	if (!GetDBMgr()->GetBattletimeRewardItemList(vBattletimeRewardItem))			return false;

	MMatchBRDescriptionMap BattletimeRewardDescriptionMap;
	MakeBattleTimeRewardDescriptionMap(vBattletimeRewardDescription, vBattletimeRewardItem, BattletimeRewardDescriptionMap);

	GetBattleTimeRewardMachine().SetBattleTimeRewardMachine(BattletimeRewardDescriptionMap);
	GetBattleTimeRewardMachine().SetLastUpdateTime(GetGlobalClockCount());

	return true;
}

void MMatchServer::MakeBattleTimeRewardDescriptionMap(vector<MMatchBRDescription*>& vBattletimeRewardDescription,
	vector<MMatchBRItem*>& vBattletimeRewardItem,
	MMatchBRDescriptionMap& BattletimeRewardDescriptionMap)
{
	MMatchBRDescriptionMap::iterator iter;

	for (int i = 0; i < (int)vBattletimeRewardDescription.size(); i++)
	{
		MMatchBRDescription* pDesc = vBattletimeRewardDescription[i];
		BattletimeRewardDescriptionMap.insert(pair<int, MMatchBRDescription*>(pDesc->GetBRID(), pDesc));
	}

	for (int i = 0; i < (int)vBattletimeRewardItem.size(); i++)
	{
		MMatchBRItem* pRewardItem = vBattletimeRewardItem[i];

		iter = BattletimeRewardDescriptionMap.find(pRewardItem->GetBRID());
		if (iter != BattletimeRewardDescriptionMap.end())
		{
			MMatchBRDescription* pDesc = iter->second;
			pDesc->AddRewardItem(pRewardItem);
		}
	}

	for (iter = BattletimeRewardDescriptionMap.begin(); iter != BattletimeRewardDescriptionMap.end();)
	{
		MMatchBRDescription* pDesc = iter->second;
		if (pDesc->GetTotalRate() != 1000)
		{
			delete pDesc;
			iter = BattletimeRewardDescriptionMap.erase(iter);

			continue;
		}

		iter++;
	}

	BattletimeRewardDescriptionMap.MakeCRC32();
}

void MMatchServer::OnAsyncResponse_GetBR_Description(MAsyncJob* pJobResult)
{
	MAsyncDBJob_GetBattleTimeRewardDescription* pJob = (MAsyncDBJob_GetBattleTimeRewardDescription*)pJobResult;

	if (MASYNC_RESULT_SUCCEED != pJob->GetResult()) {
		mlog("MMatchServer::OnAsyncResponse_GetBR_Description - ����\n");
		return;
	}

	MMatchBRDescriptionMap BattletimeRewardDescriptionMap;
	MakeBattleTimeRewardDescriptionMap(pJob->GetBattleTimeRewardDescription(), pJob->GetBattleTimeRewardItem(), BattletimeRewardDescriptionMap);

	GetBattleTimeRewardMachine().SetBattleTimeRewardMachine(BattletimeRewardDescriptionMap);
}

void MMatchServer::OnAsyncResponse_GetCharBRInfo(MAsyncJob* pJobResult)
{
	MAsyncDBJob_GetCharBRInfo* pJob = (MAsyncDBJob_GetCharBRInfo*)pJobResult;

	if (MASYNC_RESULT_SUCCEED != pJob->GetResult()) {
		mlog("MMatchServer::OnAsyncResponse_GetCharBRInfo - ����\n");
		return;
	}

	MMatchObject* pObj = GetObject(pJob->GetOwnerUID());
	if (pObj == NULL) return;
	if (pObj->GetCharInfo() == NULL) return;

	MMatchCharBRInfo* pInfo = pObj->GetCharInfo()->GetBRInfoMap().Get(pJob->GetBRID());
	if (pInfo == NULL) return;

	pInfo->SetBRInfo(pJob->GetCharBRInfo().GetBRID(), pJob->GetCharBRInfo().GetBRTID(), pJob->GetCharBRInfo().GetBattleTime(),
		pJob->GetCharBRInfo().GetRewardCount(), pJob->GetCharBRInfo().GetKillCount());

	pInfo->SetCheckSkip(false);

#ifdef _DEBUG

	mlog("CID(%d), �� BR Info(%d, %d, %d, %d)�� ���ԵǾ����ϴ�.\n", pJob->GetCID(), pJob->GetCharBRInfo().GetBRID(),
		pJob->GetCharBRInfo().GetBRTID(), pJob->GetCharBRInfo().GetBattleTime(), pJob->GetCharBRInfo().GetKillCount());

#endif
}

void MMatchServer::OnAsyncResponse_UpdateCharBRInfo(MAsyncJob* pJobResult)
{
	MAsyncDBJob_UpdateCharBRInfo* pJob = (MAsyncDBJob_UpdateCharBRInfo*)pJobResult;

	if (MASYNC_RESULT_SUCCEED != pJob->GetResult()) {
		mlog("MMatchServer::OnAsyncResponse_UpdateCharBRInfo - ����\n");
		return;
	}
}

void MMatchServer::OnAsyncResponse_RewardCharBR(MAsyncJob* pJobResult)
{
	MAsyncDBJob_RewardCharBR* pJob = (MAsyncDBJob_RewardCharBR*)pJobResult;

	if (MASYNC_RESULT_SUCCEED != pJob->GetResult()) {
		mlog("MMatchServer::OnAsyncResponse_RewardCharBR - ����\n");
		return;
	}

	MMatchObject* pObj = GetObject(pJob->GetOwnerUID());
	if (pObj == NULL) return;
	if (pObj->GetCharInfo() == NULL) return;

	const WORD			wRentHourPeriod = pJob->GetRentHourPeriod();
	const DWORD			dwCIID = pJob->GetCIID();
	const unsigned int	nItemID = pJob->GetItemID();
	const int			nItemCount = pJob->GetItemCnt();

	WORD wRentMinPeriod = RENT_PERIOD_UNLIMITED;
	if (RENT_PERIOD_UNLIMITED != pJob->GetRentHourPeriod())
	{
		wRentMinPeriod = (pJob->GetRentHourPeriod() * 60) - 1;
	}

	if (NULL != MGetMatchItemDescMgr()->GetItemDesc(nItemID))
	{
		MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

		MMatchItem* pItem = pObj->GetCharInfo()->m_ItemList.GetItemByCIID(dwCIID);
		if (pItem == NULL)
		{
			WORD wRentMinPeriod = RENT_PERIOD_UNLIMITED;
			if (RENT_PERIOD_UNLIMITED != pJob->GetRentHourPeriod())
			{
				wRentMinPeriod = (wRentHourPeriod * 60) - 1;
			}

			pObj->GetCharInfo()->m_ItemList.CreateItem(MMatchItemMap::UseUID(), dwCIID, nItemID,
				RENT_PERIOD_UNLIMITED != pJob->GetRentHourPeriod(), wRentMinPeriod, wRentHourPeriod, nItemCount);
		}
		else
		{
			if (pItem->GetDescID() == pJob->GetItemID() && pDesc->IsSpendableItem())
			{
				pItem->IncreaseCount(pJob->GetItemCnt());
			}
			else
			{
				return;
			}
		}
	}
	else if (NULL != MGetMatchServer()->GetGambleMachine().GetGambleItemByGambleItemID(pJob->GetItemID()))
	{
		const MMatchCharGambleItem* pGItem = pObj->GetCharInfo()->m_GambleItemManager.GetGambleItemByCIID(pJob->GetCIID());
		if (pGItem == NULL)	pObj->GetCharInfo()->m_GambleItemManager.AddGambleItem(MMatchItemMap::UseUID(), dwCIID, nItemID, nItemCount);
		else					pObj->GetCharInfo()->m_GambleItemManager.SetGambleItemCount(dwCIID, pGItem->GetItemCount() + nItemCount);
	}
	else { ASSERT(0); return; }

#ifdef _DEBUG

	mlog("CID(%d), BattleTime Reward!! - Time(%d), ItemID(%d), ItemCnt(%d), ItemPeriod(%d)\n",
		pJob->GetCID(), pJob->GetBattleTime(), pJob->GetItemID(), pJob->GetItemCnt(), pJob->GetRentHourPeriod());

#endif

	MMatchBRDescription* pDesc = GetBattleTimeRewardMachine().GetBattleTimeRewardDescription(pJob->GetBRID());
	if (pDesc != NULL)
	{
		if (pDesc->GetRewardCount() == 0)
		{
			RouteCmdBattleTimeReward(pJob->GetOwnerUID(), pObj->GetStageUID(), pDesc->GetName().c_str(), pDesc->GetResetDesc().c_str(),
				pJob->GetItemID(), pJob->GetItemCnt(), pJob->GetRentHourPeriod(), -1);
		}
		else
		{
			RouteCmdBattleTimeReward(pJob->GetOwnerUID(), pObj->GetStageUID(), pDesc->GetName().c_str(), pDesc->GetResetDesc().c_str(),
				pJob->GetItemID(), pJob->GetItemCnt(), pJob->GetRentHourPeriod(), pDesc->GetRewardCount() - pJob->GetRewardCount() - 1);
		}
	}

	ResponseCharacterItemList(pJob->GetOwnerUID());
	}

void MMatchServer::RouteCmdBattleTimeReward(const MUID& uidPlayer, MUID& uidStage, const char* pszName, const char* pszResetDesc, int nItemID, int nItemCnt, int nRentHourPeriod, int nRemainRewardCnt)
{
	MCommand* pNew = CreateCommand(MC_MATCH_REWARD_BATTLE_TIME, MUID(0, 0));

	pNew->AddParameter(new MCmdParamUID(uidPlayer));
	pNew->AddParameter(new MCmdParamStr(pszName));
	pNew->AddParameter(new MCmdParamStr(pszResetDesc));
	pNew->AddParameter(new MCmdParamUInt(nItemID));
	pNew->AddParameter(new MCmdParamUInt(nItemCnt));
	pNew->AddParameter(new MCmdParamUInt(nRentHourPeriod));
	pNew->AddParameter(new MCmdParamInt(nRemainRewardCnt));

	RouteToBattle(uidStage, pNew);
}

void MMatchServer::OnAsyncRequest_RewardCharBP(const MUID& uidPlayer, int nBRID, int nBRTID, int nRewardCount, int nBattleTime, int nKillCount, int nItemID, int nItemCnt, int nRentHourPeriod)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	if (pObj->GetCharInfo() == NULL) return;

	bool bIsSpendable = false;

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc != NULL)
	{
		bIsSpendable = pItemDesc->IsSpendableItem();
	}
	else
	{
		if (GetGambleMachine().GetGambleItemByGambleItemID(nItemID) != NULL)
		{
			bIsSpendable = true;
		}
		else
		{
			return;
		}
	}

	MAsyncDBJob_RewardCharBR* pJob = new MAsyncDBJob_RewardCharBR(uidPlayer);
	pJob->Input(pObj->GetCharInfo()->m_nCID, nBRID, nBRTID, nRewardCount, nBattleTime, nKillCount, nItemID, nItemCnt, nRentHourPeriod, bIsSpendable);
	pObj->m_DBJobQ.DBJobQ.push_back(pJob);
}

void MMatchServer::OnAsyncRequest_UpdateCharBRInfo(const MUID& uidPlayer, int nBRID, int nBRTID, int nRewardCount, int nBattleTime, int nKillCount)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	if (pObj->GetCharInfo() == NULL) return;

	MAsyncDBJob_UpdateCharBRInfo* pJob = new MAsyncDBJob_UpdateCharBRInfo(uidPlayer);
	pJob->Input(pObj->GetCharInfo()->m_nCID, nBRID, nBRTID, nRewardCount, nBattleTime, nKillCount);
	pObj->m_DBJobQ.DBJobQ.push_back(pJob);
}

void MMatchServer::CheckMemoryCorruption()
{
#ifdef _CHECK_MEMORY_CORRUPTION
	if (_CrtCheckMemory() != 0) return;

	mlog("CheckMemoryCorruption...throw...");
	throw "heap corruption";
#endif
}

void MMatchServer::Log(unsigned int nLogLevel, const char* szLog)
{
#ifdef _DEBUG
	if ((nLogLevel & LOG_DEBUG) == LOG_DEBUG)
	{
		OutputDebugString(szLog);
		OutputDebugString("\n");
	}
#endif

	CTime theTime = CTime::GetCurrentTime();
	CString szTime = theTime.Format("[%c] ");

	if ((nLogLevel & LOG_FILE) == LOG_FILE)
	{
		char szTemp[1024];
		strcpy(szTemp, szTime);
		strcat(szTemp, szLog);
		strcat(szTemp, "\n");
		mlog(szTemp);
	}
}