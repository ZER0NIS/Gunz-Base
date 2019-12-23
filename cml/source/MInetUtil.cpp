#include "stdafx.h"
#include "Winsock2.h"
#include "MInetUtil.h"
#include <iostream>
#include <string>
#include <crtdbg.h>


using std::string;

void MConvertCompactIP(char* szOut, const char* szInputDottedIP)
{
	in_addr addr;
	addr.S_un.S_addr = inet_addr(szInputDottedIP);
	sprintf(szOut, "%03u%03u%03u%03u", addr.S_un.S_un_b.s_b1, addr.S_un.S_un_b.s_b2, 
											addr.S_un.S_un_b.s_b3, addr.S_un.S_un_b.s_b4);
}



void GetLocalIP( char* szOutIP, int nSize )
{
	if( (0 == szOutIP) || (0 >= nSize) )
		return;

    unsigned int optval = 1 ;

    string ip;
    char szHostName[256];
    PHOSTENT pHostInfo;

    if( gethostname(szHostName,sizeof(szHostName)) ==0)
    {
        if((pHostInfo = gethostbyname(szHostName)) != NULL)
        {
            ip = inet_ntoa(*(struct in_addr *)*pHostInfo->h_addr_list); 
			if( ip.length() <= nSize )
				strncpy( szOutIP, ip.c_str(), ip.length() );
        }
    }
}


const bool MGetIPbyHostName( const string& strName, string& outIP )
{
	_ASSERT( 0 == outIP.length() );
	_ASSERT( 0 < strName.length() );
	_ASSERT( isalpha(strName[0]) );

	if( 0 == strName.length() )
	{
		return false;
	}

	if( !isalpha(strName[0]) )
	{
		outIP = strName;
		return true;
	}

	// WSAStartup을 먼저 해줘야 한다.
	WSADATA wsaData;
	if( 0 != WSAStartup(MAKEWORD(2, 2), &wsaData) )
	{
		return false;
	}

	struct hostent* pRemoteHost;
	pRemoteHost = gethostbyname( strName.c_str() );
	if( NULL == pRemoteHost )
	{
		return false;
	}

	in_addr addr;
	addr.s_addr = *(u_long *) pRemoteHost->h_addr_list[0];

	outIP.clear();
	outIP = string( inet_ntoa(addr) );

	return true;
}