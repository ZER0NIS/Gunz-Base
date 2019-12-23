#ifndef _MDUELTOURNAMENT_TICKERPICKER_H
#define _MDUELTOURNAMENT_TICKERPICKER_H

class MDuelTournamentTicket;
class MDuelTournamentTicketPicker
{
protected:
	list <MDuelTournamentTicket *> m_TicketList;
public:
	void AddTicket(MDuelTournamentTicket *pTicket);
	void Shuffle();

	bool PickMatch(list<MDuelTournamentTicket*> *pTicketList, int nPlayerCount);
};

#endif