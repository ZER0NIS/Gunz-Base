#ifndef _ZCHARACTERBUFF_H
#define _ZCHARACTERBUFF_H

class ZShortBuff : public MMatchShortBuff{};
class ZBuffSummary : public MMatchBuffSummary{};

class ZCharacterBuff
{
protected:
public:
	ZCharacterBuff(void);
	~ZCharacterBuff(void){}

protected:	
	ZShortBuff m_ShortBuff[MAX_CHARACTER_SHORT_BUFF_COUNT];

	ZBuffSummary m_BuffSummary;

public:
	void Clear();

	bool SetShortBuff(int nIndex, MUID& uidBuff, int nBuffID, int nRegTime, int nPeriodRemainder);	
	ZShortBuff* GetShortBuff(int nIndex);
	ZBuffSummary* GetBuffSummary() { return &m_BuffSummary; }
};

#endif
