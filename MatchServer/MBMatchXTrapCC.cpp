#include "stdafx.h"
#include "MDebug.h"
#include "MBMatchServer.h"
#include "MBMatchXTrapCC.h"

MBMatchXTrapCC::MBMatchXTrapCC(MUID uId, const DWORD dwCurTime) : MBMatchUserSecurityInfo(dwCurTime)
{
	memset(m_sServerBuf, 0, sizeof(m_sServerBuf));
	memset(m_sComBuf, 0, sizeof(m_sComBuf));
	m_UID = uId;
	m_dwLastUpdateTime = dwCurTime;
}

MBMatchXTrapCC::~MBMatchXTrapCC()
{
}

DWORD MBMatchXTrapCC::XTrapSessionInit()
{
	return XTrap_S_SessionInit(XTRAP_CS4_MAXTIMEOUT, XTRAP_CS4_FILE_COUNT, sXTrapCS_Buf, m_sServerBuf);
}

DWORD MBMatchXTrapCC::XTrapGetHashData()
{
	DWORD retVal = XTrap_CS_Step1(m_sServerBuf, m_sComBuf);
//	if (retVal != XTRAP_CC_RETURN_OK)
//	{
//		MMatchObject *pObj = MGetMatchServer()->GetObject(m_UID);
//		if (pObj == NULL)
//		{
//			LOG(LOG_PROG, "XTRAP ERROR=[XTrap_CS_Step1()] retVal:[%d] , pObj == NULL\n", retVal);
//			return retVal;
//		}
//		LOG(LOG_PROG, "XTRAP ERROR=[XTrap_CS_Step1()] retVal:[%d] , AID:[%d]\n", retVal, pObj->GetAccountInfo()->m_nAID);
//	}
	return retVal;
}

DWORD MBMatchXTrapCC::XTrapCheckHashData(unsigned char *pComBuf)
{
	DWORD retVal = XTrap_CS_Step3(m_sServerBuf, pComBuf);
//	if (retVal != XTRAP_CC_RETURN_OK)
//	{
//		MMatchObject *pObj = MGetMatchServer()->GetObject(m_UID);
//		if (pObj == NULL)
//		{
//			LOG(LOG_PROG, "XTRAP ERROR=[XTrap_CS_Step3()] retVal:[%d] , pObj == NULL\n", retVal);
//			return retVal;
//		}
//		LOG(LOG_PROG, "XTRAP ERROR=[XTrap_CS_Step3()] retVal:[%d] , AID:[%d]\n", retVal, pObj->GetAccountInfo()->m_nAID);
//	}
	return retVal;
}

bool LoadXTrapFile()
{
	DWORD	dwRet = 0;
	FILE	*fi;

	fi = fopen("Gunz_map1.cs3", "rb");
	if (fi == NULL)
	{
		mlog("Gunz_map1.cs3 Load failed!\n");
		return false;
	}

	fread(sXTrapCS_Buf[0], XTRAP_CS4_BUFSIZE_MAP, 1, fi);
	fclose(fi);

	fi = fopen("Gunz_map2.cs3", "rb");
	if (fi == NULL)
	{
		mlog("Gunz_map2.cs3 Load failed!\n");
		return false;
	}

	fread(sXTrapCS_Buf[1], XTRAP_CS4_BUFSIZE_MAP, 1, fi);
	fclose(fi);

	if (XTrap_S_LoadDll())
	{
		mlog("XtrapCC.dll Load failed!\n");
		return false;
	}

	int retVal = XTrap_S_Start(XTRAP_CS4_MAXTIMEOUT, 2, sXTrapCS_Buf, NULL);
	if (retVal != 0)
	{
		mlog("XTrap_S_Start() => [%d]\n", retVal);
		return false;
	}
	return true;
}

bool ReloadXTrapMapFile()
{
#ifdef _DEBUG
	memset(sXTrapCS_Buf[0], 0, XTRAP_CS4_BUFSIZE_MAP);
	memset(sXTrapCS_Buf[1], 0, XTRAP_CS4_BUFSIZE_MAP);

	DWORD	dwRet = 0;
	FILE	*fi;

	fi = fopen("Gunz_map1.cs3", "rb");
	if (fi == NULL)
	{
		mlog("Gunz_map1.cs3 Reload failed!\n");
		return false;
	}

	fread(sXTrapCS_Buf[0], XTRAP_CS4_BUFSIZE_MAP, 1, fi);
	fclose(fi);

	fi = fopen("Gunz_map2.cs3", "rb");
	if (fi == NULL)
	{
		mlog("Gunz_map2.cs3 Reload failed!\n");
		return false;
	}

	fread(sXTrapCS_Buf[1], XTRAP_CS4_BUFSIZE_MAP, 1, fi);
	fclose(fi);
#endif
	return true;
}