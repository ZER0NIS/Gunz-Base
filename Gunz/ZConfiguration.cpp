#include "stdafx.h"

#include "ZConfiguration.h"
#include "Mint.h"
#include "ZInterface.h"
//#include "ZGameInterface.h"
#include "ZLocatorList.h"
#include "ZGameTypeList.h"
#include "ZLocale.h"

#include "BaseConfig.h"

ZConfiguration	g_Configuration;
ZConfiguration* ZGetConfiguration()		{ return &g_Configuration; }


//	LANGID LangID = LANG_KOREAN;			/* Korean */
//#ifdef LOCALE_JAPAN
//	LangID = LANG_JAPANESE;					/* Japanese */
//#elif  LOCALE_US
//	LangID = LANG_ENGLISH;					/* International */
//#elif  LOCALE_BRAZIL
//	LangID = LANG_PORTUGUESE;				/* Brazil */
//#elif  LOCALE_INDIA
//	LangID = LANG_ENGLISH;					/* India */
//#endif



unsigned int ZLanguageSetting_forNHNUSA::m_idLang = 0;

void ZLanguageSetting_forNHNUSA::SetLanguageIndexFromCmdLineStr(const char* cmdline)
{
	// -언어별 id는 하드코딩;-
	// xml\usa\locale.xml 에 있는 선택가능한 언어 목록엔 다음과 같은 인덱스가 지정되어 있다
	const unsigned int id_USA = 0;
	const unsigned int id_GRM = 1;
	const unsigned int id_SPN = 2;

	m_idLang = id_USA;

	if (NULL == cmdline) return;

	if (NULL!= strstr(cmdline, "&u100e:2=en"))		m_idLang = id_USA;
	else if (NULL!= strstr(cmdline, "&u100e:2=es")) m_idLang = id_SPN;
	else if (NULL!= strstr(cmdline, "&u100e:2=de")) m_idLang = id_GRM;
}

ZLanguageSetting_forNHNUSA g_LanguageSettingForNHNUSA;
ZLanguageSetting_forNHNUSA* ZGetLanguageSetting_forNHNUSA() { return &g_LanguageSettingForNHNUSA; }

ZConfiguration::ZConfiguration()
{
	Init();

	strcpy( m_szServerIP, "0,0,0,0");
	m_nServerPort = 6000;
	
	strcpy( m_szBAReportAddr, "www.battlearena.com");
	strcpy( m_szBAReportDir, "incoming");
	
	m_nServerCount = 0;

	m_pLocatorList = new ZLocatorList;
	m_pTLocatorList = new ZLocatorList;

	m_pGameTypeList = new ZGameTypeList;

	m_bIsComplete = false;
	m_bReservedSave = false;
}

ZConfiguration::~ZConfiguration()
{
	if (m_bReservedSave)
		Save( Z_LOCALE_XML_HEADER);

	Destroy();
	SAFE_DELETE(m_pLocatorList);
	SAFE_DELETE(m_pTLocatorList);
	SAFE_DELETE(m_pGameTypeList);
}

void ZConfiguration::Destroy()
{
	m_Locale.vecSelectableLanguage.clear();

	m_pLocatorList->Clear();
	m_pTLocatorList->Clear();
	m_pGameTypeList->Clear();

	while(m_HotKeys.size())
	{
		Mint::GetInstance()->UnregisterHotKey(m_HotKeys.begin()->first);
		delete m_HotKeys.begin()->second;
		m_HotKeys.erase(m_HotKeys.begin());
	}
}

unsigned long int GetVirtKey(const char *key)
{
	int n=atoi(key+1);
	if((key[0]=='f' || key[0]=='F') && n>=1 && n<=12)
		return VK_F1+n-1;

	if(key[0]>='a' && key[0]<='z')
		return 'A'+key[0]-'a';

	return key[0];
}

char *GetKeyName(unsigned long int nVirtKey,char *out)
{
	if(nVirtKey>=VK_F1 && nVirtKey<=VK_F12)
		sprintf(out,"F%d",nVirtKey-VK_F1+1);
	else
		sprintf(out,"%d",nVirtKey);

	return out;
}

bool ZConfiguration::Load()
{
	bool retValue;

#if defined(_PUBLISH) && defined(ONLY_LOAD_MRS_FILES)
	MZFile::SetReadMode(MZIPREADFLAG_ZIP | MZIPREADFLAG_MRS | MZIPREADFLAG_MRS2 | MZIPREADFLAG_FILE);
#endif

	if ( !LoadLocale(FILENAME_LOCALE) )
	{
		mlog( "Cannot open %s file.\n", FILENAME_LOCALE);
		return false;
	}

	InitConfigIfNotExists();

	retValue = LoadConfig(FILENAME_CONFIG);


#if defined(_PUBLISH) && defined(ONLY_LOAD_MRS_FILES)
	MZFile::SetReadMode(MZIPREADFLAG_MRS2);
#endif

	if ( !LoadSystem(FILENAME_SYSTEM))
	{
		mlog( "Cannot open %s file.\n", FILENAME_SYSTEM);
		return false;
	}

	if ( !retValue)
		return false;


	return retValue;
}


bool ZConfiguration::Load_StringResDependent()
{
	string strFileNameTCFG(FILENAME_GTCFG);
#ifndef _DEBUG
	strFileNameTCFG += "";
#endif	
	if ( !LoadGameTypeCfg(strFileNameTCFG.c_str()) )
	{
		mlog( "Cannot open %s file.\n", strFileNameTCFG);
		return false;
	}
	return true;
}

bool ZConfiguration::LoadLocale(const char* szFileName)
{
	MXmlDocument	xmlLocale;
	MXmlElement		parentElement, serverElement, bindsElement;
	MXmlElement		childElement;
	MXmlElement		selectableLangsElem;

	mlog("Load XML from memory : %s", szFileName);

	if (!xmlLocale.LoadFromFile(szFileName, ZApplication::GetFileSystem()))
	{
		mlog("- FAIL\n");

		return false;
	}

	mlog("- SUCCESS\n");

	parentElement = xmlLocale.GetDocumentElement();
	int iCount = parentElement.GetChildNodeCount();

	if (!parentElement.IsEmpty())
	{
		if( parentElement.FindChildNode(ZTOK_LOCALE, &childElement) )
		{
			char szCountry[ 16 ]	= "";
			char szLanguage[ 16 ]	= "";
			int nMaxPlayers = 126;

			childElement.GetChildContents( szCountry, ZTOK_LOCALE_COUNTRY );
			childElement.GetChildContents( szLanguage, ZTOK_LOCALE_LANGUAGE );
			childElement.GetChildContents( &nMaxPlayers, ZTOK_LOCALE_MAXPLAYERS);
			if (childElement.FindChildNode(ZTOK_LOCALE_SELECTABLE_LANGUAGES, &selectableLangsElem))
				ParseLocaleSelectableLanguages(selectableLangsElem);

			if( (0 == szCountry) || (0 == szLanguage) )
			{
				mlog( "config.xml - Country or Language is invalid.\n" );
				return false;
			}

			m_Locale.strCountry			= szCountry;
			m_Locale.strDefaultLanguage	= szLanguage;
			m_Locale.nMaxPlayers		= nMaxPlayers;

			mlog( "Country : (%s), Language : (%s)\n", szCountry, szLanguage );
		}
	}
	xmlLocale.Destroy();

	return true;
}

void ZConfiguration::ParseLocaleSelectableLanguages(MXmlElement& selectableLangsElem)
{
	char szTag[256];
	char szLanguageID[256];
	char szLanguageName[256];
	MXmlElement elem;
	int numChild = selectableLangsElem.GetChildNodeCount();

	for (int i=0; i<numChild; ++i)
	{
		elem = selectableLangsElem.GetChildNode(i);
		elem.GetTagName(szTag);
		if (strcmp(szTag, ZTOK_LOCALE_LANGUAGE) == 0)
		{
			bool bID	= elem.GetAttribute(szLanguageID, "id");
			bool bName	= elem.GetAttribute(szLanguageName, "name");
			//_ASSERT(bID && bName);

			ZCONFIG_SELECTABLE_LANGUAGE langSelectable;
			langSelectable.strLanguage = szLanguageID;
			langSelectable.strLanguageName = szLanguageName;
			m_Locale.vecSelectableLanguage.push_back(langSelectable);
		}
	}
}


bool ZConfiguration::LoadGameTypeCfg(const char* szFileName)
{
	MXmlDocument xmlIniData;

	mlog("Load XML from memory : %s", szFileName);

	if (!xmlIniData.LoadFromFile(szFileName, ZApplication::GetFileSystem()))
	{
		mlog("- FAIL\n");

		return false;
	}

	mlog("- SUCCESS\n");

	MXmlElement rootElement, chrElement, attrElement;

	char szTagName[ 256];

	rootElement = xmlIniData.GetDocumentElement();

	int iCount = rootElement.GetChildNodeCount();

	for ( int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode( i);
		chrElement.GetTagName( szTagName);

		if (szTagName[0] == '#') continue;

		if ( !stricmp( szTagName, ZTOK_GAME_TYPE)) 
		{
			int nID = 0;
			chrElement.GetAttribute( &nID, "id");

			m_pGameTypeList->ParseGameTypeList( nID, chrElement);
		}
	}

	return true;
}

bool ZConfiguration::LoadSystem(const char* szFileName)
{
	mlog("Load XML from memory : %s", szFileName);

	MXmlDocument xmlConfig;
	if (!xmlConfig.LoadFromFile(szFileName, ZApplication::GetFileSystem()))
	{
		mlog("- FAIL\n");
		return false;
	}

	mlog("- SUCCESS\n");

	MXmlElement		parentElement = xmlConfig.GetDocumentElement();
	MXmlElement		serverElement, childElement;

	int iCount = parentElement.GetChildNodeCount();

	if (!parentElement.IsEmpty())
	{
		m_ServerList.clear();

		m_nServerCount = 0;
		while ( 1)
		{
			char szText[ 256];
			sprintf( szText, "%s%d", ZTOK_SERVER, m_nServerCount);
			if (parentElement.FindChildNode( szText, &serverElement))
			{
				char szServerIP[ 32];
				char szName[ 32];
				int nServerPort;
				int nServerType;
				serverElement.GetChildContents( szServerIP,		ZTOK_IP);
				serverElement.GetChildContents( &nServerPort,	ZTOK_PORT);
				serverElement.GetChildContents( &nServerType,	ZTOK_TYPE);
				serverElement.GetChildContents( szName,			ZTOK_NAME);

                ZSERVERNODE ServerNode;
				strcpy( ServerNode.szAddress, szServerIP);
				strcpy( ServerNode.szName, szName);
				ServerNode.nPort = nServerPort;
				ServerNode.nType = nServerType;

				m_ServerList.insert( map<int,ZSERVERNODE>::value_type( m_nServerCount, ServerNode));

				m_nServerCount++;
			}
			else
				break;
		}

		if (parentElement.FindChildNode(ZTOK_LOCALE_BAREPORT, &childElement))
		{
			childElement.GetChildContents( m_szBAReportAddr, ZTOK_ADDR);
			childElement.GetChildContents( m_szBAReportDir,  ZTOK_DIR);
		}

		if (parentElement.FindChildNode(ZTOK_LOCALE_XMLHEADER, &childElement))
		{
			childElement.GetContents(m_Locale.szXmlHeader);
		}

		if (parentElement.FindChildNode(ZTOK_SKIN, &childElement))
		{
			childElement.GetContents(m_szInterfaceSkinName);
		}

		if (parentElement.FindChildNode(ZTOK_LOCALE_DEFFONT, &childElement))
		{
			childElement.GetContents(m_Locale.szDefaultFont);
		}

		if (parentElement.FindChildNode(ZTOK_LOCALE_IME, &childElement))
		{
 			childElement.GetContents(&m_Locale.bIMESupport);

			MEvent::SetIMESupport( m_Locale.bIMESupport);
		}
		if (parentElement.FindChildNode(ZTOK_LOCALE_HOMEPAGE, &childElement))
		{
			childElement.GetChildContents( m_Locale.szHomepageUrl,		ZTOK_LOCALE_HOMEPAGE_URL);
			childElement.GetChildContents( m_Locale.szHomepageTitle,	ZTOK_LOCALE_HOMEPAGE_TITLE);
		}
		if (parentElement.FindChildNode(ZTOK_LOCALE_EMBLEM_URL, &childElement))
		{
			childElement.GetContents( m_Locale.szEmblemURL);
		}
		if (parentElement.FindChildNode(ZTOK_LOCALE_TEMBLEM_URL, &childElement))
		{
			childElement.GetContents( m_Locale.szTEmblemURL);
		}
		if (parentElement.FindChildNode(ZTOK_LOCALE_CASHSHOP_URL, &childElement))
		{
			childElement.GetContents( m_Locale.szCashShopURL);
		}
		if (parentElement.FindChildNode(ZTOK_LOCATOR_LIST, &childElement))
		{
			m_pLocatorList->ParseLocatorList(childElement);
		}
		if (parentElement.FindChildNode(ZTOK_TLOCATOR_LIST, &childElement))
		{
			m_pTLocatorList->ParseLocatorList(childElement);
		}
	}
	xmlConfig.Destroy();

	m_bIsComplete = true;
	return true;
}

void ZConfiguration::InitConfigIfNotExists()
{
	FILE* pFile = fopen(FILENAME_CONFIG, "r");
	if (pFile) {
		fclose(pFile);
		return;
	}

	pFile = fopen(FILENAME_CONFIG, "w");
	fwrite(szBaseConfigFile, sizeof(char), sizeof(szBaseConfigFile) - 1, pFile);

	fclose(pFile);
}

bool ZConfiguration::LoadConfig(const char* szFileName)
{
	MXmlDocument	xmlConfig;
	MXmlElement		parentElement, serverElement, bindsElement;
	MXmlElement		childElement;

	mlog( "Load Config from file : %s", szFileName );

	xmlConfig.Create();
	if (!xmlConfig.LoadFromFile(szFileName)) 
	{
		mlog( "- FAIL\n");
		xmlConfig.Destroy();
		return false;
	}
	mlog( "- SUCCESS\n");

	parentElement = xmlConfig.GetDocumentElement();
	int iCount = parentElement.GetChildNodeCount();

	if (!parentElement.IsEmpty())
	{
		if (parentElement.FindChildNode( ZTOK_SERVER, &serverElement))
		{
			serverElement.GetChildContents( m_szServerIP,	ZTOK_IP);
			serverElement.GetChildContents( &m_nServerPort,	ZTOK_PORT);
		}
		if (parentElement.FindChildNode(ZTOK_VIDEO, &childElement))
		{
			childElement.GetChildContents(&m_Video.nWidth, ZTOK_VIDEO_WIDTH);
			childElement.GetChildContents(&m_Video.nHeight, ZTOK_VIDEO_HEIGHT);
			childElement.GetChildContents(&m_Video.nColorBits, ZTOK_VIDEO_COLORBITS);
			childElement.GetChildContents(&m_Video.bFullScreen, ZTOK_VIDEO_FULLSCREEN);
			childElement.GetChildContents(&m_Video.nGamma, ZTOK_VIDEO_GAMMA);
			childElement.GetChildContents(&m_Video.bReflection,	ZTOK_VIDEO_REFLECTION );
			childElement.GetChildContents(&m_Video.bLightMap, ZTOK_VIDEO_LIGHTMAP );
			childElement.GetChildContents(&m_Video.bDynamicLight, ZTOK_VIDEO_DYNAMICLIGHT );
			childElement.GetChildContents(&m_Video.bShader, ZTOK_VIDEO_SHADER );
			childElement.GetChildContents(&m_Video.nAntiAlias, ZTOK_VIDEO_ANTIALIAS );
			childElement.GetChildContents(&m_Video.nCharTexLevel, ZTOK_VIDEO_CHARTEXLEVEL );
			childElement.GetChildContents(&m_Video.nMapTexLevel, ZTOK_VIDEO_MAPTEXLEVEL );
			childElement.GetChildContents(&m_Video.nEffectLevel, ZTOK_VIDEO_EFFECTLEVEL );
			childElement.GetChildContents(&m_Video.nTextureFormat, ZTOK_VIDEO_TEXTUREFORMAT );
			childElement.GetChildContents(&m_Video.bTerrible, "NHARDWARETNL");
			childElement.GetChildContents(&m_MovingPicture.iResolution, ZTOK_MOVINGPICTURE_RESOLUTION );
			childElement.GetChildContents(&m_MovingPicture.iFileSize, ZTOK_MOVINGPICTURE_FILESIZE );
		}
		if (parentElement.FindChildNode(ZTOK_AUDIO, &childElement))
		{
			childElement.GetChildContents(&m_Audio.bBGMEnabled, ZTOK_AUDIO_BGM_ENABLED);
			childElement.GetChildContents(&m_Audio.fBGMVolume, ZTOK_AUDIO_BGM_VOLUME);
			childElement.GetChildContents(&m_Audio.fEffectVolume, ZTOK_AUDIO_EFFECT_VOLUME);
			childElement.GetChildContents(&m_Audio.bBGMMute, ZTOK_AUDIO_BGM_MUTE);
			childElement.GetChildContents(&m_Audio.bEffectMute, ZTOK_AUDIO_EFFECT_MUTE);
			childElement.GetChildContents(&m_Audio.b8BitSound, ZTOK_AUDIO_8BITSOUND);
			childElement.GetChildContents(&m_Audio.bInverse, ZTOK_AUDIO_INVERSE);
			childElement.GetChildContents(&m_Audio.bHWMixing, ZTOK_AUDIO_HWMIXING);
			childElement.GetChildContents(&m_Audio.bHitSound, ZTOK_AUDIO_HITSOUND);
			childElement.GetChildContents(&m_Audio.bNarrationSound, ZTOK_AUDIO_NARRATIONSOUND);
			//childElement.GetChildContents(&m_Audio.b3DSound, ZTOK_AUDIO_3D_SOUND);
			m_Audio.b3DSound = true;
		}
		if (parentElement.FindChildNode(ZTOK_MOUSE, &childElement))
		{
			childElement.GetChildContents(&m_Mouse.fSensitivity, ZTOK_MOUSE_SENSITIVITY);
			childElement.GetChildContents(&m_Mouse.bInvert, ZTOK_MOUSE_INVERT);
		}
		if (parentElement.FindChildNode(ZTOK_JOYSTICK, &childElement))
		{
			childElement.GetChildContents(&m_Joystick.fSensitivity, ZTOK_JOYSTICK_SENSITIVITY);
			childElement.GetChildContents(&m_Joystick.bInvert, ZTOK_JOYSTICK_INVERT);
		}
		if (parentElement.FindChildNode(ZTOK_KEYBOARD, &childElement))
		{
			for(int i=0; i<ZACTION_COUNT; i++){
				char szItemName[256];
				strcpy(szItemName, m_Keyboard.ActionKeys[i].szName);
				_strupr(szItemName);

				MXmlNode keyNode;
				if (childElement.FindChildNode(szItemName, &keyNode))
				{
					MXmlElement actionKeyElement = keyNode;

					const int ID_UNDEFINED = -2;
					int nKey;
					actionKeyElement.GetAttribute(&nKey,"alt",ID_UNDEFINED); // "alt"에 값이 없다면 ID_UNDEFINED 를 세팅
					if(nKey!=ID_UNDEFINED) // "alt"에 값이 없다면 스킵
						m_Keyboard.ActionKeys[i].nVirtualKeyAlt = nKey;
					actionKeyElement.GetContents(&m_Keyboard.ActionKeys[i].nVirtualKey);
				}
			}
		}

		if( parentElement.FindChildNode(ZTOK_MACRO, &childElement) )
		{
			//char buf[8][256];

			//childElement.GetChildContents(buf[0], ZTOK_MACRO_F1, 255);
			//childElement.GetChildContents(buf[1], ZTOK_MACRO_F2, 255);
			//childElement.GetChildContents(buf[2], ZTOK_MACRO_F3, 255);
			//childElement.GetChildContents(buf[3], ZTOK_MACRO_F4, 255);
			//childElement.GetChildContents(buf[4], ZTOK_MACRO_F5, 255);
			//childElement.GetChildContents(buf[5], ZTOK_MACRO_F6, 255);
			//childElement.GetChildContents(buf[6], ZTOK_MACRO_F7, 255);
			//childElement.GetChildContents(buf[7], ZTOK_MACRO_F8, 255);

			//for (int i = 0; i < 8; i++)
			//{
			//	strcpy(m_Macro.szMacro[i], ZGetStringResManager()->GetStringFromXml(buf[i]));
			//}

			// 여기선 읽기만 함. 
			// string.xml을 읽은 후 다시 컨버팅 함.
			// config.xml에 있는 lcale정보로 string.xml의 국가를 결정하기 때문에 이 부분은 바로 처리할 수 없음.
			childElement.GetChildContents(m_Macro.szMacro[0], ZTOK_MACRO_F1, 255);
			childElement.GetChildContents(m_Macro.szMacro[1], ZTOK_MACRO_F2, 255);
			childElement.GetChildContents(m_Macro.szMacro[2], ZTOK_MACRO_F3, 255);
			childElement.GetChildContents(m_Macro.szMacro[3], ZTOK_MACRO_F4, 255);
			childElement.GetChildContents(m_Macro.szMacro[4], ZTOK_MACRO_F5, 255);
			childElement.GetChildContents(m_Macro.szMacro[5], ZTOK_MACRO_F6, 255);
			childElement.GetChildContents(m_Macro.szMacro[6], ZTOK_MACRO_F7, 255);
			childElement.GetChildContents(m_Macro.szMacro[7], ZTOK_MACRO_F8, 255);
		}

		if (parentElement.FindChildNode(ZTOK_ETC, &childElement))
		{
			childElement.GetChildContents(&m_Etc.nNetworkPort1, ZTOK_ETC_NETWORKPORT1);
			childElement.GetChildContents(&m_Etc.nNetworkPort2, ZTOK_ETC_NETWORKPORT2);
			childElement.GetChildContents(&m_Etc.bBoost, ZTOK_ETC_BOOST);
			childElement.GetChildContents(&m_Etc.bRejectNormalChat, ZTOK_ETC_REJECT_NORMALCHAT);
			childElement.GetChildContents(&m_Etc.bRejectTeamChat, ZTOK_ETC_REJECT_TEAMCHAT);
			childElement.GetChildContents(&m_Etc.bRejectClanChat, ZTOK_ETC_REJECT_CLANCHAT);
			childElement.GetChildContents(&m_Etc.bRejectWhisper, ZTOK_ETC_REJECT_WHISPER);
			childElement.GetChildContents(&m_Etc.bRejectInvite, ZTOK_ETC_REJECT_INVITE);
			childElement.GetChildContents(&m_Etc.nCrossHair, ZTOK_ETC_CROSSHAIR);
			childElement.GetChildContents(&m_Etc.nFrameLimit_perSecond, ZTOK_ETC_FRAMELIMIT_PERSECOND);

			m_Etc.szLanguage[0] = 0;

#ifdef _MULTILANGUAGE
	#ifdef LOCALE_NHNUSAA
			SetSelectedLanguageIndex( g_LanguageSettingForNHNUSA.GetLanguageSetting());
	#else
			// 기본적으로는 게임내부 옵션에서 선택한 언어를 config에 저장했던 것을 로딩
			childElement.GetChildContents( m_Etc.szLanguage, ZTOK_ETC_LANGUAGE, 32);
	#endif
#endif //_MULTILANGUAGE

		}

		ValidateSelectedLanguage();

		/*
		if (parentElement.FindChildNode(ZTOK_BINDS, &bindsElement))
		{
			for(int i=0;i<bindsElement.GetChildNodeCount();i++)
			{
				char tagname[256];
				MXmlElement bind=bindsElement.GetChildNode(i);
				bind.GetTagName(tagname);
				if(strcmp(tagname,ZTOK_BIND)==0)
				{
					char key[256],command[256];
					int ctrl,alt,shift;

					bind.GetAttribute(key,ZTOK_KEY);
					bind.GetAttribute(&ctrl,ZTOK_KEY_CTRL);
					bind.GetAttribute(&alt,ZTOK_KEY_ALT);
					bind.GetAttribute(&shift,ZTOK_KEY_SHIFT);
					bind.GetContents(command);

					ZHOTKEY *photkey=new ZHOTKEY;
					photkey->nModifier=0;

					if(ctrl) photkey->nModifier|=MOD_CONTROL;
					if(alt) photkey->nModifier|=MOD_ALT;
					if(shift) photkey->nModifier|=MOD_SHIFT;

					photkey->nVirtKey=GetVirtKey(key);

					photkey->command=string(command);

					int nHotkeyID=MRegisterHotKey(photkey->nModifier,photkey->nVirtKey);
					m_HotKeys.insert(ZHOTKEYS::value_type(nHotkeyID,photkey));
				}
			}
		}
		*/
	}

	//if( m_Video.bTerrible )
	//{
	//	//m_Video.nCharTexLevel = 2;
	//	//m_Video.nMapTexLevel = 2;
	//	//m_Video.nEffectLevel = 2;
	//	//m_Video.bDynamicLight = false;
	//	//m_Video.bReflection = false;
	//}
	//else 
	//{
	//	m_Video.bLightMap = false; // 최하위 버전이 아닐 경우 라이트 맵을 끄지 못한다
	//}

	xmlConfig.Destroy();

	return true;
}

// 핫키는 봉인되었다.
bool ZConfiguration::LoadHotKey(const char* szFileName)
{
	MXmlDocument	xmlConfig;
	MXmlElement		parentElement, serverElement, bindsElement;
	MXmlElement		childElement;

	xmlConfig.Create();
	if (!xmlConfig.LoadFromFile(szFileName)) 
	{
		xmlConfig.Destroy();
		return false;
	}

	parentElement = xmlConfig.GetDocumentElement();
	int iCount = parentElement.GetChildNodeCount();

	if (!parentElement.IsEmpty())
	{
		if (parentElement.FindChildNode(ZTOK_BINDS, &bindsElement))
		{
			for(int i=0;i<bindsElement.GetChildNodeCount();i++)
			{
				char tagname[256];
				MXmlElement bind=bindsElement.GetChildNode(i);
				bind.GetTagName(tagname);
				if(strcmp(tagname,ZTOK_BIND)==0)
				{
					char key[256],command[256];
					bool ctrl,alt,shift;

					bind.GetAttribute(key,ZTOK_KEY);
					bind.GetAttribute(&ctrl,ZTOK_KEY_CTRL);
					bind.GetAttribute(&alt,ZTOK_KEY_ALT);
					bind.GetAttribute(&shift,ZTOK_KEY_SHIFT);
					bind.GetContents(command);

					ZHOTKEY *photkey=new ZHOTKEY;
					photkey->nModifier=0;

					if(ctrl) photkey->nModifier|=MOD_CONTROL;
					if(alt) photkey->nModifier|=MOD_ALT;
					if(shift) photkey->nModifier|=MOD_SHIFT;

					photkey->nVirtKey=GetVirtKey(key);

					photkey->command=string(command);

					int nHotkeyID=Mint::GetInstance()->RegisterHotKey(photkey->nModifier,photkey->nVirtKey);

					m_HotKeys.insert(ZHOTKEYS::value_type(nHotkeyID,photkey));
				}
			}
		}
	}

	xmlConfig.Destroy();

	return true;

}

bool ZConfiguration::SaveToFile(const char *szFileName, const char* szHeader)
{
	char buffer[256];

	MXmlDocument	xmlConfig;

	xmlConfig.Create();
	xmlConfig.CreateProcessingInstruction(szHeader);

	MXmlElement		aRootElement;

	aRootElement=xmlConfig.CreateElement("XML");
	aRootElement.AppendText("\n\t");

	xmlConfig.AppendChild(aRootElement);

	// Check FirstTime Loading

	// Server
	{
		MXmlElement	serverElement=aRootElement.CreateChildElement(ZTOK_SERVER);
		serverElement.AppendText("\n\t\t");

		MXmlElement		aElement;
		aElement = serverElement.CreateChildElement(ZTOK_IP);
		aElement.SetContents(m_szServerIP);

		serverElement.AppendText("\n\t\t");
		aElement = serverElement.CreateChildElement(ZTOK_PORT);
		sprintf(buffer,"%d",m_nServerPort);
		aElement.SetContents(buffer);

		serverElement.AppendText("\n\t");
	}

	aRootElement.AppendText("\n\n\t");

	// Skin
	{
		MXmlElement	skinElement=aRootElement.CreateChildElement(ZTOK_SKIN);
		skinElement.SetContents(m_szInterfaceSkinName);
		skinElement.AppendText("");
	}

	aRootElement.AppendText("\n\n\t");

	// Video
	{
		MXmlElement	parentElement=aRootElement.CreateChildElement(ZTOK_VIDEO);

		parentElement.AppendText("\n\t\t");
		MXmlElement		aElement;
		aElement = parentElement.CreateChildElement(ZTOK_VIDEO_WIDTH);
		char temp[256];
		sprintf(temp, "%d", m_Video.nWidth);
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_VIDEO_HEIGHT);
		sprintf(temp, "%d", m_Video.nHeight);
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_VIDEO_COLORBITS);
		sprintf(temp, "%d", m_Video.nColorBits);
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_VIDEO_GAMMA);
		sprintf(temp, "%d", m_Video.nGamma);
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_VIDEO_FULLSCREEN);
		if(m_Video.bFullScreen==true) strcpy(temp, "TRUE");
		else strcpy(temp, "FALSE");
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_VIDEO_REFLECTION);
		if(m_Video.bReflection==true) strcpy(temp, "true");
		else strcpy(temp, "false");
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_VIDEO_LIGHTMAP);
		if(m_Video.bLightMap==true) strcpy(temp, "true");
		else strcpy(temp, "false");
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_VIDEO_DYNAMICLIGHT);
		if(m_Video.bDynamicLight==true) strcpy(temp, "true");
		else strcpy(temp, "false");
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_VIDEO_SHADER);
		if(m_Video.bShader==true) strcpy(temp, "true");
		else strcpy(temp, "false");
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_VIDEO_ANTIALIAS);
		sprintf(temp, "%d", m_Video.nAntiAlias);
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_VIDEO_CHARTEXLEVEL);
		sprintf(temp, "%d", m_Video.nCharTexLevel);
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_VIDEO_MAPTEXLEVEL);
		sprintf(temp, "%d", m_Video.nMapTexLevel);
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_VIDEO_EFFECTLEVEL);
		sprintf(temp, "%d", m_Video.nEffectLevel);
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_VIDEO_TEXTUREFORMAT);
		sprintf(temp, "%d", m_Video.nTextureFormat);
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_MOVINGPICTURE_RESOLUTION);
		sprintf(temp, "%d", m_MovingPicture.iResolution);
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_MOVINGPICTURE_FILESIZE);
		sprintf(temp, "%d", m_MovingPicture.iFileSize);
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement("NHARDWARETNL");
		sprintf(temp, "%s", m_Video.bTerrible ? "true" : "false" );
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t");
	}

	aRootElement.AppendText("\n\n\t");

	// Audio
	{
		MXmlElement	parentElement=aRootElement.CreateChildElement(ZTOK_AUDIO);

		parentElement.AppendText("\n\t\t");
		MXmlElement		aElement;
		char temp[256];

		aElement = parentElement.CreateChildElement(ZTOK_AUDIO_BGM_MUTE);
		if(m_Audio.bBGMMute==true) strcpy(temp, "true");
		else strcpy(temp, "false");
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_AUDIO_BGM_VOLUME);
		sprintf(temp, "%f", m_Audio.fBGMVolume );
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_AUDIO_EFFECT_MUTE);
		if(m_Audio.bEffectMute==true) strcpy(temp, "true");
		else strcpy(temp, "false");
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_AUDIO_EFFECT_VOLUME);
		sprintf(temp, "%f", m_Audio.fEffectVolume);
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_AUDIO_3D_SOUND);
		sprintf(temp, "%d", m_Audio.b3DSound);
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_AUDIO_8BITSOUND);
		if(m_Audio.b8BitSound==true) strcpy(temp, "true");
		else strcpy(temp, "false");
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_AUDIO_INVERSE);
		if(m_Audio.bInverse==true) strcpy(temp, "true");
		else strcpy(temp, "false");
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_AUDIO_HWMIXING);
		if(m_Audio.bHWMixing==true) strcpy(temp, "true");
		else strcpy(temp, "false");
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_AUDIO_HITSOUND);
 		if(m_Audio.bHitSound==true) strcpy(temp, "true");
		else strcpy(temp, "false");
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_AUDIO_NARRATIONSOUND);
 		if(m_Audio.bNarrationSound==true) strcpy(temp, "true");
		else strcpy(temp, "false");
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t");
	}

	aRootElement.AppendText("\n\n\t");

	// Mouse
	{
		MXmlElement	parentElement=aRootElement.CreateChildElement(ZTOK_MOUSE);

		parentElement.AppendText("\n\t\t");
		MXmlElement		aElement;
		aElement = parentElement.CreateChildElement(ZTOK_MOUSE_SENSITIVITY);
		char temp[256];
		// 버그리포트 => 옵션->키보드->마우스->마우스 감도에서 최하로 설정했을때 마우스가 안움직인다. 
		// 최소인 0바로전 값이 0.0135f이기때문에 이값보다 낮으면 0.0125f로 세팅 20090313 by kammir
		/*if(m_Mouse.fSensitivity < 0.01f)
			m_Mouse.fSensitivity = 0.0125f;*/
		m_Mouse.fSensitivity = ValidateMouseSensitivityInFloat(m_Mouse.fSensitivity);
		sprintf(temp, "%f", m_Mouse.fSensitivity);
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_MOUSE_INVERT);
		if(m_Mouse.bInvert==true) strcpy(temp, "TRUE");
		else strcpy(temp, "FALSE");
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t");
	}

	aRootElement.AppendText("\n\n\t");
	// Joystick
	{
		MXmlElement	parentElement=aRootElement.CreateChildElement(ZTOK_JOYSTICK);

		parentElement.AppendText("\n\t\t");
		MXmlElement		aElement;
		aElement = parentElement.CreateChildElement(ZTOK_JOYSTICK_SENSITIVITY);
		char temp[256];
		sprintf(temp, "%f", m_Joystick.fSensitivity);
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_JOYSTICK_INVERT);
		if(m_Joystick.bInvert==true) strcpy(temp, "TRUE");
		else strcpy(temp, "FALSE");
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t");
	}

	aRootElement.AppendText("\n\n\t");
	// Control
	{
		MXmlElement	parentElement=aRootElement.CreateChildElement(ZTOK_KEYBOARD);
		for(int i=0; i<ZACTION_COUNT; i++){
			char szItemName[256];
			strcpy(szItemName, m_Keyboard.ActionKeys[i].szName);
			_strupr(szItemName);

			if(szItemName[0]!=0){
				parentElement.AppendText("\n\t\t");
				MXmlElement		aElement;
				aElement = parentElement.CreateChildElement(szItemName);
				char temp[256];
				sprintf(temp, "%d", m_Keyboard.ActionKeys[i].nVirtualKey);
				aElement.SetContents(temp);
				aElement.SetAttribute("alt",m_Keyboard.ActionKeys[i].nVirtualKeyAlt);
			}
		}
		parentElement.AppendText("\n\t");
	}

	aRootElement.AppendText("\n\n\t");

	// Macro
//	if( parentElement.FindChildNode(ZTOK_MACRO, &childElement) )
	{
		MXmlElement	parentElement=aRootElement.CreateChildElement(ZTOK_MACRO);

		parentElement.AppendText("\n\t\t");

		MXmlElement		aElement;

		char* _temp[ZCONFIG_MACRO_MAX] = {
			ZTOK_MACRO_F1,
			ZTOK_MACRO_F2,
			ZTOK_MACRO_F3,
			ZTOK_MACRO_F4,
			ZTOK_MACRO_F5,
			ZTOK_MACRO_F6,
			ZTOK_MACRO_F7,
			ZTOK_MACRO_F8,
		};

		for(int i=0;i<ZCONFIG_MACRO_MAX;i++) {

			aElement = parentElement.CreateChildElement( _temp[i] );
			aElement.SetContents(m_Macro.szMacro[i]);

			parentElement.AppendText("\n\t\t");
		}
		parentElement.AppendText("\n\t");
	}

	aRootElement.AppendText("\n\n\t");

	// Etc
	{
		MXmlElement	parentElement=aRootElement.CreateChildElement(ZTOK_ETC);

		MXmlElement		aElement;

		// Network port
		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_ETC_NETWORKPORT1);
		char temp[256];
		sprintf(temp, "%d", m_Etc.nNetworkPort1);
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_ETC_NETWORKPORT2);
		sprintf(temp, "%d", m_Etc.nNetworkPort2);
		aElement.SetContents(temp);

		// Boost
		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_ETC_BOOST);
		sprintf(temp, "%s", m_Etc.bBoost?"TRUE":"FALSE");
		aElement.SetContents(temp);

		// 언어 선택
#ifdef _MULTILANGUAGE
		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_ETC_LANGUAGE);
		sprintf(temp, "%s", GetSelectedLanguage());
		aElement.SetContents(temp);
	#ifdef LOCALE_NHNUSAA
		//NHNUSA는 config.xml에 의존하지 않으므로 언어변경후 재시작을 위해서 언어설정을 별도로 보존해둔다
		g_LanguageSettingForNHNUSA.SetLanguageSetting( GetSelectedLanguageIndex());
	#endif
#endif //_MULTILANGUAGE

		// Reject normal chat
		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_ETC_REJECT_NORMALCHAT);
		sprintf(temp, "%s", m_Etc.bRejectNormalChat?"TRUE":"FALSE");
		aElement.SetContents(temp);

		// Reject team chat
		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_ETC_REJECT_TEAMCHAT);
		sprintf(temp, "%s", m_Etc.bRejectTeamChat?"TRUE":"FALSE");
		aElement.SetContents(temp);

		// Reject clan chat
		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_ETC_REJECT_CLANCHAT);
		sprintf(temp, "%s", m_Etc.bRejectClanChat?"TRUE":"FALSE");
		aElement.SetContents(temp);

		// Reject whisper
		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_ETC_REJECT_WHISPER);
		sprintf(temp, "%s", m_Etc.bRejectWhisper?"TRUE":"FALSE");
		aElement.SetContents(temp);

		// Reject invite
		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_ETC_REJECT_INVITE);
		sprintf(temp, "%s", m_Etc.bRejectInvite?"TRUE":"FALSE");
		aElement.SetContents(temp);

		// crosshair
		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_ETC_CROSSHAIR);
		sprintf(temp, "%d", m_Etc.nCrossHair);
		aElement.SetContents(temp);

		// FrameLimit
		parentElement.AppendText("\n\t\t");
		aElement = parentElement.CreateChildElement(ZTOK_ETC_FRAMELIMIT_PERSECOND);
		sprintf(temp, "%d", m_Etc.nFrameLimit_perSecond);
		aElement.SetContents(temp);

		parentElement.AppendText("\n\t");
	}

	aRootElement.AppendText("\n\n\t");

	// Bind : 봉인

	/*
	aRootElement.AppendText("\n\t");
	MXmlElement bindsElement=aRootElement.CreateChildElement(ZTOK_BINDS);

	for(ZHOTKEYS::iterator i=m_HotKeys.begin();i!=m_HotKeys.end();i++)
	{
		bindsElement.AppendText("\n\t\t");

		MXmlElement bind=bindsElement.CreateChildElement(ZTOK_BIND);

		ZHOTKEY *photkey=(*i).second;

		bind.SetContents(photkey->command.c_str());
		

		char buffer[256];
		bind.SetAttribute(ZTOK_KEY,GetKeyName(photkey->nVirtKey,buffer));

		bind.SetAttribute(ZTOK_KEY_CTRL,photkey->nModifier & MOD_CONTROL ? 1 : 0);
		bind.SetAttribute(ZTOK_KEY_ALT,photkey->nModifier & MOD_ALT ? 1 : 0);
		bind.SetAttribute(ZTOK_KEY_SHIFT,photkey->nModifier & MOD_SHIFT ? 1 : 0);
		
	}
	bindsElement.AppendText("\n\t");
	*/

	aRootElement.AppendText("\n");


//	LANGID LangID = LANG_KOREAN;			/* Korean : 이거 정말 하드코딩 박기 싫었는디... 쩝... -_-;;; */
//#ifdef LOCALE_JAPAN
//	LangID = LANG_JAPANESE;					/* Japanese */
//#elif  LOCALE_US
//	LangID = LANG_ENGLISH;					/* International */
//#elif  LOCALE_BRAZIL
//	LangID = LANG_PORTUGUESE;				/* Brazil */
//#elif  LOCALE_INDIA
//	LangID = LANG_ENGLISH;					/* India */
//#endif

	return xmlConfig.SaveToFile(szFileName);
}

ZHOTKEY *ZConfiguration::GetHotkey(int nID) 
{ 
	ZHOTKEYS::iterator found=m_HotKeys.find(nID);
	return found==m_HotKeys.end() ? NULL : found->second;
}

void ZConfiguration::Init()
{
	m_Video.bFullScreen = false;
	m_Video.nWidth = 1024;
	m_Video.nHeight = 768;
	m_Video.nColorBits = 32;
	m_Video.nGamma = 255;
	m_Video.bShader		= true;
	m_Video.nAntiAlias = 2;
	m_Video.bLightMap	= false;
	m_Video.bReflection	= true;
	m_Video.nCharTexLevel = 0;//기본 고품질
	m_Video.nMapTexLevel = 0;
	m_Video.nEffectLevel = Z_VIDEO_EFFECT_HIGH;
	m_Video.nTextureFormat = 1;
	m_Video.bTerrible = false;
	m_MovingPicture.iResolution = 0;
	m_MovingPicture.iFileSize = 0;

	m_Audio.bBGMEnabled = true;
	m_Audio.fBGMVolume	= 0.3f;
	m_Audio.bBGMMute	= false;
	m_Audio.bEffectMute = false;
	m_Audio.b3DSound	= true;
	m_Audio.b8BitSound	= false;
	m_Audio.bInverse	= false;
	m_Audio.bHWMixing	= false;
	m_Audio.bHitSound	= true;
	m_Audio.bNarrationSound	= true;

	m_Mouse.fSensitivity = 1.f;
	m_Mouse.bInvert = false;

	m_Joystick.fSensitivity = 1.f;
	m_Joystick.bInvert = false;

	m_Macro.SetString(0,"");
	m_Macro.SetString(1,"");
	m_Macro.SetString(2,"");
	m_Macro.SetString(3,"");
	m_Macro.SetString(4,"");
	m_Macro.SetString(5,"");
	m_Macro.SetString(6,"");
	m_Macro.SetString(7,"");
	m_Macro.SetString(8,"");

	m_Etc.nNetworkPort1 = 7700;
	m_Etc.nNetworkPort2 = 7800;
	m_Etc.nCrossHair = 0;
	m_Etc.nFrameLimit_perSecond = 0;
	m_Etc.bInGameNoChat = false;

	m_bOptimization = false;
	
	memset(m_szServerIP, 0, sizeof(m_szServerIP));
	strcpy(m_szServerIP, "0.0.0.0");
	m_nServerPort = 6000;
	strcpy(m_szInterfaceSkinName, DEFAULT_INTERFACE_SKIN);

	LoadDefaultKeySetting();	



	strcpy( m_Locale.szDefaultFont, "굴림");
	strcpy( m_Locale.szXmlHeader, "version=\"1.0\" encoding=\"UTF-8\"");
	m_Locale.szHomepageUrl[0] = 0;
	m_Locale.szHomepageTitle[0] = 0;
	strcpy(m_Locale.szEmblemURL, "");
	strcpy(m_Locale.szTEmblemURL, "");
	strcpy(m_Locale.szCashShopURL, "http://www.gunzonline.com/");
	m_Locale.bIMESupport = false;

	m_bViewGameChat = true;
}

void ZConfiguration::LoadDefaultKeySetting()
{
	// option.xml에서 각 액션키에 대응되는 위젯은 ActionKey 접미사를 붙여서 위젯 이름을 정해야 함
	// ex: 무기 사용 설정 위젯은 'UseWeaponActionKey' 
	static ZACTIONKEYDESCRIPTION DefaultActionKeys[ZACTION_COUNT] = {
		{"UseWeapon",	0x1D,258},	// 'ctrl' or	mouse LButton
		{"UseWeapon2",	259,-1},		// mouse RButton
		{"PrevousWeapon",0x10, 256},	// 'q' or wheel up
		{"NextWeapon", 0x12,257},	// 'e' or wheel down

		{"Forward",		0x11, -1},	// 'w'
		{"Back",		0x1f, -1},	// 's'
		{"Left",		0x1e, -1},	// 'a'
		{"Right",		0x20, -1},	// 'd'

		{"MeleeWeapon", 0x02, -1},		// '1'
		{"PrimaryWeapon",0x03, -1},		// '2'
		{"SecondaryWeapon",0x04, -1},	// '3'
		{"Item1",		0x05, -1},		// '4'
		{"Item2",		0x06, -1},		// '5'
		{"CommunityItem1",		0x07, -1},		// '6'
		{"CommunityItem2",		0x08, -1},		// '7'

		{"Reload",		0x13,-1},	// 'r'
		{"Jump",		0x39,-1},	// space
		{"Score",		0x0f,-1},	// tab
		{"Menu",		0x01,-1},	// esc

		{"Taunt",		0x29,-1},	// '`'
		{"Bow",			0x07,-1},	// '6'
		{"Wave",		0x08,-1},	// '7'
		{"Laugh",		0x09,-1},	// '8'
		{"Cry",			0x0a,-1},	// '9'
		{"Dance",		0x0b,-1},	// '0'

		{"ScreenShot",	0x58,-1},	// 'F12'
		{"Record",		0x57,-1},	// 'F11'
		{"MovingPicture",0x44,-1},	// 'F10'
		{"Defence",		0x2a,-1},	// 'shift'
		{"ToggleChat",	0x2f,-1},	// 'v'

		{"MouseSensitivityDec", 0x1A, -1},		//'['
		{"MouseSensitivityInc", 0x1B, -1}		//']'

	};

	// Define된 ID와 Description의 개수가 정확히 맞아야 한다.
	//_ASSERT(ZACTION_COUNT==sizeof(DefaultActionKeys)/sizeof(ZACTIONKEYDESCRIPTION));

	memcpy(m_Keyboard.ActionKeys, DefaultActionKeys, sizeof(ZACTIONKEYDESCRIPTION)*ZACTION_COUNT);
}

ZSERVERNODE ZConfiguration::GetServerNode( int nNum)
{
	ZSERVERNODE ServerNode;

	map<int,ZSERVERNODE>::iterator iterator;

	iterator = m_ServerList.find( nNum);
	if ( iterator != m_ServerList.end())
	{
		ServerNode = (*iterator).second;
	}

	return ServerNode;
}


const bool ZConfiguration::LateStringConvert()
{
	char buf[ZCONFIG_MACRO_MAX][256];

	for (int i = 0; i < ZCONFIG_MACRO_MAX; i++)
	{
		memset( buf[i], 0, 256 );
		strcpy( buf[i], m_Macro.szMacro[i] );
		memset( m_Macro.szMacro[i],0, 256 );
		strcpy(m_Macro.szMacro[i], ZGetStringResManager()->GetStringFromXml(buf[i]));
	}

	return true;
}

unsigned int ZConfiguration::GetSelectedLanguageIndex()
{
	size_t size = m_Locale.vecSelectableLanguage.size();
	for (unsigned int i=0; i<size; ++i)
	{
		if (m_Locale.vecSelectableLanguage[i].strLanguage == m_Etc.szLanguage)
			return i;
	}
	return -1;
}

void ZConfiguration::ValidateSelectedLanguage()
{
	// 현재 선택된 언어가 선택가능한 언어 목록에 존재하는지 확인한다. 아니라면 디폴트 언어로 세팅한다
	if (-1 == GetSelectedLanguageIndex())
	{
#ifdef _MULTILANGUAGE
		if (GetSelectedLanguage()[0] != 0) //_ASSERT(0);	// 빈문자열이면 괜찮지만, 뭔가 들어있는데 목록에 없다면 문제있음
#endif //_MULTILANGUAGE

		// 디폴트언어로 세팅
		strcpy(m_Etc.szLanguage, m_Locale.strDefaultLanguage.c_str());
	}
}

void ZConfiguration::SetSelectedLanguageIndex( unsigned int i )
{
	if (i >= m_Locale.vecSelectableLanguage.size())
	{
		//_ASSERT(0);
		return;
	}

	strcpy(m_Etc.szLanguage, m_Locale.vecSelectableLanguage[i].strLanguage.c_str());
}

int ZConfiguration::ValidateMouseSensitivityInInt( int i )
{
	if (i < MOUSE_SENSITIVITY_MIN)
		i = MOUSE_SENSITIVITY_MIN;

	else if (i > MOUSE_SENSITIVITY_MAX)
		i = MOUSE_SENSITIVITY_MAX;

	return i;
}

float ZConfiguration::ValidateMouseSensitivityInFloat( float f )
{
	return ValidateMouseSensitivityInInt(f*MOUSE_SENSITIVITY_MAX) / float(MOUSE_SENSITIVITY_MAX);
}

float ZConfiguration::GetMouseSensitivityInFloat()
{
	return Z_MOUSE_SENSITIVITY;
}

int ZConfiguration::GetMouseSensitivityInInt()
{
	return (int(ZGetConfiguration()->GetMouse()->fSensitivity * MOUSE_SENSITIVITY_MAX));
}

float ZConfiguration::SetMouseSensitivityInFloat(float f)
{
	float validated = ValidateMouseSensitivityInFloat(f);
	
	Z_MOUSE_SENSITIVITY = validated;
	return validated;
}

int ZConfiguration::SetMouseSensitivityInInt(int i)
{
	int validated = ValidateMouseSensitivityInInt(i);

	Z_MOUSE_SENSITIVITY = validated / (float)MOUSE_SENSITIVITY_MAX;
	return validated;
}