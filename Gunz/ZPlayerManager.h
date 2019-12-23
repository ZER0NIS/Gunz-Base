#pragma once

#include <map>
#include <string>
#include "MUID.h"

using namespace std;



class ZPlayerInfo
{
private:
	char	m_szName[ 32];
	int		m_nRank;
	int		m_nKill;
	int		m_nDeath;
	float	m_fWinningRatio;


public:
	ZPlayerInfo()		{ m_szName[ 0] = m_nRank = m_nKill = m_nDeath = m_fWinningRatio = 0;		}
	ZPlayerInfo( const char* name, int rank, int kill, int death)
	{
		strcpy( m_szName, name);
		m_nRank = rank;
		m_nKill = kill;
		m_nDeath = death;
		m_fWinningRatio = (kill <= 0)  ?  0.0f : ( (float)kill / (float)( kill + death) * 100.0f);
	}

	const char* GetName()		{ return m_szName;			}
	int GetRank()				{ return m_nRank;			}
	int GetKill()				{ return m_nKill;			}
	int GetDeath()				{ return m_nDeath;			}
	float GetWinningRatio()		{ return m_fWinningRatio;	}
};




class ZPlayerManager
{
private :
	map<MUID,ZPlayerInfo*>		m_PlayerList;


public:
	ZPlayerManager();
	~ZPlayerManager();


	static ZPlayerManager* GetInstance();

	void AddPlayer( MUID& uID, ZPlayerInfo* pInfo);
	void AddPlayer( MUID& uID, const char* name, int rank, int kill, int death);
	void RemovePlayer( MUID& uID);
	void Clear();
	ZPlayerInfo* Find( MUID& uID);
};



/// 인스턴스를 구함
inline ZPlayerManager* ZGetPlayerManager()
{
	return ZPlayerManager::GetInstance();
}
