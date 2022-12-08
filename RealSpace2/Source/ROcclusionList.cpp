#include "stdafx.h"
#include "MXml.h"
#include "ROcclusionList.h"
#include "RToken.h"
#include "RealSpace2.h"

_NAMESPACE_REALSPACE2_BEGIN

ROcclusion::ROcclusion()
{
	nCount = 0;
	pVertices = NULL;
	pPlanes = NULL;
}

ROcclusion::~ROcclusion()
{
	SAFE_DELETE(pVertices);
	SAFE_DELETE(pPlanes);
}

ROcclusionList::~ROcclusionList()
{
	for (iterator i = begin(); i != end(); i++)
		delete* i;
}

bool ROcclusionList::Open(MXmlElement* pElement)
{
	MXmlElement	aOcclusionNode, aChild;
	int nCount = pElement->GetChildNodeCount();

	char szTagName[256], szContents[256];
	for (int i = 0; i < nCount; i++)
	{
		aOcclusionNode = pElement->GetChildNode(i);
		aOcclusionNode.GetTagName(szTagName);

		if (stricmp(szTagName, RTOK_OCCLUSION) == 0)
		{
			ROcclusion* poc = new ROcclusion;
			aOcclusionNode.GetAttribute(szContents, RTOK_NAME);
			poc->Name = szContents;

			list<rvector> winding;

			int nChildCount = aOcclusionNode.GetChildNodeCount();
			int j;
			for (j = 0; j < nChildCount; j++)
			{
				aChild = aOcclusionNode.GetChildNode(j);
				aChild.GetTagName(szTagName);
				aChild.GetContents(szContents);

				auto ReadVector = [&](auto& v) {
					sscanf(szContents, "%f %f %f", &v.x, &v.y, &v.z);
				};

				if (stricmp(szTagName, RTOK_POSITION) == 0) {
					rvector temp;
					ReadVector(temp);
					winding.push_back(temp);
				}
			}

			poc->nCount = winding.size();
			poc->pVertices = new rvector[poc->nCount];
			list<rvector>::iterator k = winding.begin();
			for (j = 0; j < poc->nCount; j++)
			{
				poc->pVertices[j] = *k;
				k++;
			}

			push_back(poc);
		}
	}
	return true;
}

bool ROcclusionList::Save(MXmlElement* pElement)
{
	MXmlElement	aOcclusionListElement = pElement->CreateChildElement(RTOK_OCCLUSIONLIST);

	for (ROcclusionList::iterator i = begin(); i != end(); i++)
	{
		aOcclusionListElement.AppendText("\n\t\t");

		ROcclusion* poc = *i;
		char buffer[256];

		MXmlElement		aElement, aChild;
		aElement = aOcclusionListElement.CreateChildElement(RTOK_OCCLUSION);

		aElement.AddAttribute(RTOK_NAME, poc->Name.c_str());

		for (int j = 0; j < poc->nCount; j++)
		{
			aElement.AppendText("\n\t\t\t");

			aChild = aElement.CreateChildElement(RTOK_POSITION);
			aChild.SetContents(Format(buffer, poc->pVertices[j]));
		}

		aElement.AppendText("\n\t\t");
	}
	aOcclusionListElement.AppendText("\n\t");
	return true;
}

bool ROcclusionList::IsVisible(rboundingbox& bb)
{
	for (ROcclusionList::iterator i = begin(); i != end(); i++)
	{
		ROcclusion* poc = *i;

		bool bVisible = false;

		for (int j = 0; j < poc->nCount + 1; j++)
		{
			if (isInPlane(&bb, &poc->pPlanes[j]))
			{
				bVisible = true;
				break;
			}
		}

		if (!bVisible)
			return false;
	}
	return true;
}

void ROcclusionList::UpdateCamera(rmatrix& matWorld, rvector& cameraPos)
{
	float	fDet;
	rmatrix invWorld;
	D3DXMatrixInverse(&invWorld, &fDet, &matWorld);

	rvector localCameraPos;
	D3DXVec3TransformCoord(&localCameraPos, &cameraPos, &invWorld);

	rmatrix trInvMat;
	D3DXMatrixTranspose(&trInvMat, &invWorld);

	for (ROcclusionList::iterator i = begin(); i != end(); i++)
	{
		ROcclusion* poc = *i;

		bool bm_pPositive = D3DXPlaneDotCoord(&poc->plane, &localCameraPos) > 0;

		D3DXPlaneTransform(poc->pPlanes, poc->pPlanes, &trInvMat);

		poc->pPlanes[0] = bm_pPositive ? poc->plane : -poc->plane;
		for (int j = 0; j < poc->nCount; j++)
		{
			if (bm_pPositive)
				D3DXPlaneFromPoints(poc->pPlanes + j + 1, &poc->pVertices[j], &poc->pVertices[(j + 1) % poc->nCount], &localCameraPos);
			else
				D3DXPlaneFromPoints(poc->pPlanes + j + 1, &poc->pVertices[(j + 1) % poc->nCount], &poc->pVertices[j], &localCameraPos);

			D3DXPlaneTransform(poc->pPlanes + j + 1, poc->pPlanes + j + 1, &trInvMat);
		}
	}
}

_NAMESPACE_REALSPACE2_END