#ifndef _MMATCHKILLTRACKER_H
#define _MMATCHKILLTRACKER_H


class MMatchKillTrackerConfig
{
private :
	friend class MMatchConfig;

	bool	m_IsUseKillTracker;
	DWORD	m_dwMaxKillCountOnTraceTime;
	DWORD	m_dwKillCountTraceTime;
	DWORD	m_dwKillTrackerUpdateTime;
	DWORD	m_dwTrackerInfoLifeTime;

private :
	void UseKillTracker() { m_IsUseKillTracker = true; }
	void SetMaxKillCountOnTraceTime( const DWORD dwCount ) { m_dwMaxKillCountOnTraceTime = dwCount; }

	void SetKillCountTraceTime( const DWORD dwTime ) 
	{ 
		m_dwKillCountTraceTime = dwTime * 60000; 
		m_dwKillTrackerUpdateTime = m_dwKillCountTraceTime / 2;
		m_dwTrackerInfoLifeTime = m_dwKillCountTraceTime * 2;
	}

public :
	MMatchKillTrackerConfig()
	{
		m_IsUseKillTracker				= false;
		m_dwMaxKillCountOnTraceTime		= 0;
		m_dwKillCountTraceTime			= 0;
		m_dwKillTrackerUpdateTime		= 0;
	}

	bool	IsUseKillTracker() const				{ return m_IsUseKillTracker; }
	DWORD	GetKillTrackerUpdateTime() const		{ return m_dwKillTrackerUpdateTime; }
	DWORD	GetMaxKillCountOnTraceTime() const		{ return m_dwMaxKillCountOnTraceTime; }
	DWORD	GetTrackerInfoLifeTime() const			{ return m_dwTrackerInfoLifeTime; }
	DWORD	GetKillCountTraceTime() const			{ return m_dwKillCountTraceTime; }
};


#endif