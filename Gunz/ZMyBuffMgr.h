#ifndef _ZMYBUFFMGR_H
#define _ZMYBUFFMGR_H

#include "ZMyBuff.h"

class ZMyBuffMgr
{
protected:
	ZMyShortBuffMap m_ShortBuffMap;

	ZMyBuffSummary	m_BuffSummary;

	bool InsertShortBuffInfo(MShortBuffInfo *pShortInfo);

public:
	ZMyBuffMgr(void);
	~ZMyBuffMgr(void);

//버프정보임시주석 	void Set(MTD_CharBuffInfo* pCharBuffInfo);
	void Clear();		
};

#endif