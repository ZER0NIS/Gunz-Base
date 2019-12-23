#include "stdafx.h"

#include "MDuelTournamentTicket.h"
#include "MDuelTournamentTicketPicker.h"

void MDuelTournamentTicketPicker::AddTicket(MDuelTournamentTicket *pTicket)
{
	m_TicketList.push_back(pTicket);
}

static bool CompareTicket(MDuelTournamentTicket* left, MDuelTournamentTicket* right) 
{
	return left->GetTP() < right->GetTP();
}

void MDuelTournamentTicketPicker::Shuffle()
{
	if (m_TicketList.empty()) return;
/*
#define TEMP_TICKET_LIST_COUNT	5
	list <MDuelTournamentTicket *> TempTicketList[TEMP_TICKET_LIST_COUNT];

	for(list <MDuelTournamentTicket *>::iterator iter = m_DuelTournamentTicketList.begin(), int i = 0;
		iter != m_DuelTournamentTicketList.end() ; ++iter, ++i ){
		TempTicketList[i % TEMP_TICKET_LIST_COUNT] = (*iter);
	}

	for(int i = 0; i < TEMP_TICKET_LIST_COUNT; i++)
	{
		TempTicketList[i].sort(CompareTicket);
	}
*/
	//m_TicketList.sort(CompareTicket);
}

bool MDuelTournamentTicketPicker::PickMatch(list<MDuelTournamentTicket*> *pTicketList, int nPlayerCount)
{
	if( nPlayerCount == 0 ) return false;

	if( (int)m_TicketList.size() < nPlayerCount ) {
		return false;
	}

	for(int i = 0; i < nPlayerCount; i++){
		pTicketList->push_back(m_TicketList.front());
		m_TicketList.pop_front();
	}

	return true;
}