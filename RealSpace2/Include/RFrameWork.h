#ifndef RFRAMEWORK_H
#define RFRAMEWORK_H

_NAMESPACE_REALSPACE2_BEGIN

// 렌더링 함수를 호출하기 위해 외부로 노출
void RFrame_Render();

// WM_ENTERIDLE 때 호출하기 위해 노출
void RFrame_UpdateRender();

// 파티클 / 기본폰트 등의 리소소는 framework 에 전역으로 있다.
// 이런 리소스를 invalidate/restore 해줘야한다.

void RSetFunction(RFUNCTIONTYPE ft,RFFUNCTION pfunc);
int RMain(const char *AppName, HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow, RMODEPARAMS *pModeParams, WNDPROC winproc=NULL, WORD nIconResID=NULL );
int RInitD3D( RMODEPARAMS* pModeParams );
int RRun();

void RFrame_Invalidate();
void RFrame_Restore();

void RFrame_ToggleFullScreen();

_NAMESPACE_REALSPACE2_END

bool IsToolTipEnable();

#endif