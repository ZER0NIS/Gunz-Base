#include "stdafx.h"

#include "RFrameWork.h"
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

#if RSDL2
SDL_Window* g_pWindow = nullptr;
//SDL_Renderer* g_pRenderer = nullptr;
SDL_Event  g_pEvent;

bool  Quit = false;
#endif

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
#if RSDL2
	return SDL_GetWindowFlags(g_pWindow) & SDL_WINDOW_INPUT_FOCUS;
#else
	return GetActiveWindow() == g_hWnd;
#endif
}

void RFrame_Create()
{
#ifdef _USE_GDIPLUS
	Gdiplus::GdiplusStartup(&g_gdiplusToken, &g_gdiplusStartupInput, NULL);
#endif

#if RSDL2
	SDL_GetWindowPosition(g_pWindow, reinterpret_cast<int*>(&g_rcWindowBounds.left), reinterpret_cast<int*>(&g_rcWindowBounds.top));
	SDL_GetWindowSize(g_pWindow, reinterpret_cast<int*>(&g_rcWindowBounds.right), reinterpret_cast<int*>(&g_rcWindowBounds.bottom));
#else
	GetWindowRect(g_hWnd, &g_rcWindowBounds);
#endif
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

#if RSDL2
	//	SDL_DestroyRenderer(g_pRenderer);
	SDL_DestroyWindow(g_pWindow);
	SDL_Quit();
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

#if RSDL2
	if (SDL_GetTicks() > g_last_mouse_move_time + RTOOLTIP_GAP)
		g_tool_tip = true;
#else
	if (timeGetTime() > g_last_mouse_move_time + RTOOLTIP_GAP)
		g_tool_tip = true;
#endif

	if (g_pFunctions[RF_RENDER])
		g_pFunctions[RF_RENDER](NULL);

#if RSDL2
	//SDL_RenderPresent(g_pRenderer);
#else
	RGetDevice()->SetStreamSource(0, NULL, 0, 0);
	RGetDevice()->SetIndices(0);
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetTexture(1, NULL);
#endif
}

void RFrame_ToggleFullScreen()
{
	RMODEPARAMS ModeParams = { RGetScreenWidth(),RGetScreenHeight(),RIsFullScreen(),RGetPixelFormat() };

	if (!ModeParams.bFullScreen)
#if RSDL2
		SDL_GetWindowPosition(g_pWindow, reinterpret_cast<int*>(&g_rcWindowBounds.left), reinterpret_cast<int*>(&g_rcWindowBounds.top));
#else
		GetWindowRect(g_hWnd, &g_rcWindowBounds);
#endif

	ModeParams.bFullScreen = !ModeParams.bFullScreen;
	RResetDevice(&ModeParams);

	if (!ModeParams.bFullScreen)
	{
#if RSDL2
		SDL_SetWindowFullscreen(g_pWindow, 0);
		SDL_SetWindowSize(g_pWindow, g_rcWindowBounds.right - g_rcWindowBounds.left, g_rcWindowBounds.bottom - g_rcWindowBounds.top);
		SDL_SetWindowPosition(g_pWindow, g_rcWindowBounds.left, g_rcWindowBounds.top);
#else
		SetWindowPos(g_hWnd, HWND_NOTOPMOST,
			g_rcWindowBounds.left, g_rcWindowBounds.top,
			(g_rcWindowBounds.right - g_rcWindowBounds.left),
			(g_rcWindowBounds.bottom - g_rcWindowBounds.top),
			SWP_SHOWWINDOW);
#endif
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
#if RSDL2
				SDL_MinimizeWindow(g_pWindow);
#else
				ShowWindow(hWnd, SW_MINIMIZE);
				UpdateWindow(hWnd);
#endif
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
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		MLog("SDL_Init failed: %s\n", SDL_GetError());
		return -1;
	}

	constexpr u32 extraWindowFlags = 0;

	g_pWindow = SDL_CreateWindow(AppName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, pModeParams->nWidth, pModeParams->nHeight, SDL_WINDOW_SHOWN | extraWindowFlags);
	if (!g_pWindow) {
		MLog("SDL_CreateWindow failed: %s\n", SDL_GetError());
		SDL_Quit();
		return -1;
	}
	/*
g_pRenderer = SDL_CreateRenderer(g_pWindow, -1, 0);
if (!g_pRenderer) {
	MLog("SDL_CreateRenderer failed: %s\n", SDL_GetError());
	SDL_DestroyWindow(g_pWindow);
	SDL_Quit();
	return -1;
}									  */

	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (SDL_GetWindowWMInfo(g_pWindow, &info) != SDL_TRUE) {
		MLog("SDL_GetWindowWMInfo failed: %s\n", SDL_GetError());
		//SDL_DestroyRenderer(g_pRenderer);
		SDL_DestroyWindow(g_pWindow);
		SDL_Quit();
		return false;
	}
	HWND hWnd = info.info.win.window;
	g_hWnd = hWnd;

	RAdjustWindow(pModeParams);

#if RSDL2
	//while (SDL_ShowCursor(FALSE) > 0);

	SDL_ShowCursor(true);
#else
	while (ShowCursor(FALSE) > 0);
#endif

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

void OnEvent(const SDL_Event* event)
{
	switch (event->type)
	{
	case SDL_SYSWMEVENT:
		if (event->syswm.msg->msg.win.msg == WM_SYSCHAR)
		{
			if (event->syswm.msg->msg.win.wParam == VK_RETURN)
			{
				RFrame_ToggleFullScreen();
			}
		}
		break;
	case SDL_APP_DIDENTERFOREGROUND:
		if (g_pFunctions[RF_ACTIVATE])
			g_pFunctions[RF_ACTIVATE](NULL);
		g_bActive = true;
		break;
	case SDL_APP_DIDENTERBACKGROUND:
		if (g_pFunctions[RF_DEACTIVATE])
			g_pFunctions[RF_DEACTIVATE](NULL);
		if (RIsFullScreen()) {
			SDL_MinimizeWindow(g_pWindow);
		}
		g_bActive = false;
		break;
	case SDL_MOUSEMOTION:
		g_last_mouse_move_time = SDL_GetTicks();
		g_tool_tip = false;
		break;
	case SDL_QUIT:
	{
		RFrame_Destroy();
		Quit = true;
	}
	break;
	}
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

constexpr double GameCycleTime = 1000.0 / 16.6; // 60 FPS

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
			const float updateTime = static_cast<float>(elapsedTime / GameCycleTime);
			const u32 updateCount = static_cast<u32>(updateTime); // Calculate update count using fixed update time from MU (25 FPS)
			accumulatedTime -= static_cast<double>(updateCount) * GameCycleTime; // Remove acummulated time

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

			if (!RFlip())
			{
				RIsReadyToRender();
			}
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				OnEvent(&event);
			}

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
/*
int RenderLoop()
{
	static double fpsCounterTime = 0.0;
	static u32 fpsCounterCount = 0;

	GlobalTimeFPS::Wait();
	double elapsedTime = 0.0;
	double currentTime = 0.0;
	static double accumulatedTime = 0.0;

	while (!Quit)
	{
		__BP(5006, "RMain::Run");

		accumulatedTime += elapsedTime;
		const float updateTime = static_cast<float>(elapsedTime / GameCycleTime);
		const u32 updateCount = static_cast<u32>(updateTime); // Calculate update count using fixed update time from MU (25 FPS)
		accumulatedTime -= static_cast<double>(updateCount) * GameCycleTime; // Remove acummulated time

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

		if (!RFlip())
		{
			RIsReadyToRender();
		}

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			OnEvent(&event);
		}

		GlobalTimeFPS::Wait();
		elapsedTime = GlobalTimeFPS::GetElapsedFrametime();
		currentTime = GlobalTimeFPS::GetFrametime();

		__EP(5006);

		MCheckProfileCount();
	}

	return true;
}
		 */

#endif

int RInitD3D(RMODEPARAMS* pModeParams)
{
	RFrame_Create();

#if RSDL2
	SDL_ShowWindow(g_pWindow);
#else
	ShowWindow(g_hWnd, SW_SHOW);
#endif

	if (!RInitDisplay(g_hWnd, pModeParams))
	{
		mlog("can't init display\n");
		return -1;
	}

	//RGetDevice()->Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0);
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