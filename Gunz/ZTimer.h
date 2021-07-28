#ifndef _ZTIMER_H
#define _ZTIMER_H

#include <list>
using namespace std;

typedef void(ZGameTimerEventCallback)(void* pParam);
class ZTimerEvent;

class ZTimer
{
private:
	bool						m_bInitialized;

	unsigned long int			m_nLastTime;
	unsigned long int			m_nNowTime;
	list<ZTimerEvent*>			m_EventList;

	BOOL* m_pbUsingQPF;
	LONGLONG* m_pllQPFTicksPerSec;
	LONGLONG* m_pllLastElapsedTime;
	DWORD* m_pThistime;
	DWORD* m_pLasttime;
	DWORD* m_pElapsed;

	void UpdateEvents();

public:
	ZTimer();
	virtual ~ZTimer();

	float UpdateFrame();
	void ResetFrame();

	void SetTimerEvent(unsigned long int nElapsedTime, ZGameTimerEventCallback* fnTimerEventCallback, void* pParam, bool bTimerOnce = false);
	void ClearTimerEvent(ZGameTimerEventCallback* fnTimerEventCallback);

	unsigned long int GetNowTick() { return m_nNowTime; }
};

class ZUpdateTimer
{
private:
	float		m_fUpdateTime;
	float		m_fElapsedTime;
public:
	ZUpdateTimer(float fUpdateTime) : m_fUpdateTime(fUpdateTime), m_fElapsedTime(0.0f) { }
	ZUpdateTimer() : m_fUpdateTime(0.0f), m_fElapsedTime(0.0f) { }
	bool Update(float fDelta)
	{
		m_fElapsedTime += fDelta;
		if (m_fElapsedTime < m_fUpdateTime) return false;

		m_fElapsedTime = 0.0f;
		return true;
	}
	void Init(float fUpdateTime) { m_fUpdateTime = fUpdateTime; m_fElapsedTime = 0.0f; }
	void Force() { m_fElapsedTime = m_fUpdateTime; }
	void SetUpdateTime(float fUpdateTime) { m_fUpdateTime = fUpdateTime; }
	float GetUpdateTime() const { return m_fUpdateTime; }
};

#endif