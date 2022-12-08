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
	RGetParticleSystem()->Restore();

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
	RGetParticleSystem()->Invalidate();

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
	else if (isOK == R_RESTORED)
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

	DWORD dwStyle;
	if (pModeParams->bFullScreen) // Fullscreen
		dwStyle = WS_VISIBLE | WS_POPUP;
	else//windowed
		dwStyle = WS_VISIBLE | WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;

	g_hWnd = CreateWindowA("RealSpace2", AppName, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT,
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

#if FPSOLD
int RenderLoop()
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
	} while (WM_QUIT != msg.message);
	return (INT)msg.wParam;
}

#else
double fpsCounterLastValue = 0.0;
u32 fpsCounterLastCount = 0;
static int RenderLoop()
{
	if (g_pFunctions[RF_CREATE])
	{
		if (g_pFunctions[RF_CREATE](NULL) != R_OK)
		{
			RFrame_Destroy();
			return -1;
		}
	}

	BOOL bGotMsg;
	MSG  msg;

	static double fpsCounterTime = 0.0;
	static u32 fpsCounterCount = 0;

	GlobalTimeFPS::Wait();
	double elapsedTime = 0.0;
	double currentTime = 0.0;
	static double accumulatedTime = 0.0;

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
			__BP(5006, "RMain::Run");

			accumulatedTime += elapsedTime;
			const u32 updateCount = static_cast<u32>(accumulatedTime / 17.0); // Calculate update count using fixed update time from (58 FPS)
			accumulatedTime -= static_cast<u32>(updateCount) * 17.0; // Remove acummulated time

			fpsCounterTime += elapsedTime;
			++fpsCounterCount;

			if (fpsCounterTime >= 1000.0)
			{
				fpsCounterLastValue = (fpsCounterTime / (double)fpsCounterCount);
				fpsCounterLastCount = fpsCounterCount;
				fpsCounterCount = 0;
				fpsCounterTime = 0.0;
			}
			if (updateCount > 0)
			{
				// Force Input 58FPS Fix
				//RFrame_UpdateInput();
			}

			RFrame_Update();
			RFrame_Render();

			GlobalTimeFPS::Wait();
			elapsedTime = GlobalTimeFPS::GetElapsedFrametime();
			currentTime = GlobalTimeFPS::GetFrametime();

			__EP(5006);

			MCheckProfileCount();
		}

		if (!g_bActive)
			Sleep(10);
	} while (WM_QUIT != msg.message);
	return (INT)msg.wParam;
}

#endif

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

#if !FPSOLD
namespace GlobalTimeFPS
{
	u32 MaxGameFPS = RGetFrameLimitPerSeceond() != 0 ? 1000 : RGetFrameLimitPerSeceond();
	u32 MaxBackgroundFPS = RGetFrameLimitPerSeceond() != 0 ? 1000 : RGetFrameLimitPerSeceond();

	double MinimumFPSTime = (1000.0 / static_cast<double>(MaxGameFPS));
	double MinimumBackgroundFPSTime = (1000.0 / static_cast<double>(MaxBackgroundFPS));

	std::chrono::steady_clock::time_point BaseTime = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point LastTime = std::chrono::steady_clock::now();
	double ElapsedTime = 0.0;
	double WorkTime = 0.0;
	bool LimitFPS = true;

	void Wait()
	{
		std::chrono::steady_clock::time_point lastTime = LastTime;
		LastTime = std::chrono::steady_clock::now();

		std::chrono::duration<double, std::milli> work_time = LastTime - lastTime;
		WorkTime = work_time.count();

		const bool active = true;
#if GZ_OPERATING_SYSTEM_TYPE == GZ_OSTYPE_MOBILE
		if (LimitFPS == true ||
			active == false)
#else
		if ((GetVerticalSync() == false && LimitFPS == true) ||
			active == false)
#endif
		{
			const double fpsTime = active == false ? MinimumBackgroundFPSTime : MinimumFPSTime;
			if (work_time.count() < fpsTime)
			{
				std::chrono::duration<double, std::milli> delta_ms(fpsTime - work_time.count());
				auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
				//std::this_thread::sleep_for(delta_ms_duration);
				LastTime = std::chrono::steady_clock::now();
			}
		}

		ElapsedTime = std::chrono::duration<double, std::milli>(LastTime - lastTime).count();
	}

	const double GetWorkFrametime()
	{
		return WorkTime;
	}

	const double GetElapsedFrametime()
	{
		return ElapsedTime;
	}

	const double GetFrametime()
	{
		return std::chrono::duration<double, std::milli>(LastTime - BaseTime).count();
	}

	const double GetRealtime()
	{
		const auto currentTime = std::chrono::steady_clock::now();
		const auto timeSinceStart = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - BaseTime);
		return double(timeSinceStart.count() / 1000000.0);
	}
};

#endif // FPSOLD
_NAMESPACE_REALSPACE2_END