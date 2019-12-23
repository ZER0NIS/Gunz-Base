#pragma once

//////////////////////////////////////////////////////////////////////////
// MConnectHistory
// 접속 플러딩을 막기위해 일정시간, 일정개수 접속 기록을 보관하고 있다.

class MConnectHistory {
public:
	void Update( DWORD dwCurrentTime );
	bool IsExist( const string& strName );
	void Add( const string& strName, DWORD dwCurrentTime );

private:
	typedef map< string, DWORD > NAMETOTIMEMAP;

	NAMETOTIMEMAP m_mapNameToTime;
};