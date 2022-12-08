#ifndef RFRAMEWORK_H
#define RFRAMEWORK_H

_NAMESPACE_REALSPACE2_BEGIN

void RFrame_Render();
void RFrame_UpdateRender();
void RFrame_Invalidate();
void RFrame_Restore();

void RSetFunction(RFUNCTIONTYPE ft, RFFUNCTION pfunc);

int RMain(const char* AppName, HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow, RMODEPARAMS* pModeParams, WNDPROC winproc = NULL, WORD nIconResID = NULL);
int RInitD3D(RMODEPARAMS* pModeParams);
int RenderLoop();

void RFrame_ToggleFullScreen();

_NAMESPACE_REALSPACE2_END

bool IsToolTipEnable();

#endif