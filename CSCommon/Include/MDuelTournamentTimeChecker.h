#pragma once

class MDuelTournamentTimeChecker
{
protected:
	bool m_bTimeStampChanged;
	CTime m_CurrTime;
	int m_nCurrMonth;
	int m_nCurrDay;

	char m_szTimeStamp[DUELTOURNAMENT_TIMESTAMP_MAX_LENGTH + 1];
public:
	MDuelTournamentTimeChecker(void);
	~MDuelTournamentTimeChecker(void){}

	void Tick(unsigned int nTick);

	void SetTimeStamp(const char* szTimeStamp);	
	bool IsSameTimeStamp(const char* szTimeStamp);

	char* GetTimeStamp() { return m_szTimeStamp; }

	bool GetTimeStampChanged()				{ return m_bTimeStampChanged; }
	void SetTimeStampChanged(bool bValue)	{ m_bTimeStampChanged = bValue; }
};
