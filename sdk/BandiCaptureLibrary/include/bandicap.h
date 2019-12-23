////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// BandiCaptureLibrary 3.0
/// 
/// Copyright(C) 2008-2018 Bandicam.com, All rights reserved.
///
/// Author: denny
///
/// Visit http://www.bandicam.com for more information.
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _BANDICAP_H
#define _BANDICAP_H

#include <windows.h>
#include <unknwn.h>
#include <mmsystem.h>
#include <stdio.h>

#ifndef ASSERT
#	include "assert.h"
#	define ASSERT(x) assert(x)
#endif

#ifndef _INC_TCHAR
#	ifdef _UNICODE
#		define _T(x)		L##x
#		define _tcslen		wcslen
#		define _stprintf	swprintf
#		define _stprintf_s  swprintf_s
#	else
#		define _T(x)	x
#		define _tcslen		strlen
#		define _stprintf	sprintf
#		define _stprintf_s	sprintf_s
#	endif
#endif


#define _BCAP_MAKE_VER(a,b,c,d)	((a<<24)+(b<<16)+(c<<8)+d)
#define BCAP_VERSION	_BCAP_MAKE_VER(3,0,1,302)		// 3.0.1.302

#define FOURCC_MPEG			mmioFOURCC('M','P','E','G')		// MPEG1
#define FOURCC_MJPG			mmioFOURCC('M','J','P','G')		// MJPEG
#define FOURCC_MP4V			mmioFOURCC('M','P','4','V')		// MPEG4
#define FOURCC_ARGB			mmioFOURCC('A','R','G','B')		// 32bit Alpha DIB (ARGB)
#define FOURCC_RGB24		mmioFOURCC('R','G','B',' ')		// 24bit DIB (RGB)
#define FOURCC_YV12			mmioFOURCC('Y','V','1','2')		// YUV420 DIB (YUV)

#define FOURCC_H264			mmioFOURCC('H','2','6','4')		// H264 - CPU
#define FOURCC_H264_APP		mmioFOURCC('A','P','P',' ')		// H264 - AMD APP
#define FOURCC_H264_SYNC	mmioFOURCC('S','Y','N','C')		// H264 - Intel Quick Sync Video
#define FOURCC_H264_CUDA	mmioFOURCC('C','U','D','A')		// H264 - Nvidia CUDA
#define FOURCC_H264_NVENC	mmioFOURCC('N','V','E','N')		// H264 - Nvidia NVENC

#define FOURCC_AUTO			mmioFOURCC('A','U','T','O')		// The codec is automatically selected by GetSuggestedVideoCodec()

#define WAVETAG_AAC			0x00FF							// HE-AAC
#define WAVETAG_MP3			0x0055							// MP3
#define WAVETAG_MP2			0x0050							// MPEG-1 Layer II
#define WAVETAG_PCM			0x0001							// PCM
#define WAVETAG_NULL		0x0000							// No Audio

// Capture mode
enum 
{
	BCAP_MODE_GDI,				// GDI Screen Capture 
	BCAP_MODE_D3D9,				// Direct3D 9 Screen Capture 
	BCAP_MODE_D3D9_SCALE,		// Direct3D 9 Screen Capture (with rescale)
	BCAP_MODE_D3D9_SWAPCHAIN,	// Direct3D 9 Screen Capture (with IDirect3DSwapChain9)
	BCAP_MODE_D3D8,				// Direct3D 8 Screen Capture 
	BCAP_MODE_D3D10,			// Direct3D 10 Screen Capture 
	BCAP_MODE_GL,				// OpenGL Screen Capture 
	BCAP_MODE_DDRAW7,			// DirectDraw 7 Screen Capture 
	BCAP_MODE_D3D11,			// Direct3D 11 Screen Capture 
};


// Image file format
enum 
{
	BCAP_IMAGE_BMP,				// BMP
	BCAP_IMAGE_PNG,				// PNG
	BCAP_IMAGE_JPG,				// JPG
	BCAP_IMAGE_CLIPBOARD,		// Clipboard
};


// Capture config structure 
struct BCAP_CONFIG
{
	enum { F_AVI = 0, F_MP4 = 1 };
	enum { V_CBR, V_VBR };
	enum { A_CBR, A_VBR };

	// How to auto-complete recording
	enum { BY_NONE, BY_TIME, BY_SIZE };

	BCAP_CONFIG() { Default(); };
	void Default()
	{
		FileType				= F_MP4;

		VideoSizeW				= 0;
		VideoSizeH				= 0;
		VideoFPS				= 30.000;

		VideoCodec				= FOURCC_H264;
		VideoRateControl		= V_VBR;
		VideoKBitrate			= 5000;
		VideoQuality			= 80;
		VideoKeyframeInterval	= 150;

		AudioChannels			= 2;
		AudioSampleRate			= 48000;

		AudioCodec				= WAVETAG_AAC;
		AudioRateControl		= A_CBR;
		AudioKBitrate			= 192;
		AudioQuality			= 30;

		LogoPath[0]				= '\0';
		LogoAlpha				= 0;
		LogoPosX				= 50;
		LogoPosY				= 100;
		LogoSizeW				= 100;
		LogoSizeH				= 100;

		CaptureRect.left		= 0;
		CaptureRect.top			= 0;
		CaptureRect.right		= 0;
		CaptureRect.bottom		= 0;

		AutoCompleteType		= BY_NONE;
		AutoCompleteValue		= 0;

		IncludeCursor			= FALSE;
		AdjustAudioMixer		= TRUE;
		SaveAudioTracksAsWav	= FALSE;
	};

	// Video file format
	INT		FileType;				

	// Video settings
	INT		VideoSizeW;				
	INT		VideoSizeH;				
	FLOAT	VideoFPS;				

	DWORD	VideoCodec;				// FOURCC_MPEG, FOURCC_MJPG, FOURCC_MP4V
	INT		VideoRateControl;		// V_CBR, V_VBR 
	INT		VideoKBitrate;			// 10k ~ 50000k		
	INT		VideoQuality;			// 30 ~ 100			
	INT		VideoKeyframeInterval;	// 0 ~ 1000 frames
	
	// Audio settings
	INT		AudioChannels;			// 1: mono, 2:stereo 
	INT		AudioSampleRate;		// samplerate

	INT		AudioCodec;				// WAVETAG_MP2, WAVETAG_PCM, WAVETAG_NULL
	INT		AudioRateControl;		// A_CBR, A_VBR 
	INT		AudioKBitrate;			// 32k ~ 320k 
	INT		AudioQuality;			// 30 ~ 100 

	// Logo settings
	WCHAR	LogoPath[MAX_PATH];		// Path of logo file(32bit png)
	INT		LogoAlpha;				// Transparency of logo [0 ~ 100]%
	INT		LogoPosX;				// Horizontal location of logo [0 ~ 100]%
	INT		LogoPosY;				// Vertical location of logo [0 ~ 100]%
	INT		LogoSizeW;				// Horizontal width of logo [0 ~ 999]%
	INT		LogoSizeH;				// Vertical height of logo [0 ~ 999]%

	// Auto-complete recording settings
	INT		AutoCompleteType;		// How to auto-complete recording
	INT		AutoCompleteValue;		// auto-complete recording value (seconds or MB)

	// etc
	RECT	CaptureRect;			// Capture target rectange

	BOOL	IncludeCursor;			// Include the mouse cursor image
	BOOL	AdjustAudioMixer;		
	BOOL	SaveAudioTracksAsWav;	// Save audio tracks while recording (.wav)
};


// IBandiCapture Interface
struct IBandiCapture : public IUnknown
{
	STDMETHOD (Verify)(LPCSTR szID, LPCSTR szKey) PURE;				// Verify the license

	STDMETHOD_(INT, GetVersion)() PURE;								// Version number

	STDMETHOD (Start)(LPCWSTR pwszFilePath, HWND hParentWnd,
					INT nCaptureMode, LONG_PTR lpParam) PURE;		// Start capture
	STDMETHOD (Pause)(BOOL bPause) PURE;							// Pause capture
	STDMETHOD (Stop)() PURE;										// Stop capture
	STDMETHOD (Work)(LONG_PTR lpParam) PURE;						
	STDMETHOD_(BOOL, IsCapturing)() PURE;							

	STDMETHOD (CheckConfig)(BCAP_CONFIG *pCfg) PURE;				
	STDMETHOD (SetConfig)(BCAP_CONFIG *pCfg) PURE;					
	STDMETHOD (GetConfig)(BCAP_CONFIG *pCfg) PURE;					

	STDMETHOD_(INT, GetCaptureTime)() PURE;							// msec
	STDMETHOD_(INT64, GetCaptureFileSize)() PURE;					// bytes

	STDMETHOD (SetPriority)(INT nPriority) PURE;					
	STDMETHOD_(INT, GetPriority)() PURE;							

	STDMETHOD (SetMinMaxFPS)(INT nMin, INT nMax) PURE;				
	STDMETHOD (GetMinMaxFPS)(INT *pnMin, INT *pnMax) PURE;			

	STDMETHOD (CaptureImage)(LPCWSTR pwszFilePath, INT nFileType, INT nQuality, INT nCaptureMode, 
					BOOL bIncludeCursor, LONG_PTR lpParam) PURE;	

	STDMETHOD_(DWORD, GetSuggestedVideoCodec)() PURE;				
	STDMETHOD_(BOOL, IsSupportedVideoCodec)(DWORD dwCodec) PURE;	
};


typedef HRESULT (WINAPI *LPCREATEBANDICAP)(UINT SDKVersion, void** pp);


////////////////////////////////////////////////////////////////////////////////////////////////////
// IBandiCapture error code
#define _FACBC  0x777
#define MAKE_BCHRESULT( code )				MAKE_HRESULT( 1, _FACBC, code )
#define MAKE_BCSTATUS( code )				MAKE_HRESULT( 0, _FACBC, code )

#define BC_OK								S_OK					// 0x00000000L
#define BC_FALSE							S_FALSE					// 0x00000001L
#define BC_OK_ONLY_VIDEO					MAKE_BCSTATUS(0x0010)	// 0x07770010L
#define BC_OK_ONLY_AUDIO					MAKE_BCSTATUS(0x0011)	// 0x07770011L
#define BC_OK_IS_NOT_CAPTURING				MAKE_BCSTATUS(0x0020)	// 0x07770020L
#define BC_OK_COMPLETED_BY_TIME				MAKE_BCSTATUS(0x0030)	// 0x07770030L
#define BC_OK_COMPLETED_BY_SIZE				MAKE_BCSTATUS(0x0031)	// 0x07770031L
#define BC_OK_COMPLETED_BY_FAT32			MAKE_BCSTATUS(0x0032)	// 0x07770032L

#define BCERR_FAIL							E_FAIL					// 0x80004005L

#define BCERR_INVALIDARG					MAKE_BCHRESULT(0x1010)	// 0x87771010L
#define BCERR_VIDEO_CAPTURE					MAKE_BCHRESULT(0x1011)	// 0x87771011L
#define BCERR_AUDIO_CAPTURE					MAKE_BCHRESULT(0x1012)	// 0x87771012L
#define BCERR_FILE_CREATE					MAKE_BCHRESULT(0x1013)	// 0x87771013L
#define BCERR_FILE_WRITE					MAKE_BCHRESULT(0x1014)	// 0x87771014L
#define BCERR_NOT_ENOUGH_DISKSPACE			MAKE_BCHRESULT(0x1015)	// 0x87771015L

#define BCERR_LOAD_LIBRARY_FAIL				MAKE_BCHRESULT(0x1020)	// 0x87771020L
#define BCERR_ALREADY_CREATED				MAKE_BCHRESULT(0x1021)	// 0x87771021L
#define BCERR_GET_PROC_ADDRESS_FAIL			MAKE_BCHRESULT(0x1022)	// 0x87771022L
#define BCERR_LIBRARY_NOT_LOADED			MAKE_BCHRESULT(0x1023)	// 0x87771023L
#define BCERR_UNSUPPORTED_OS				MAKE_BCHRESULT(0x1024)	// 0x87771024L

#define BCERR_INVALID_VERSION				MAKE_BCHRESULT(0x1030)	// 0x87771030L
#define BCERR_NOT_ENOUGH_MEMORY				MAKE_BCHRESULT(0x1031)	// 0x87771031L


#ifndef _BANDICAP_NO_UTIL_CLASS

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utility class for dll load
//
#ifdef _WIN64
	#ifdef _DEBUG
	#	define BANDICAP_DLL_FILE_NAME			_T("bdcap64_d.dll")
	#else
	#	define BANDICAP_DLL_FILE_NAME			_T("bdcap64.dll")
	#endif
	#define BANDICAP_RELEASE_DLL_FILE_NAME		_T("bdcap64.dll")
#else
	#ifdef _DEBUG
	#	define BANDICAP_DLL_FILE_NAME			_T("bdcap32_d.dll")
	#else
	#	define BANDICAP_DLL_FILE_NAME			_T("bdcap32.dll")
	#endif
	#define BANDICAP_RELEASE_DLL_FILE_NAME		_T("bdcap32.dll")
#endif


class CBandiCaptureLibrary : public IBandiCapture
{
public :
	CBandiCaptureLibrary()
	{
		m_hDll = NULL;
		m_pCreateBandiCap = NULL;
		m_pCap = NULL;
	}

	~CBandiCaptureLibrary()
	{
		Destroy();
	}

	HRESULT Create(LPCTSTR szDllPathName)
	{
		if(m_hDll) {ASSERT(0); return BCERR_ALREADY_CREATED;}

		m_hDll = LoadLibrary(szDllPathName);
		if(m_hDll==NULL)
		{
			m_hDll = LoadLibrary(BANDICAP_RELEASE_DLL_FILE_NAME);
			if(m_hDll==NULL)
			{
				ASSERT(0);
				return BCERR_LOAD_LIBRARY_FAIL;
			}
		}

		m_pCreateBandiCap = (LPCREATEBANDICAP)GetProcAddress(m_hDll, "CreateBandiCapture");

		if(m_pCreateBandiCap==NULL)
		{
			ASSERT(0);
			FreeLibrary(m_hDll);
			m_hDll = NULL;
			return BCERR_GET_PROC_ADDRESS_FAIL;
		}

		HRESULT hr = CreateNewBandiCaptureInstance(&m_pCap);
		if(FAILED(hr))
		{
			ASSERT(0);
			FreeLibrary(m_hDll);
			m_hDll = NULL;
			return hr;
		}

		return S_OK;
	}

	BOOL	IsCreated()
	{
		return m_pCap ? TRUE : FALSE;
	}

	void	Destroy()
	{
		if(m_pCap)
		{
			m_pCap->Stop();
			m_pCap->Release();
		}
		m_pCap = NULL;
		m_pCreateBandiCap = NULL;
		if(m_hDll) FreeLibrary(m_hDll);
		m_hDll = NULL;
	}

	LPCTSTR MakePathnameByDate(LPCTSTR szTargetDirectory, LPCTSTR szName, LPCTSTR szExt, LPTSTR szBuf, int nBufLen)
	{
		SYSTEMTIME	sysTime;
		GetLocalTime(&sysTime);

#if _MSC_VER < 1400	// below 2005
		if(szTargetDirectory[_tcslen(szTargetDirectory)-1] == '\\')
			_stprintf(szBuf, _T("%s%s %d-%02d-%02d %02d-%02d-%02d-%03d.%s"), szTargetDirectory, szName, sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds, szExt);
		else
			_stprintf(szBuf, _T("%s\\%s %d-%02d-%02d %02d-%02d-%02d-%03d.%s"), szTargetDirectory, szName, sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds, szExt);
#else				// 2005, 2008
		if(szTargetDirectory[_tcslen(szTargetDirectory)-1] == '\\')
			_stprintf_s(szBuf, nBufLen, _T("%s%s %d-%02d-%02d %02d-%02d-%02d-%03d.%s"), szTargetDirectory, szName, sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds, szExt);
		else
			_stprintf_s(szBuf, nBufLen, _T("%s\\%s %d-%02d-%02d %02d-%02d-%02d-%03d.%s"), szTargetDirectory, szName, sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds, szExt);
#endif

		return szBuf;
	}

public :		
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** p) { return S_OK; }
	ULONG   STDMETHODCALLTYPE AddRef( void){return 0;}
	ULONG   STDMETHODCALLTYPE Release( void){return 0;}

public :		// IBandiCapture

	STDMETHODIMP	Verify(LPCSTR szID, LPCSTR szKey)
	{
		if(m_pCap==NULL){ASSERT(0); return E_FAIL;}
		return m_pCap->Verify(szID, szKey);
	}

	STDMETHODIMP_(INT)	GetVersion()
	{
		return m_pCap->GetVersion();
	}

	STDMETHODIMP	Start(LPCSTR pszFilePath, HWND hParentWnd, INT nCaptureMode, LONG_PTR lpParam)
	{
		if(m_pCap==NULL){ASSERT(0); return E_FAIL;}

		WCHAR wszPath[MAX_PATH+1];
		MultiByteToWideChar(CP_ACP, NULL, pszFilePath, -1, wszPath, MAX_PATH);
		return m_pCap->Start(wszPath, hParentWnd, nCaptureMode, lpParam);
	}

	STDMETHODIMP	Start(LPCWSTR pwszFilePath, HWND hParentWnd, INT nCaptureMode, LONG_PTR lpParam)
	{
		if(m_pCap==NULL){ASSERT(0); return E_FAIL;}
		return m_pCap->Start(pwszFilePath, hParentWnd, nCaptureMode, lpParam);
	}

	STDMETHODIMP Pause(BOOL bPause)
	{
		if(m_pCap==NULL){ASSERT(0); return E_FAIL;}
		return m_pCap->Pause(bPause);
	}

	STDMETHODIMP Stop()
	{
		if(m_pCap==NULL){ASSERT(0); return E_FAIL;}
		return m_pCap->Stop();
	}

	STDMETHODIMP Work(LONG_PTR lpParam)
	{
		if(m_pCap==NULL){ASSERT(0); return E_FAIL;}
		return m_pCap->Work(lpParam);
	}

	STDMETHODIMP_(BOOL) IsCapturing()
	{
		if(m_pCap==NULL) return FALSE;
		return m_pCap->IsCapturing();
	}

	STDMETHODIMP CheckConfig(BCAP_CONFIG *pCfg)
	{
		if(m_pCap==NULL){ASSERT(0); return E_FAIL;}
		return m_pCap->CheckConfig(pCfg);
	}

	STDMETHODIMP SetConfig(BCAP_CONFIG *pCfg)
	{
		if(m_pCap==NULL){ASSERT(0); return E_FAIL;}
		return m_pCap->SetConfig(pCfg);
	}

	STDMETHODIMP GetConfig(BCAP_CONFIG *pCfg)
	{
		if(m_pCap==NULL){ASSERT(0); return E_FAIL;}
		return m_pCap->GetConfig(pCfg);
	}

	STDMETHODIMP_(INT) GetCaptureTime()
	{
		if(m_pCap==NULL){ASSERT(0); return 0;}
		return m_pCap->GetCaptureTime();
	}

	STDMETHODIMP_(INT64) GetCaptureFileSize()
	{
		if(m_pCap==NULL){ASSERT(0); return 0;}
		return m_pCap->GetCaptureFileSize();
	}

	STDMETHODIMP SetPriority(INT nPriority)
	{
		if(m_pCap==NULL){ASSERT(0); return E_FAIL;}
		return m_pCap->SetPriority(nPriority);
	}

	STDMETHODIMP_(INT) GetPriority()
	{
		if(m_pCap==NULL){ASSERT(0); return 0;}
		return m_pCap->GetPriority();
	}

	STDMETHODIMP SetMinMaxFPS(INT nMin, INT nMax)
	{
		if(m_pCap==NULL){ASSERT(0); return E_FAIL;}
		return m_pCap->SetMinMaxFPS(nMin, nMax);
	}

	STDMETHODIMP GetMinMaxFPS(INT *pnMin, INT *pnMax)
	{
		if(m_pCap==NULL){ASSERT(0); return E_FAIL;}
		return m_pCap->GetMinMaxFPS(pnMin, pnMax);
	}

	STDMETHODIMP CaptureImage(LPCSTR pszFilePath, INT nFileType, INT nQuality, INT nCaptureMode, 
							BOOL bIncludeCursor, LONG_PTR lpParam)
	{
		if(m_pCap==NULL){ASSERT(0); return E_FAIL;}

		WCHAR wszPath[MAX_PATH+1];
		MultiByteToWideChar(CP_ACP, NULL, pszFilePath, -1, wszPath, MAX_PATH);

		return m_pCap->CaptureImage(wszPath, nFileType, nQuality, nCaptureMode, bIncludeCursor, lpParam);
	}

	STDMETHODIMP CaptureImage(LPCWSTR pwszFilePath, INT nFileType, INT nQuality, INT nCaptureMode, 
							BOOL bIncludeCursor, LONG_PTR lpParam)
	{
		if(m_pCap==NULL){ASSERT(0); return E_FAIL;}
		return m_pCap->CaptureImage(pwszFilePath, nFileType, nQuality, nCaptureMode, bIncludeCursor, lpParam);
	}
	

	
	STDMETHODIMP_(DWORD) GetSuggestedVideoCodec()
	{
		if(m_pCap==NULL){ASSERT(0); return 0;}
		return m_pCap->GetSuggestedVideoCodec();
	}

	STDMETHODIMP_(BOOL) IsSupportedVideoCodec(DWORD dwCodec)
	{
		if(m_pCap==NULL){ASSERT(0); return FALSE;}
		return m_pCap->IsSupportedVideoCodec(dwCodec);
	}

private :
	HRESULT CreateNewBandiCaptureInstance(IBandiCapture** ppBaCapture)
	{
		if(m_pCreateBandiCap==NULL){ASSERT(0); return BCERR_LIBRARY_NOT_LOADED;}
		*ppBaCapture = NULL;
		HRESULT hr = m_pCreateBandiCap(BCAP_VERSION, (void**)ppBaCapture);
		if(FAILED(hr)) ASSERT(0);
		return hr;
	}

private :
	HMODULE				m_hDll;
	LPCREATEBANDICAP	m_pCreateBandiCap;

public :
	IBandiCapture*		m_pCap;
};


// PRESETS
enum BCAP_PRESET
{
	BCAP_PRESET_DEFAULT,
	BCAP_PRESET_HALFSIZE,
	BCAP_PRESET_320x240,
	BCAP_PRESET_400x300,
	BCAP_PRESET_512x384,
	BCAP_PRESET_576x432,
	BCAP_PRESET_640x480,
	BCAP_PRESET_800x600,

	BCAP_PRESET_MPEG1,
	BCAP_PRESET_MJPEG,
	BCAP_PRESET_MJPEG_HIGH_QUALITY,
	BCAP_PRESET_MPEG4,

	BCAP_PRESET_YOUTUBE,
	BCAP_PRESET_YOUTUBE_HIGH_QUALITY,
	BCAP_PRESET_YOUTUBE_HIGH_DEFINITION,
	BCAP_PRESET_NAVER_BLOG,
	BCAP_PRESET_DAUM_TVPOT,

	BCAP_PRESET_VIDEO_EDITING,
};

// SETTING CONFIG BY PRESET
inline BOOL BCapConfigPreset(BCAP_CONFIG* pConfig, BCAP_PRESET preset)
{
	pConfig->Default();

	switch(preset)
	{
	case BCAP_PRESET_DEFAULT :
		break;

	case BCAP_PRESET_HALFSIZE :
		pConfig->VideoSizeW = -1;			// half size
		pConfig->VideoSizeH = -1;
		break;

	case BCAP_PRESET_320x240 :
		pConfig->VideoSizeW = 320;			// 320x240
		pConfig->VideoSizeH = 0;
		pConfig->VideoFPS = 24;
		break;
	case BCAP_PRESET_400x300 :
		pConfig->VideoSizeW = 400;			// 400x300
		pConfig->VideoSizeH = 0;
		pConfig->VideoFPS = 24;
		break;
	case BCAP_PRESET_512x384 :
		pConfig->VideoSizeW = 512;			// 512x384
		pConfig->VideoSizeH = 0;
		pConfig->VideoFPS = 24;
		break;
	case BCAP_PRESET_576x432 :
		pConfig->VideoSizeW = 576;			// 576x432
		pConfig->VideoSizeH = 0;
		pConfig->VideoFPS = 24;
		break;
	case BCAP_PRESET_640x480 :
		pConfig->VideoSizeW = 640;			// 640x480
		pConfig->VideoSizeH = 0;
		break;
	case BCAP_PRESET_800x600 :
		pConfig->VideoSizeW = 800;			// 800x600
		pConfig->VideoSizeH = 0;
		break;

	case BCAP_PRESET_MPEG1 :				// MPEG1
		pConfig->VideoCodec	= FOURCC_MPEG;
		pConfig->VideoQuality = 80;
		pConfig->AudioCodec	= WAVETAG_MP2;
		pConfig->AudioSampleRate = 44100;
		break;

	case BCAP_PRESET_MJPEG :				// MJPEG
		pConfig->FileType = BCAP_CONFIG::F_AVI;
		pConfig->VideoCodec	= FOURCC_MJPG;
		pConfig->VideoQuality = 80;
		pConfig->AudioCodec	= WAVETAG_PCM;
		pConfig->AudioSampleRate = 24000;
		break;
	case BCAP_PRESET_MJPEG_HIGH_QUALITY :	// MJPEG with High qulaity audio & video
		pConfig->FileType = BCAP_CONFIG::F_AVI;
		pConfig->VideoCodec	= FOURCC_MJPG;
		pConfig->VideoQuality = 90;
		pConfig->AudioCodec	= WAVETAG_PCM;
		pConfig->AudioSampleRate = 44100;
		break;

	case BCAP_PRESET_MPEG4 :				// MPEG4
		pConfig->FileType = BCAP_CONFIG::F_AVI;
		pConfig->VideoCodec	= FOURCC_MP4V;
		pConfig->VideoQuality = 80;
		pConfig->AudioCodec	= WAVETAG_PCM;
		pConfig->AudioSampleRate = 24000;
		break;

	case BCAP_PRESET_YOUTUBE :
		pConfig->VideoQuality = 70;
		pConfig->VideoSizeW = 320;			// 320x240
		pConfig->VideoSizeH = 0;
		pConfig->VideoFPS = 24;
		pConfig->AudioChannels = 1;			// mono
		pConfig->AudioSampleRate = 22050;	// 22k
		break;

	case BCAP_PRESET_YOUTUBE_HIGH_QUALITY :
		pConfig->VideoSizeW = 480;			// 480x272
		pConfig->VideoSizeH = 0;
		pConfig->VideoFPS = 30;
		pConfig->AudioChannels = 1;			// mono
		pConfig->AudioSampleRate = 44100;	// 44k
		break;

	case BCAP_PRESET_YOUTUBE_HIGH_DEFINITION :
		pConfig->VideoSizeW = 0;			// 1280x720
		pConfig->VideoSizeH = 0;
		pConfig->VideoFPS = 30;
		pConfig->AudioChannels = 2;			// stereo
		pConfig->AudioSampleRate = 44100;	// 44k
		break;

	case BCAP_PRESET_NAVER_BLOG :
		pConfig->VideoSizeW = 0;			// 480p
		pConfig->VideoSizeH = 480;
		pConfig->VideoFPS = 24;
		pConfig->AudioSampleRate = 44100;	// 44k
		break;

	case BCAP_PRESET_DAUM_TVPOT :
		pConfig->VideoSizeW = 512;			// 512x384
		pConfig->VideoSizeH = 0;
		pConfig->VideoFPS = 24;
		pConfig->AudioSampleRate = 44100;	// 22k
		break;

	case BCAP_PRESET_VIDEO_EDITING :		// for video edting (premier, vegas)
		pConfig->FileType = BCAP_CONFIG::F_AVI;
		pConfig->AudioCodec = 0x0001;		// pcm
		break;
	default :								// error
		ASSERT(0);
		return FALSE;
	}
	return TRUE;
}


#endif // _BANDICAP_NO_UTIL_CLASS

#endif // _BANDICAP_H
