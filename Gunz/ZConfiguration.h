#ifndef _ZCONFIGURATION_H
#define _ZCONFIGURATION_H

#include "MXml.h"
#include <string>
#include <map>
#include "ZActionDef.h"

using namespace std;

#define FILENAME_LOCALE		"system/locale.xml"
#define FILENAME_CONFIG		"config.xml"
#define FILENAME_SYSTEM		"system/system.xml"
#define FILENAME_GTCFG		"system/gametypecfg.xml"

struct ZHOTKEY {
	unsigned long int nVirtKey;
	unsigned long int nModifier;
	string command;
};


struct ZSERVERNODE
{
	char	szName[ 32];
	char	szAddress[ 32];
	int		nPort;
	int		nType;
};


typedef map<int,ZHOTKEY*> ZHOTKEYS;


struct ZCONFIG_VIDEO
{
	bool bFullScreen;
	int nWidth;
	int nHeight;
	int nColorBits;
	int nGamma;
	bool bReflection;
	bool bLightMap;
	bool bDynamicLight;
	bool bShader;
	int nAntiAlias;
	int	nCharTexLevel;
	int nMapTexLevel;
	int nEffectLevel;
	int nTextureFormat;
	bool bTerrible;
};

struct ZCONFIG_AUDIO{
	bool	bBGMEnabled;
	bool bBGMMute;
	float	fBGMVolume;
	bool bEffectMute;
	float	fEffectVolume;
	bool b3DSound;
	bool b8BitSound;
	bool bInverse;
	bool bHWMixing;
	bool bHitSound;
	bool bNarrationSound;
};

struct ZCONFIG_MOUSE{
	float	fSensitivity;
	bool	bInvert;
};

struct ZCONFIG_JOYSTICK{
	float	fSensitivity;
	bool	bInvert;
};

struct ZACTIONKEYDESCRIPTION{
	char	szName[256];
	int		nVirtualKey;
	int		nVirtualKeyAlt;	// 액션당 키 2개씩 할당할수있다
};

struct ZCONFIG_MOVINGPICTURE{
	int		iResolution;		// 동영상 캡쳐 해상도
	int		iFileSize;			// 동영상 파일크기(용량제한)
};

struct ZCONFIG_KEYBOARD{
	ZACTIONKEYDESCRIPTION	ActionKeys[ZACTION_COUNT];
};

#define ZCONFIG_MACRO_MAX 8

struct ZCONFIG_MACRO
{
	char szMacro[ ZCONFIG_MACRO_MAX ][256];

	void SetString(int i,char* str) {
		if(i<0 || i>ZCONFIG_MACRO_MAX-1)
			return;
		if(str==NULL)
			return;

		int nLen = min( 256, (int)strlen( str));
		strncpy(szMacro[i], str, nLen);
		szMacro[i][nLen] = 0;
	}

	char* GetString(int i) {
		if(i<0 || i>ZCONFIG_MACRO_MAX-1)
			return NULL;
		return szMacro[i];
	}
};

struct ZCONFIG_ETC{
	int			nNetworkPort1;
	int			nNetworkPort2;
	bool		bBoost;
	bool		bRejectNormalChat;
	bool		bRejectTeamChat;
	bool		bRejectClanChat;
	bool		bRejectWhisper;
	bool		bRejectInvite;
	int			nCrossHair;
	bool		bInGameNoChat;			// 게임중 대화 차단 여부: 게임중 채팅입력, 일반말, 귓말, 팀채팅 차단됨.
										// 오직 클랜채팅, 채팅 명령어만 허용함.
	char		szLanguage[32];			// 현재 선택된 언어
	int			nFrameLimit_perSecond;
};

/// 로케일과 관련된 설정값
struct ZCONFIG_SELECTABLE_LANGUAGE {
	string		strLanguage;			// 내부적으로 언어를 구별하기 위한 문자열 "USA" "KOR" "GRM" 등
	string		strLanguageName;		// 유저에게 보여주기 위한 언어명
};
struct ZCONFIG_LOCALE {
	char		szDefaultFont[ 32];
	char		szXmlHeader[256];
	char		szHomepageUrl[256];
	char		szHomepageTitle[128];
	char		szEmblemURL[256];
	char		szTEmblemURL[256];
	char		szCashShopURL[256];
	bool		bIMESupport;

	string		strCountry;
	string		strDefaultLanguage;
	int			nMaxPlayers;

	vector<ZCONFIG_SELECTABLE_LANGUAGE> vecSelectableLanguage;

};

class ZLocatorList;
class ZGameTypeList;
class ZLocale;

class ZConfiguration {
private:
	ZCONFIG_VIDEO		m_Video;
	ZCONFIG_AUDIO		m_Audio;
	ZCONFIG_MOUSE		m_Mouse;
	ZCONFIG_JOYSTICK	m_Joystick;
	ZCONFIG_KEYBOARD	m_Keyboard;
	ZCONFIG_MACRO		m_Macro;
	ZCONFIG_ETC			m_Etc;
	ZCONFIG_LOCALE		m_Locale;
	ZCONFIG_MOVINGPICTURE	m_MovingPicture;
	char				m_szBAReportAddr[256];
	char				m_szBAReportDir[256];
	char				m_szInterfaceSkinName[256];

	char				m_szServerIP[256];
	int					m_nServerPort;
	bool				m_bOptimization;

	int					m_nServerCount;
	bool				m_bViewGameChat;		// 게임중 채팅을 볼지 안볼지 여부

	ZLocatorList*		m_pLocatorList;
	ZLocatorList*		m_pTLocatorList;

	ZGameTypeList*		m_pGameTypeList;

	bool				m_bIsComplete;
	bool				m_bReservedSave;		// 단축키로 실시간 변경한 값을 파일로 아직 저장안한 경우

protected:
	void ParseLocaleSelectableLanguages(MXmlElement& selectableLangsElem);

public:
	ZConfiguration();
	virtual ~ZConfiguration();

	void Init();
	void Destroy();
	void LoadDefaultKeySetting();

	void InitConfigIfNotExists();

	bool Load();
	bool Load_StringResDependent();
	bool LoadLocale(const char* szFileName);
	bool LoadGameTypeCfg(const char* szFileName);
	bool LoadSystem(const char* szFileName);
	bool LoadConfig(const char* szFileName);
	bool Save( const char* szHeader)	{ return SaveToFile(FILENAME_CONFIG, szHeader); }
	bool SaveToFile(const char*szFileName, const char* szHeader);
	bool LoadHotKey(const char* szFileName);

	ZLocatorList* GetLocatorList()	{ return m_pLocatorList; }
	ZLocatorList* GetTLocatorList()	{ return m_pTLocatorList; }

	ZGameTypeList* GetGameTypeList()	{ return m_pGameTypeList; }

	char*	GetBAReportAddr()		{ return m_szBAReportAddr; }
	char*	GetBAReportDir()		{ return m_szBAReportDir; }

//	char*	GetXmlHeader()			{ return m_Locale.szXmlHeader; }

	char*	GetInterfaceSkinName()	{ return m_szInterfaceSkinName; }
	char*	GetServerIP()			{ return m_szServerIP; }
	int		GetServerPort()			{ return m_nServerPort; }
	int		GetServerCount()		{ return m_nServerCount; }
	ZSERVERNODE	GetServerNode( int nNum);
//	char*	GetDefaultFont()		{ return m_Locale.szDefaultFont; }
//	bool	GetIMESupport()			{ return m_Locale.bIMESupport; }

	bool GetViewGameChat()			{ return m_bViewGameChat; }
	void SetViewGameChat(bool b)	{ m_bViewGameChat = b; }

	void SetForceOptimization(bool b) {	m_bOptimization = b;}
	bool GetForceOptimization() const {	return m_bOptimization;}

	ZHOTKEY *GetHotkey(int nID);

	ZHOTKEYS m_HotKeys;

	map<int,ZSERVERNODE>	m_ServerList;

	ZCONFIG_VIDEO* GetVideo()		{ return &m_Video; }
	ZCONFIG_AUDIO* GetAudio()		{ return &m_Audio; }
	ZCONFIG_MOUSE* GetMouse()		{ return &m_Mouse; }
	ZCONFIG_KEYBOARD* GetKeyboard() { return &m_Keyboard; }
	ZCONFIG_JOYSTICK* GetJoystick()	{ return &m_Joystick; }
	ZCONFIG_ETC* GetEtc()			{ return &m_Etc; }
	ZCONFIG_MACRO* GetMacro()		{ return &m_Macro; }
	ZCONFIG_LOCALE* GetLocale()		{ return &m_Locale; }
	ZCONFIG_MOVINGPICTURE* GetMovingPicture() { return &m_MovingPicture;}

	const bool IsComplete()			{ return m_bIsComplete; }
	void ReserveSave()				{ m_bReservedSave = true; }

	const bool LateStringConvert();

public:
	// 마우스 감도설정값 최소 0.0125f 최대 1.0000f
#define MOUSE_SENSITIVITY_MIN 125
#define MOUSE_SENSITIVITY_MAX 10000

	int ValidateMouseSensitivityInInt(int i);
	float ValidateMouseSensitivityInFloat(float f);

	int GetMouseSensitivityInInt();
	float GetMouseSensitivityInFloat();
	
	float SetMouseSensitivityInFloat(float f);
	int SetMouseSensitivityInInt(int i);

public:
	// 현재 디폴트 언어가 선택되어 있는가
	bool IsUsingDefaultLanguage()	{ return m_Locale.strDefaultLanguage == m_Etc.szLanguage; }
	// 현재 선택된 언어ID문자열을 반환
	const char* GetSelectedLanguage() { return m_Etc.szLanguage; }
	// 현재 선택된 언어의 인덱스를 반환
	unsigned int GetSelectedLanguageIndex();
	// 언어 선택을 바꿈
	void SetSelectedLanguageIndex(unsigned int i);
private:
	void ValidateSelectedLanguage();
};


ZConfiguration*	ZGetConfiguration();



//////////////////////////////////////////////////////////////
// NHNUSA에서는 런처가 커맨드라인으로 선택언어를 넘겨준다. 즉 NHN 버전은 유저의 선택 언어를 config.xml에 저장하지 않는다.
// 대신 기동시 커맨드라인을 파싱해 여기에 선택언어를 저장해두고 실행중 언어변경은 여기 저장해 놓은 값에 의존한다. (보통은 config.xml에 의존)
class ZLanguageSetting_forNHNUSA
{
	static unsigned int m_idLang;

public:
	static void SetLanguageIndexFromCmdLineStr(const char* cmdline);// 커맨드라인에서 언어선택을 알아낸다

	static void SetLanguageSetting(unsigned int idLang) { m_idLang = idLang; }
	static unsigned int GetLanguageSetting() { return m_idLang; }
};
ZLanguageSetting_forNHNUSA* ZGetLanguageSetting_forNHNUSA();
//////////////////////////////////////////////////////////////




#define ZTOK_SERVER		"SERVER"
#define ZTOK_IP				"IP"
#define ZTOK_PORT			"PORT"
#define ZTOK_TYPE			"TYPE"
#define ZTOK_NAME			"NAME"

#define ZTOK_ADDR			"ADDR"
#define ZTOK_DIR			"DIR"
#define ZTOK_SKIN			"SKIN"
#define ZTOK_LOCATOR_LIST	"LOCATORLIST"
#define ZTOK_TLOCATOR_LIST	"TLOCATORLIST"

#define ZTOK_GAME_TYPE		"GAMETYPE"

#define ZTOK_LOCALE_XMLHEADER		"XMLHEADER"
#define ZTOK_LOCALE_BAREPORT		"BAREPORT"
#define ZTOK_LOCALE_DEFFONT			"DEFFONT"
#define ZTOK_LOCALE_IME				"IME"
#define ZTOK_LOCALE_HOMEPAGE		"HOMEPAGE"
#define ZTOK_LOCALE_HOMEPAGE_URL	"URL"
#define ZTOK_LOCALE_HOMEPAGE_TITLE	"TITLE"
#define ZTOK_LOCALE_EMBLEM_URL		"EMBLEM_URL"
#define ZTOK_LOCALE_TEMBLEM_URL		"TEMBLEM_URL"
#define ZTOK_LOCALE_CASHSHOP_URL	"CASHSHOP_URL"

#define ZTOK_BINDS		"BINDS"
#define ZTOK_BIND		"BIND"
#define ZTOK_KEY		"key"

#define ZTOK_KEY_CTRL	"ctrl"
#define ZTOK_KEY_ALT	"alt"
#define ZTOK_KEY_SHIFT	"shift"

#define ZTOK_VIDEO						"VIDEO"
#define ZTOK_AUDIO						"AUDIO"
#define ZTOK_VIDEO_WIDTH				"WIDTH"
#define ZTOK_VIDEO_HEIGHT				"HEIGHT"
#define ZTOK_VIDEO_FULLSCREEN			"FULLSCREEN"
#define ZTOK_VIDEO_COLORBITS			"COLORBITS"
#define ZTOK_VIDEO_GAMMA				"GAMMA"
#define ZTOK_VIDEO_REFLECTION			"REFLECTION"
#define ZTOK_VIDEO_LIGHTMAP				"LIGHTMAP"
#define ZTOK_VIDEO_DYNAMICLIGHT			"DYNAMICLIGHT"
#define ZTOK_VIDEO_SHADER				"SHADER"
#define ZTOK_VIDEO_ANTIALIAS			"ANTIALIASING"
#define ZTOK_VIDEO_CHARTEXLEVEL			"CHARACTORTEXTURELEVEL"
#define ZTOK_VIDEO_MAPTEXLEVEL			"MAPTEXTURELEVEL"
#define ZTOK_VIDEO_EFFECTLEVEL			"EFFECTLEVEL"
#define ZTOK_VIDEO_TEXTUREFORMAT		"TEXTUREFORMAT"
#define ZTOK_MOVINGPICTURE_RESOLUTION	"MOVINGPICTURERESOLUTION"	// 동영상 캡쳐 해상도
#define ZTOK_MOVINGPICTURE_FILESIZE		"MOVINGPICTUREFILSIZE"		// 동영상 캡쳐 파일크기(용량제한)

#define ZTOK_AUDIO_BGM_ENABLED			"BGM_ENABLED"
#define ZTOK_AUDIO_BGM_VOLUME			"BGM_VOLUME"
#define ZTOK_AUDIO_EFFECT_VOLUME		"EFFECT_VOLUME"
#define ZTOK_AUDIO_BGM_MUTE				"BGM_MUTE"
#define ZTOK_AUDIO_EFFECT_MUTE			"EFFECT_MUTE"
#define ZTOK_AUDIO_3D_SOUND				"SOUND_3D"
#define ZTOK_AUDIO_8BITSOUND			"A8BITSOUND"
#define ZTOK_AUDIO_INVERSE				"INVERSESOUND"
#define ZTOK_AUDIO_HWMIXING				"HWMIXING"
#define ZTOK_AUDIO_HITSOUND				"HITSOUND"
#define ZTOK_AUDIO_NARRATIONSOUND		"NARRATIONSOUND"

#define ZTOK_MOUSE						"MOUSE"
#define ZTOK_MOUSE_SENSITIVITY			"SENSITIVITY"
#define ZTOK_MOUSE_INVERT				"INVERT"

#define ZTOK_KEYBOARD					"KEYBOARD"

#define ZTOK_JOYSTICK					"JOYSTICK"
#define ZTOK_JOYSTICK_SENSITIVITY		"SENSITIVITY"
#define ZTOK_JOYSTICK_INVERT			"INVERT"

#define ZTOK_MACRO				"MACRO"
#define ZTOK_MACRO_F1			"F1"
#define ZTOK_MACRO_F2			"F2"
#define ZTOK_MACRO_F3			"F3"
#define ZTOK_MACRO_F4			"F4"
#define ZTOK_MACRO_F5			"F5"
#define ZTOK_MACRO_F6			"F6"
#define ZTOK_MACRO_F7			"F7"
#define ZTOK_MACRO_F8			"F8"

#define ZTOK_ETC					"ETC"
#define ZTOK_ETC_NETWORKPORT1		"NETWORKPORT1"
#define ZTOK_ETC_NETWORKPORT2		"NETWORKPORT2"
#define ZTOK_ETC_BOOST				"BOOST"
#define ZTOK_ETC_REJECT_NORMALCHAT	"REJECT_NORMALCHAT"
#define ZTOK_ETC_REJECT_TEAMCHAT	"REJECT_TEAMCHAT"
#define ZTOK_ETC_REJECT_CLANCHAT	"REJECT_CLANCHAT"
#define ZTOK_ETC_REJECT_WHISPER		"REJECT_WHISPER"
#define ZTOK_ETC_REJECT_INVITE		"REJECT_INVITE"
#define ZTOK_ETC_CROSSHAIR			"CROSSHAIR"
#define ZTOK_ETC_LANGUAGE			"LANGUAGE"
#define ZTOK_ETC_FRAMELIMIT_PERSECOND "FRAMELIMIT"

#define ZTOK_LOCALE					"LOCALE"
#define ZTOK_LOCALE_COUNTRY			"COUNTRY"
#define ZTOK_LOCALE_LANGUAGE		"LANGUAGE"
#define ZTOK_LOCALE_MAXPLAYERS		"MAXPLAYERS"
#define ZTOK_LOCALE_SELECTABLE_LANGUAGES "SELECTABLE_LANGUAGES"

// Variables
#define Z_MOUSE_SENSITIVITY			(ZGetConfiguration()->GetMouse()->fSensitivity)
#define Z_MOUSE_INVERT			(ZGetConfiguration()->GetMouse()->bInvert)

#define Z_JOYSTICK_SENSITIVITY	(ZGetConfiguration()->GetJoystick()->fSensitivity)
#define Z_JOYSTICK_INVERT		(ZGetConfiguration()->GetJoystick()->bInvert)

#define Z_AUDIO_BGM_MUTE		(ZGetConfiguration()->GetAudio()->bBGMMute)
#define Z_AUDIO_BGM_VOLUME		(ZGetConfiguration()->GetAudio()->fBGMVolume)
#define Z_AUDIO_EFFECT_MUTE		(ZGetConfiguration()->GetAudio()->bEffectMute)
#define Z_AUDIO_EFFECT_VOLUME	(ZGetConfiguration()->GetAudio()->fEffectVolume)
#define Z_AUDIO_3D_SOUND		(ZGetConfiguration()->GetAudio()->b3DSound)
#define Z_AUDIO_8BITSOUND		(ZGetConfiguration()->GetAudio()->b8BitSound)
#define Z_AUDIO_INVERSE			(ZGetConfiguration()->GetAudio()->bInverse)
#define Z_AUDIO_HWMIXING		(ZGetConfiguration()->GetAudio()->bHWMixing)
#define Z_AUDIO_HITSOUND		(ZGetConfiguration()->GetAudio()->bHitSound)
#define Z_AUDIO_NARRATIONSOUND	(ZGetConfiguration()->GetAudio()->bNarrationSound)

#define Z_VIDEO_GAMMA_VALUE		(ZGetConfiguration()->GetVideo()->nGamma)
#define Z_VIDEO_REFLECTION		(ZGetConfiguration()->GetVideo()->bReflection)
#define Z_VIDEO_LIGHTMAP		(ZGetConfiguration()->GetVideo()->bLightMap)
#define Z_VIDEO_DYNAMICLIGHT	(ZGetConfiguration()->GetVideo()->bDynamicLight)
#define Z_VIDEO_SHADER			(ZGetConfiguration()->GetVideo()->bShader)
#define Z_VIDEO_ANTIALIAS		(ZGetConfiguration()->GetVideo()->nAntiAlias)
#define Z_VIDEO_FULLSCREEN		(ZGetConfiguration()->GetVideo()->bFullScreen )
#define Z_VIDEO_WIDTH			(ZGetConfiguration()->GetVideo()->nWidth )
#define Z_VIDEO_HEIGHT			(ZGetConfiguration()->GetVideo()->nHeight )
#define Z_VIDEO_BPP				(ZGetConfiguration()->GetVideo()->nColorBits )

#define Z_ETC_NETWORKPORT1		(ZGetConfiguration()->GetEtc()->nNetworkPort1)
#define Z_ETC_NETWORKPORT2		(ZGetConfiguration()->GetEtc()->nNetworkPort2)
#define Z_ETC_BOOST				(ZGetConfiguration()->GetEtc()->bBoost)
#define Z_ETC_REJECT_NORMALCHAT	(ZGetConfiguration()->GetEtc()->bRejectNormalChat)
#define Z_ETC_REJECT_TEAMCHAT	(ZGetConfiguration()->GetEtc()->bRejectTeamChat)
#define Z_ETC_REJECT_CLANCHAT	(ZGetConfiguration()->GetEtc()->bRejectClanChat)
#define Z_ETC_REJECT_WHISPER	(ZGetConfiguration()->GetEtc()->bRejectWhisper)
#define Z_ETC_REJECT_INVITE		(ZGetConfiguration()->GetEtc()->bRejectInvite)
#define Z_ETC_CROSSHAIR			(ZGetConfiguration()->GetEtc()->nCrossHair)
#define Z_ETC_FRAMELIMIT_PERSECOND	(ZGetConfiguration()->GetEtc()->nFrameLimit_perSecond)


#define Z_LOCALE_DEFAULT_FONT	(ZGetConfiguration()->GetLocale()->szDefaultFont)
#define Z_LOCALE_XML_HEADER		(ZGetConfiguration()->GetLocale()->szXmlHeader)
#define Z_LOCALE_HOMEPAGE_URL	(ZGetConfiguration()->GetLocale()->szHomepageUrl)
#define Z_LOCALE_HOMEPAGE_TITLE	(ZGetConfiguration()->GetLocale()->szHomepageTitle)
#define Z_LOCALE_EMBLEM_URL		(ZGetConfiguration()->GetLocale()->szEmblemURL)
#define Z_LOCALE_TEMBLEM_URL	(ZGetConfiguration()->GetLocale()->szTEmblemURL)
#define Z_LOCALE_CASHSHOP_URL	(ZGetConfiguration()->GetLocale()->szCashShopURL)
#define Z_LOCALE_IME_SUPPORT	(ZGetConfiguration()->GetLocale()->bIMESupport)

#define Z_VIDEO_EFFECT_HIGH		0
#define Z_VIDEO_EFFECT_NORMAL	1
#define Z_VIDEO_EFFECT_LOW		2

#endif
