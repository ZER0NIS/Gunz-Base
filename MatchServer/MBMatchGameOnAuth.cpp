#include "stdafx.h"
#include "MDebug.h"
#include "MMatchGlobal.h"
#include "MBMatchGameOnAuth.h"
#include "MLocale.h"

MBMatchGameOnModule::MBMatchGameOnModule()
{
}


MBMatchGameOnModule::~MBMatchGameOnModule()
{
}


bool MBMatchGameOnModule::InitModule()
{
	if ( ! m_Gcc.Load_GamechuDLL( "GCCertification.dll"))
	{
		mlog( "GCCertification.dll not found!\n");
		return false;
	}

	mlog( "GCCertification.dll successfully loaded!\n");
	return true;
}


int MBMatchGameOnModule::CheckCertification( const wchar_t* wszSting, const wchar_t* wszStatIndex)
{
	gcBaseUserInfo myData;

	// return 0: success Certification
	// return other value : Error Certification 
	//          1 : md5 value error
	//         10 : TimeStamp over
	//        100 : Parsing Error
	//        101 : UserINfo Error
	//        102 : InputParam Error
	//       1000 : Unknown Error

#ifdef _DEBUG
	int ret = m_Gcc.CheckCertification( wszSting, wszStatIndex, L"!on@on#Gunz!", &myData, 0xFFFFFFFF);
	mlog( "%s : %d\n", __FUNCTION__, ret);
#else
	int ret = m_Gcc.CheckCertification( wszSting, wszStatIndex, L"!on@on#Gunz!", &myData, 0xFFFFFFFF);			// 여기 나중에 수정 요망 : 0xFFFFFFFF -> 600
#endif

	if ( ret == 0)
	{
		char szBuff[ 64];
		WideCharToMultiByte( CP_UTF8, 0, myData.szUserSerialNumber, -1, szBuff, 64, 0, 0);
		m_strUserSerialNum = szBuff;

		WideCharToMultiByte( CP_UTF8, 0, myData.szNickName, -1, szBuff, 64, 0, 0);
		m_strNickName = szBuff;
		m_nPCBang = myData.nPCBang;
		m_nSex = myData.nSex;
		m_nAge = myData.nAge;
		m_nLocation = myData.nLocation;
		m_nBirthYear = myData.nBirthYear;
	}

	return ret;
}


MBMatchGameOnModule& GetGameOnModule() 
{
	return MBMatchGameOnModule::GetInstance();
}

