#pragma once

#include <SDL.h>
#include <SDL_syswm.h>

#define RSDL2 1

_NAMESPACE_REALSPACE2_BEGIN

#if RSDL2
extern SDL_Window* g_pWindow;
//extern SDL_Renderer* g_pRenderer;
extern SDL_Event  g_pEvent;
#endif

void RFrame_Render();
void RFrame_UpdateRender();
void RFrame_Invalidate();
void RFrame_Restore();

int RMain(const char* AppName, HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow, RMODEPARAMS* pModeParams, WNDPROC winproc = NULL, WORD nIconResID = NULL);
int RInitD3D(RMODEPARAMS* pModeParams);
int RenderLoop();

void RFrame_ToggleFullScreen();

_NAMESPACE_REALSPACE2_END

bool IsToolTipEnable();
