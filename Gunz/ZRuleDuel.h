#ifndef _ZRULE_DUEL_H
#define _ZRULE_DUEL_H

#include "ZRule.h"


class ZRuleDuel : public ZRule
{
public:
	MTD_DuelQueueInfo QInfo;

	ZRuleDuel(ZMatch* pMatch);
	virtual ~ZRuleDuel();

	virtual bool OnCommand(MCommand* pCommand);

	int	GetQueueIdx(const MUID& uidChar);
};

#endif