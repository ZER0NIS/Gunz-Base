#pragma once

#include "../sdk/BandiCaptureLibrary/Include/bandicap.h"

// 동영상 캡쳐...2008.10.02
class ZBandiCapturer
{
public:
	ZBandiCapturer() : m_bInited(false), m_hHandle(0), m_pDevice(NULL) {}
	~ZBandiCapturer() {}

	bool Init(HWND nHwnd, LPDIRECT3DDEVICE9 pDevice);

	bool Start();
	void Stop();
	void ToggleStart();
	void DrawCapture(MDrawContext* pDC);
	bool IsCapturing();
	void SetResolution(int iResolution);
	int GetResolution();

private:
	HWND					m_hHandle;
	LPDIRECT3DDEVICE9		m_pDevice;
	bool					m_bInited;
	CBandiCaptureLibrary	m_bandiCaptureLibrary;
	TCHAR					m_pathName[MAX_PATH];
	int						m_iResolution;
	long					m_lFileSize;
	float					m_fToggleFileSize;

	void OnDrawCapture(MDrawContext* pDC);
	BCAP_CONFIG GetConfig();
	void GetFolder(TCHAR* szoutFolder);

	int get_file_length( char *fn ); // 파일용량 가져오기(네이버 발췌)
};

void SetBandiCaptureConfig(int iResolution);
void SetBandiCaptureFileSize(int iFileSize);