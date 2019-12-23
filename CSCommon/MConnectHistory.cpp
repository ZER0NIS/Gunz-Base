#include "stdafx.h"
#include "MConnectHistory.h"

const int RECONNECT_TIME		= 5*1000;	///< 재접 시간은 최소 5초

void MConnectHistory::Update( DWORD dwCurrentTime )
{
	for( NAMETOTIMEMAP::iterator itr = m_mapNameToTime.begin(); itr!= m_mapNameToTime.end(); )
	{
		// 보관시간을 지나면 지운다
		if( RECONNECT_TIME < (dwCurrentTime - itr->second) )
		{
			itr = m_mapNameToTime.erase( itr );
		}
		else
			++itr;
	}
}

bool MConnectHistory::IsExist( const string& strName )
{
	NAMETOTIMEMAP::iterator itr = m_mapNameToTime.find(strName);
	return ( itr!=m_mapNameToTime.end() );
}

void MConnectHistory::Add( const string& strName, DWORD dwCurrentTime )
{
	m_mapNameToTime.insert( NAMETOTIMEMAP::value_type(strName, dwCurrentTime) );
}

