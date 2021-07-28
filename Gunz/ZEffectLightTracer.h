#ifndef ZEFFECTLIGHTTRACER_H
#define ZEFFECTLIGHTTRACER_H

#include "ZEffectBillboard.h"

#include "mempool.h"

class ZEffectLightTracer : public ZEffectBillboard, public CMemPoolSm<ZEffectLightTracer>
{
protected:
	unsigned long int m_nStartTime;

	rvector	m_LightTracerDir;
	rvector	m_Start, m_End;
	float		m_fLength;

public:
	ZEffectLightTracer(ZEffectBillboardSource* pEffectBillboardSource, rvector& Start, rvector& End);
	virtual ~ZEffectLightTracer(void);

	virtual bool Draw(unsigned long int nTime);
};

#endif