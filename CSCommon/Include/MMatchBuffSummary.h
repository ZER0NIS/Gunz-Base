#ifndef _MMATCHBUFFSUMMARY_H
#define _MMATCHBUFFSUMMARY_H

#include "MMatchBuff.h"

class MMatchBuffSummary
{
protected:
	MMatchBuffInfo m_BuffSummary[MMBET_END];

	bool m_bCleared;
public:
	MMatchBuffSummary();
	~MMatchBuffSummary();

	void Clear();
	void AddBuff(int nBuffID);

	MMatchBuffInfo* GetBuffSummary(MMatchBuffEffectType nType) { return &m_BuffSummary[(int)nType]; }

	int GetHP();
	int GetAP();

	float GetSpeedRatio(float fDefault);

	int	GetRespawnTime(int nDefault);

	int GetDoteHP();
	int GetDoteAP();
};

#endif