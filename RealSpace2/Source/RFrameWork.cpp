#include "stdafx.h"
#include <windows.h>
#include "MDebug.h"
#include "RealSpace2.h"
#include "RParticleSystem.h"
#include "RFont.h"
#include "RMeshUtil.h"

#pragma comment(lib,"winmm.lib")

#define RTOOLTIP_GAP 700
static DWORD g_last_mouse_move_time = 0;
static bool g_tool_tip = false;

bool IsToolTipEnable() {
	return g_tool_tip;
}

_NAMESPACE_REALSPACE2_BEGIN

extern HWND g_hWnd;

bool g_bActive;
extern bool g_bFixedResolution;

RECT g_rcWindowBounds;
WNDPROC	g_WinProc = NULL;
RFFUNCTION g_pFunctions[RF_ENDOFRFUNCTIONTYPE] = { NULL, };
extern int gNumTrisRendered;

#ifdef _USE_GDIPLUS
#include "unknwn.h"
#include "gdiplus.h"

Gdiplus::GdiplusStartupInput	g_gdiplusStartupInput;
ULONG_PTR 						g_gdiplusToken = NULL;
#endif

void RSetFunction(RFUNCTIONTYPE ft, RFFUNCTION pfunc)
{
	g_pFunctions[ft] = pfunc;
}

bool RIsActive()
{
	return GetActiveWindow() == g_hWnd;
}

void RFrame_Create()
{
#ifdef _USE_GDIPLUS
	Gdiplus::GdiplusStartup(&g_gdiplusToken, &g_gdiplusStartupInput, NULL);
#endif
	GetWindowRect(g_hWnd, &g_rcWindowBounds);
}

void RFrame_Init()
{
}

void RFrame_Restore()
{
	RParticleSystem::Restore();
	if (g_pFunctions[RF_RESTORE])
		g_pFunctions[RF_RESTORE](NULL);
}

void RFrame_Destroy()
{
	if (g_pFunctions[RF_DESTROY])
		g_pFunctions[RF_DESTROY](NULL);

	mlog("Rframe_destory::closeDisplay\n");
	RCloseDisplay();

#ifdef _USE_GDIPLUS
	Gdiplus::GdiplusShutdown(g_gdiplusToken);
#endif
}

void RFrame_Invalidate()
{
	RParticleSystem::Invalidate();
	if (g_pFunctions[RF_INVALIDATE])
		g_pFunctions[RF_INVALIDATE](NULL);
}

void RFrame_Update()
{
	if (g_pFunctions[RF_UPDATE])
		g_pFunctions[RF_UPDATE](NULL);
}

void RFrame_Render()
{
	if (!RIsActive() && RIsFullScreen()) return;

	RRESULT isOK = RIsReadyToRender();
	if (isOK == R_NOTREADY)
		return;
	else
		if (isOK == R_RESTORED)
		{
			RMODEPARAMS ModeParams = { RGetScreenWidth(),RGetScreenHeight(),RIsFullScreen(),RGetPixelFormat() };
			RResetDevice(&ModeParams);
			mlog("devices Restored. \n");
		}

	if (timeGetTime() > g_last_mouse_move_time + RTOOLTIP_GAP)
		g_tool_tip = true;

	if (g_pFunctions[RF_RENDER])
		g_pFunctions[RF_RENDER](NULL);

	RGetDevice()->SetStreamSource(0, NULL, 0, 0);
	RGetDevice()->SetIndices(0);
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetTexture(1, NULL);
}

void RFrame_ToggleFullScreen()
{
	RMODEPARAMS ModeParams = { RGetScreenWidth(),RGetScreenHeight(),RIsFullScreen(),RGetPixelFormat() };

	if (!ModeParams.bFullScreen)
		GetWindowRect(g_hWnd, &g_rcWindowBounds);

	ModeParams.bFullScreen = !ModeParams.bFullScreen;
	RResetDevice(&ModeParams);

	if (!ModeParams.bFullScreen)
	{
		SetWindowPos(g_hWnd, HWND_NOTOPMOST,
			g_rcWindowBounds.left, g_rcWindowBounds.top,
			(g_rcWindowBounds.right - g_rcWindowBounds.left),
			(g_rcWindowBounds.bottom - g_rcWindowBounds.top),
			SWP_SHOWWINDOW);
	}
}

long FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
#ifndef _PUBLISH
	case WM_SYSCHAR:
		if (wParam == VK_RETURN)
			RFrame_ToggleFullScreen();
		return 0;
#endif
	case WM_SYSCOMMAND:
	{
		switch (wParam)
		{
		case SC_PREVWINDOW:
		case SC_NEXTWINDOW:
		case SC_KEYMENU:
		{
			return 0;
		}
		break;
		}
	}
	break;

	case WM_ACTIVATEAPP:
	{
		if (wParam == TRUE) {
			if (g_pFunctions[RF_ACTIVATE])
				g_pFunctions[RF_ACTIVATE](NULL);
			g_bActive = TRUE;
		}
		else {
			if (g_pFunctions[RF_DEACTIVATE])
				g_pFunctions[RF_DEACTIVATE](NULL);

			if (RIsFullScreen()) {
				ShowWindow(hWnd, SW_MINIMIZE);
				UpdateWindow(hWnd);
			}
			g_bActive = FALSE;
		}
	}
	break;

	case WM_MOUSEMOVE:
	{
		g_last_mouse_move_time = timeGetTime();
		g_tool_tip = false;
	}
	break;

	case WM_CLOSE:
	{
		RFrame_Destroy();
		PostQuitMessage(0);
		return 0;
	}
	break;
	}
	return g_WinProc(hWnd, message, wParam, lParam);
}

#ifndef _PUBLISH

#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);

#else

#define __BP(i,n) ;
#define __EP(i) ;

#endif

int RMain(const char* AppName, HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow, RMODEPARAMS* pModeParams, WNDPROC winproc, WORD nIconResID)
{
	g_WinProc = winproc ? winproc : DefWindowProc;

	WNDCLASS    wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(DWORD);
	wc.hInstance = this_inst;
	wc.hIcon = LoadIcon(this_inst, MAKEINTRESOURCE(nIconResID));
	wc.hCursor = 0;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "RealSpace2";
	if (!RegisterClass(&wc)) return FALSE;

	DWORD dwStyle = pModeParams->bFullScreen ? WS_POPUP | WS_SYSMENU : WS_POPUP | WS_CAPTION | WS_SYSMENU;
	g_hWnd = CreateWindow("RealSpace2", AppName, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT,
		pModeParams->nWidth, pModeParams->nHeight, NULL, NULL, this_inst, NULL);

	RAdjustWindow(pModeParams);

	while (ShowCursor(FALSE) > 0);

	return 0;
}

void RFrame_UpdateRender()
{
	__BP(5006, "RMain::Run");

	RFrame_Update();
	RFrame_Render();

	__BP(5007, "RMain::RFlip");
	RFlip();
	__EP(5007);

	__EP(5006);
}

int RRun()
{
	if (g_pFunctions[RF_CREATE])
	{
		if (g_pFunctions[RF_CREATE](NULL) != R_OK)
		{
			RFrame_Destroy();
			return -1;
		}
	}

	RFrame_Init();

	BOOL bGotMsg;
	MSG  msg;
	do
	{
		bGotMsg = PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE);
		if (bGotMsg)
		{
			if (msg.message <= WM_USER + 25)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			RFrame_UpdateRender();
		}

		if (!g_bActive)
			Sleep(10);
		}while (WM_QUIT != msg.message);
		return (INT)msg.wParam;
	}

int RInitD3D(RMODEPARAMS* pModeParams)
{
	RFrame_Create();

	ShowWindow(g_hWnd, SW_SHOW);
	if (!RInitDisplay(g_hWnd, pModeParams))
	{
		mlog("can't init display\n");
		return -1;
	}

	RGetDevice()->Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0);
	RFlip();

	return 0;
}

_NAMESPACE_REALSPACE2_END