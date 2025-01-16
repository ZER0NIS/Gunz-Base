#include "stdafx.h"
#include "RealSpace2.h"
#include "MDebug.h"
#include "RParticleSystem.h"
#include "RFrameWork.h"
#include "RBaseTexture.h"
#include "RMeshUtil.h"
#include "RFont.h"
#include "DxErr.h"

int nSettingAA = 2;
_NAMESPACE_REALSPACE2_BEGIN

#pragma comment ( lib, "d3d9.lib" )

bool DynamicResourceLoading = true;

bool IsDynamicResourceLoad() {
	return DynamicResourceLoading;
}

extern RFFUNCTION g_pFunctions[RF_ENDOFRFUNCTIONTYPE];

bool g_bHardwareTNL, g_bFullScreen, g_bSupportVS, g_bAvailUserClipPlane;
int g_nScreenWidth, g_nScreenHeight, g_nPicmip;
RPIXELFORMAT		g_PixelFormat;
LPDIRECT3D9			g_pD3D = NULL;
LPDIRECT3DDEVICE9	g_pd3dDevice = NULL;
D3DADAPTER_IDENTIFIER9	g_DeviceID;
D3DPRESENT_PARAMETERS	g_d3dpp;
HWND					g_hWnd;
MZFileSystem* g_pFileSystem = NULL;
RParticleSystem			g_ParticleSystem;
HMODULE					g_hD3DLibrary = NULL;
D3DCAPS9		m_d3dcaps;
float g_fFogNear;
float g_fFogFar;
DWORD g_dwFogColor;
bool g_bFog = false;

int	g_nVidioMemory = 0;

int g_nFrameCount = 0, g_nLastFrameCount = 0;
float g_fFPS = 0;
int g_nFrameLimitValue = 0;
DWORD g_dwLastTime = timeGetTime();
DWORD g_dwLastFPSTime = timeGetTime();

bool		g_bTrilinear = true;
const bool	bTripleBuffer = false;
bool		g_bQuery = false;

D3DMULTISAMPLE_TYPE	g_MultiSample = D3DMULTISAMPLE_NONE;
void RSetQuery(bool b) { g_bQuery = b; }
bool RIsQuery() { return g_bQuery; }
bool RIsFullScreen() { return g_bFullScreen; }
bool RIsHardwareTNL() { return g_bHardwareTNL; }
bool RIsSupportVS() { return g_bSupportVS; }
bool RIsTrilinear() { return g_bTrilinear; }
int	 RGetApproxVMem() { if (g_pd3dDevice == 0) return 0; return g_pd3dDevice->GetAvailableTextureMem() * 0.5f; }
bool RIsAvailUserClipPlane() { return g_bAvailUserClipPlane; }
int RGetScreenWidth() { return g_nScreenWidth; }
int RGetScreenHeight() { return g_nScreenHeight; }
int RGetIsWidthScreen() { return (float(g_nScreenHeight) / float(g_nScreenWidth) == 0.625f) ? 1 : 0; }
int RGetIs16x9() { return (float(g_nScreenHeight) / float(g_nScreenWidth) == 0.5625f) ? 1 : 0; }
float RGetWidthScreen() { return float(g_nScreenHeight) / float(g_nScreenWidth); }

int RGetPicmip() { return g_nPicmip; }
RPIXELFORMAT RGetPixelFormat() { return g_PixelFormat; }

LPDIRECT3DDEVICE9	RGetDevice() { return g_pd3dDevice; }

bool CheckVideoAdapterSupported();

unsigned long g_rsnRenderFlags = RRENDER_CLEAR_BACKBUFFER;
void RSetRenderFlags(unsigned long nFlags)
{
	g_rsnRenderFlags = nFlags;
}
unsigned long RGetRenderFlags() { return g_rsnRenderFlags; }

int RGetVidioMemory() {
	return g_nVidioMemory;
}

HRESULT RError(int nErrCode)
{
	RSetError(nErrCode);
	if (g_pFunctions[RF_ERROR])
		return g_pFunctions[RF_ERROR](NULL);

	return R_OK;
}

bool CreateDirect3D9()
{
	if (!g_pD3D)
	{
		g_hD3DLibrary = LoadLibrary("d3d9.dll");

		if (!g_hD3DLibrary)
		{
			mlog("Error, could not load d3d9.dll");
			return false;
		}

		typedef IDirect3D9* (__stdcall* D3DCREATETYPE)(UINT);
		D3DCREATETYPE d3dCreate = (D3DCREATETYPE)GetProcAddress(g_hD3DLibrary, "Direct3DCreate9");

		if (!d3dCreate)
		{
			mlog("Error, could not get proc adress of Direct3DCreate9.");
			return false;
		}

		g_pD3D = (*d3dCreate)(D3D_SDK_VERSION);

		if (!g_pD3D)
		{
			mlog("Error initializing D3D.");
			return false;
		}
	}

	g_pD3D->GetAdapterIdentifier(0, 0, &g_DeviceID);

	return true;
}

D3DADAPTER_IDENTIFIER9* RGetAdapterID()
{
	if (!g_pD3D) _ASSERT(0);
	return &g_DeviceID;
}

BOOL IsCompressedTextureFormatOk(D3DFORMAT TextureFormat)
{
	HRESULT hr = g_pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		g_PixelFormat,
		0,
		D3DRTYPE_TEXTURE,
		TextureFormat);

	return SUCCEEDED(hr);
}

void RSetFileSystem(MZFileSystem* pFileSystem) { g_pFileSystem = pFileSystem; }

bool QueryFeature(RQUERYFEATURETYPE feature)
{
	switch (feature)
	{
	case RQF_HARDWARETNL: return (m_d3dcaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0;
	case RQF_USERCLIPPLANE: return (m_d3dcaps.MaxUserClipPlanes > 0);
	case RQF_VS11: return (m_d3dcaps.VertexShaderVersion >= D3DVS_VERSION(1, 1));
	case RQF_VS20: return (m_d3dcaps.VertexShaderVersion >= D3DVS_VERSION(2, 0));
	case RQF_PS10: return (m_d3dcaps.PixelShaderVersion >= D3DPS_VERSION(1, 0));
	case RQF_PS20: return (m_d3dcaps.PixelShaderVersion >= D3DPS_VERSION(2, 0));
	case RQF_PS30: return (m_d3dcaps.PixelShaderVersion >= D3DPS_VERSION(3, 0));
	case RQF_R32F:
		return D3D_OK == CheckResourceFormat(D3DFMT_R32F, D3DRTYPE_TEXTURE, D3DUSAGE_RENDERTARGET);
	case RQF_A32B32G32R32F:
		return D3D_OK == CheckResourceFormat(D3DFMT_A32B32G32R32F, D3DRTYPE_TEXTURE, D3DUSAGE_RENDERTARGET);
	case RQF_A16B16G16R16F:
		return D3D_OK == CheckResourceFormat(D3DFMT_A16B16G16R16F, D3DRTYPE_TEXTURE, D3DUSAGE_RENDERTARGET);
	case RQF_R16F:
		return D3D_OK == CheckResourceFormat(D3DFMT_R16F, D3DRTYPE_TEXTURE, D3DUSAGE_RENDERTARGET);
	case RQF_RGB16:
		return D3D_OK == CheckResourceFormat(D3DFMT_R5G6B5, D3DRTYPE_TEXTURE, D3DUSAGE_RENDERTARGET);
	case RQF_G16R16F:
		return D3D_OK == CheckResourceFormat(D3DFMT_G16R16F, D3DRTYPE_TEXTURE, D3DUSAGE_RENDERTARGET);
	case RQF_G32R32F:
		return D3D_OK == CheckResourceFormat(D3DFMT_G32R32F, D3DRTYPE_TEXTURE, D3DUSAGE_RENDERTARGET);
	case RQF_VERTEXTEXTURE:
		return D3D_OK == CheckResourceFormat(D3DFMT_R32F, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_VERTEXTEXTURE);
	case RQF_HWSHADOWMAP:
		return SUCCEEDED(CheckResourceFormat(D3DFMT_D24S8, D3DRTYPE_TEXTURE, D3DUSAGE_DEPTHSTENCIL));
	case RQF_WFOG: return (m_d3dcaps.RasterCaps & D3DPRASTERCAPS_WFOG) != 0;
	case RQF_MRTINDEPENDENTBITDEPTHS: return (m_d3dcaps.PrimitiveMiscCaps & D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS) != 0;
	case RQF_RGB16_RTF:
		return  (S_OK == CheckResourceFormat(D3DFMT_R5G6B5, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_FILTER));
	case RQF_R32F_RTF:
		return (S_OK == CheckResourceFormat(D3DFMT_R32F, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_FILTER));
	case RQF_A8R8G8B8_RTF:
		return (S_OK == CheckResourceFormat(D3DFMT_A8R8G8B8, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_FILTER));
	case RQF_A32B32G32R32F_RTF:
		return (S_OK == CheckResourceFormat(D3DFMT_A32B32G32R32F, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_FILTER));
	case RQF_A16B16G16R16F_RTF:
		return (S_OK == CheckResourceFormat(D3DFMT_A16B16G16R16F, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_FILTER));
	case RQF_R16F_RTF:
		return (S_OK == CheckResourceFormat(D3DFMT_R16F, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_FILTER));
	case RQF_G32R32F_RTF:
		return (S_OK == CheckResourceFormat(D3DFMT_G32R32F, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_FILTER));
	case RQF_MRTBLEND_R32F:
		return (S_OK == CheckResourceFormat(D3DFMT_R32F, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING));
	case RQF_MRTBLEND_G16R16F:
		return (S_OK == CheckResourceFormat(D3DFMT_G16R16F, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING));
	case RQF_MRTBLEND_A8R8G8B8:
		return (S_OK == CheckResourceFormat(D3DFMT_A8R8G8B8, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING));

	default: _ASSERT(FALSE);
	}
	return false;
}

HRESULT CheckResourceFormat(D3DFORMAT fmt, D3DRESOURCETYPE resType, DWORD dwUsage)
{
	HRESULT hr = S_OK;
	IDirect3D9* tempD3D = NULL;
	RGetDevice()->GetDirect3D(&tempD3D);
	D3DCAPS9 devCaps;
	RGetDevice()->GetDeviceCaps(&devCaps);

	D3DDISPLAYMODE displayMode;
	tempD3D->GetAdapterDisplayMode(devCaps.AdapterOrdinal, &displayMode);

	hr = tempD3D->CheckDeviceFormat(devCaps.AdapterOrdinal, devCaps.DeviceType, displayMode.Format, dwUsage, resType, fmt);

	tempD3D->Release(), tempD3D = NULL;

	return hr;
}

static void InitDevice()
{
	g_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, g_bTrilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE);
	g_pd3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, g_bTrilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE);

	g_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);

	RSetWBuffer(true);

	float fMaxBias = -1.0f;

	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, *(unsigned long*)&fMaxBias);
	g_pd3dDevice->SetSamplerState(1, D3DSAMP_MIPMAPLODBIAS, *(unsigned long*)&fMaxBias);

	if (g_MultiSample)
		g_pd3dDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	else
		g_pd3dDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);

	g_nVidioMemory = g_pd3dDevice->GetAvailableTextureMem() / 2;

	mlog("Video memory %f \n", g_nVidioMemory / float(1024 * 1024));

	if (D3DERR_NOTAVAILABLE == g_pd3dDevice->CreateQuery(D3DQUERYTYPE_OCCLUSION, NULL))
		g_bQuery = false;
	else
		g_bQuery = true;

	RBeginScene();
}

void CheckMipFilter()
{
	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();

	pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	DWORD dwNumPasses;
	HRESULT hr = pd3dDevice->ValidateDevice(&dwNumPasses);
	if (hr == D3DERR_UNSUPPORTEDTEXTUREFILTER) {
		g_bTrilinear = false;
	}
}

D3DDISPLAYMODE g_d3ddm;

bool RInitDisplay(HWND hWnd, const RMODEPARAMS* params)
{
	if (CreateDirect3D9() == false)
	{
		RError(RERROR_CANNOT_CREATE_D3D);
		return false;
	}

	if (CheckVideoAdapterSupported() == false)
	{
		if (RError(RERROR_INVALID_DEVICE) != R_OK) return false;
	}

	g_pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &m_d3dcaps);

	g_bAvailUserClipPlane = (m_d3dcaps.MaxUserClipPlanes > 0) ? true : false;

	if (m_d3dcaps.RasterCaps & D3DPRASTERCAPS_WFOG) mlog("WFog Enabled Device.\n");

	if (QueryFeature(RQF_WFOG)) mlog("WFog Enabled Device.\n");

	g_bSupportVS = (m_d3dcaps.VertexShaderVersion >= D3DVS_VERSION(1, 1));

	if (!QueryFeature(RQF_VS11)) mlog("Vertex Shader isn't supported\n");
	else
	{
		if (m_d3dcaps.MaxVertexShaderConst < 250)
		{
			mlog("Too small Constant Number to use Hardware Skinning so Disable Vertex Shader\n");
			g_bSupportVS = false;
		}
	}

	HRESULT hr;
	hr = g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &g_d3ddm);

	g_bFullScreen = params->bFullScreen;
	g_nScreenWidth = params->nWidth;
	g_nScreenHeight = params->nHeight;
	g_PixelFormat = params->bFullScreen ? params->PixelFormat : g_d3ddm.Format;

	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferWidth = params->nWidth;
	g_d3dpp.BackBufferHeight = params->nHeight;
	g_d3dpp.BackBufferCount = bTripleBuffer ? 2 : 1;
	g_d3dpp.Windowed = !params->bFullScreen;
	g_d3dpp.BackBufferFormat = g_PixelFormat;
	g_d3dpp.EnableAutoDepthStencil = TRUE;

	if (nSettingAA == 0)
	{
		if (SUCCEEDED(g_pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_PixelFormat, params->bFullScreen, D3DMULTISAMPLE_4_SAMPLES, NULL))) {
			g_MultiSample = D3DMULTISAMPLE_4_SAMPLES;
		}
		else
		{
			g_MultiSample = D3DMULTISAMPLE_NONE;
		}
	}
	else if (nSettingAA == 1)
	{
		if (SUCCEEDED(g_pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_PixelFormat, params->bFullScreen, D3DMULTISAMPLE_2_SAMPLES, NULL))) {
			g_MultiSample = D3DMULTISAMPLE_2_SAMPLES;
		}
		else
		{
			g_MultiSample = D3DMULTISAMPLE_NONE;
		}
	}
	else
		g_MultiSample = D3DMULTISAMPLE_NONE;

	g_d3dpp.MultiSampleType = g_MultiSample;

	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	g_d3dpp.Flags = NULL;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	DWORD BehaviorFlags = D3DCREATE_FPU_PRESERVE |
		(QueryFeature(RQF_HARDWARETNL) ? D3DCREATE_HARDWARE_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING);

	D3DDEVTYPE d3dDevType = D3DDEVTYPE_HAL;

	if (IsDynamicResourceLoad())
		BehaviorFlags |= D3DCREATE_MULTITHREADED;

#ifndef _NVPERFHUD
	hr = g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, d3dDevType, hWnd, BehaviorFlags, &g_d3dpp, &g_pd3dDevice);
	if (FAILED(hr))
	{
		SAFE_RELEASE(g_pD3D);
		mlog("can't create device\n");
		return false;
	}
#else
	UINT AdapterToUse = D3DADAPTER_DEFAULT;
	D3DDEVTYPE DeviceType = D3DDEVTYPE_HAL;

	for (UINT Adapter = 0; Adapter < g_pD3D->GetAdapterCount(); Adapter++)
	{
		D3DADAPTER_IDENTIFIER9 Identifier;
		HRESULT Res = g_pD3D->GetAdapterIdentifier(Adapter, 0, &Identifier);
		if (strstr(Identifier.Description, "PerfHUD") != 0)
		{
			AdapterToUse = Adapter;
			DeviceType = D3DDEVTYPE_REF;
			break;
		}
	}
	if (FAILED(g_pD3D->CreateDevice(AdapterToUse, DeviceType, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice)))
	{
		mlog("can't create device\n");
		return false;
	}

#endif

	mlog("device created.\n");

	RSetViewport(0, 0, g_nScreenWidth, g_nScreenHeight);
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x000000, 1.0f, 0);

	CheckMipFilter();
	InitDevice();
	RFrame_Restore();
	RBaseTexture_Create();
	if (!RFontCreate())
	{
		RCloseDisplay();
		mlog("can't create font\n");
		return false;
	}

	g_hWnd = hWnd;

	return true;
}

bool RCloseDisplay()
{
	RFontDestroy();
	RBaseTexture_Destroy();
	RFrame_Invalidate();
	RBaseTexture_Invalidate();

	if (g_pd3dDevice)
	{
		g_pd3dDevice->EndScene();
		SAFE_RELEASE(g_pd3dDevice);
	}

	SAFE_RELEASE(g_pD3D);
	return true;
}

void RAdjustWindow(const RMODEPARAMS* pModeParams)
{
	if ((GetWindowLong(g_hWnd, GWL_STYLE) & WS_CHILD) != 0)
		return;

	if (pModeParams->bFullScreen)
	{
		SetWindowLong(g_hWnd, GWL_STYLE, WS_POPUP | WS_SYSMENU);

		RECT rt = { 0, 0, pModeParams->nWidth, pModeParams->nHeight };
		AdjustWindowRect(&rt, GetWindowLong(g_hWnd, GWL_STYLE), FALSE);
		SetWindowPos(g_hWnd, HWND_NOTOPMOST, 0, 0, rt.right - rt.left, rt.bottom - rt.top, SWP_SHOWWINDOW);
	}
	else
		SetWindowLong(g_hWnd, GWL_STYLE, WS_POPUP | WS_CAPTION | WS_SYSMENU);
}

void RResetDevice(const RMODEPARAMS* params)
{
	mlog("Reset Device \n");

	RFrame_Invalidate();
	RBaseTexture_Invalidate();

	g_bFullScreen = params->bFullScreen;
	g_nScreenWidth = params->nWidth;
	g_nScreenHeight = params->nHeight;
	g_PixelFormat = params->bFullScreen ? params->PixelFormat : g_d3ddm.Format;

	g_d3dpp.Windowed = !params->bFullScreen;
	g_d3dpp.BackBufferWidth = g_nScreenWidth;
	g_d3dpp.BackBufferHeight = g_nScreenHeight;
	g_d3dpp.BackBufferFormat = g_PixelFormat;

	if (nSettingAA == 0)
	{
		if (SUCCEEDED(g_pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_PixelFormat, params->bFullScreen, D3DMULTISAMPLE_4_SAMPLES, NULL))) {
			g_MultiSample = D3DMULTISAMPLE_4_SAMPLES;
		}
		else
		{
			g_MultiSample = D3DMULTISAMPLE_NONE;
		}
	}
	else if (nSettingAA == 1)
	{
		if (SUCCEEDED(g_pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_PixelFormat, params->bFullScreen, D3DMULTISAMPLE_2_SAMPLES, NULL))) {
			g_MultiSample = D3DMULTISAMPLE_2_SAMPLES;
		}
		else
		{
			g_MultiSample = D3DMULTISAMPLE_NONE;
		}
	}
	else
		g_MultiSample = D3DMULTISAMPLE_NONE;

	g_d3dpp.MultiSampleType = g_MultiSample;
	HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);

	_ASSERT(hr == D3D_OK);
	if (hr != D3D_OK) {
		mlog("device reset failed : %s\n", DXGetErrorString(hr));
		int* a = 0;
		*a = 1;
	}

	InitDevice();

	RAdjustWindow(params);

	ShowWindow(g_hWnd, SW_SHOW);
	UpdateWindow(g_hWnd);

	RSetViewport(0, 0, g_nScreenWidth, g_nScreenHeight);

	RUpdateCamera();

	RResetTransform();

	RBaseTexture_Restore();
	RFrame_Restore();
}

RRESULT RIsReadyToRender()
{
	if (!g_pd3dDevice) return R_NOTREADY;

	HRESULT hr;
	if (FAILED(hr = g_pd3dDevice->TestCooperativeLevel()))
	{
		if (D3DERR_DEVICELOST == hr)
			return R_NOTREADY;

		if (D3DERR_DEVICENOTRESET == hr)
			return R_RESTORED;
	}
	return R_OK;
}
#if OLDFPS

#define FPS_INTERVAL	1000

#endif

static DWORD g_clear_color = 0x00000000;

void SetClearColor(DWORD c) {
	g_clear_color = c;
}

bool REndScene()
{
	g_pd3dDevice->EndScene();
	return true;
}

bool RBeginScene()
{
	g_pd3dDevice->BeginScene();
	return true;
}

bool RFlip()
{
	REndScene();

	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

	// Custom: Replaced && to &, it's a bitwise flag. Stupid MAIET.
	// k, did not touch it now
	if (g_rsnRenderFlags & RRENDER_CLEAR_BACKBUFFER)
	{
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, g_clear_color, 1.0f, 0L);
	}
	else
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, g_clear_color, 1.0f, 0L);

	RBeginScene();

#if OLDFPS

	{
		g_nFrameCount++;
		DWORD currentTime = timeGetTime();

		{
			float fFrameLimit = 0;
			if (g_nFrameLimitValue > 0)
				fFrameLimit = 1000 / g_nFrameLimitValue;
			if ((currentTime - g_dwLastTime) < fFrameLimit)
			{
				Sleep((int)fFrameLimit - (currentTime - g_dwLastTime));
				currentTime = timeGetTime();
			}

			g_dwLastTime = currentTime;
		}

		if (g_dwLastFPSTime + FPS_INTERVAL < currentTime)
		{
			g_fFPS = (g_nFrameCount - g_nLastFrameCount) * FPS_INTERVAL / ((float)(currentTime - g_dwLastFPSTime) * (FPS_INTERVAL / 1000));
			g_dwLastFPSTime = currentTime;
			g_nLastFrameCount = g_nFrameCount;
		}
	}
#endif

	return true;
}

void RDrawLine(rvector& v1, rvector& v2, DWORD dwColor)
{
	struct LVERTEX {
		float x, y, z;
		DWORD color;
	};

	LVERTEX ver[2] = { {v1.x,v1.y,v1.z,dwColor},{v2.x,v2.y,v2.z,dwColor} };

	HRESULT hr = RGetDevice()->DrawPrimitiveUP(D3DPT_LINELIST, 1, ver, sizeof(LVERTEX));
}

rvector RGetTransformCoord(rvector& coord)
{
	rvector ret;
	D3DXVec3TransformCoord(&ret, &coord, &RViewProjectionViewport);
	return ret;
}

bool SaveMemoryBmp(int x, int y, void* data, void** retmemory, int* nsize)
{
	unsigned char* memory = NULL, * dest = NULL;

	if (!data) return false;

	int nBytesPerLine = (x * 3 + 3) / 4 * 4;
	int nMemoryNeed = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nBytesPerLine * y;
	memory = new unsigned char[nMemoryNeed];
	if (!memory) return false;

	*nsize = nMemoryNeed;
	*retmemory = memory;

	dest = memory;
	BITMAPFILEHEADER* pbmfh = (BITMAPFILEHEADER*)dest;
	dest += sizeof(BITMAPFILEHEADER);
	BITMAPINFOHEADER* pbmih = (BITMAPINFOHEADER*)dest;
	dest += sizeof(BITMAPINFOHEADER);

	pbmfh->bfType = 0x4D42;
	pbmfh->bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + x * y * 3;
	pbmfh->bfReserved1 = 0;
	pbmfh->bfReserved2 = 0;
	pbmfh->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	ZeroMemory(pbmih, sizeof(BITMAPINFOHEADER));
	pbmih->biSize = sizeof(BITMAPINFOHEADER);
	pbmih->biWidth = x;
	pbmih->biHeight = y;
	pbmih->biPlanes = 1;
	pbmih->biBitCount = 24;
	pbmih->biCompression = BI_RGB;
	pbmih->biSizeImage = 0;
	pbmih->biXPelsPerMeter = 0;
	pbmih->biYPelsPerMeter = 0;
	pbmih->biClrUsed = 0;
	pbmih->biClrImportant = 0;

	int i, j;

	DWORD* p = (DWORD*)data + (y - 1) * x;

	for (i = y - 1; i >= 0; i--)
	{
		for (j = 0; j < x; j++)
		{
			*dest++ = *((BYTE*)p + j * 4 + 0);
			*dest++ = *((BYTE*)p + j * 4 + 1);
			*dest++ = *((BYTE*)p + j * 4 + 2);
		}
		if (x * 3 % 4 != 0)
		{
			unsigned char zero[] = { 0,0,0,0 };
			memcpy(dest, zero, 4 - x * 3 % 4);
			dest += 4 - x * 3 % 4;
		}
		p -= x;
	}

	return true;
}

#ifdef _USE_GDIPLUS
#include "unknwn.h"
#include "gdiplus.h"
using namespace Gdiplus;

int GetCodecClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;
	UINT  size = 0;

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}
	}

	free(pImageCodecInfo);
	return -1;
}

bool RSaveAsJpeg(int x, int y, void* data, const char* szFilename)
{
	BitmapData bitmapData;
	bitmapData.Width = x,
		bitmapData.Height = y,
		bitmapData.Stride = 4 * bitmapData.Width;
	bitmapData.PixelFormat = PixelFormat32bppARGB;
	bitmapData.Scan0 = data;
	bitmapData.Reserved = NULL;

	Rect rect(0, 0, x, y);
	Bitmap bitmap(x, y, 4 * bitmapData.Width, PixelFormat32bppARGB, (BYTE*)data);
	bitmap.LockBits(&rect, ImageLockModeWrite | ImageLockModeUserInputBuf, PixelFormat32bppARGB, &bitmapData);
	bitmap.UnlockBits(&bitmapData);

	WCHAR wstrName[256];
	int nNameLen = strlen(szFilename) + 1;
	MultiByteToWideChar(CP_ACP, 0, szFilename, -1, wstrName, nNameLen - 1);
	wstrName[nNameLen - 1] = 0;

	CLSID  Clsid;
	int ret = GetCodecClsid(L"image/jpeg", &Clsid);;
	if (bitmap.Save(wstrName, &Clsid, NULL) == Ok)
		return true;
	else
		return false;
}
#endif

bool RSaveAsBmp(int x, int y, void* data, const char* szFilename)
{
	void* memory;
	int nSize;

	if (!SaveMemoryBmp(x, y, data, &memory, &nSize))
		return false;

	FILE* file;
	file = fopen(szFilename, "wb+");
	if (!file) return false;

	fwrite(memory, nSize, 1, file);
	fclose(file);

	delete memory;

	return true;
}

bool RScreenShot(int x, int y, void* data, const char* szFilename)
{
	char szFullFileName[_MAX_DIR];

#ifdef _USE_GDIPLUS
	sprintf(szFullFileName, "%s.jpg", szFilename);
	return RSaveAsJpeg(x, y, data, szFullFileName);
#else
	sprintf(szFullFileName, "%s.bmp", szFilename);
	return RSaveAsBmp(x, y, data, szFullFileName);
#endif
}

bool RGetScreenLine(int sx, int sy, rvector* pos, rvector* dir)
{
	rvector scrpoint = rvector((float)sx, (float)sy, 0.1f);

	rmatrix inv;
	float det;
	if (D3DXMatrixInverse(&inv, &det, &RViewProjectionViewport) == NULL)
		return false;

	rvector worldpoint;
	D3DXVec3TransformCoord(&worldpoint, &scrpoint, &inv);

	*pos = RCameraPosition;
	*dir = worldpoint - RCameraPosition;
	D3DXVec3Normalize(dir, dir);

	return true;
}

rvector RGetIntersection(int x, int y, rplane& plane)
{
	rvector scrpoint = rvector((float)x, (float)y, 0.1f);

	rmatrix inv;
	float det;
	D3DXMatrixInverse(&inv, &det, &RViewProjectionViewport);

	rvector worldpoint;
	D3DXVec3TransformCoord(&worldpoint, &scrpoint, &inv);

	rvector ret;
	D3DXPlaneIntersectLine(&ret, &plane, &worldpoint, &RCameraPosition);

	return ret;
}

bool RGetIntersection(rvector& a, rvector& b, rplane& plane, rvector* pIntersection)
{
	rvector d = b - a;

	float t = -(plane.a * d.x + plane.b * d.y + plane.c * d.z + plane.d)
		/ (plane.a * a.x + plane.b * a.y + plane.c * a.z);

	if (t < 0.0f || t > 1.0f)
	{
		return false;
	}
	else
	{
		*pIntersection = a + t * d;
	}
	return true;
}

LPDIRECT3DSURFACE9 RCreateImageSurface(const char* filename)
{
	char* buffer;
	MZFile mzf;

	if (g_pFileSystem)
	{
		if (!mzf.Open(filename, g_pFileSystem)) {
			if (!mzf.Open(filename))
				return false;
		}
	}
	else
	{
		if (!mzf.Open(filename))
			return false;
	}

	buffer = new char[mzf.GetLength()];
	mzf.Read(buffer, mzf.GetLength());

	LPDIRECT3DSURFACE9 pSurface = NULL;
	D3DXIMAGE_INFO info;

	RGetDevice()->CreateOffscreenPlainSurface(1, 1, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &pSurface, NULL);
	D3DXLoadSurfaceFromFileInMemory(pSurface, NULL, NULL, buffer, mzf.GetLength(), NULL, D3DX_FILTER_NONE, 0, &info);
	pSurface->Release();

	HRESULT hr;

	hr = RGetDevice()->CreateOffscreenPlainSurface(info.Width, info.Height, info.Format, D3DPOOL_SCRATCH, &pSurface, NULL);
	_ASSERT(hr == D3D_OK);
	hr = D3DXLoadSurfaceFromFileInMemory(pSurface, NULL, NULL, buffer, mzf.GetLength(), NULL, D3DX_FILTER_NONE, 0, &info);
	_ASSERT(hr == D3D_OK);

	delete buffer;
	mzf.Close();

	return pSurface;
}

void RSetGammaRamp(unsigned short nGammaValue)
{
	D3DCAPS9 caps;
	RGetDevice()->GetDeviceCaps(&caps);
	if (!(caps.Caps2 & D3DCAPS2_FULLSCREENGAMMA)) return;

	D3DGAMMARAMP gramp;

	RGetDevice()->GetGammaRamp(0, &gramp);

	for (int i = 0; i < 256; i++)
	{
		gramp.red[i] = ((i * nGammaValue) > 65535 ? 65535 : (i * nGammaValue));
		gramp.green[i] = ((i * nGammaValue) > 65535 ? 65535 : (i * nGammaValue));
		gramp.blue[i] = ((i * nGammaValue) > 65535 ? 65535 : (i * nGammaValue));
	}
	RGetDevice()->SetGammaRamp(0, D3DSGR_CALIBRATE, &gramp);
}

void RSetFrameLimitPerSeceond(unsigned short nFrameLimit)
{
	switch (nFrameLimit)
	{
	case 0: { g_nFrameLimitValue = 0; }	break;
	case 1: { g_nFrameLimitValue = 60; }	break;
	case 2: { g_nFrameLimitValue = 120; }	break;
	case 3: { g_nFrameLimitValue = 240; }	break;
	case 4: { g_nFrameLimitValue = 333; }	break;
	default: { g_nFrameLimitValue = 0; }	break;
	}
}

void RDrawCylinder(rvector origin, float fRadius, float fHeight, int nSegment)
{
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetTexture(1, NULL);
	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	for (int i = 0; i < nSegment; i++)
	{
		float fAngle = i * 2 * PI_FLOAT / (float)nSegment;
		float fAngle2 = (i + 1) * 2 * PI_FLOAT / (float)nSegment;

		rvector a = fRadius * rvector(cos(fAngle), sin(fAngle), 0) + origin;
		rvector b = fRadius * rvector(cos(fAngle2), sin(fAngle2), 0) + origin;

		RDrawLine(a + rvector(0, 0, fHeight), b + rvector(0, 0, fHeight), 0xffff0000);
		RDrawLine(a - rvector(0, 0, fHeight), b - rvector(0, 0, fHeight), 0xffff0000);

		RDrawLine(a + rvector(0, 0, fHeight), a - rvector(0, 0, fHeight), 0xffff0000);
	}
}

void RDrawCorn(rvector center, rvector pole, float fRadius, int nSegment)
{
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetTexture(1, NULL);
	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	rvector dir = pole - center;
	Normalize(dir);

	rvector up = DotProduct(dir, rvector(0, 0, 1)) > DotProduct(dir, rvector(0, 1, 0)) ?
		rvector(0, 1, 0) : rvector(0, 0, 1);

	rvector x, y;
	CrossProduct(&x, up, dir);
	Normalize(x);
	CrossProduct(&y, x, dir);
	Normalize(y);

	for (int i = 0; i < nSegment; i++)
	{
		float fAngle = i * 2 * PI_FLOAT / (float)nSegment;
		float fAngle2 = (i + 1) * 2 * PI_FLOAT / (float)nSegment;

		rvector a = fRadius * (x * cos(fAngle) + y * sin(fAngle)) + center;
		rvector b = fRadius * (x * cos(fAngle2) + y * sin(fAngle2)) + center;

		RDrawLine(a, pole, 0xffff0000);
		RDrawLine(a, b, 0xffff0000);
	}
}

void RDrawSphere(rvector origin, float fRadius, int nSegment)
{
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetTexture(1, NULL);
	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	for (int i = 0; i < nSegment; i++)
	{
		float fAngleZ = i * 2 * PI_FLOAT / (float)nSegment;
		float fAngleZ2 = (i + 1) * 2 * PI_FLOAT / (float)nSegment;
		for (int j = 0; j < nSegment; j++)
		{
			float fAngle = j * 2 * PI_FLOAT / (float)nSegment;
			float fAngle2 = (j + 1) * 2 * PI_FLOAT / (float)nSegment;

			rvector a = fRadius * rvector(cos(fAngle) * cos(fAngleZ), sin(fAngle) * cos(fAngleZ), sin(fAngleZ)) + origin;
			rvector b = fRadius * rvector(cos(fAngle2) * cos(fAngleZ), sin(fAngle2) * cos(fAngleZ), sin(fAngleZ)) + origin;

			RDrawLine(a, b, 0xffff0000);

			b = fRadius * rvector(cos(fAngle) * cos(fAngleZ2), sin(fAngle) * cos(fAngleZ2), sin(fAngleZ2)) + origin;

			RDrawLine(a, b, 0xffff0000);
		}
	}
}

void RDrawAxis(rvector origin, float fSize)
{
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetTexture(1, NULL);
	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	RDrawLine(origin, origin + rvector(fSize, 0, 0), 0xffff0000);
	RDrawLine(origin, origin + rvector(0, fSize, 0), 0xff00ff00);
	RDrawLine(origin, origin + rvector(0, 0, fSize), 0xff0000ff);
}

void RDrawCircle(rvector origin, float fRadius, int nSegment)
{
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetTexture(1, NULL);
	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	for (int i = 0; i < nSegment; i++)
	{
		float fAngle = i * 2 * PI_FLOAT / (float)nSegment;
		float fAngle2 = (i + 1) * 2 * PI_FLOAT / (float)nSegment;

		rvector a = fRadius * rvector(cos(fAngle), sin(fAngle), 0) + origin;
		rvector b = fRadius * rvector(cos(fAngle2), sin(fAngle2), 0) + origin;

		RDrawLine(a, b, 0xffff0000);
	}
}

void RDrawArc(rvector origin, float fRadius, float fAngle1, float fAngle2, int nSegment, DWORD color)
{
	float fAngle = fAngle2 - fAngle1;
	for (int i = 0; i < nSegment; i++)
	{
		float fAngleA = fAngle1 + (i * fAngle / (float)nSegment);
		float fAngleB = fAngle1 + ((i + 1) * fAngle / (float)nSegment);

		rvector a = fRadius * rvector(cos(fAngleA), sin(fAngleA), 0) + origin;
		rvector b = fRadius * rvector(cos(fAngleB), sin(fAngleB), 0) + origin;

		RDrawLine(a, b, color);
	}
}

void RSetWBuffer(bool bEnable)
{
	if (bEnable) {
		if (m_d3dcaps.RasterCaps & D3DPRASTERCAPS_WBUFFER) {
			RGetDevice()->SetRenderState(D3DRS_ZENABLE, D3DZB_USEW);
		}
		else {
			RGetDevice()->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		}
	}
	else {
		RGetDevice()->SetRenderState(D3DRS_ZENABLE, FALSE);
	}
}

int RGetAdapterModeCount(D3DFORMAT Format, UINT Adapter)
{
	return g_pD3D->GetAdapterModeCount(Adapter, Format);
}

bool REnumAdapterMode(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
{
	if (pMode == 0)
		return false;
	g_pD3D->EnumAdapterModes(Adapter, Format, Mode, pMode);
	return true;
}

void RSetFog(bool bFog, float fNear, float fFar, DWORD dwColor)
{
	g_bFog = bFog;
	g_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, g_bFog);
	if (bFog)
	{
		g_fFogNear = fNear;
		g_fFogFar = fFar;
		g_dwFogColor = dwColor;
		g_pd3dDevice->SetRenderState(D3DRS_FOGCOLOR, dwColor);
		g_pd3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
		g_pd3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD*)(&g_fFogNear));
		g_pd3dDevice->SetRenderState(D3DRS_FOGEND, *(DWORD*)(&g_fFogFar));
	}
}

bool RGetFog() { return g_bFog; }
float RGetFogNear() { return g_fFogNear; }
float RGetFogFar() { return g_fFogFar; }
DWORD RGetFogColor() { return g_dwFogColor; }

bool CheckVideoAdapterSupported()
{
	bool bSupported = true;

	D3DADAPTER_IDENTIFIER9* ai = RGetAdapterID();
	if (ai == NULL) return false;

	if (ai->VendorId == 0x8086)
	{
		if (ai->DeviceId == 0x7125)
			bSupported = false;
	}
	if (ai->VendorId == 0x121a) {
		bSupported = false;
	}

	return bSupported;
}

void ChangeAA(int AALevel)
{
	nSettingAA = AALevel;
}

_NAMESPACE_REALSPACE2_END