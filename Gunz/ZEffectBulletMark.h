#ifndef ZEFFECTBULLETMARK_H
#define ZEFFECTBULLETMARK_H

#include "ZEffectBillboard.h"

class ZEffectBulletMark : public ZEffectBillboard{
protected:
	unsigned long int m_nStartTime;
public:
	ZEffectBulletMark(ZEffectBillboardSource* pEffectBillboardSource, rvector& Pos, rvector& Normal);
	virtual ~ZEffectBulletMark(void);

	virtual bool Draw(unsigned long int nTime);
	virtual bool DrawTest(unsigned long int nTime);
};

#endif
