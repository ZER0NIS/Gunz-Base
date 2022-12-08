#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <tchar.h>

#include "MXml.h"

#include "RealSpace2.h"
#include "RMesh.h"
#include "RMeshMgr.h"

#include "MDebug.h"

#include "RAnimationMgr.h"
#include "RVisualmeshMgr.h"

#include "MZFileSystem.h"
#include "fileinfo.h"

#include "RShaderMgr.h"

_NAMESPACE_REALSPACE2_BEGIN

RMeshNode* RMesh::UpdateNodeAniMatrix(RMeshNode* pNode)
{
	D3DXMATRIX ani_mat;

	RMeshNode* pTMeshNode = NULL;

	pTMeshNode = pNode;

	D3DXMatrixIdentity(&ani_mat);

	GetNodeAniMatrix(pNode, ani_mat);

	if (m_pVisualMesh)
	{
		pTMeshNode = m_pVisualMesh->GetParts(pNode->m_PartsType);

		if (pTMeshNode == NULL) {
			pTMeshNode = pNode;
		}
		else if (pTMeshNode->m_PartsType != pNode->m_PartsType) {
			pTMeshNode = pNode;
		}
	}
	else {
		pTMeshNode = pNode;
	}

	pTMeshNode->m_mat_result = ani_mat;

	return pTMeshNode;
}

void RMesh::GetNodeAniMatrix(RMeshNode* pMeshNode, D3DXMATRIX& ani_mat)
{
	RAnimation* pAniSet = GetNodeAniSet(pMeshNode);
	int frame = pMeshNode->GetNodeAniSetFrame();

	AnimationType ani_type = RAniType_Bone;

	if (pAniSet) {
		ani_type = pAniSet->GetAnimationType();
	}

	if (ani_type == RAniType_Tm)
	{
		_RGetAniMat(pMeshNode, frame, ani_mat);
	}
	else if (ani_type == RAniType_Vertex)
	{
	}
	else
	{
		_RGetRotAniMat(pMeshNode, frame, ani_mat);
		_RGetPosAniMat(pMeshNode, frame, ani_mat);

		CalcLookAtParts(ani_mat, pMeshNode, m_pVisualMesh);

		if (pMeshNode->m_pParent) {
			D3DXMatrixMultiply(&ani_mat, &ani_mat, &pMeshNode->m_pParent->m_mat_result);
		}
	}
}

void RMesh::_RGetAniMat(RMeshNode* pMeshNode, int frame, D3DXMATRIX& t_ani_mat)
{
	if (pMeshNode == NULL) {
		mlog("_RGetAniMat() pMeshNode==NULL\n");
		return;
	}

	RAnimationNode* pANode = pMeshNode->m_pAnimationNode;

	bool bAni = false;

	if (pANode)
		if (pANode->m_mat_cnt)
			bAni = true;

	if (bAni) {
		rvector v;

		t_ani_mat = pANode->GetTMValue(frame);
	}
	else {
	}
}

void RMesh::_RGetRotAniMat(RMeshNode* pMeshNode, int frame, D3DXMATRIX& t_ani_mat)
{
	D3DXMATRIX buffer, Inv;

	if (pMeshNode == NULL) {
		return;
	}

	RAnimationNode* pANode = pMeshNode->m_pAnimationNode;

	bool bAni = false;

	if (pANode)
		if (pANode->m_rot_cnt)
			bAni = true;

	if (bAni) {
		D3DXQUATERNION out = pANode->GetRotValue(frame);

		D3DXMatrixRotationQuaternion(&t_ani_mat, &out);
	}
	else {
		D3DXMatrixIdentity(&buffer);

		if (m_isNPCMesh && (pMeshNode->m_WeaponDummyType != weapon_dummy_etc))
		{
			memcpy(&buffer, &pMeshNode->m_mat_local, sizeof(D3DXMATRIX));
		}
		else {
			if (pMeshNode->m_pParent) {
				RMatInv(Inv, pMeshNode->m_pParent->m_mat_base);
				D3DXMatrixMultiply(&buffer, &pMeshNode->m_mat_base, &Inv);
			}
			else {
				memcpy(&buffer, &pMeshNode->m_mat_local, sizeof(D3DXMATRIX));
			}
		}

		buffer._41 = buffer._42 = buffer._43 = 0;
		D3DXMatrixMultiply(&t_ani_mat, &t_ani_mat, &buffer);
	}
}

void RMesh::_RGetPosAniMat(RMeshNode* pMeshNode, int frame, D3DXMATRIX& t_ani_mat)
{
	if (pMeshNode == NULL) {
		return;
	}

	D3DXMATRIX buffer, Inv;

	RAnimationNode* pANode = pMeshNode->m_pAnimationNode;

	if (pMeshNode->m_LookAtParts == lookat_parts_spine1) {
		if (m_pAniSet[1]) {
			t_ani_mat._41 = pMeshNode->m_spine_local_pos.x;
			t_ani_mat._42 = pMeshNode->m_spine_local_pos.y;
			t_ani_mat._43 = pMeshNode->m_spine_local_pos.z;

			return;
		}
	}

	bool bAni = false;

	if (pANode)
		if (pANode->m_pos_cnt)
			bAni = true;

	if (bAni) {
		rvector v;

		v = pANode->GetPosValue(frame);

		t_ani_mat._41 = v.x;
		t_ani_mat._42 = v.y;
		t_ani_mat._43 = v.z;
	}
	else {
		D3DXMatrixIdentity(&buffer);

		if (m_isNPCMesh && pMeshNode->m_WeaponDummyType != weapon_dummy_etc)
		{
			buffer = pMeshNode->m_mat_local;
		}
		else {
			if (pMeshNode->m_pParent) {
				buffer = pMeshNode->m_mat_base * pMeshNode->m_pParent->m_mat_inv;
			}
			else {
				buffer = pMeshNode->m_mat_local;
			}
		}

		t_ani_mat._41 = buffer._41;
		t_ani_mat._42 = buffer._42;
		t_ani_mat._43 = buffer._43;
	}
}

void MakeRotMatrix(rmatrix* pOut, rvector& pos, rvector& dir, rvector& up)
{
	D3DXMatrixIdentity(pOut);

	rvector right;
	D3DXVec3Cross(&right, &up, &dir);
	D3DXVec3Cross(&up, &dir, &right);

	D3DXVec3Normalize(&up, &up);
	D3DXVec3Normalize(&dir, &dir);
	D3DXVec3Normalize(&right, &right);

	pOut->_11 = right.x;
	pOut->_12 = right.y;
	pOut->_13 = right.z;

	pOut->_21 = up.x;
	pOut->_22 = up.y;
	pOut->_23 = up.z;

	pOut->_31 = dir.x;
	pOut->_32 = dir.y;
	pOut->_33 = dir.z;

	pOut->_41 = pos.x;
	pOut->_42 = pos.y;
	pOut->_43 = pos.z;
}

#define MAX_XA_LEFT		90.f
#define MAX_XA_RIGHT	-90.f

#define MAX_YA_FRONT	50.f
#define MAX_YA_BACK		-70.f

void RMesh::CalcLookAtParts(D3DXMATRIX& pAniMat, RMeshNode* pMeshNode, RVisualMesh* pVisualMesh)
{
	if (pMeshNode && pVisualMesh) {
		float add_value = pVisualMesh->m_FrameTime.GetValue();
		float add_value_npc = 0.f;

		float rot_x = pVisualMesh->m_vRotXYZ.x;
		float rot_y = pVisualMesh->m_vRotXYZ.y;

		if (m_isNPCMesh) {
			add_value_npc = add_value;
		}

		if (rot_x > MAX_XA_LEFT)		rot_x = MAX_XA_LEFT;
		if (rot_x < MAX_XA_RIGHT)	rot_x = MAX_XA_RIGHT;

		if (rot_y > MAX_YA_FRONT)	rot_y = MAX_YA_FRONT;
		if (rot_y < MAX_YA_BACK)		rot_y = MAX_YA_BACK;

		if (pMeshNode->m_LookAtParts == lookat_parts_spine1) {
			rmatrix mx, my;

			float rot_y2 = rot_y + add_value_npc;

			mx = RGetRotX(rot_x * .6f);
			my = RGetRotY(rot_y2 * .6f);

			D3DXMatrixMultiply(&pAniMat, &pAniMat, &mx);
			D3DXMatrixMultiply(&pAniMat, &pAniMat, &my);
		}

		if (pMeshNode->m_LookAtParts == lookat_parts_spine2) {
			rmatrix mx, my;

			float rot_y2 = rot_y + add_value;

			mx = RGetRotX(rot_x * .5f);
			my = RGetRotY(rot_y2 * .5f);

			D3DXMatrixMultiply(&pAniMat, &pAniMat, &mx);
			D3DXMatrixMultiply(&pAniMat, &pAniMat, &my);
		}

		if (pMeshNode->m_LookAtParts == lookat_parts_head) {
			rmatrix mx, my;

			float rot_y2 = rot_y + add_value * 0.5f;

			mx = RGetRotX(rot_x * .3f);
			my = RGetRotY(rot_y2 * .3f);

			D3DXMatrixMultiply(&pAniMat, &pAniMat, &mx);
			D3DXMatrixMultiply(&pAniMat, &pAniMat, &my);
		}
	}
}

void RMesh::CalcLookAtParts2(D3DXMATRIX& pAniMat, RMeshNode* pMeshNode, RVisualMesh* pVisualMesh)
{
	if (pMeshNode && pVisualMesh) {
		D3DXMATRIX* mat = NULL;

		bool ch = false;

		if (pMeshNode->m_LookAtParts == lookat_parts_spine1) {
			mat = &pMeshNode->m_mat_result;
			ch = true;
		}

		if (pMeshNode->m_LookAtParts == lookat_parts_spine2) {
			mat = &pMeshNode->m_mat_result;
			ch = true;
		}

		if (pMeshNode->m_LookAtParts == lookat_parts_head) {
			mat = &pMeshNode->m_mat_result;
			ch = true;
		}

		if (ch) {
			mat->_11 = pVisualMesh->m_UpperRotMat._11;
			mat->_12 = pVisualMesh->m_UpperRotMat._12;
			mat->_13 = pVisualMesh->m_UpperRotMat._13;

			mat->_21 = pVisualMesh->m_UpperRotMat._21;
			mat->_22 = pVisualMesh->m_UpperRotMat._22;
			mat->_23 = pVisualMesh->m_UpperRotMat._23;

			mat->_31 = pVisualMesh->m_UpperRotMat._31;
			mat->_32 = pVisualMesh->m_UpperRotMat._32;
			mat->_33 = pVisualMesh->m_UpperRotMat._33;
		}
	}
}

void RMesh::RenderFrameSingleParts() {
}

void RMesh::RenderFrameMultiParts() {
}

RAnimation* RMesh::GetNodeAniSet(RMeshNode* pNode)
{
	if (!pNode) return NULL;

	RAnimation* pAniSet = m_pAniSet[0];

	if (m_pAniSet[1]) {
		if (pNode->m_CutPartsType == cut_parts_upper_body)
			pAniSet = m_pAniSet[1];
	}
	return pAniSet;
}

void GetQuat(D3DXQUATERNION& q, rvector& dir1, rvector& dir2)
{
	rvector axis;
	CrossProduct(&axis, dir1, dir2);
	Normalize(axis);
	float theta = DotProduct(dir1, dir2);

	float rad = acos(theta);

	D3DXQuaternionRotationAxis(&q, &axis, rad);
}

void GetMat(rmatrix& m, rvector& dir1, rvector& dir2)
{
	D3DXQUATERNION q;
	GetQuat(q, dir1, dir2);
	D3DXMatrixRotationQuaternion(&m, &q);
}

rmatrix makematrix(rvector pos, rvector dir, rvector up)
{
	rmatrix m;
	rvector right;
	D3DXVec3Cross(&right, &up, &dir);
	D3DXVec3Normalize(&right, &right);

	D3DXVec3Cross(&up, &right, &dir);
	D3DXVec3Normalize(&up, &up);

	D3DXVec3Normalize(&dir, &dir);

	D3DXMatrixIdentity(&m);

	m._11 = right.x;
	m._12 = right.y;
	m._13 = right.z;

	m._21 = up.x;
	m._22 = up.y;
	m._23 = up.z;

	m._31 = dir.x;
	m._32 = dir.y;
	m._33 = dir.z;

	m._41 = pos.x;
	m._42 = pos.y;
	m._43 = pos.z;

	return m;
}

void CalcNodeMatrix(RVisualMesh* pVMesh, RMeshNode* pNode, bool upLimit)
{
	if (pNode == NULL) return;

	rvector vTargetPos = pVMesh->m_vTargetPos;
	rmatrix world = pVMesh->m_WorldMat;

	rmatrix rot;

	rmatrix m = pNode->m_mat_result * world;

	draw_box(&m, rvector(5, 5, 5), rvector(-5, -5, -5), 0xff88ffff);

	draw_box(&m, rvector(5, 5, 5), rvector(-5, -5, -5), 0xff22ff22);

	rvector lpos = vTargetPos;

	rvector hpos = rvector(m._41, m._42, m._43);
	rvector dir1 = rvector(m._31, m._32, m._33);
	rvector dir2 = lpos - hpos;
	if (upLimit) {
		dir1.y = 0.f;
		dir2.y = 0.f;
	}

	Normalize(dir1);
	Normalize(dir2);

	GetMat(rot, dir1, dir2);

	m._41 = 0.f;
	m._42 = 0.f;
	m._43 = 0.f;

	m = m * rot;

	m._41 = pNode->m_mat_result._41;
	m._42 = pNode->m_mat_result._42;
	m._43 = pNode->m_mat_result._43;

	pNode->m_mat_result = m;
}

void RMesh::RenderFrame()
{
	D3DXMATRIX	s;

	if (m_list.empty())
		return;

	RMeshNodeHashList_Iter it_obj = m_list.begin();

	if (!m_pAniSet[0])
		return;

	RMeshNode* pHeadMeshNode = NULL;
	RMeshNode* pSpine1MeshNode = NULL;
	RMeshNode* pSpine2MeshNode = NULL;

	while (it_obj != m_list.end()) {
		RMeshNode* pMeshNode = (*it_obj);

		UpdateNodeAniMatrix(pMeshNode);

		it_obj++;
	}
}

void RMesh::SetFrame(int nFrame, int nFrame2)
{
	if (nFrame < 0 || nFrame >= m_max_frame[0])
		return;

	m_frame[0] = nFrame;

	if (nFrame2 != -1) {
		if (nFrame2 < 0 || nFrame2 >= m_max_frame[1])
			return;
		m_frame[1] = nFrame2;
	}
	else {
		m_frame[1] = 0;
	}
}

_NAMESPACE_REALSPACE2_END