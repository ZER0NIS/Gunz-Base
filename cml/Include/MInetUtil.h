#ifndef _MINETUTIL_H
#define _MINETUTIL_H



#include <string>

using std::string;



/// . 있는 IP 문자열을 . 없는 IP 문자열(12바이트)로 변환
void MConvertCompactIP(char* szOut, const char* szInputDottedIP);

void GetLocalIP( char* szOutIP, int nSize );

const bool MGetIPbyHostName( const string& strName, string& outIP );


#endif