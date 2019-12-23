#ifndef _MDUELTOURNAMENT_TICKERPICKER_H
#define _MDUELTOURNAMENT_TICKERPICKER_H

class MDuelTournamentTicket;
class MDuelTournamentTicketPicker
{
protected:
	list <MDuelTournamentTicket *> m_DuelTournamentTicketList;
public:
	void AddTicket(MUID &uidPlayer, int nTP, int nLevel, int nWins, int nLoses, int nTick);
	void Shuffle();
};

#endif