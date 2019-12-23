#pragma once


#include <vector>
#include <string>


using namespace std;


class MHackingChatList
{
public :
	MHackingChatList() {}
	~MHackingChatList() {}


	const bool IsHackChat( const char* pszChat, const size_t nLen )
	{
		if( 0 == strlen(pszChat) )
		{
			return false;
		}

		for( size_t i = 0; i < m_ChatList.size(); ++i )
		{
			if( 0 == strncmp(m_ChatList[i].c_str(), pszChat, nLen) )
			{
				return true;
			}
		}

		return false;
	}


	void AddHackChat( const char* szChat )
	{
		m_ChatList.push_back( string(szChat) );
	}


	const bool Init()
	{
		m_ChatList.clear();

		const char szChatListFile[] = "hackingchatlist.txt";

		FILE* fp = fopen( "hackingchatlist.txt", "r" );
		if( NULL == fp )
		{
			return false;
		}

		char szChat[ 1024 ] = {0,};
		while( 0 != fgets(szChat, 1024, fp) )
		{
			m_ChatList.push_back( string(szChat) );
			
			mlog( "add hacking chat : %s\n", szChat );
			if( !IsHackChat(szChat, strlen(szChat)) )
			{
				return false;
			}
			memset( szChat, 0, 1024 );
		}

		return true;
	}


private :
	vector<string> m_ChatList;
};