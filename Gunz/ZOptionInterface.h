#pragma once

#include "RTypes.h"

class ZOptionInterface
{
	int		mOldScreenWidth;
	int		mOldScreenHeight;

	RPIXELFORMAT	mnOldBpp;

	bool	mbTimer;
	DWORD	mTimerTime;

public:
	ZOptionInterface(void);
	virtual ~ZOptionInterface(void);

	void Update(void);

	void InitInterfaceOption(void);
	bool SaveInterfaceOption(void);

	void OptimizationVideoOption();

	void Resize(int w, int h);
	bool ResizeWidgetRecursive(MWidget* pWidget);
	bool ResizeWidget(const char* szName, int w, int h);
	void AdjustMultipliedWidgetsManually();
	void ResizeDefaultFont(int newScreenHeight);

	void ShowResizeConfirmDialog(bool Resized);
	bool IsDiffScreenResolution();
	bool TestScreenResolution();
	void GetOldScreenResolution();
	bool SetTimer(bool b = false, float time = 0.0f);

	void ShowNetworkPortConfirmDialog();
	bool IsDiffNetworkPort();

	void OnActionKeySet(ZActionKey* pActionKey, int key);
};

DECLARE_LISTENER(ZGetOptionGammaSliderChangeListener)
DECLARE_LISTENER(ZGetLoadDefaultKeySettingListener)
DECLARE_LISTENER(ZSetOptimizationListener)
DECLARE_LISTENER(ZGetSaveOptionButtonListener)
DECLARE_LISTENER(ZGetCancelOptionButtonListener)
DECLARE_LISTENER(ZGetGammaOption)
DECLARE_LISTENER(ZGetCancelResizeConfirmListener)
DECLARE_LISTENER(ZGetRequestResizeListener)
DECLARE_LISTENER(ZGetNetworkPortChangeRestartListener)
DECLARE_LISTENER(ZGetNetworkPortChangeCancelListener)
DECLARE_LISTENER(ZGetViewConfirmCancelListener)
DECLARE_LISTENER(ZGetViewConfrimAcceptListener)
DECLARE_LISTENER(ZGet8BitSoundListener)
DECLARE_LISTENER(ZGet16BitSoundListener)
DECLARE_LISTENER(ZGetMouseSensitivitySliderListener)
DECLARE_LISTENER(ZGetMouseSensitivityEditListener)
