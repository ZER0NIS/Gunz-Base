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

#ifndef _PUBLISH

#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);

#else

#define __BP(i,n) ;
#define __EP(i) ;

#endif

_NAMESPACE_REALSPACE2_BEGIN

bool RMesh::ReadXmlElement(MXmlElement* PNode, const char* Path)
{
	char NodeName[256];
	char IDName[256];
	char FileName[256];
	char PathFileName[256];
	char MotionTypeID[256];
	char MotionLoopTypeID[256];
	char SoundFileName[256];
	char GameMotion[256];
	char PathSoundFileName[256];

	RMesh* pMesh = NULL;

	int nMTypeID = -1;
	bool bSoundMap = false;
	AnimationLoopType MLoopType = RAniLoopType_Loop;

	int nCnt = PNode->GetChildNodeCount();

	MXmlElement Node;

	for (int i = 0; i < nCnt; i++) {
		Node = PNode->GetChildNode(i);

		Node.GetTagName(NodeName);

		if (NodeName[0] == '#') continue;

		if (strcmp(NodeName, "AddBaseModel") == 0) {
			Node.GetAttribute(IDName, "name");
			Node.GetAttribute(FileName, "filename");

			if (Path) {
				strcpy(PathFileName, Path);
				strcat(PathFileName, FileName);
			}
			else {
				strcpy(PathFileName, FileName);
			}

			ReadElu(PathFileName);

			m_isMultiAniSet = true;
		}
		else if (strcmp(NodeName, "CharacterModel") == 0)
		{
			m_isCharacterMesh = true;
		}
		else if (strcmp(NodeName, "NPCModel") == 0)
		{
			m_isNPCMesh = true;
		}
		else if (strcmp(NodeName, "MakeAnimationMap") == 0)
		{
			m_ani_mgr.MakeListMap((int)eq_weapon_end);
		}
		else if (strcmp(NodeName, "AddParts") == 0)
		{
			extern bool IsDynamicResourceLoad();
			if (IsDynamicResourceLoad())
				continue;
			if (RMesh::m_parts_mesh_loading_skip == 0) {
				Node.GetAttribute(IDName, "name");
				Node.GetAttribute(FileName, "filename");

				if (!m_parts_mgr) {
					m_parts_mgr = new RMeshMgr;
				}

				if (Path[0]) {
					strcpy(PathFileName, Path);
					strcat(PathFileName, FileName);
				}
				else
					strcpy(PathFileName, FileName);

				int ret = m_parts_mgr->Add(PathFileName);

				if (ret == -1) {
					return false;
				}
			}
		}
		else if (strcmp(NodeName, "AddParticle") == 0) {
			Node.GetAttribute(IDName, "name");
			Node.GetAttribute(FileName, "dummy_name");

			m_ParticleLinkInfo.Set(IDName, FileName);
		}
		else if (strcmp(NodeName, "AddAnimation") == 0) {
			SoundFileName[0] = NULL;
			Node.GetAttribute(IDName, "name");
			Node.GetAttribute(FileName, "filename");
			Node.GetAttribute(MotionTypeID, "motion_type");
			Node.GetAttribute(MotionLoopTypeID, "motion_loop_type");
			Node.GetAttribute(SoundFileName, "sound");
			Node.GetAttribute(&bSoundMap, "soundmap", false);
			Node.GetAttribute(GameMotion, "gm");

			if (strcmp(MotionLoopTypeID, "normal") == 0) {
				MLoopType = RAniLoopType_Normal;
			}
			else if (strcmp(MotionLoopTypeID, "loop") == 0) {
				MLoopType = RAniLoopType_Loop;
			}
			else if (strcmp(MotionLoopTypeID, "onceidle") == 0) {
				MLoopType = RAniLoopType_OnceIdle;
			}
			else if (strcmp(MotionLoopTypeID, "lastframe") == 0) {
				MLoopType = RAniLoopType_HoldLastFrame;
			}
			else if (strcmp(MotionLoopTypeID, "onceLowerbody") == 0) {
				MLoopType = RAniLoopType_OnceLowerBody;
			}

			int nGameMotion = atoi(GameMotion);

			if (GetToolMesh())
				nGameMotion = 0;

			nMTypeID = atoi(MotionTypeID);

			if (Path[0]) {
				strcpy(PathFileName, Path);
				strcat(PathFileName, FileName);
			}
			else
				strcpy(PathFileName, FileName);

			RAnimation* pAni = NULL;

			if (nGameMotion == 1) {
				pAni = m_ani_mgr.AddGameLoad(IDName, PathFileName, -1, nMTypeID);
			}
			else {
				pAni = m_ani_mgr.Add(IDName, PathFileName, -1, nMTypeID);
			}

			if (pAni) {
				pAni->SetAnimationLoopType(MLoopType);

				if (SoundFileName[0] == NULL) {
					int len = (int)strlen(FileName);
					strncpy(SoundFileName, FileName, len - 8);
					SoundFileName[len - 8] = NULL;

					strcpy(PathSoundFileName, "/sound/effect/");
					strcat(PathSoundFileName, SoundFileName);
				}

				pAni->SetSoundFileName(SoundFileName);
				pAni->SetSoundRelatedToMap(bSoundMap);
			}
		}
	}

	return true;
}

bool RMesh::ReadXml(const char* filename)
{
	MXmlDocument	XmlDoc;
	MXmlElement		PNode, Node;

	char Path[256];
	Path[0] = NULL;

	std::string BackupName = filename;

	GetPath(filename, Path);

	if (!XmlDoc.LoadFromFile(filename, g_pFileSystem))
		return false;

	PNode = XmlDoc.GetDocumentElement();

	if (ReadXmlElement(&PNode, Path) == false) {
		XmlDoc.Destroy();
		return false;
	}

	XmlDoc.Destroy();

	SetFileName((char*)BackupName.c_str());

	return true;
}

bool RMesh::SaveXml(const char* fname)
{
	return true;
}

void ConvertOldFaceInfo(RFaceInfo* pFInfo, RFaceInfoOld* pOldFInfo, int cnt)
{
	for (int i = 0; i < cnt; i++) {
		memcpy(pFInfo[i].m_point_index, pOldFInfo[i].m_point_index, sizeof(int) * 3);
		memcpy(pFInfo[i].m_point_tex, pOldFInfo[i].m_point_tex, sizeof(D3DXVECTOR3) * 3);

		pFInfo[i].m_mtrl_id = pOldFInfo[i].m_mtrl_id;
		pFInfo[i].m_sg_id = 0;
	}
}

bool e_sort_str(RMeshNode* _a, RMeshNode* _b)
{
	if (_a->m_Name < _a->m_Name)
		return true;
	return false;
}

int CheckEf(char* str)
{
	if (!str) return 0;

	int _strlen = (int)strlen(str);

	for (int i = 0; i < _strlen - 3; i++) {
		if (str[i] == '_') {
			if (str[i + 1] == 'e') {
				if (str[i + 2] == 'f') {
					return 1;
				}
			}
		}
	}
	return 0;
}

int CheckEfAlign(char* str, int& ef, int& align)
{
	if (!str || str[0] == 0)	return -1;

	static char word1[] = "ef_algn";
	static char word2[] = "algn";

	int str_len = (int)strlen(str);

	int word1_len = (int)strlen(word1);
	int word2_len = (int)strlen(word2);

	char c;

	ef = 0;
	align = 0;

	bool bCheck = true;

	for (int i = 0; i < str_len; i++) {
		c = str[i];
		if (c == word1[0]) {
			bCheck = true;
			for (int j = 1; j < word1_len; j++) {
				if (str[i + j] != word1[j]) {
					bCheck = false;
				}
			}
			if (bCheck == true) {
				ef = 1;
				if (str[i + word1_len] == '0')
					align = 1;
				else if (str[i + word1_len] == '1')
					align = 2;
			}
		}
		else if (c == word2[0]) {
			bCheck = true;
			for (int j = 1; j < word2_len; j++) {
				if (str[i + j] != word2[j]) {
					bCheck = false;
				}
			}
			if (bCheck == true) {
				if (str[i + word2_len] == '0')
					align = 1;
				else if (str[i + word2_len] == '1')
					align = 2;
			}
		}
	}
	return -1;
}

void RMesh::CheckNameToType(RMeshNode* pMeshNode)
{
	pMeshNode->m_PartsType = eq_parts_etc;
	pMeshNode->m_isDummy = false;
	pMeshNode->m_isWeaponMesh = false;

	char* pName = pMeshNode->GetName();

#define NCMPNAME(name,n)	(strnicmp(pName,name,n)==0)
#define CMPNAME(name)		(stricmp(pName,name)==0)

	if (NCMPNAME("Bip", 3)) {
		pMeshNode->m_isDummyMesh = true;
	}
	else if (NCMPNAME("Bone", 4))		pMeshNode->m_isDummyMesh = true;
	else if (NCMPNAME("Dummy", 5)) {
		pMeshNode->m_isDummyMesh = true;
		pMeshNode->m_isDummy = true;
	}
	else if (NCMPNAME("eq_wr", 5) || NCMPNAME("eq_wl", 5) || NCMPNAME("eq_wd", 5)) {
		if (NCMPNAME("eq_wd_katana", 12)) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_katana;
		}
		else if (NCMPNAME("eq_wl_pistol", 12)) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_left_pistol;
		}
		else if (NCMPNAME("eq_wr_pistol", 12)) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_pistol;
		}
		else if (NCMPNAME("eq_wd_pistol", 12)) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_pistol;
		}
		else if (NCMPNAME("eq_wl_smg", 9)) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_left_smg;
		}
		else if (NCMPNAME("eq_wr_smg", 9)) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_smg;
		}
		else if (NCMPNAME("eq_wd_smg", 9)) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_smg;
		}
		else if (NCMPNAME("eq_wd_shotgun", 13)) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_shotgun;
		}
		else if (NCMPNAME("eq_wd_rifle", 11)) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_rifle;
		}
		else if (NCMPNAME("eq_wd_grenade", 13)) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_grenade;
		}
		else if (NCMPNAME("eq_wd_item", 10)) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_item;
		}
		else if (NCMPNAME("eq_wl_dagger", 12)) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_left_dagger;
		}
		else if (NCMPNAME("eq_wr_dagger", 12)) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_dagger;
		}
		else if (NCMPNAME("eq_wd_medikit", 13)) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_item;
		}
		else if (NCMPNAME("eq_wd_trap", 10)) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_item;
		}
		else if (NCMPNAME("eq_wd_rl", 8)) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_rlauncher;
		}
		else if (NCMPNAME("eq_wd_sword", 11)) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_sword;
		}
		else if (NCMPNAME("eq_wr_blade", 11)) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_blade;
		}
		else if (NCMPNAME("eq_wl_blade", 11)) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_left_blade;
		}
	}
	else if (NCMPNAME("eq_", 3)) {
		if (NCMPNAME("eq_head", 7)) {
			pMeshNode->m_PartsType = eq_parts_head;
			pMeshNode->m_AlphaSortValue = 1.f;
		}
		else if (NCMPNAME("eq_face", 7)) {
			pMeshNode->m_PartsType = eq_parts_face;
			pMeshNode->m_AlphaSortValue = 2.f;
		}
		else if (NCMPNAME("eq_chest", 8)) {
			pMeshNode->m_PartsType = eq_parts_chest;
			pMeshNode->m_AlphaSortValue = 3.f;
		}
		else if (NCMPNAME("eq_hands", 8)) {
			pMeshNode->m_PartsType = eq_parts_hands;
			pMeshNode->m_AlphaSortValue = 4.f;
		}
		else if (NCMPNAME("eq_legs", 7)) {
			pMeshNode->m_PartsType = eq_parts_legs;
			pMeshNode->m_AlphaSortValue = 5.f;
		}
		else if (NCMPNAME("eq_feet", 7)) {
			pMeshNode->m_PartsType = eq_parts_feet;
			pMeshNode->m_AlphaSortValue = 6.f;
		}
		else if (NCMPNAME("eq_sunglass", 11)) {
			pMeshNode->m_PartsType = eq_parts_sunglass;
			pMeshNode->m_AlphaSortValue = 0.5f;
		}
	}
	else if (NCMPNAME("collision_", 10)) {
		pMeshNode->m_isCollisionMesh = true;
		pMeshNode->m_isDummyMesh = true;
	}
	if (NCMPNAME("lastmodel", 9)) {
		pMeshNode->m_isLastModel = true;
	}

	if (NCMPNAME("deffect", 7)) {
		pMeshNode->m_isDummyMesh = true;
		pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_Effect;
	}

	if (NCMPNAME("Bip", 3)) {
		if (NCMPNAME("Bip01 L", 7)) {
			if (NCMPNAME("Bip01 L Calf", 12)) {
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LCalf;
			}
			else if (NCMPNAME("Bip01 L Clavile", 15)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LClavicle;
			}
			else if (NCMPNAME("Bip01 L Finger0", 15)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LFinger0;
			}
			else if (NCMPNAME("Bip01 L FingerNub", 17)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LFingerNub;
			}
			else if (NCMPNAME("Bip01 L Foot", 12)) {
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LFoot;
			}
			else if (NCMPNAME("Bip01 L ForeArm", 15)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LForeArm;
			}
			else if (NCMPNAME("Bip01 L Hand", 12)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LHand;
			}
			else if (NCMPNAME("Bip01 L Thigh", 13)) {
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LThigh;
			}
			else if (NCMPNAME("Bip01 L Toe0Nub", 15)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LToe0Nub;
			}
			else if (NCMPNAME("Bip01 L Toe0", 12)) {
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LToe0;
			}
			else if (NCMPNAME("Bip01 L UpperArm", 16)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LUpperArm;
			}
		}
		else if (NCMPNAME("Bip01 R", 7)) {
			if (NCMPNAME("Bip01 R Calf", 12)) {
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RCalf;
			}
			else if (NCMPNAME("Bip01 R Clavile", 15)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RClavicle;
			}
			else if (NCMPNAME("Bip01 R Finger0", 15)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RFinger0;
			}
			else if (NCMPNAME("Bip01 R FingerNub", 17)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RFingerNub;
			}
			else if (NCMPNAME("Bip01 R Foot", 12)) {
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RFoot;
			}
			else if (NCMPNAME("Bip01 R ForeArm", 15)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RForeArm;
			}
			else if (NCMPNAME("Bip01 R Hand", 12)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RHand;
			}
			else if (NCMPNAME("Bip01 R Thigh", 13)) {
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RThigh;
			}
			else if (NCMPNAME("Bip01 R Toe0Nub", 15)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RToe0Nub;
			}
			else if (NCMPNAME("Bip01 R Toe0", 12)) {
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RToe0;
			}
			else if (NCMPNAME("Bip01 R UpperArm", 16)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RUpperArm;
			}
		}
		else {
			if (CMPNAME("Bip01")) {
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_Root;
			}
			else if (CMPNAME("Bip01 Head")) {
				pMeshNode->m_LookAtParts = lookat_parts_head;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_Head;
			}
			else if (CMPNAME("Bip01 HeadNub")) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_HeadNub;
			}
			else if (CMPNAME("Bip01 Neck")) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_Neck;
			}
			else if (CMPNAME("Bip01 Pelvis")) {
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_Pelvis;
			}
			else if (CMPNAME("Bip01 Spine")) {
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_LookAtParts = lookat_parts_spine;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_Spine;
			}
			else if (CMPNAME("Bip01 Spine1")) {
				pMeshNode->m_LookAtParts = lookat_parts_spine1;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_Spine1;
			}
			else if (CMPNAME("Bip01 Spine2")) {
				pMeshNode->m_LookAtParts = lookat_parts_spine2;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_Spine2;
			}
		}
	}

	if (NCMPNAME("Bone", 4)) {
		pMeshNode->m_CutPartsType = cut_parts_lower_body;
	}

	if (NCMPNAME("Dummy", 5)) {
		pMeshNode->m_CutPartsType = cut_parts_lower_body;
	}

	int ef = 0;
	int align = 0;

	CheckEfAlign(pMeshNode->GetName(), ef, align);

	ef = CheckEf(pMeshNode->GetName());

	if (ef == 1) {
		m_bEffectSort = true;
		m_LitVertexModel = true;
	}

	pMeshNode->m_nAlign = align;

	if (NCMPNAME("muzzle_flash", 12)) {
		pMeshNode->m_WeaponDummyType = weapon_dummy_muzzle_flash;
	}
	else if (NCMPNAME("empty_cartridge01", 17)) {
		pMeshNode->m_WeaponDummyType = weapon_dummy_cartridge01;
	}
	else if (NCMPNAME("empty_cartridge02", 17)) {
		pMeshNode->m_WeaponDummyType = weapon_dummy_cartridge02;
	}

#undef NCMPNAME
}

bool RMesh::ReadNewElu(MZFile* mzf, const char* fname) {
#define MZF_READ(x,y) { if(!mzf->Read((x),(y))) return false; }
#define MZF_READ_XOR(x,y) { if(!mzf->Read((x),(y))) return false; }
#define MZF_READ_XOR_SPECIAL(x,y) { if(!mzf->Read((x),(y))) return false; for (int i = 0; i < y; ++i) *(BYTE*)(x + i) ^= 0; }
	int i;
	unsigned short count_mesh = 0;
	MZF_READ_XOR(&count_mesh, 2);

	for (i = 0; i < count_mesh; i++)
	{
		char Name[256];
		memset(Name, 0, sizeof(Name));

		unsigned short count_name = 0;
		unsigned short count_name_Parent = 0;
		MZF_READ_XOR(&count_name, 2);
		MZF_READ_XOR(Name, count_name);
		MZF_READ_XOR(&count_name_Parent, 2);
		RMeshNode* pMeshNode = new RMeshNode;
		D3DXMatrixIdentity(&pMeshNode->m_mat_base);
		pMeshNode->m_id = m_data_num;
		memset(pMeshNode->m_Parent, 0, sizeof(pMeshNode->m_Parent));
		pMeshNode->m_pParentMesh = this;
		pMeshNode->m_pBaseMesh = this;
		pMeshNode->SetName(Name);
		MZF_READ_XOR(pMeshNode->m_Parent, count_name_Parent);
		MZF_READ_XOR(pMeshNode->m_axis_scale, sizeof(D3DXVECTOR3));
		MZF_READ_XOR(pMeshNode->m_mat_etc, sizeof(D3DXMATRIX));
		MZF_READ_XOR(pMeshNode->m_mat_ref, sizeof(D3DXMATRIX));
		MZF_READ_XOR(pMeshNode->m_mat_inv, sizeof(D3DXMATRIX));
		MZF_READ_XOR(&pMeshNode->m_axis_rot_angle, sizeof(float));
		MZF_READ_XOR(pMeshNode->m_mat_base, sizeof(D3DXMATRIX));
		MZF_READ_XOR(pMeshNode->m_mat_ref_inv, sizeof(D3DXMATRIX));
		MZF_READ_XOR(pMeshNode->m_axis_rot, sizeof(D3DXVECTOR3));
		MZF_READ_XOR(&pMeshNode->m_axis_scale_angle, sizeof(float));
		MZF_READ_XOR(pMeshNode->m_mat_local, sizeof(D3DXMATRIX));
		MZF_READ_XOR(pMeshNode->m_mat_result, sizeof(D3DXMATRIX));
		MZF_READ_XOR(pMeshNode->m_mat_scale, sizeof(D3DXMATRIX));
		MZF_READ_XOR(pMeshNode->m_ap_scale, sizeof(D3DXVECTOR3));
		MZF_READ_XOR(pMeshNode->m_mat_flip, sizeof(D3DXMATRIX));

		CheckNameToType(pMeshNode);
		MZF_READ_XOR(&pMeshNode->m_point_num, 4);

		if (pMeshNode->m_point_num != 0) {
			pMeshNode->m_point_list = new D3DXVECTOR3[pMeshNode->m_point_num];
			memset(pMeshNode->m_point_list, 0, pMeshNode->m_point_num * sizeof(D3DXVECTOR3));
			int p = 0;
			for (p = 0; p < pMeshNode->m_point_num; p++)
			{
				MZF_READ_XOR(pMeshNode->m_point_list[p], sizeof(D3DXVECTOR3));
			}
			pMeshNode->CalcLocalBBox();
		}

		MZF_READ_XOR(&pMeshNode->m_face_num, 4);

		if (pMeshNode->m_face_num) {
			pMeshNode->m_face_list = new RFaceInfo[pMeshNode->m_face_num];
			pMeshNode->m_face_normal_list = new RFaceNormalInfo[pMeshNode->m_face_num];

			memset(pMeshNode->m_face_list, 0, pMeshNode->m_face_num * sizeof(RFaceInfo));

			MZF_READ(pMeshNode->m_face_list, sizeof(RFaceInfo) * pMeshNode->m_face_num);
			MZF_READ(pMeshNode->m_face_normal_list, sizeof(RFaceNormalInfo) * pMeshNode->m_face_num);
		}

		MZF_READ_XOR(&pMeshNode->m_point_color_num, 4);

		if (pMeshNode->m_point_color_num) {
			pMeshNode->m_point_color_list = new D3DXVECTOR3[pMeshNode->m_point_color_num];
			MZF_READ_XOR(pMeshNode->m_point_color_list, sizeof(D3DXVECTOR3) * pMeshNode->m_point_color_num);
		}

		if ((pMeshNode->m_point_num == 0) || (pMeshNode->m_face_num == 0)) {
			pMeshNode->m_isDummyMesh = true;
		}

		MZF_READ_XOR(&pMeshNode->m_mtrl_id, 4);

		MZF_READ_XOR(&pMeshNode->m_physique_num, 4);

		if (pMeshNode->m_physique_num) {
			m_isPhysiqueMesh = true;
			pMeshNode->m_physique = new RPhysiqueInfo[pMeshNode->m_physique_num];
			ZeroMemory(pMeshNode->m_physique, pMeshNode->m_physique_num * sizeof(RPhysiqueInfo));

			for (int j = 0; j < pMeshNode->m_physique_num; j++) {
				int infcount = 0;
				MZF_READ_XOR(&infcount, 1);
				pMeshNode->m_physique[j].m_num = infcount;
				for (int k = 0; k < infcount; k++)
				{
					int countiname = 0;
					MZF_READ_XOR(&pMeshNode->m_physique[j].m_parent_id[k], 4);
					MZF_READ_XOR(&pMeshNode->m_physique[j].m_weight[k], 4);
					MZF_READ_XOR(&pMeshNode->m_physique[j].m_offset[k], 12);
					MZF_READ_XOR(&countiname, 2);
					MZF_READ_XOR(&pMeshNode->m_physique[j].m_parent_name[k], countiname);
				}
			}
		}
		if (pMeshNode->m_point_color_num > 0 && pMeshNode->m_PartsType == eq_parts_chest)
			pMeshNode->m_isClothMeshNode = true;

		m_list.PushBack(pMeshNode);

		m_data.push_back(pMeshNode);
		m_data_num++;

		if (MAX_MESH_NODE_TABLE != (int)m_data.capacity())
		{
			mlog("m_data number is not quite right..! (%d)\n", (int)m_data.capacity());
			return false;
		}
	}
	unsigned short count_mtrl;
	MZF_READ_XOR(&count_mtrl, 2);
	i = 0;

	if (count_mtrl)
	{
		for (i = 0; i < count_mtrl; i++) {
			RMtrl* node = new RMtrl;
			MZF_READ_XOR(&node->m_mtrl_id, 4);
			MZF_READ_XOR(&node->m_sub_mtrl_id, 4);

			MZF_READ_XOR(&node->m_ambient, sizeof(D3DXCOLOR));
			MZF_READ_XOR(&node->m_diffuse, sizeof(D3DXCOLOR));
			MZF_READ_XOR(&node->m_specular, sizeof(D3DXCOLOR));

			MZF_READ_XOR(&node->m_power, 4);
			MZF_READ_XOR(&node->m_sub_mtrl_num, 4);
			unsigned short count_name;
			unsigned short count_opa_name;

			MZF_READ_XOR(&count_name, 2);
			memset(node->m_name, 0, sizeof(node->m_name));
			MZF_READ_XOR(node->m_name, count_name);
			memset(node->m_opa_name, 0, sizeof(node->m_opa_name));
			MZF_READ_XOR(&count_opa_name, 2);
			MZF_READ_XOR(node->m_opa_name, count_opa_name);

			// Custom : Fix NewElu material

			char twoside = 0;
			MZF_READ_XOR(&twoside, sizeof(char));
			node->m_bTwoSided = (twoside < 0) ? false : true;

			char additive = 0;
			MZF_READ_XOR(&additive, sizeof(char));
			node->m_bAdditive = (additive < 0) ? false : true;

			int alpha_test = 0;
			MZF_READ_XOR(&alpha_test, sizeof(int));
			node->m_nAlphaTestValue = alpha_test;
			node->m_bAlphaTestMap = alpha_test != 0;

			char diffuse = 0;
			MZF_READ_XOR(&diffuse, sizeof(char));
			node->m_bDiffuseMap = (diffuse < 0) ? false : true;

			char alpha_map = 0;
			MZF_READ_XOR(&alpha_map, sizeof(char));
			node->m_bAlphaMap = (alpha_map < 0) ? false : true;

			node->CheckAniTexture();

			m_mtrl_list_ex.Add(node);
		}
	}

	return true;
#undef MZF_READ
}

bool RMesh::ReadOldElu(MZFile* mzf, ex_hd_t* m_phd_t) {
#define MZF_READ(x,y) { if(!mzf->Read((x),(y))) return false; }

	int i, j, k;
	for (i = 0; i < m_phd_t->mtrl_num; i++) {
		RMtrl* node = new RMtrl;

		MZF_READ(&node->m_mtrl_id, 4);
		MZF_READ(&node->m_sub_mtrl_id, 4);

		MZF_READ(&node->m_ambient, sizeof(D3DXCOLOR));
		MZF_READ(&node->m_diffuse, sizeof(D3DXCOLOR));
		MZF_READ(&node->m_specular, sizeof(D3DXCOLOR));

		MZF_READ(&node->m_power, 4);

		node->m_power *= 100.f;

		if (m_phd_t->ver <= EXPORTER_MESH_VER3)
			if (node->m_power == 2000.f)
				node->m_power = 0.f;

		MZF_READ(&node->m_sub_mtrl_num, 4);

		if (m_phd_t->ver < EXPORTER_MESH_VER7) {
			MZF_READ(&node->m_name, MAX_NAME_LEN);
			MZF_READ(&node->m_opa_name, MAX_NAME_LEN);
		}
		else {
			MZF_READ(&node->m_name, MAX_PATH_NAME_LEN);
			MZF_READ(&node->m_opa_name, MAX_PATH_NAME_LEN);
		}

		if (m_phd_t->ver > EXPORTER_MESH_VER2) {
			int twoside = 0;
			MZF_READ(&twoside, sizeof(int));
			node->m_bTwoSided = twoside ? true : false;
		}

		if (m_phd_t->ver > EXPORTER_MESH_VER4) {
			int additive = 0;
			MZF_READ(&additive, sizeof(int));
			node->m_bAdditive = additive ? true : false;
		}

		if (m_phd_t->ver > EXPORTER_MESH_VER7)
		{
			int alpha_test = 0;
			MZF_READ(&alpha_test, sizeof(int));
			node->m_bAlphaTestMap = alpha_test ? true : false;
			node->m_nAlphaTestValue = alpha_test;
		}

		if (node->m_name[0]) {
			int	 len = strlen(node->m_name);
			char _temp[5];

			strncpy(_temp, &node->m_name[len - 4], 4);

			_temp[4] = 0;

			if (stricmp(_temp, ".tga") == 0) {
				node->m_bDiffuseMap = true;
			}

			if (node->m_opa_name[0]) {
				node->m_bAlphaMap = true;
				node->m_bDiffuseMap = false;
			}
		}

		if (node->m_bAlphaTestMap) {
			node->m_bAlphaMap = false;
			node->m_bDiffuseMap = false;
		}

		node->CheckAniTexture();

		m_mtrl_list_ex.Add(node);
	}

	bool bNeedScaleMat = false;
	D3DXMATRIX smat;

	for (i = 0; i < m_phd_t->mesh_num; i++) {
		bNeedScaleMat = false;

		RMeshNode* pMeshNode = new RMeshNode;

		D3DXMatrixIdentity(&pMeshNode->m_mat_base);

		pMeshNode->m_id = m_data_num;
		pMeshNode->m_pParentMesh = this;
		pMeshNode->m_pBaseMesh = this;

		char Name[256];
		Name[0] = NULL;

		MZF_READ(Name, MAX_NAME_LEN);
		MZF_READ(pMeshNode->m_Parent, MAX_NAME_LEN);
		MZF_READ(&pMeshNode->m_mat_base, sizeof(D3DXMATRIX));

#ifdef _DEBUG
		char name[256];
		sprintf(name, Name);
		TrimStr(name, name);
#endif
		pMeshNode->SetName(Name);

		pMeshNode->m_mat_ref = pMeshNode->m_mat_base;
		D3DXMatrixInverse(&pMeshNode->m_mat_ref_inv, 0, &pMeshNode->m_mat_ref);

		if (m_phd_t->ver >= EXPORTER_MESH_VER2) {
			MZF_READ(&pMeshNode->m_ap_scale, sizeof(D3DXVECTOR3));
		}
		else {
			pMeshNode->m_ap_scale.x = 1.f;
			pMeshNode->m_ap_scale.y = 1.f;
			pMeshNode->m_ap_scale.z = 1.f;
		}

		if (m_phd_t->ver >= EXPORTER_MESH_VER4) {
			MZF_READ(&pMeshNode->m_axis_rot, sizeof(D3DXVECTOR3));
			MZF_READ(&pMeshNode->m_axis_rot_angle, sizeof(float));

			MZF_READ(&pMeshNode->m_axis_scale, sizeof(D3DXVECTOR3));
			MZF_READ(&pMeshNode->m_axis_scale_angle, sizeof(float));

			MZF_READ(&pMeshNode->m_mat_etc, sizeof(D3DXMATRIX));

			D3DXMATRIX scalemat;
			D3DXMATRIX scalepivot;
			D3DXMATRIX scalepivotinv;
			D3DXMATRIX flipmat;

			D3DXMatrixScaling(&scalemat, pMeshNode->m_ap_scale.x, pMeshNode->m_ap_scale.y, pMeshNode->m_ap_scale.z);
			D3DXMatrixRotationAxis(&scalepivot, &pMeshNode->m_axis_scale, pMeshNode->m_axis_scale_angle);
			D3DXMatrixInverse(&scalepivotinv, NULL, &scalepivot);

			D3DXMatrixIdentity(&flipmat);

			pMeshNode->m_mat_flip = flipmat;

			pMeshNode->m_mat_etc = scalepivotinv * scalemat * scalepivot;
		}
		else {
			D3DXMatrixIdentity(&pMeshNode->m_mat_etc);
			D3DXMatrixIdentity(&pMeshNode->m_mat_flip);
		}

		memcpy(&pMeshNode->m_mat_local, &pMeshNode->m_mat_base, sizeof(D3DXMATRIX));

		pMeshNode->m_mat_result = pMeshNode->m_mat_base;

		D3DXMatrixScaling(&pMeshNode->m_mat_scale, pMeshNode->m_ap_scale.x, pMeshNode->m_ap_scale.y, pMeshNode->m_ap_scale.z);

		RMatInv(pMeshNode->m_mat_inv, pMeshNode->m_mat_local);

		CheckNameToType(pMeshNode);

		MZF_READ(&pMeshNode->m_point_num, 4);

		if (pMeshNode->m_point_num) {
			pMeshNode->m_point_list = new D3DXVECTOR3[pMeshNode->m_point_num];
			memset(pMeshNode->m_point_list, 0, pMeshNode->m_point_num * sizeof(D3DXVECTOR3));

			MZF_READ(pMeshNode->m_point_list, sizeof(D3DXVECTOR3) * pMeshNode->m_point_num);

			pMeshNode->CalcLocalBBox();
		}

		MZF_READ(&pMeshNode->m_face_num, 4);

		if (pMeshNode->m_face_num) {
			pMeshNode->m_face_list = new RFaceInfo[pMeshNode->m_face_num];
			pMeshNode->m_face_normal_list = new RFaceNormalInfo[pMeshNode->m_face_num];

			memset(pMeshNode->m_face_list, 0, pMeshNode->m_face_num * sizeof(RFaceInfo));

			if (m_phd_t->ver >= EXPORTER_MESH_VER6) {
				MZF_READ(pMeshNode->m_face_list, sizeof(RFaceInfo) * pMeshNode->m_face_num);
				MZF_READ(pMeshNode->m_face_normal_list, sizeof(RFaceNormalInfo) * pMeshNode->m_face_num);
			}
			else if (m_phd_t->ver > EXPORTER_MESH_VER2) {
				MZF_READ(pMeshNode->m_face_list, sizeof(RFaceInfo) * pMeshNode->m_face_num);
			}
			else {
				RFaceInfoOld* pInfo = new RFaceInfoOld[pMeshNode->m_face_num];
				MZF_READ(pInfo, sizeof(RFaceInfoOld) * pMeshNode->m_face_num);

				ConvertOldFaceInfo(pMeshNode->m_face_list, pInfo, pMeshNode->m_face_num);

				delete[] pInfo;
			}
		}

		if (m_phd_t->ver >= EXPORTER_MESH_VER6) {
			MZF_READ(&pMeshNode->m_point_color_num, 4);

			if (pMeshNode->m_point_color_num) {
				pMeshNode->m_point_color_list = new D3DXVECTOR3[pMeshNode->m_point_color_num];
				MZF_READ(pMeshNode->m_point_color_list, sizeof(D3DXVECTOR3) * pMeshNode->m_point_color_num);
			}
		}

		if ((pMeshNode->m_point_num == 0) || (pMeshNode->m_face_num == 0)) {
			pMeshNode->m_isDummyMesh = true;
		}

		MZF_READ(&pMeshNode->m_mtrl_id, 4);

		MZF_READ(&pMeshNode->m_physique_num, 4);

		if (pMeshNode->m_physique_num) {
			m_isPhysiqueMesh = true;

			pMeshNode->m_physique = new RPhysiqueInfo[pMeshNode->m_physique_num];
			ZeroMemory(pMeshNode->m_physique, pMeshNode->m_physique_num * sizeof(RPhysiqueInfo));

			for (int j = 0; j < pMeshNode->m_physique_num; j++) {
				MZF_READ(&pMeshNode->m_physique[j], sizeof(RPhysiqueInfo));
			}
		}

		D3DXPLANE	plane;
		D3DXVECTOR3	vv[3];

		if (m_phd_t->ver < EXPORTER_MESH_VER6) {
			if (pMeshNode->m_face_num) {
				for (int a = 0; a < pMeshNode->m_face_num; a++) {
					vv[0] = pMeshNode->m_point_list[pMeshNode->m_face_list[a].m_point_index[0]];
					vv[1] = pMeshNode->m_point_list[pMeshNode->m_face_list[a].m_point_index[1]];
					vv[2] = pMeshNode->m_point_list[pMeshNode->m_face_list[a].m_point_index[2]];

					D3DXPlaneFromPoints(&plane, &vv[0], &vv[1], &vv[2]);
					D3DXPlaneNormalize(&plane, &plane);

					pMeshNode->m_face_normal_list[a].m_normal.x = plane.a;
					pMeshNode->m_face_normal_list[a].m_normal.y = plane.b;
					pMeshNode->m_face_normal_list[a].m_normal.z = plane.c;
				}
			}

			if (pMeshNode->m_point_num && pMeshNode->m_point_num)
			{
				D3DXVECTOR3* pPointNormal = new D3DXVECTOR3[pMeshNode->m_point_num];
				memset(pPointNormal, 0, sizeof(D3DXVECTOR3) * pMeshNode->m_point_num);

				for (k = 0; k < pMeshNode->m_face_num; k++) {
					for (j = 0; j < 3; j++) {
						pPointNormal[pMeshNode->m_face_list[k].m_point_index[j]] =
							pPointNormal[pMeshNode->m_face_list[k].m_point_index[j]] + pMeshNode->m_face_normal_list[k].m_normal;
					}
				}

				for (k = 0; k < pMeshNode->m_point_num; k++) {
					pPointNormal[k] = pPointNormal[k] / 3.f;
					D3DXVec3Normalize(&pPointNormal[k], &pPointNormal[k]);
				}

				for (k = 0; k < pMeshNode->m_face_num; k++) {
					for (j = 0; j < 3; j++) {
						pMeshNode->m_face_normal_list[k].m_pointnormal[j] = pPointNormal[pMeshNode->m_face_list[k].m_point_index[j]];
					}
				}

				delete[] pPointNormal;
			}
		}

		if (pMeshNode->m_point_color_num > 0 && pMeshNode->m_PartsType == eq_parts_chest)
			pMeshNode->m_isClothMeshNode = true;

		m_list.PushBack(pMeshNode);

		m_data.push_back(pMeshNode);
		m_data_num++;

		if (MAX_MESH_NODE_TABLE != (int)m_data.capacity())
		{
			mlog("m_data number is not quite right..! (%d)\n", (int)m_data.capacity());
		}
	}

	return true;

#undef MZF_READ
}

bool RMesh::ReadElu(const char* fname)
{
#define MZF_READ(x,y) { if(!mzf.Read((x),(y))) return false; }

	auto RMeshReadElu = MBeginProfile("RMesh::ReadElu");

	char Path[256];
	char Name[256];

	Path[0] = NULL;
	Name[0] = NULL;

	GetPath(fname, Path);

	int len = strlen(Path);

	if (strncmp(&fname[len], "ef_", 3) == 0) {
		m_bEffectSort = true;
		m_LitVertexModel = true;
	}
	else {
		m_mtrl_list_ex.SetObjectTexture(true);
	}

	SetFileName(fname);

	m_data_num = 0;

	MZFile mzf;

	if (g_pFileSystem) {
		if (!mzf.Open(fname, g_pFileSystem)) {
			if (!mzf.Open(fname)) {
				mlog("----------> in zip ( %s ) file not found!! \n ", fname);
				return false;
			}
		}
	}
	else {
		if (!mzf.Open(fname)) {
			mlog("----------> %s file not found!! \n ", fname);
			return false;
		}
	}

	ex_hd_t t_hd;

	MZF_READ(&t_hd, sizeof(ex_hd_t));

	if (t_hd.sig != EXPORTER_SIG) {
		mlog("%s elu file 파일 식별 실패.\n", fname);
		return false;
	}

	if (t_hd.mesh_num != -1 && t_hd.mtrl_num != -1) {
		if (!ReadOldElu(&mzf, &t_hd))
			return false;
	}
	else
	{
		ReadNewElu(&mzf, fname);
	}

	if (m_isCharacterMesh) {
		rmatrix _pbm;

		_pbm._11 = 0.f;
		_pbm._12 = 1.f;
		_pbm._13 = 0.f;
		_pbm._14 = 0.f;

		_pbm._21 = 0.1504f;
		_pbm._22 = -0.f;
		_pbm._23 = 0.9886f;
		_pbm._24 = 0.f;

		_pbm._31 = 0.9886f;
		_pbm._32 = 0.f;
		_pbm._33 = -0.1504f;
		_pbm._34 = 0.f;

		_pbm._41 = 9.0528f;
		_pbm._42 = 0.f;
		_pbm._43 = 9.8982f;
		_pbm._44 = 1.f;

		AddNode("eq_sunglass", "Bip01 Head", _pbm);
	}

	mzf.Close();

	ConnectMatrix();

	if (m_is_map_object) {
		ClearVoidMtrl();
	}

	if (m_mtrl_auto_load) {
		m_mtrl_list_ex.Restore(RGetDevice(), Path);
	}

	ConnectMtrl();

	if (m_bEffectSort) {
		m_list.sort(e_sort_str);

		RMeshNodeHashList_Iter it_obj = m_list.begin();

		int cnt = 0;

		while (it_obj != m_list.end()) {
			RMeshNode* pMeshNode = (*it_obj);

			m_data[cnt] = pMeshNode;
			pMeshNode->m_id = cnt;

			cnt++;
			it_obj++;
		}
	}

	CheckNodeAlphaMtrl();
	MakeAllNodeVertexBuffer();

	m_isMeshLoaded = true;

	MEndProfile(RMeshReadElu);

	return true;
}

bool RMesh::SaveElu(const char* fname)
{
	return true;
}

bool RMesh::AddNode(char* name, char* pname, rmatrix& base_mat)
{
	RMeshNode* pMeshNode = new RMeshNode;

	pMeshNode->m_id = m_data_num;
	pMeshNode->m_pParentMesh = this;
	pMeshNode->m_pBaseMesh = this;

	pMeshNode->SetName(name);
	strcpy(pMeshNode->m_Parent, pname);

	pMeshNode->m_mat_base = base_mat;
	pMeshNode->m_mat_ref = base_mat;
	pMeshNode->m_mat_result = base_mat;
	pMeshNode->m_mat_add = base_mat;

	pMeshNode->m_isAddMeshNode = true;

	D3DXMatrixInverse(&pMeshNode->m_mat_ref_inv, 0, &pMeshNode->m_mat_ref);

	m_list.PushBack(pMeshNode);
	m_data.push_back(pMeshNode);
	m_data_num++;

	CheckNameToType(pMeshNode);

	return true;
}

bool RMesh::DelNode(RMeshNode* data)
{
	return false;
}

void RMesh::MakeAllNodeVertexBuffer()
{
	RMeshNode* pMeshNode = NULL;
	DWORD flag = 0;

	bool isShader = RIsSupportVS();

	{
		for (int i = 0; i < m_data_num; ++i)
		{
			pMeshNode = m_data[i];

			if (pMeshNode->m_isDummyMesh) continue;
			if (!pMeshNode->m_point_num) continue;

			if (pMeshNode->m_physique && pMeshNode->m_point_color_num == 0 && isShader)
			{
				pMeshNode->MakeVSVertexBuffer();
			}

			if (pMeshNode->m_physique) {
				pMeshNode->MakeNodeBuffer(USE_VERTEX_SW);
			}
			else {
				if (RIsHardwareTNL())	flag = USE_VERTEX_HW | USE_VERTEX_SW;
				else 					flag = USE_VERTEX_SW;

				pMeshNode->MakeNodeBuffer(flag);
			}
		}
	}
}

void RMesh::ConnectMatrix()
{
	RMeshNode* pMeshNode = NULL;

	for (int i = 0; i < m_data_num; i++) {
		pMeshNode = m_data[i];

		if (pMeshNode == NULL) continue;

		if (pMeshNode->m_Parent[0] != 0) {
			CalcLocalMatrix(pMeshNode);
		}
		int t_p_num, ret, j;

		if (pMeshNode->m_physique_num) {
			for (int q = 0; q < pMeshNode->m_physique_num; q++) {
				t_p_num = pMeshNode->m_physique[q].m_num;

				if (t_p_num > MAX_PHYSIQUE_KEY)
					t_p_num = MAX_PHYSIQUE_KEY;

				for (j = 0; j < t_p_num; j++) {
					ret = _FindMeshId(&pMeshNode->m_physique[q].m_parent_name[j][0]);

					if (ret != -1) {
						pMeshNode->m_physique[q].m_parent_id[j] = ret;
					}
					else {
						mlog("ReadElu() : physique %d %d %s not found !!\n", i, j, &pMeshNode->m_physique[q].m_parent_name[j][0]);
					}
				}
			}
		}

		if (m_isNPCMesh)
		{
			int id = _FindMeshId(pMeshNode->m_Parent);

			RMeshNode* pPN = NULL;

			if (id != -1)
				pPN = m_data[id];

			if (pPN) {
				if (!stricmp(pPN->GetName(), "Bip01 L Hand") || !stricmp(pPN->GetName(), "Bip01 R Hand"))
					pMeshNode->m_bNpcWeaponMeshNode = true;
			}
		}
	}
}

bool RMesh::CalcLocalMatrix(RMeshNode* pNode)
{
	if (!pNode)
		return false;

	D3DXMATRIX dest, inv;

	D3DXMatrixIdentity(&inv);

	RMeshNode* pPN = NULL;

	int id = _FindMeshId(pNode->m_Parent);

	if (id != -1) {
		pPN = m_data[id];

		if (pPN) {
			pNode->m_pParent = pPN;

			D3DXMatrixInverse(&inv, NULL, &pPN->m_mat_base);
			D3DXMatrixMultiply(&pNode->m_mat_local, &pNode->m_mat_base, &inv);

			return true;
		}
	}

	if (pNode->m_Parent[0])
		mlog("%s 노드의 %s 부모 못 찾음\n", pNode->m_Name, pNode->m_Parent);

	return false;
}

bool RMesh::ConnectPhysiqueParent(RMeshNode* pNode)
{
	int p_num, ret, j;

	if (!pNode)		return false;

	if (pNode->m_physique_num == 0)
		return true;

	if (pNode->m_bConnectPhysique)
		return true;

	for (int i = 0; i < pNode->m_physique_num; i++) {
		p_num = pNode->m_physique[i].m_num;

		if (p_num > MAX_PHYSIQUE_KEY)
			p_num = MAX_PHYSIQUE_KEY;

		for (j = 0; j < p_num; j++) {
			ret = _FindMeshId(&pNode->m_physique[i].m_parent_name[j][0]);

			if (ret != -1) {
				pNode->m_physique[i].m_parent_id[j] = ret;
			}
			else {
				mlog("ReadElu() : physique %d %d %s not found !!\n", i, j, &pNode->m_physique[i].m_parent_name[j][0]);
			}
		}
	}

	pNode->m_bConnectPhysique = true;

	if (RIsSupportVS())
		if (pNode->m_point_color_num == 0)
			pNode->MakeVSVertexBuffer();

	return true;
}

void RMesh::CheckNodeAlphaMtrl() {
	RMeshNodeHashList_Iter it_obj = m_list.begin();

	RMeshNode* pMeshNode = NULL;
	RMtrlMgr* pMtrlList = NULL;

	RMtrl* pMtrl, * pSMtrl;

	while (it_obj != m_list.end()) {
		pMeshNode = (*it_obj);

		if (pMeshNode->m_mtrl_id != -1) {
			if (m_mtrl_list_ex.GetNum()) {
				if (pMtrl = m_mtrl_list_ex.Get_s(pMeshNode->m_mtrl_id, -1)) {
					if (pMtrl->m_sub_mtrl_num) {
						for (int i = 0; i < pMtrl->m_sub_mtrl_num; i++) {
							pSMtrl = m_mtrl_list_ex.Get_s(pMeshNode->m_mtrl_id, i);

							if (pSMtrl->m_bAlphaMap || pSMtrl->m_bDiffuseMap) {
								pMeshNode->m_isAlphaMtrl = true;
								break;
							}
						}
					}
					else {
						if (pMtrl->m_bAlphaMap || pMtrl->m_bDiffuseMap)
							pMeshNode->m_isAlphaMtrl = true;
					}
				}
			}
		}

		it_obj++;
		continue;
	}
}

void RMesh::ClearVoidMtrl()
{
	RMeshNode* pMeshNode = NULL;
	RMtrl* pMtrl = NULL;
	RMtrl* pSMtrl = NULL;

	int mtrl_size = m_mtrl_list_ex.size();

	if (!mtrl_size)
		return;

	m_mtrl_list_ex.ClearUsedCheck();

	RMeshNodeHashList_Iter it_obj = m_list.begin();

	while (it_obj != m_list.end()) {
		pMeshNode = (*it_obj);

		if (pMeshNode) {
			pMtrl = m_mtrl_list_ex.Get_s(pMeshNode->m_mtrl_id, -1);

			if (pMtrl) {
				pMtrl->m_bUse = true;

				if (pMtrl->m_sub_mtrl_num) {
					for (int i = 0; i < pMtrl->m_sub_mtrl_num; i++) {
						pSMtrl = m_mtrl_list_ex.Get_s(pMeshNode->m_mtrl_id, i);

						if (pSMtrl) {
							pSMtrl->m_bUse = true;
						}
					}
				}
			}
		}
		it_obj++;
	}

	m_mtrl_list_ex.ClearUsedMtrl();

	mtrl_size = m_mtrl_list_ex.size();
}

_NAMESPACE_REALSPACE2_END