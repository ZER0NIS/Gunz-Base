#include "GlobalTypes.h"
#include "MUtil.h"
#include <windows.h>
#include <mmsystem.h>

#pragma comment( lib, "winmm.lib" )

typedef BOOL (*MMTIMERCALLBACK)(DWORD);

class MMTimer
{
public:
    MMTimer();
    ~MMTimer();
	
	// Multimedia Timer
    BOOL Create(UINT nPeriod, UINT nRes, DWORD dwUser,  MMTIMERCALLBACK pfnCallback);
	void Destroy();

protected:
    static void CALLBACK TimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

    MMTIMERCALLBACK m_pfnCallback;

    DWORD m_dwUser;
    UINT m_nPeriod;
    UINT m_nRes;
    UINT m_nIDTimer;
};
