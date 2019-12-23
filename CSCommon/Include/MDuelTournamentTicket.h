#ifndef _MDUELTOURNAMENT_TICKER_H
#define _MDUELTOURNAMENT_TICKER_H

class MDuelTournamentTicket
{
protected:
	MUID m_uidPlayer;

	int	m_nTP;
	int m_nLevel;
	int	m_nWins;
	int m_nLoses;

	unsigned long m_nTickCount;
public:
	MDuelTournamentTicket(MUID &uidPlayer, int nTP, int nLevel, int nWins, int nLoses) {
		m_uidPlayer = uidPlayer;

		m_nTP = nTP;
		m_nLevel = nLevel;
		m_nWins = nWins;
		m_nLoses = nLoses;

		m_nTickCount = 0;
	}

	MDuelTournamentTicket(MUID &uidPlayer, int nTP, int nLevel, int nWins, int nLoses, int nTick) {
		m_uidPlayer = uidPlayer;

		m_nTP = nTP;
		m_nLevel = nLevel;
		m_nWins = nWins;
		m_nLoses = nLoses;

		m_nTickCount = nTick;
	}

	MUID GetPlayerUID()		{ return m_uidPlayer; }
	int GetTP()				{ return m_nTP; }
	int GetLevel()			{ return m_nLevel; }
	int	GetWins()			{ return m_nWins; }
	int GetLoses()			{ return m_nLoses; }

	unsigned long GetTickCount() { return m_nTickCount; }		

	void UpdateTick()		{ m_nTickCount++; }
};

#endif