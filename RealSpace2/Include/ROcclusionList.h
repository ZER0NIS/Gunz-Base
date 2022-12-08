#pragma	once

#include <string>
#include "RTypes.h"
#include "RNameSpace.h"

_NAMESPACE_REALSPACE2_BEGIN

class ROcclusion {
public:
	ROcclusion();
	~ROcclusion();

	inline void CalcPlane() { D3DXPlaneFromPoints(&plane, pVertices, pVertices + 1, pVertices + 2); }

	int nCount;
	rvector* pVertices;
	rplane* pPlanes;
	rplane plane;
	std::string	Name;
};

class ROcclusionList : public list<ROcclusion*> {
public:
	virtual ~ROcclusionList();
	bool Open(MXmlElement* pElement);
	bool Save(MXmlElement* pElement);

	void UpdateCamera(rmatrix& matWorld, rvector& cameraPos);
	bool IsVisible(rboundingbox& bb);
};

_NAMESPACE_REALSPACE2_END
