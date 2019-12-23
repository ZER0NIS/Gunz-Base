#include "stdafx.h"
#include "MUtil.h"
#include "MDebug.h"
#include <ctime>


#ifdef WIN32
#include <Windows.h>
#include <Unknwn.h>

void D3DDeleter::operator()(IUnknown* ptr) const {
	ptr->Release();
}
#endif


std::string MGetStrLocalTime(unsigned short wYear, unsigned short wMon, unsigned short wDay,
	unsigned short wHour, unsigned short wMin, MDateType DateType)
{
	const char* FormatString = [&]() -> const char* {
		switch (DateType)
		{
		case MDT_Y:
			return "%Y";
		case MDT_YM:
			return "%Y-%m";
		case MDT_YMD:
			return "%Y-%m-%d";
		case MDT_YMDH:
			return "%Y-%m-%d %H";
		case MDT_YMDHM:
			return "%Y-%m-%d %H-%M";
		default:
			return nullptr;
		}
	}();
	if (!FormatString)
		return{};

	auto t = time(nullptr);
	tm TM;
#ifdef _MSC_VER
	auto ret = localtime_s(&TM, &t);
#else
	auto ret = localtime_r(&t, &TM);
#endif
	if (ret != 0)
	{
		MLog("MGetStrLocalTime -- localtime_s failed with error code %d\n", ret);
		return{};
	}

	char buf[128];
	auto size = strftime(buf, sizeof(buf), FormatString, &TM);

	return{ buf, size };
}

bool SplitStrIP( const std::string& strIP, std::vector<BYTE>& vIP )
{
	if( strIP.empty() || (7 > strIP.length()) ) 
		return false;

	size_t a, b, c;
	char szPos1[ 4 ] = {0,};
	char szPos2[ 4 ] = {0,};
	char szPos3[ 4 ] = {0,};
	char szPos4[ 4 ] = {0,};

	a = strIP.find( "." );
	if(std::string::npos == a )
		return false;

	b = strIP.find( ".", a + 1 );
	if(std::string::npos == b )
		return false;

	c = strIP.find( ".", b + 1 );
	if(std::string::npos == c )
		return false;

	strncpy_safe( szPos1, &strIP[0], a );
	strncpy_safe( szPos2, &strIP[a + 1], b - a - 1 );
	strncpy_safe( szPos3, &strIP[b + 1], c - b - 1 );
	strncpy_safe( szPos4, &strIP[c + 1], strIP.length() - c - 1 );

	vIP.push_back( static_cast<BYTE>(atoi(szPos1)) );
	vIP.push_back( static_cast<BYTE>(atoi(szPos2)) );
	vIP.push_back( static_cast<BYTE>(atoi(szPos3)) );
	vIP.push_back( static_cast<BYTE>(atoi(szPos4)) );

	return true;
}