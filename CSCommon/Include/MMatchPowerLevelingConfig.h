#ifndef _MMATCHPOWERLEVELING_H
#define _MMATCHPOWERLEVELING_H


class MMatchPowerLevelingConfig
{
private :
	friend class MMatchConfig;

	bool	m_IsUsePowerLevelingDBBlock;
	DWORD	m_dwPowerLevelingDBBlockTime;

private :
	void UsePowerLevelingDBBlock() { m_IsUsePowerLevelingDBBlock = true; }
	void SetPowerLevelingDBBlockTime( const DWORD dwTime ) { m_dwPowerLevelingDBBlockTime = dwTime * 60000; }


public :
	MMatchPowerLevelingConfig()
	{
		m_IsUsePowerLevelingDBBlock		= false;
		m_dwPowerLevelingDBBlockTime	= 0;
	}

	bool IsUsePowerLevelingDBBlock() const		{ return m_IsUsePowerLevelingDBBlock; }
	DWORD GetPowerLevelingDBBlockTime() const	{ return m_dwPowerLevelingDBBlockTime; }
};



#endif