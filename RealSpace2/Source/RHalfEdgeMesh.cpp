#include "stdafx.h"
#include "MDebug.h"
#include "RMeshUtil.h"
#include "RMesh.h"
#include "RealSpace2.h"
#include "RHalfEdgeMesh.h"

_NAMESPACE_REALSPACE2_BEGIN

#define DEFAULT_NUM_VALENCE	8

struct sHEVertex
{
	sHEEdge* pEdge;
	rvector		pos;
	rvector		normal;
	rvector		color;
	float		tu, tv;
	int			valence;

	vector<sHEEdge*>	ReverseMap;

	sHEVertex();

	sHEVertex	operator+(sHEVertex& arg_);
	sHEVertex	operator-(sHEVertex& arg_);
	void		operator+=(sHEVertex& arg_);
	void		operator-=(sHEVertex& arg_);
};

sHEVertex sHEVertex::operator+(sHEVertex& arg_)
{
	sHEVertex result;
	result.pos = this->pos + arg_.pos;
	result.normal = this->normal + arg_.normal;
	result.tu = this->tu + arg_.tu;
	result.tv = this->tv + arg_.tv;
	return result;
}

sHEVertex sHEVertex::operator-(sHEVertex& arg_)
{
	sHEVertex result;
	result.pos = this->pos - arg_.pos;
	result.normal = this->normal - arg_.normal;
	result.tu = this->tu - arg_.tu;
	result.tv = this->tv - arg_.tv;
	return result;
}

void	sHEVertex::operator+=(sHEVertex& arg_)
{
	this->pos += arg_.pos;
	this->normal += arg_.normal;
	this->tu += arg_.tu;
	this->tv += arg_.tv;
}

void	sHEVertex::operator-=(sHEVertex& arg_)
{
	this->pos -= arg_.pos;
	this->normal -= arg_.normal;
	this->tu -= arg_.tu;
	this->tv -= arg_.tv;
}

sHEVertex	operator*(sHEVertex arg1_, float arg2_)
{
	sHEVertex result;
	result.pos = arg1_.pos * arg2_;
	result.normal = arg1_.normal * arg2_;
	result.tu = arg1_.tu * arg2_;
	result.tv = arg1_.tv * arg2_;
	return result;
}

sHEVertex	operator*(float arg2_, sHEVertex arg1_)
{
	sHEVertex result;
	result.pos = arg1_.pos * arg2_;
	result.normal = arg1_.normal * arg2_;
	result.tu = arg1_.tu * arg2_;
	result.tv = arg1_.tv * arg2_;
	return result;
}

sHEVertex::sHEVertex()
{
	ReverseMap.reserve(DEFAULT_NUM_VALENCE);
	pEdge = 0;
	valence = 0;
	pos = rvector(0, 0, 0);
	normal = rvector(0, 0, 0);
	color = rvector(0, 0, 0);
	tu = 0;
	tv = 0;
}

struct sHEFace
{
	sHEEdge* pEdge;
	sHEFace();
};

sHEFace::sHEFace()
{
	pEdge = 0;
}

struct sHEEdge
{
	sHEVertex* pVertex;
	sHEFace* pFace;
	sHEEdge* pPrev;
	sHEEdge* pNext;
	sHEEdge* pPair;
	bool		bMark;

	sHEEdge();
};

sHEEdge::sHEEdge()
{
	pVertex = 0;
	pFace = 0;
	pPrev = 0;
	pNext = 0;
	pPair = 0;
	bMark = false;
}

void RHEMesh::SplitEdge(sHEEdge* pEdge_, sHEVertex* pVertex_, sHEEdge* nEdge_, sHEEdge* npEdge_)
{
	nEdge_->pVertex = pVertex_;
	nEdge_->pFace = pEdge_->pFace;
	nEdge_->pPrev = pEdge_;
	nEdge_->pNext = pEdge_->pNext;
	nEdge_->pPair = pEdge_->pPair;

	pEdge_->pNext->pPrev = nEdge_;
	pEdge_->pNext = nEdge_;

	nEdge_->bMark = TRUE;

	if (npEdge_ != NULL)
	{
		npEdge_->pVertex = pVertex_;
		npEdge_->pFace = pEdge_->pPair->pFace;
		npEdge_->pPrev = pEdge_->pPair;
		npEdge_->pNext = pEdge_->pPair->pNext;
		npEdge_->pPair = pEdge_;

		pEdge_->pPair->pNext->pPrev = npEdge_;
		pEdge_->pPair->pNext = npEdge_;

		pEdge_->pPair->pPair = nEdge_;
		pEdge_->pPair = npEdge_;

		npEdge_->bMark = TRUE;
	}

	pVertex_->pEdge = nEdge_;
}

void RHEMesh::AddEdge(sHEVertex* pVertex_, sHEFace* pFace_, sHEEdge* pPrev_, sHEEdge* pNext_, sHEEdge* pPair_, sHEEdge* nEdge_)
{
	nEdge_->pVertex = pVertex_;
	nEdge_->pFace = pFace_;
	nEdge_->pPrev = pPrev_;
	nEdge_->pNext = pNext_;
	nEdge_->pPair = pPair_;
}

sHEFace* RHEMesh::SplitFaceLocal(sHEFace* pFace_, sHEFace* nFace_, sHEEdge* nEdge1_, sHEEdge* nEdge2_)
{
	sHEEdge* pEdge1_prev = pFace_->pEdge;
	sHEEdge* pEdge1_next = pEdge1_prev->pPrev;
	sHEEdge* pEdge2_prev = pEdge1_next->pPrev;
	sHEEdge* pEdge2_next = pEdge1_prev->pNext;

	sHEVertex* pVertex1 = pEdge2_next->pVertex;
	sHEVertex* pVertex2 = pEdge1_next->pVertex;

	AddEdge(pVertex1, pFace_, pEdge1_prev, pEdge1_next, nEdge2_, nEdge1_);
	AddEdge(pVertex2, nFace_, pEdge2_prev, pEdge2_next, nEdge1_, nEdge2_);

	pEdge1_prev->pNext = nEdge1_;
	pEdge1_next->pPrev = nEdge1_;

	pEdge2_prev->pNext = nEdge2_;
	pEdge2_next->pPrev = nEdge2_;

	nFace_->pEdge = pEdge2_next->pNext;
	pEdge2_next->pNext->pFace = nFace_;

	if (!(pFace_->pEdge->pNext->pNext->pNext == pFace_->pEdge))
	{
		_ASSERT(!"");
		return NULL;
	}

	sHEEdge* peCurr = pFace_->pEdge;

	peCurr = peCurr->pNext;
	while (peCurr != pFace_->pEdge)
	{
		peCurr->pFace = pFace_;
		peCurr = peCurr->pNext;
	}

	if (nFace_->pEdge == nFace_->pEdge->pNext->pNext->pNext)
	{
		peCurr = nFace_->pEdge;
		peCurr = peCurr->pNext;
		while (peCurr != nFace_->pEdge)
		{
			peCurr->pFace = pFace_;
			peCurr = peCurr->pNext;
		}

		return NULL;
	}

	return nFace_;
}
RHEMesh::RHEMesh()
	: mVertices(0), mFaces(0), mEdges(0)
{
}

RHEMesh::~RHEMesh()
{
	SAFE_DELETE_ARRAY(mVertices);
	SAFE_DELETE_ARRAY(mFaces);
	SAFE_DELETE_ARRAY(mEdges);
}

void RHEMesh::GetSubdivisionMesh(RMeshNode* pMeshNode_, eSubdivisionType eType_, rvector* pPointList_)
{
	int			i, j, k;
	int			index;
	int			iVertex, iFace, iEdge;

	sHEFace* nFace;
	sHEVertex* nVertex;
	sHEEdge* nEdge;
	sHEEdge* peTemp;
	sHEEdge* peFirst;
	sHEVertex* pvTemp;
	sHEEdge* peDestTemp;
	sHEVertex* pvDestTemp;

	miNumVertices = pMeshNode_->m_point_num;
	miNumEdges = pMeshNode_->m_face_num * 3;
	miNumFaces = pMeshNode_->m_face_num;

	mVertices = new sHEVertex[miNumVertices];
	mEdges = new sHEEdge[miNumEdges];
	mFaces = new sHEFace[miNumFaces];

	memset(mVertices, 0, sizeof(sHEVertex) * miNumVertices);
	memset(mEdges, 0, sizeof(sHEEdge) * miNumEdges);
	memset(mFaces, 0, sizeof(sHEFace) * miNumFaces);

	iVertex = 0;
	for (i = 0; i < miNumVertices; ++i)
	{
		nVertex = &mVertices[i];

		if (pPointList_)
		{
			nVertex->pos = pPointList_[i];
		}
		else
		{
			nVertex->pos = pMeshNode_->m_point_list[i];
		}
		++iVertex;
	}

	iFace = 0;
	iEdge = 0;
	for (i = 0; i < miNumFaces; ++i)
	{
		nFace = &mFaces[i];

		peTemp = NULL;
		peFirst = NULL;

		for (j = 0; j < 3; ++j)
		{
			index = pMeshNode_->m_face_list[i].m_point_index[j];

			nEdge = &mEdges[iEdge];
			AddEdge(&mVertices[index], nFace, peTemp, NULL, NULL, nEdge);

			if (peTemp != NULL)
			{
				peTemp->pNext = nEdge;
			}
			else
			{
				peFirst = nEdge;
			}

			if (mVertices[index].pEdge == NULL)
			{
				mVertices[index].tu = pMeshNode_->m_face_list[i].m_point_tex[j].x;
				mVertices[index].tv = pMeshNode_->m_face_list[i].m_point_tex[j].y;
				mVertices[index].pEdge = nEdge;
			}

			if (nFace->pEdge == NULL)
			{
				nFace->pEdge = nEdge;
			}

			nEdge->pVertex = &mVertices[index];

			mVertices[index].ReverseMap.push_back(nEdge);

			peTemp = nEdge;

			++iEdge;

			mVertices[index].normal += pMeshNode_->m_face_normal_list[i].m_pointnormal[j];
		}

		peFirst->pPrev = nEdge;
		peTemp->pNext = peFirst;

		++iFace;
	}

	for (i = 0; i < miNumVertices; ++i)
	{
		D3DXVec3Normalize(&mVertices[index].normal, &mVertices[index].normal);
	}

	for (i = 0; i < miNumEdges; ++i)
	{
		peTemp = &mEdges[i];
		if (peTemp->pPair != NULL)
		{
			if (peTemp->pPair->pPair != peTemp)
			{
				_ASSERT(!"");
			}
			continue;
		}
		pvTemp = peTemp->pVertex;
		pvDestTemp = peTemp->pNext->pVertex;

		for (k = 0; k < (int)pvDestTemp->ReverseMap.size(); ++k)
		{
			peDestTemp = pvDestTemp->ReverseMap[k];
			if (peDestTemp->pNext->pVertex == pvTemp)
			{
				peTemp->pPair = peDestTemp;
				peDestTemp->pPair = peTemp;
				break;
			}
		}
	}

	for (i = 0; i < miNumVertices; ++i)
	{
		mVertices[i].ReverseMap.clear();
		mVertices[i].ReverseMap.resize(0);
	}
}

bool RHEMesh::Validate_HEMESH()
{
	int i;

	for (i = 0; i < miNumEdges; ++i)
	{
		sHEEdge* pCurr = &mEdges[i];
		if (!(pCurr == pCurr->pNext->pPrev))
		{
			_ASSERT(!"");
			return false;
		}
	}

	for (i = 0; i < miNumEdges; ++i)
	{
		sHEEdge* pCurr = &mEdges[i];
		if (!(pCurr == pCurr->pPrev->pNext))
		{
			_ASSERT(!"");
			return false;
		}
	}

	for (i = 0; i < miNumEdges; ++i)
	{
		sHEEdge* pCurr = &mEdges[i];
		if (pCurr->pPair == NULL)
		{
			continue;
		}
		if (!(pCurr == pCurr->pPair->pPair))
		{
			_ASSERT(!"");
			return false;
		}
	}

	for (i = 0; i < miNumEdges; ++i)
	{
		sHEEdge* pCurr = &mEdges[i];

		if (!(pCurr == pCurr->pNext->pNext->pNext))
		{
			_ASSERT(!"");
			return false;
		}
	}

	for (i = 0; i < miNumVertices; ++i)
	{
		sHEVertex* pCurr = &mVertices[i];

		if (!(pCurr == pCurr->pEdge->pVertex))
		{
			_ASSERT(!"");
			return false;
		}
	}

	for (i = 0; i < miNumFaces; ++i)
	{
		sHEFace* pCurr = &mFaces[i];

		if (!(pCurr == pCurr->pEdge->pFace))
		{
			_ASSERT(!"");
			return false;
		}
	}
	return true;
}

int	RSubdivisionMesh::GetValence(sHEVertex* pVertex_)
{
	bool reverse = false;

	int valence = 0;

	const sHEEdge* pEdge = pVertex_->pEdge;
	sHEEdge* pCurr = pVertex_->pEdge;
	do
	{
		++valence;
		if (pEdge->pPair != NULL)
		{
			if (reverse)
			{
				pEdge = pEdge->pPair->pPrev;
			}
			else
			{
				pEdge = pEdge->pPair->pNext;
			}
		}
		else
		{
			if (reverse)
			{
				break;
			}
			else
			{
				reverse = true;
				pEdge = pVertex_->pEdge->pPrev;
			}
		}
	} while (pEdge != pVertex_->pEdge);

	return valence;
}

void	RHEMesh::Draw()
{
	int							i, j;
	int							nVertex = 0;
	int							nPrimitive = 0;
	vector<sHEFace*>::iterator	itor;
	sHEFace* pfCurr;
	sHEEdge* peCurr;

	RVertex* pVertices = new RVertex[miNumFaces * 3];

	for (i = 0; i < miNumFaces; ++i)
	{
		pfCurr = &mFaces[i];
		peCurr = pfCurr->pEdge;
		for (j = 0; j < 3; ++j)
		{
			pVertices[nVertex].p = peCurr->pVertex->pos;
			pVertices[nVertex].n = peCurr->pVertex->normal;
			pVertices[nVertex].tu = peCurr->pVertex->tu;
			pVertices[nVertex].tv = peCurr->pVertex->tv;

			peCurr = peCurr->pNext;
			peCurr = peCurr->pPrev->pNext;

			if (peCurr->pPair != NULL)
			{
				peCurr = peCurr->pPair->pPair;
			}

			++nVertex;
		}
		++nPrimitive;
	}

	RGetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	RGetDevice()->SetFVF(RVertexType);
	RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, nPrimitive, pVertices, sizeof(RVertex));
	RGetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	delete[]	pVertices;
}

bool RSubdivisionMesh::Validate()
{
	int i;

	for (i = 0; i < miSubNumEdges; ++i)
	{
		sHEEdge* pCurr = &mSubDestEdge[i];
		if (!(pCurr == pCurr->pNext->pPrev))
		{
			_ASSERT(!"");
			return false;
		}
	}

	for (i = 0; i < miSubNumEdges; ++i)
	{
		sHEEdge* pCurr = &mSubDestEdge[i];
		if (!(pCurr == pCurr->pPrev->pNext))
		{
			_ASSERT(!"");
			return false;
		}
	}

	for (i = 0; i < miSubNumEdges; ++i)
	{
		sHEEdge* pCurr = &mSubDestEdge[i];
		if (pCurr->pPair == NULL)
		{
			continue;
		}
		if (!(pCurr == pCurr->pPair->pPair))
		{
			_ASSERT(!"");
			return false;
		}
	}

	for (i = 0; i < miSubNumVertices; ++i)
	{
		sHEVertex* pCurr = &mSubDestVertex[i];

		if (!(pCurr == pCurr->pEdge->pVertex))
		{
			_ASSERT(!"");
			return false;
		}
	}

	for (i = 0; i < miSubNumFaces; ++i)
	{
		sHEFace* pCurr = &mSubDestFace[i];

		if (!(pCurr == pCurr->pEdge->pFace))
		{
			_ASSERT(!"");
			return false;
		}
	}

	return true;
}

void RSubdivisionMesh::Subdivide(int subLevel_)
{
	--subLevel_;
	if (subLevel_ < 0)
	{
		return;
	}

	int i;

	if (mbIsFirstTime)
	{
		memset(mSubDestEdge, 0, sizeof(sHEEdge) * miSubEdgeSize);
		memset(mSubDestVertex, 0, sizeof(sHEVertex) * miSubVertexSize);
		memset(mSubDestFace, 0, sizeof(sHEFace) * miSubFaceSize);
		memset(mSubEdge, 0, sizeof(sHEEdge) * miSubEdgeSize);
		memset(mSubVertex, 0, sizeof(sHEVertex) * miSubVertexSize);
		memset(mSubFace, 0, sizeof(sHEFace) * miSubFaceSize);

		miSubNumEdges = miNumEdges;
		miSubNumVertices = miNumVertices;
		miSubNumFaces = miNumFaces;

		memcpy(mSubDestEdge, mEdges, sizeof(sHEEdge) * miNumEdges);
		memcpy(mSubDestVertex, mVertices, sizeof(sHEVertex) * miNumVertices);
		memcpy(mSubDestFace, mFaces, sizeof(sHEFace) * miNumFaces);

		INT64 disp_edge = ((BYTE*)mSubDestEdge - (BYTE*)mEdges);
		INT64 disp_vertex = ((BYTE*)mSubDestVertex - (BYTE*)mVertices);
		INT64 disp_face = ((BYTE*)mSubDestFace - (BYTE*)mFaces);

		BYTE** ppAddress;
		for (i = 0; i < miNumFaces; ++i)
		{
			ppAddress = (BYTE**)&mSubDestFace[i].pEdge;
			*ppAddress = *ppAddress + disp_edge;
		}
		for (i = 0; i < miNumVertices; ++i)
		{
			ppAddress = (BYTE**)&mSubDestVertex[i].pEdge;
			*ppAddress = *ppAddress + disp_edge;
		}
		for (i = 0; i < miNumEdges; ++i)
		{
			ppAddress = (BYTE**)&mSubDestEdge[i].pVertex;
			*ppAddress = *ppAddress + disp_vertex;

			ppAddress = (BYTE**)&mSubDestEdge[i].pFace;
			*ppAddress = *ppAddress + disp_face;

			ppAddress = (BYTE**)&mSubDestEdge[i].pPrev;
			*ppAddress = *ppAddress + disp_edge;

			ppAddress = (BYTE**)&mSubDestEdge[i].pNext;
			*ppAddress = *ppAddress + disp_edge;

			if (mSubDestEdge[i].pPair != NULL)
			{
				ppAddress = (BYTE**)&mSubDestEdge[i].pPair;
				*ppAddress = *ppAddress + disp_edge;
			}
		}
		mbIsFirstTime = FALSE;
	}

	sHEEdge* eTemp = mSubEdge;
	sHEVertex* vTemp = mSubVertex;
	sHEFace* fTemp = mSubFace;

	mSubEdge = mSubDestEdge;
	mSubVertex = mSubDestVertex;
	mSubFace = mSubDestFace;

	mSubDestEdge = eTemp;
	mSubDestVertex = vTemp;
	mSubDestFace = fTemp;
	Smoothing();

	for (i = 0; i < miSubNumVertices; ++i)
	{
		mSubDestVertex[i].valence = GetValence(&mSubDestVertex[i]);
		mSubVertex[i].valence = GetValence(&mSubVertex[i]);
	}

	Refinement();
	if (subLevel_ > 0)
	{
		Subdivide(subLevel_);
	}

	mbIsFirstTime = TRUE;
}

void	RButterflyMesh::Refinement()
{
	int			i;
	vector<sHEEdge*>::iterator	itor;

	sHEEdge* peCurr;
	sHEVertex* pvCurr;
	sHEVertex* pvDest;
	sHEVertex* nVertex;

	int			iBaseVertexIndex = miSubNumVertices;
	int			iBaseEdgeIndex = miSubNumEdges;

	Validate();

	for (i = 0; i < miSubNumEdges; ++i)
	{
		peCurr = &mSubEdge[i];

		if (peCurr->bMark)
		{
			continue;
		}

		pvCurr = peCurr->pVertex;
		pvDest = peCurr->pNext->pVertex;

		nVertex = &mSubDestVertex[iBaseVertexIndex++];

		if (peCurr->pPair)
		{
			if (pvCurr->valence == 6 && pvDest->valence == 6)
			{
				sHEVertex* pvTemp[6];
				sHEEdge* peTemp;

				peTemp = peCurr->pPrev;
				pvTemp[0] = peTemp->pVertex;

				peTemp = peTemp->pPair->pPrev;
				pvTemp[2] = peTemp->pVertex;

				peTemp = peCurr->pNext;
				peTemp = peTemp->pPair->pPrev;
				pvTemp[3] = peTemp->pVertex;

				peTemp = peCurr->pPair;

				peTemp = peTemp->pPrev;
				pvTemp[1] = peTemp->pVertex;

				peTemp = peTemp->pPair->pPrev;
				pvTemp[4] = peTemp->pVertex;

				peTemp = peCurr->pPair;
				peTemp = peTemp->pNext;
				peTemp = peTemp->pPair->pPrev;
				pvTemp[5] = peTemp->pVertex;

				*nVertex = 0.5 * (*pvCurr + *pvDest);
				*nVertex += 0.125 * (*pvTemp[0] + *pvTemp[1]);
				*nVertex -= 0.0625 * (*pvTemp[2] + *pvTemp[3] + *pvTemp[4] + *pvTemp[5]);
			}

			else if (pvCurr->valence == 6 && pvDest->valence != 6)
			{
				if (pvDest->valence == 3)
				{
					CalAddedVertex(peCurr->pPair, pvCurr, pvDest, 3, nVertex);
				}
				else if (pvDest->valence == 4)
				{
					CalAddedVertex(peCurr->pPair, pvCurr, pvDest, 4, nVertex);
				}
				else
				{
					CalAddedVertex(peCurr->pPair, pvCurr, pvDest, pvDest->valence, nVertex);
				}
			}
			else if (pvCurr->valence != 6 && pvDest->valence == 6)
			{
				if (pvCurr->valence == 3)
				{
					CalAddedVertex(peCurr, pvDest, pvCurr, 3, nVertex);
				}
				else if (pvCurr->valence == 4)
				{
					CalAddedVertex(peCurr, pvDest, pvCurr, 4, nVertex);
				}
				else
				{
					CalAddedVertex(peCurr, pvDest, pvCurr, pvCurr->valence, nVertex);
				}
			}

			else
			{
				sHEVertex	vTemp1, vTemp2;
				if (pvDest->valence == 3)
				{
					CalAddedVertex(peCurr->pPair, pvCurr, pvDest, 3, &vTemp1);
				}
				else if (pvDest->valence == 4)
				{
					CalAddedVertex(peCurr->pPair, pvCurr, pvDest, 4, &vTemp1);
				}
				else
				{
					CalAddedVertex(peCurr->pPair, pvCurr, pvDest, pvDest->valence, &vTemp1);
				}

				if (pvCurr->valence == 3)
				{
					CalAddedVertex(peCurr, pvDest, pvCurr, 3, &vTemp2);
				}
				else if (pvCurr->valence == 4)
				{
					CalAddedVertex(peCurr, pvDest, pvCurr, 4, &vTemp2);
				}
				else
				{
					CalAddedVertex(peCurr, pvDest, pvCurr, pvCurr->valence, &vTemp2);
				}

				*nVertex = 0.5 * (vTemp1 + vTemp2);
			}
		}
		else
		{
			sHEVertex* pvTemp;

			pvTemp = peCurr->pPrev->pVertex;

			sHEVertex invVertex;
			invVertex = *pvDest - *pvTemp;
			invVertex += *pvCurr;

			*nVertex = 0.5625 * (*pvCurr + *pvDest);
			*nVertex -= 0.0625 * (*pvTemp + invVertex);
		}

		peCurr->bMark = TRUE;
		if (peCurr->pPair != NULL)
		{
			peCurr->pPair->bMark = TRUE;
		}

		peCurr = &mSubDestEdge[i];

		if (peCurr->pPair != NULL)
		{
			SplitEdge(peCurr, nVertex, &mSubDestEdge[iBaseEdgeIndex++], &mSubDestEdge[iBaseEdgeIndex++]);
		}
		else
		{
			SplitEdge(peCurr, nVertex, &mSubDestEdge[iBaseEdgeIndex++]);
		}
	}

	sHEFace* pFace;
	sHEFace* nFace;
	sHEEdge* nEdge1;
	sHEEdge* nEdge2;

	int	iBaseFaceIndex = miSubNumFaces;

	for (i = 0; i < miSubNumFaces; ++i)
	{
		pFace = &mSubDestFace[i];

		while (pFace != NULL)
		{
			nFace = &mSubDestFace[iBaseFaceIndex++];
			nEdge1 = &mSubDestEdge[iBaseEdgeIndex++];
			nEdge2 = &mSubDestEdge[iBaseEdgeIndex++];
			pFace = SplitFaceLocal(pFace, nFace, nEdge1, nEdge2);
		}
	}

	sHEVertex* pvTemp;
	sHEVertex* pvDestTemp;
	for (i = 0; i < miSubNumVertices; ++i)
	{
		pvTemp = &mSubVertex[i];
		pvDestTemp = &mSubDestVertex[i];
		if (pvTemp->valence == 6)
		{
		}
		else
		{
		}
	}

	miSubNumEdges = iBaseEdgeIndex;
	miSubNumVertices = iBaseVertexIndex;
	miSubNumFaces = iBaseFaceIndex;
}

void	RButterflyMesh::CalAddedVertex(sHEEdge* peCurr_, sHEVertex* pvCurr_, sHEVertex* pvDest_,
	int valence_, sHEVertex* nVertex_)
{
	nVertex_->pos = rvector(0, 0, 0);
	nVertex_->normal = rvector(0, 0, 0);
	nVertex_->color = rvector(0, 0, 0);
	nVertex_->tu = nVertex_->tv = 0.0f;
	if (valence_ == 3)
	{
		bool reverse = false;

		sHEEdge* peTemp;
		sHEVertex* pvTemp;

		*nVertex_ = 0.75 * *pvDest_;

		peTemp = peCurr_;
		peTemp = peTemp->pPair;
		pvTemp = peTemp->pVertex;
		*nVertex_ += 0.41667 * *pvTemp;

		peTemp = peTemp->pNext;
		if (peTemp->pPair != NULL)
		{
			peTemp = peTemp->pPair;
			pvTemp = peTemp->pVertex;
		}
		else
		{
			peTemp = peTemp->pNext;
			pvTemp = peTemp->pVertex;
			reverse = true;
		}

		*nVertex_ += -0.08333 * *pvTemp;

		if (reverse)
		{
			peTemp = peCurr_->pPrev;
			pvTemp = peTemp->pVertex;
		}
		else
		{
			peTemp = peTemp->pNext;
			peTemp = peTemp->pNext;
			pvTemp = peTemp->pVertex;
		}

		*nVertex_ += -0.08333 * *pvTemp;
	}
	else if (valence_ == 4)
	{
		bool reverse = false;

		sHEEdge* peTemp;
		sHEVertex* pvTemp;

		*nVertex_ = 0.75 * *pvDest_;

		peTemp = peCurr_;
		peTemp = peTemp->pPair;
		pvTemp = peTemp->pVertex;
		*nVertex_ += 0.375 * *pvTemp;

		peTemp = peTemp->pNext;
		peTemp = peTemp->pPair;

		if (peTemp == NULL)
		{
			reverse = true;
		}
		else
		{
			peTemp = peTemp->pNext;
			peTemp = peTemp->pNext;
			pvTemp = peTemp->pVertex;
		}

		if (reverse)
		{
			peTemp = peCurr_->pPrev;
			peTemp = peTemp->pPair;
			peTemp = peTemp->pPrev;
			pvTemp = peTemp->pVertex;
		}

		*nVertex_ += -0.125 * *pvTemp;
	}
	else
	{
		bool reverse = false;
		sHEEdge* peTemp;
		sHEVertex* pvTemp;

		peTemp = peCurr_;
		peTemp = peTemp->pPair;
		pvTemp = peTemp->pVertex;
		float weight_sum = 0;
		float weight;

		weight = (1.25 + 0.5) / valence_;
		*nVertex_ = weight * *pvTemp;

		weight_sum = weight;

		for (int j = 1; j < valence_; ++j)
		{
			weight = ((0.25 + cos((2 * D3DX_PI * j) / valence_)) +
				(0.5 * cos((4 * D3DX_PI * j) / valence_))) / valence_;

			if (reverse)
			{
				pvTemp = peTemp->pVertex;

				if (j < valence_ - 1)
				{
					peTemp = peTemp->pPair;
					peTemp = peTemp->pPrev;
				}
			}
			else
			{
				peTemp = peTemp->pNext;
				if (peTemp->pPair == NULL)
				{
					pvTemp = peTemp->pNext->pVertex;

					reverse = true;
					peTemp = peCurr_;
					peTemp = peTemp->pPrev;
				}
				else
				{
					peTemp = peTemp->pPair;
					pvTemp = peTemp->pVertex;
				}
			}

			*nVertex_ += weight * *pvTemp;

			weight_sum += weight;
		}

		weight = 1 - weight_sum;

		*nVertex_ += weight * *pvDest_;
	}
}

void RButterflyMesh::Smoothing()
{
	int i;

	memcpy(mSubDestEdge, mSubEdge, sizeof(sHEEdge) * miSubNumEdges);
	memcpy(mSubDestVertex, mSubVertex, sizeof(sHEVertex) * miSubNumVertices);
	memcpy(mSubDestFace, mSubFace, sizeof(sHEFace) * miSubNumFaces);

	INT64 disp_edge = ((BYTE*)mSubDestEdge - (BYTE*)mSubEdge);
	INT64 disp_vertex = ((BYTE*)mSubDestVertex - (BYTE*)mSubVertex);
	INT64 disp_face = ((BYTE*)mSubDestFace - (BYTE*)mSubFace);

	BYTE** ppAddress;
	for (i = 0; i < miSubNumFaces; ++i)
	{
		ppAddress = (BYTE**)&mSubDestFace[i].pEdge;
		*ppAddress = *ppAddress + disp_edge;
	}
	for (i = 0; i < miSubNumVertices; ++i)
	{
		ppAddress = (BYTE**)&mSubDestVertex[i].pEdge;
		*ppAddress = *ppAddress + disp_edge;
	}
	for (i = 0; i < miSubNumEdges; ++i)
	{
		ppAddress = (BYTE**)&mSubDestEdge[i].pVertex;
		*ppAddress = *ppAddress + disp_vertex;

		ppAddress = (BYTE**)&mSubDestEdge[i].pFace;
		*ppAddress = *ppAddress + disp_face;

		ppAddress = (BYTE**)&mSubDestEdge[i].pPrev;
		*ppAddress = *ppAddress + disp_edge;

		ppAddress = (BYTE**)&mSubDestEdge[i].pNext;
		*ppAddress = *ppAddress + disp_edge;

		if (mSubDestEdge[i].pPair != NULL)
		{
			ppAddress = (BYTE**)&mSubDestEdge[i].pPair;
			*ppAddress = *ppAddress + disp_edge;
		}
	}
}

RSubdivisionMesh::RSubdivisionMesh()
	:mbNormal(false), mbColor(false), mbTexCoord(false), mbIsFirstTime(true),
	miSubNumEdges(0), miSubNumVertices(0), miSubNumFaces(0),
	mSubEdge(0), mSubVertex(0), mSubFace(0),
	mSubDestEdge(0), mSubDestVertex(0), mSubDestFace(0)
{
}

void RSubdivisionMesh::init(int subLevel_)
{
	miSubLevel = subLevel_;

	miSubEdgeSize = miNumEdges << (2 * subLevel_);
	miSubFaceSize = miNumFaces << (2 * subLevel_);
	miSubVertexSize = miSubEdgeSize;

	mSubEdge = new sHEEdge[miSubEdgeSize];
	mSubVertex = new sHEVertex[miSubVertexSize];
	mSubFace = new sHEFace[miSubFaceSize];
	mSubDestEdge = new sHEEdge[miSubEdgeSize];
	mSubDestVertex = new sHEVertex[miSubVertexSize];
	mSubDestFace = new sHEFace[miSubFaceSize];

	memset(mSubEdge, 0, sizeof(sHEEdge) * miSubEdgeSize);
	memset(mSubVertex, 0, sizeof(sHEVertex) * miSubVertexSize);
	memset(mSubFace, 0, sizeof(sHEFace) * miSubFaceSize);
	memset(mSubDestEdge, 0, sizeof(sHEEdge) * miSubEdgeSize);
	memset(mSubDestVertex, 0, sizeof(sHEVertex) * miSubVertexSize);
	memset(mSubDestFace, 0, sizeof(sHEFace) * miSubFaceSize);
}

RSubdivisionMesh::~RSubdivisionMesh()
{
	SAFE_DELETE_ARRAY(mSubEdge);
	SAFE_DELETE_ARRAY(mSubVertex);
	SAFE_DELETE_ARRAY(mSubFace);

	SAFE_DELETE_ARRAY(mSubDestEdge);
	SAFE_DELETE_ARRAY(mSubDestVertex);
	SAFE_DELETE_ARRAY(mSubDestFace);
}

void	RButterflyMesh::Draw()
{
	int							i, j;
	int							nVertex = 0;
	int							nPrimitive = 0;
	vector<sHEFace*>::iterator	itor;
	sHEFace* pfCurr;
	sHEEdge* peCurr;

	RVertex* pVertices = new RVertex[miSubNumFaces * 3];

	for (i = 0; i < miSubNumFaces; ++i)
	{
		pfCurr = &mSubDestFace[i];
		peCurr = pfCurr->pEdge;
		for (j = 0; j < 3; ++j)
		{
			pVertices[nVertex].p = peCurr->pVertex->pos;
			pVertices[nVertex].n = rvector(0, 1, 0);
			pVertices[nVertex].tu = peCurr->pVertex->tu;
			pVertices[nVertex].tv = peCurr->pVertex->tv;

			peCurr = peCurr->pNext;
			if (peCurr->pPair != NULL)
			{
				peCurr = peCurr->pPair->pPair;
			}

			++nVertex;
		}
		++nPrimitive;
	}

	RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	RGetDevice()->SetFVF(RVertexType);
	RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, nPrimitive, pVertices, sizeof(RVertex));
	RGetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	RGetDevice()->SetRenderState(D3DRS_LIGHTING, TRUE);

	delete[]	pVertices;
}

void	RButterflyMesh::Reset(rvector* pPointList_, rvector* pNormalList_)
{
	for (int i = 0; i < miNumVertices; ++i)
	{
		mVertices[i].pos = pPointList_[i];
		if (pNormalList_ != 0)
		{
			mVertices[i].normal = pPointList_[i];
		}
	}
}

_NAMESPACE_REALSPACE2_END