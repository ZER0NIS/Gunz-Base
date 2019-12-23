#ifndef _MDUELTOURNAMENT_GROUP_H
#define _MDUELTOURNAMENT_GROUP_H

#include "MDuelTournamentTicket.h"

#include <list>
#include <map>
using namespace std;

#include "MUID.h"
#include "MMatchGlobal.h"

class MDuelTournamentWaitGroup
{
protected:
	MDUELTOURNAMENTTYPE		m_nType;
	map<MUID, MDuelTournamentTicket*> m_PlayerTicketMap;

	unsigned int m_nRegTime;
	int m_nTickCount;

public:
	MDuelTournamentWaitGroup(MDUELTOURNAMENTTYPE nType, unsigned int nRegTime)
	{ 
		m_nType = nType;
		m_nRegTime = nRegTime;

		m_nTickCount = 0;
	}

	MDUELTOURNAMENTTYPE GetDuelTournamentType()				{ return m_nType; }
	void SetDuelTournamentType(MDUELTOURNAMENTTYPE nType)		{ m_nType = nType; }

	unsigned int GetRegTime()		{ return m_nRegTime; }	
	int GetTickCount()				{ return m_nTickCount; }

	size_t GetPlayerTicketCount()							{ return m_PlayerTicketMap.size(); }
	map<MUID, MDuelTournamentTicket*>* GetPlayerTicketMap()	{ return &m_PlayerTicketMap; }

	void AddPlayerTicket(MUID uidPlayer, MDuelTournamentTicket *pTicket) 
	{
		m_PlayerTicketMap.insert(pair<MUID, MDuelTournamentTicket*>(uidPlayer, pTicket));
	}

	MDuelTournamentTicket* FindPlayerTicket(MUID uidPlayer) 
	{
		map<MUID, MDuelTournamentTicket*>::iterator iter = m_PlayerTicketMap.find(uidPlayer);
		if( iter == m_PlayerTicketMap.end() )
			return NULL;

		return (*iter).second;
	}

	void RemovePlayerTicket(MUID uidPlayer) 
	{
		map<MUID, MDuelTournamentTicket*>::iterator iter = m_PlayerTicketMap.find(uidPlayer);
		if( iter == m_PlayerTicketMap.end() ) 
			return;

		m_PlayerTicketMap.erase(iter);
	}

	void UpdateTick() { 
		m_nTickCount++; 
	}
};


class MDuelTournamentPickedGroup : public vector<MUID>
{
public:
	void Shuffle() {
		int numElem = (int)size();
		int k = 0;
		MUID temp;
		for (int i=0; i<numElem; ++i) {
			k = rand() % numElem;
			temp = (*this)[i];
			(*this)[i] = (*this)[k];
			(*this)[k] = temp;
		}
	}
};

#endif