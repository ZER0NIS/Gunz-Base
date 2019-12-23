#ifndef _RERROR_H
#define _RERROR_H

/// error def

#include "RTypes.h"

enum RERROR
{
	ROK = 0,

	// d3d관련
	RERROR_CANNOT_CREATE_D3D	= 1000,
	RERROR_INVALID_DEVICE		= 1001,
	
	// 기타
	RERROR_CLOTH_BONE_MISSING	= 2000,	// RCharCloth init할때 본이 없는 경우 (실수로 메시에 vertexcolor가 들어가서 clothmesh로 간주되는 경우가 있음)
	RERROR_CLOTH_PHYSIQUE_MISSING = 2001, // RCharCloth init할때 피직이 없는 메시를 갖는 경우
	
};





//-------------------------------------------------------
void RSetError(int nErrCode);
int RGetLastError(void);


#endif