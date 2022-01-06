#ifndef _RERROR_H
#define _RERROR_H

#include "RTypes.h"

enum RERROR
{
	ROK = 0,

	RERROR_CANNOT_CREATE_D3D = 1000,
	RERROR_INVALID_DEVICE = 1001,

	RERROR_CLOTH_BONE_MISSING = 2000,
	RERROR_CLOTH_PHYSIQUE_MISSING = 2001,
};

void RSetError(int nErrCode);
int RGetLastError(void);

#endif