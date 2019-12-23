#ifndef MMATCHOBJECTCHARBUFF_H
#define MMATCHOBJECTCHARBUFF_H

#include "MMatchBuff.h"
#include "MMatchBuffSummary.h"

class MMatchObject;
class MMatchObjectCharBuff
{
protected:
	static MUID				m_uidGenerate;
	static MCriticalSection	m_csUIDGenerateLock;

public:
	static MUID UseUID() 
	{
		m_csUIDGenerateLock.Lock();
		{
			m_uidGenerate.Increase();	
		}		
		m_csUIDGenerateLock.Unlock();
		return m_uidGenerate;
	}

protected:
	MMatchObject* m_pObj;
	unsigned int m_nLastCheckBuffInfoTime;

public:
	void SetObject(MMatchObject* pObj)					{ m_pObj = pObj; }
	MMatchObject* GetObject()							{ return m_pObj; }

	unsigned int GetLastCheckBuffInfoTime()				{ return m_nLastCheckBuffInfoTime; }
	void SetLastCheckBuffInfoTime(unsigned int nVal)	{ m_nLastCheckBuffInfoTime = nVal; }

protected:
	MMatchShortBuffMap m_ShortBuffInfoMap;

	MMatchBuffSummary m_BuffSummary;

public:
	bool ApplyShortBuffInfo(int nBuffID, int nBuffSecondPeriod);
	void DeleteShortBuffInfo(MUID& uidBuff);	

public:
	MMatchObjectCharBuff();
	~MMatchObjectCharBuff();	
	
	void FreeCharBuffInfo();

	bool Tick(int nGlobalTick);	

	void MakeBuffSummary();
	MMatchBuffSummary*	GetBuffSummary()			{ return &m_BuffSummary;	 }
	MMatchShortBuffMap* GetShortBuffInfoMap()		{ return &m_ShortBuffInfoMap;}
};


#endif