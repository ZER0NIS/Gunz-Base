// types for realspace 2 . 2001-10-4 created.
#pragma once

#include <string>
#include <list>
#include "GlobalTypes.h"
#include "RNameSpace.h"
#include "rvector.h"
#include "rquaternion.h"

#include "d3dx9math.h"

using namespace std;

_NAMESPACE_REALSPACE2_BEGIN

enum rsign { NEGATIVE= -1, ZERO= 0, POSITIVE= 1 };

#define RPIXELFORMAT D3DFORMAT


enum RQUERYFEATURETYPE {
	RQF_HARDWARETNL = 0,
	RQF_USERCLIPPLANE,
	RQF_WFOG,
	RQF_VS11,
	RQF_VS20,
	RQF_PS10,
	RQF_PS11,
	RQF_PS20,
	RQF_PS30,
	RQF_R32F,
	RQF_A32B32G32R32F,
	RQF_A16B16G16R16F,
	RQF_R16F,
	RQF_RGB16,
	RQF_G16R16F,
	RQF_G32R32F,
	RQF_VERTEXTEXTURE,				// VS30의 Vertex Texture 기능이 지원 되는가
	RQF_HWSHADOWMAP,				// 새도우맵에서 하드웨어 PCF (Percentage Closer Filtering) 샘플링 지원
	RQF_MRTINDEPENDENTBITDEPTHS,	// 일부 카드에서 R32F 포멧에 대해 POST PIXEL SHADER BLENDING(디더링, 알파 테스트, 포그(안개:fog), 블렌드, 래스터 처리, 마스킹)이 안먹힘. 그에대한 지원 유무 http://telnet.or.kr/sec_directx/index.html?init_mode=api_contents_read&api_no=60
	// 텍스쳐 필터 지원 여부
	// To check if a format supports texture filter types other than D3DTEXF_POINT (which is always supported), call IDirect3D9::CheckDeviceFormat with D3DUSAGE_QUERY_FILTER.
	RQF_RGB16_RTF,
	RQF_R32F_RTF,
	RQF_A8R8G8B8_RTF,					// RFMT_A8R8G8B8
	RQF_A32B32G32R32F_RTF,
	RQF_A16B16G16R16F_RTF,
	RQF_R16F_RTF,
	RQF_G32R32F_RTF,
	RQF_MRTBLEND_R32F,
	RQF_MRTBLEND_G16R16F,
	RQF_MRTBLEND_A8R8G8B8,
};

enum RRESULT {
	R_UNKNOWN = -1,
	R_OK = 0,
	R_NOTREADY = 1,
	R_RESTORED = 2,

	R_ERROR_LOADING = 1000,
	
	
};


struct RMODEPARAMS {
	int nWidth,nHeight;
	bool bFullScreen;
	RPIXELFORMAT PixelFormat;
};

#define RM_FLAG_ADDITIVE		0x0001
#define RM_FLAG_USEOPACITY		0x0002
#define RM_FLAG_TWOSIDED		0x0004
#define RM_FLAG_NOTWALKABLE		0x0008		
#define RM_FLAG_CASTSHADOW		0x0010
#define RM_FLAG_RECEIVESHADOW	0x0020
#define RM_FLAG_PASSTHROUGH		0x0040		
#define RM_FLAG_HIDE			0x0080		
#define RM_FLAG_PASSBULLET		0x0100		
#define RM_FLAG_PASSROCKET		0x0200		
#define RM_FLAG_USEALPHATEST	0x0400
#define RM_FLAG_AI_NAVIGATION	0x1000


#define rvector D3DXVECTOR3
#define rmatrix D3DXMATRIX
#define rplane D3DXPLANE

#define rvector2 D3DXVECTOR2

struct rboundingbox
{
	union {
	struct {
		float minx,miny,minz,maxx,maxy,maxz;
	};
	struct {
		rvector vmin,vmax;
	};
	float m[2][3];
	};

	rvector Point(int i) const { return rvector( (i&1)?vmin.x:vmax.x, (i&2)?vmin.y:vmax.y, (i&4)?vmin.z:vmax.z );  }
	
	void Add(const rvector &kPoint)
	{
		if ( vmin.x > kPoint.x )	vmin.x = kPoint.x;
		if ( vmin.y > kPoint.y )	vmin.y = kPoint.y;
		if ( vmin.z > kPoint.z )	vmin.z = kPoint.z;
		if ( vmax.x < kPoint.x )	vmax.x = kPoint.x;
		if ( vmax.y < kPoint.y )	vmax.y = kPoint.y;
		if ( vmax.z < kPoint.z )	vmax.z = kPoint.z;
	}

	rboundingbox() {}
};

/*
struct rplueckercoord {
public:
	rplueckercoord() {}
	rplueckercoord(const rvector &origin,const rvector &target) 
	{	u=origin-target;CrossProduct(&v,origin,target); }
	rvector u,v;
	inline friend float operator * (const rplueckercoord& p1, const rplueckercoord& p2) 
	{ return DotProduct(p1.u,p2.v)+DotProduct(p1.v,p2.u); }
};
*/

// º¤ÅÍ

inline float Magnitude(const rvector &x) { return D3DXVec3Length(&x); }
inline float MagnitudeSq(const rvector &x)	{ return D3DXVec3LengthSq(&x); }
inline void Normalize(rvector &x) { D3DXVec3Normalize(&x,&x);}
inline float DotProduct(const rvector &a,const rvector &b) { return D3DXVec3Dot(&a,&b); }
inline void CrossProduct(rvector *result,const rvector &a,const rvector &b) { D3DXVec3Cross(result,&a,&b); }

//void SetPlane(rplane& plane, rvector& point1, rvector& point2, rvector& point3);

// Çà·Ä

void MakeWorldMatrix(rmatrix *pOut,rvector pos,rvector dir,rvector up);			// el ¸ðµ¨ÀÇ world matrix ¸¦ ¸¸µç´Ù.



// help Æã¼Çµé..
#ifndef TOLER
#define TOLER 0.001
#endif
#define IS_ZERO(a) ((fabs((double)(a)) < (double) TOLER))
#define IS_EQ(a,b) ((fabs((double)(a)-(b)) >= (double) TOLER) ? 0 : 1)
#define IS_EQ3(a,b) (IS_EQ((a).x,(b).x)&&IS_EQ((a).y,(b).y)&&IS_EQ((a).z,(b).z))
#define SIGNOF(a) ( (a)<-TOLER ? NEGATIVE : (a)>TOLER ? POSITIVE : ZERO )
#define RANDOMFLOAT ((float)rand()/(float)RAND_MAX)

// ÇÑÁ¡¿¡¼­ Á÷¼±±îÁöÀÇ °Å¸®.. line1,line2 ´Â Á÷¼±À§ÀÇ µÎ Á¡.
float GetDistance(const rvector &position,const rvector &line1,const rvector &line2);
// ÇÑÁ¡¿¡¼­ °¡Àå °¡±î¿î ¼±ºÐÀ§ÀÇ Á¡
rvector GetNearestPoint(const rvector &position,const rvector &a,const rvector &b);
// ÇÑÁ¡¿¡¼­ ¼±ºÐ±îÁöÀÇ °Å¸®
float GetDistanceLineSegment(const rvector &position,const rvector &a,const rvector &b);
// ¼±ºÐ°ú ¼±ºÐ »çÀÌÀÇ °Å¸®.. ¼±ºÐ (a,aa) °ú ¼±ºÐ (c,cc)ÀÇ °Å¸®.
float GetDistanceBetweenLineSegment(const rvector &a,const rvector &aa,const rvector &c,const rvector &cc,rvector *ap,rvector *cp);
// ÇÑÁ¡¿¡¼­ Æò¸é±îÁöÀÇ °Å¸®
float GetDistance(const rvector &position,const rplane &plane);
// ¼±ºÐ(a,aa) ¿¡¼­ Æò¸é±îÁöÀÇ °¡Àå °¡±î¿î ¼±ºÐÀ§ÀÇ Á¡.
rvector GetNearestPoint(const rvector &a,const rvector &aa,const rplane &plane);
// ¼±ºÐ(a,aa) ¿¡¼­ Æò¸é±îÁöÀÇ °Å¸®
float GetDistance(const rvector &a,const rvector &aa,const rplane &plane);
// Æò¸é¿¡¼­ boundingbox¿ÍÀÇ ÃÖ´ë°Å¸®
float GetDistance(rboundingbox *bb,rplane *plane);
// Æò¸é¿¡¼­ boundingbox¿ÍÀÇ ÃÖ¼Ò,ÃÖ´ë°Å¸®
void GetDistanceMinMax(rboundingbox &bb,rplane &plane,float *MinDist,float *MaxDist);
// ÇÑÁ¡°ú boundingboxÀÇ ÃÖ¼Ò°Å¸®
float GetDistance(const rboundingbox &bb,const rvector &point);
// »ï°¢ÇüÀÇ ¸éÀû
float GetArea(rvector &v1,rvector &v2,rvector &v3);

// µÎ º¤ÅÍÀÇ x, y»ó¿¡¼­ÀÇ °¢µµ
float GetAngleOfVectors(rvector &ta,rvector &tb);

// ¿øÇüº¸°£µÈ vector.. a,b´Â normalized µÇ¾îÀÖ¾î¾ßÇÔ.
rvector InterpolatedVector(rvector &a,rvector &b,float x);

bool IsIntersect(rboundingbox *bb1,rboundingbox *bb2);
bool isInPlane(rboundingbox *bb,rplane *plane);
bool IsInSphere(const rboundingbox &bb,const rvector &point,float radius);
bool isInViewFrustum(const rvector &point,rplane *plane);
bool isInViewFrustum(const rvector &point,float radius,rplane *plane);		// bounding sphere
bool isInViewFrustum(rboundingbox *bb,rplane *plane);
bool isInViewFrustum(const rvector &point1,const rvector &point2,rplane *planes);	// ¼±ºÐ
bool isInViewFrustumWithZ(rboundingbox *bb,rplane *plane);
bool isInViewFrustumwrtnPlanes(rboundingbox *bb,rplane *plane,int nplane);

bool IsIntersect( const rvector& orig, const rvector& dir, rvector& v0, rvector& v1, rvector& v2, float* t);
bool isLineIntersectBoundingBox(rvector &origin,rvector &dir,rboundingbox &bb);
bool IsIntersect( rvector& line_begin_, rvector& line_end_, rboundingbox& box_);
bool IsIntersect(rvector& line_begin_, rvector& line_dir_, rvector& center_, float radius_, float* dist = NULL, rvector* p = NULL );

// ¿ø°ú ¼±ºÐÀÇ ±³Â÷Á¡ ±¸ÇÏ´Â ÇÔ¼ö. dir´Â normalizeµÇ¾î ÀÖ¾î¾ß ÇÑ´Ù
bool IsIntersect(const rvector& orig, const rvector& dir, const rvector& center, const float radius, rvector* p = NULL);

// µÎ Æò¸éÀ» Áö³ª´Â Á÷¼±ÀÇ ¹æÁ¤½ÄÀ» ±¸ÇÑ´Ù 
bool GetIntersectionOfTwoPlanes(rvector *pOutDir,rvector *pOutAPoint,rplane &plane1,rplane &plane2);

void MergeBoundingBox(rboundingbox *dest,rboundingbox *src);

// aabb box ¸¦ Æ®·£½ºÆû ÇÑ´Ù. ´õ Ä¿Áø´Ù
void TransformBox( rboundingbox* result, const rboundingbox& src, const rmatrix& matrix );


// º¯È¯ ¸ÅÅ©·Îµé

#define FLOAT2RGB24(r, g, b) ( ( ((long)((r) * 255)) << 16) | (((long)((g) * 255)) << 8) | (long)((b) * 255))
#define VECTOR2RGB24(v)		FLOAT2RGB24((v).x,(v).y,(v).z)
#define BYTE2RGB24(r,g,b)	((DWORD) (((BYTE) (b)|((WORD) (g) << 8))|(((DWORD) (BYTE) (r)) << 16)))
#define BYTE2RGB32(a,r,g,b)	((DWORD) (((BYTE) (b)|((WORD) (g) << 8))|(((DWORD) (BYTE) (r)) << 16)|(((DWORD) (BYTE) (a)) << 24)))
#define DWORD2VECTOR(x)		rvector(float(((x)& 0xff0000) >> 16)/255.f, float(((x) & 0xff00) >> 8)/255.f,float(((x) & 0xff))/255.f)

typedef void (*RFPROGRESSCALLBACK)(void *pUserParams,float fProgress);

_NAMESPACE_REALSPACE2_END