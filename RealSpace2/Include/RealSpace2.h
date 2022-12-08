#ifndef __REALSPACE2_H
#define __REALSPACE2_H

#include "GlobalTypes.h"
#include "RMath.h"

#include "RTypes.h"
#include "RError.h"
#include "MZFileSystem.h"

#pragma comment(lib,"d3dx9.lib")

#include "RNameSpace.h"
_NAMESPACE_REALSPACE2_BEGIN

class RParticleSystem;

bool	RIsActive();
bool	RIsQuery();
void	RSetQuery(bool b);
bool	RIsFullScreen();
bool	RIsHardwareTNL();
bool	RIsSupportVS();
bool	RIsAvailUserClipPlane();
bool	RIsTrilinear();
int		RGetApproxVMem();
int		RGetScreenWidth();
int		RGetScreenHeight();
int     RGetIsWidthScreen();
int		RGetIs16x9();
float   RGetWidthScreen();
inline float RGetAspect() { return float(RGetScreenWidth()) / RGetScreenHeight(); }
int		RGetPicmip();
RPIXELFORMAT RGetPixelFormat();
D3DADAPTER_IDENTIFIER9* RGetAdapterID();
void SetClearColor(DWORD c);
int		RGetVidioMemory();
void	RSetWBuffer(bool bEnable);
bool IsDynamicResourceLoad();

int RGetAdapterModeCount(D3DFORMAT Format, UINT Adapter = D3DADAPTER_DEFAULT);
bool REnumAdapterMode(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode);

#if FPSOLD
extern int g_nFrameCount, g_nLastFrameCount;
extern float g_fFPS;
#else
extern double fpsCounterLastValue;
extern u32 fpsCounterLastCount;

namespace GlobalTimeFPS
{
	void Wait();

	const double GetWorkFrametime();
	const double GetElapsedFrametime();
	const double GetFrametime();
	const double GetRealtime();
};

#endif

extern int g_nFrameLimitValue;
extern HWND	g_hWnd;
extern MZFileSystem* g_pFileSystem;

bool RInitDisplay(HWND hWnd, const RMODEPARAMS* params);
bool RCloseDisplay();
void RSetFileSystem(MZFileSystem* pFileSystem);
void RAdjustWindow(const RMODEPARAMS* pModeParams);

LPDIRECT3DDEVICE9	RGetDevice();

void ChangeAA(int AALevel);
void RResetDevice(const RMODEPARAMS* params);
RRESULT RIsReadyToRender();
bool RFlip();
bool REndScene();
bool RBeginScene();

enum RRENDER_FLAGS
{
	RRENDER_CLEAR_BACKBUFFER = 0x000001,
};

void RSetRenderFlags(unsigned long nFlags);
unsigned long RGetRenderFlags();

void RSetFog(bool bFog, float fNear = 0, float fFar = 0, DWORD dwColor = 0xFFFFFFFF);
bool RGetFog();
float RGetFogNear();
float RGetFogFar();
DWORD RGetFogColor();

extern rvector RCameraPosition, RCameraDirection, RCameraUp;
extern rmatrix RView, RProjection, RViewProjection, RViewport, RViewProjectionViewport;

void RSetCamera(const rvector& from, const rvector& at, const rvector& up);
void RUpdateCamera();

void RSetProjection(float fFov, float fNearZ, float fFarZ);
void RSetProjection(float fFov, float fAspect, float fNearZ, float fFarZ);

inline rplane* RGetViewFrustum();
void RSetViewport(int x1, int y1, int x2, int y2);
D3DVIEWPORT9* RGetViewport();

void RResetTransform();

extern D3DPRESENT_PARAMETERS g_d3dpp;

bool RGetScreenLine(int sx, int sy, rvector* pos, rvector* dir);

rvector RGetIntersection(int x, int y, rplane& plane);
bool RGetIntersection(rvector& a, rvector& b, rplane& plane, rvector* pIntersection);

rvector RGetTransformCoord(rvector& coord);

void RDrawLine(rvector& v1, rvector& v2, DWORD dwColor);
void RDrawCylinder(rvector origin, float fRadius, float fHeight, int nSegment);
void RDrawCorn(rvector center, rvector pole, float fRadius, int nSegment);
void RDrawSphere(rvector origin, float fRadius, int nSegment);
void RDrawAxis(rvector origin, float fSize);
void RDrawCircle(rvector origin, float fRadius, int nSegment);
void RDrawArc(rvector origin, float fRadius, float fAngle1, float fAngle2, int nSegment, DWORD color);

bool RSaveAsBmp(int x, int y, void* data, const char* szFilename);

bool RScreenShot(int x, int y, void* data, const char* szFilename);

LPDIRECT3DSURFACE9 RCreateImageSurface(const char* filename);

void RSetGammaRamp(unsigned short nGammaValue = 255);
void RSetFrameLimitPerSeceond(unsigned short nFrameLimit = 0);
inline int RGetFrameLimitPerSeceond() { return g_nFrameLimitValue; }

void RSetWBuffer(bool bEnable);

enum RFUNCTIONTYPE {
	RF_CREATE = 0,
	RF_DESTROY,
	RF_RENDER,
	RF_UPDATE,
	RF_INVALIDATE,
	RF_RESTORE,
	RF_ACTIVATE,
	RF_DEACTIVATE,
	RF_ERROR,

	RF_ENDOFRFUNCTIONTYPE
};

typedef RRESULT(*RFFUNCTION)(void* Params);

extern rplane RViewFrustum[6];
inline rplane* RGetViewFrustum()
{
	return RViewFrustum;
}

bool			QueryFeature(RQUERYFEATURETYPE feature);
HRESULT        CheckResourceFormat(D3DFORMAT fmt, D3DRESOURCETYPE resType, DWORD dwUsage);

_NAMESPACE_REALSPACE2_END

#endif