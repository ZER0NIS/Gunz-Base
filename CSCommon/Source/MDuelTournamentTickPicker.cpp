#include "stdafx.h"

#include "MDuelTournamentTicket.h"
#include "MDuelTournamentTickPicker.h"

void MDuelTournamentTicketPicker::AddTicket(MUID &uidPlayer, int nTP, int nLevel, int nWins, int nLoses, int nTick)
{
	m_DuelTournamentTicketList.push_back( new MDuelTournamentTicket(uidPlayer, nTP, nLevel, nWins, nLoses, nTick) );
}

static bool CompareTicket(MDuelTournamentTicket* left, MDuelTournamentTicket* right) 
{
	return left->GetTP() < right->GetTP();
}

void MDuelTournamentTicketPicker::Shuffle()
{
	if (m_DuelTournamentTicketList.empty()) return;
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
	m_DuelTournamentTicketList.sort(CompareTicket);
}
