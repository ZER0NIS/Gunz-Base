#include "stdafx.h"

#include "ZInitialLoading.h"
#include "Mint4R2.h"
#include "RealSpace2.h"
#include "MDebug.h"
#include "ZTips.h"
#include "ZGameInterface.h"
#include "ZGlobal.h"
#include "ZMyInfo.h"

using namespace RealSpace2;

struct CUSTOMVERTEX {
	FLOAT	x, y, z, rhw;
	FLOAT	tu, tv;
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_TEX1)
#define NUM_BAR_BITMAP	1
#define IL_TEXT_BUFFER_SIZE 512
#define ADJUST_SIZE				0.5f
#define ADJUST_SIZE2			0.0f
#define FADEIN_NUM_DRAWING		120
#define FADEOUT_NUM_DRAWING		120
#define BLENDING_NUM_DRAWING	240

ZInitialLoading	ZInitialLoading::msInstance;
ZInitialLoading* ZGetInitialLoading()
{
	return ZInitialLoading::GetInstance();
}

bool ZInitialLoading::Initialize(int numScene_,
	float x_, float y_, float w_, float h_,
	float sx_, float sy_, float sw_, float sh_, bool bTipsVisible)
{
	SAFE_DELETE_ARRAY(mBitmaps);
	SAFE_DELETE_ARRAY(mBitmap_Bar);

	mBitmaps = new MBitmapR2[numScene_];
	mBitmap_Bar = new MBitmapR2[NUM_BAR_BITMAP];
	miNumScene = numScene_;
	miCurrScene = 0;

	mx = x_;		my = y_;		mw = w_;	mh = h_;
	msx = sx_;	msy = sy_;	msw = sw_;	msh = sh_;

	mbUseEnable = true;
	mbText = FALSE;

	mbBitmap = false;
	mbBitmapBar = false;

	m_pLoadingStr = NULL;

	return true;
}

bool ZInitialLoading::AddBitmap(int index_, const char* bitmapName_)
{
	if (index_ >= miNumScene || index_ < 0)
	{
		return false;
	}
	char Buff[64];
	sprintf(Buff, "Scene%d", index_);
	if (!mBitmaps[index_].Create(Buff, RGetDevice(), bitmapName_))
	{
		mlog("Fail to create Bitmap for Initial loading\n");
		return false;
	}
	mbBitmap = true;
	return true;
}

bool	ZInitialLoading::AddBitmapBar(const char* bitmapName_)
{
	if (!mBitmap_Bar[0].Create("Progress_Bar", RGetDevice(), bitmapName_))
	{
		mlog("Fail to create Bitmap for Initial loading\n");
		return false;
	}
	mbBitmapBar = true;
	return true;
}

static int Floorer2PowerSize(int v)
{
	if (v <= 2)		return 2;
	else if (v <= 4)		return 4;
	else if (v <= 8)		return 8;
	else if (v <= 16)		return 16;
	else if (v <= 32)		return 32;
	else if (v <= 64)		return 64;
	else if (v <= 128)		return 128;
	else if (v <= 256)		return 256;
	else if (v <= 512)		return 512;
	else if (v <= 1024)	return 1024;

	_ASSERT(FALSE);

	return 2;
}

void	ZInitialLoading::Draw(LOADING_SCENE_MODE mode_, int destIndex_, bool bFlip_, bool fromThread_)
{
	RRESULT isOK = RIsReadyToRender();
	if (isOK == R_NOTREADY)
		return;

	if (!mbUseEnable) return;
	if (!mbBitmap) return;

	if (mBitmaps[miCurrScene].m_pTexture == NULL)
	{
		return;
	}

	int	numDrawing = 1;

	float ftw = (float)mBitmaps[miCurrScene].m_pTexture->GetWidth();
	float fth = (float)mBitmaps[miCurrScene].m_pTexture->GetHeight();
	float msw2 = msw;
	float msh2 = msh;

	D3DFORMAT d3dformat = mBitmaps[miCurrScene].m_pTexture->GetFormat();

	if (d3dformat == D3DFMT_DXT1 ||
		d3dformat == D3DFMT_DXT2 ||
		d3dformat == D3DFMT_DXT3 ||
		d3dformat == D3DFMT_DXT4 ||
		d3dformat == D3DFMT_DXT5)
	{
		msw2 = (float)Floorer2PowerSize((int)msw);
		msh2 = (float)Floorer2PowerSize((int)msh);
	}

	CUSTOMVERTEX Sprite[4] = {
		{mx - ADJUST_SIZE , my - ADJUST_SIZE , 0, 1.0f, (msx) / ftw       , (msy) / fth },
		{mx + mw - ADJUST_SIZE2, my - ADJUST_SIZE , 0, 1.0f, (msx + msw2) / ftw, (msy) / fth },
		{mx + mw - ADJUST_SIZE2, my + mh - ADJUST_SIZE2, 0, 1.0f, (msx + msw2) / ftw, (msy + msh2) / fth },
		{mx - ADJUST_SIZE , my + mh - ADJUST_SIZE2, 0, 1.0f, (msx) / ftw       , (msy + msh2) / fth},
	};

	float blendFactor = 0;
	switch (mode_)
	{
	case MODE_DEFAULT:
		numDrawing = 1;
		RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		break;

	case MODE_FADEOUT:
		blendFactor = 1;
		numDrawing = FADEIN_NUM_DRAWING;
		RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
		RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		break;

	case MODE_FADEIN:
		blendFactor = 0;
		numDrawing = FADEOUT_NUM_DRAWING;
		RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
		RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		break;

	default:
		numDrawing = 1;
		RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		break;
	}

	float fGradeBlendFacter = 1.f;
	if (!m_bGradeFadeOutStart && timeGetTime() - m_Grade_Time > GRADE_TIMER - 2)
	{
		numDrawing = FADEOUT_NUM_DRAWING;
		fGradeBlendFacter = 1.f;
		m_bGradeFadeOutStart = true;
	}
	for (int i = 0; i < numDrawing; ++i)
	{
		RGetDevice()->Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 0, 0);
		if (mode_ == MODE_FADEIN)
		{
			blendFactor += 1.0f / FADEIN_NUM_DRAWING;
			RGetDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, (DWORD)((BYTE)(0xFF * blendFactor)) << 24);
		}
		else if (mode_ == MODE_FADEOUT)
		{
			blendFactor -= 1.0f / FADEOUT_NUM_DRAWING;
			RGetDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, (DWORD)((BYTE)(0xFF * blendFactor)) << 24);
		}

		RGetDevice()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		RGetDevice()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

		RGetDevice()->SetFVF(D3DFVF_CUSTOMVERTEX);
		RGetDevice()->SetTexture(0, mBitmaps[miCurrScene].m_pTexture->GetTexture());
		RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Sprite, sizeof(CUSTOMVERTEX));
		if (m_pLoadingStr && mpDC) {
			DrawBar((float)(RGetScreenWidth() * 0.0f), (float)(RGetScreenHeight() * 0.9833f),
				(float)(RGetScreenWidth() * 1.0f), (float)(RGetScreenHeight() * 0.0166666666666667f), mPercentage * 0.01f);
		}
		else {
			DrawBar((float)(RGetScreenWidth() * 0.0f), (float)(RGetScreenHeight() * 0.0f),
				(float)(RGetScreenWidth() * 1.0f), (float)(RGetScreenHeight() * 0.0116666666666667f), mPercentage * 0.01f);
		}
		if (m_pLoadingStr && mpDC)
		{
			char buffer[256];
			sprintf(buffer, "Loading %s", m_pLoadingStr);
			int nWidth = mpDC->m_Font.GetTextWidth(buffer);
			int x = (int)(RGetScreenWidth() * 0.0125);
			int y = (int)(RGetScreenHeight() * 0.95f);
			mpDC->m_Font.DrawText(x, y, buffer, 0xFFCCCCCC);
		}

		if (mpDC)
		{
			int nWidth = mpDC->m_Font.GetTextWidth(m_szNotice);
			int x = (int)(RGetScreenWidth() * 0.5) - nWidth / 2;
			int y = (int)(RGetScreenHeight() * 0.86f);
			mpDC->m_Font.DrawText(x, y, m_szNotice, 0xFFFFFFFF);

			if (m_bGradeFadeOutStart)
			{
				fGradeBlendFacter -= 1.0f / FADEOUT_NUM_DRAWING;
				if (i == numDrawing - 1)
				{
					fGradeBlendFacter = 0.f;
					m_bGrade = false;
				}
			}
			if (m_bGrade)
				DrawGrade(fGradeBlendFacter);
		}

		if (mode_ == MODE_DEFAULT && mbText)
			DrawText();

		if (bFlip_) { RFlip(); }
	}
}

void ZInitialLoading::DrawBar(float x_, float y_, float w_, float h_, float percent_)
{
	if (!mbBitmapBar) return;
	if (percent_ > 1.0f) return;

	CUSTOMVERTEX Sprite[4] = {
		{x_ - ADJUST_SIZE, y_ - ADJUST_SIZE, 0, 1.0f, 0, 0 },
		{x_ + (w_ * percent_) - ADJUST_SIZE2, y_ - ADJUST_SIZE, 0, 1.0f, w_ / h_ * percent_, 0 },
		{x_ + (w_ * percent_) - ADJUST_SIZE2, y_ + h_ - ADJUST_SIZE2, 0, 1.0f, w_ / h_ * percent_, 1 },
		{x_ - ADJUST_SIZE, y_ + h_ - ADJUST_SIZE2, 0, 1.0f, 0, 1 },
	};

	RGetDevice()->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

	RGetDevice()->SetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);

	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	RGetDevice()->SetFVF(D3DFVF_CUSTOMVERTEX);
	RGetDevice()->SetTexture(0, mBitmap_Bar[0].m_pTexture->GetTexture());
	RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Sprite, sizeof(CUSTOMVERTEX));
}

bool ZInitialLoading::AddBitmapGrade(const char* bitmapName_)
{
	if (!mBitmap_Grade) return false;
	if (!mBitmap_Grade[0].Create("Scene", RGetDevice(), bitmapName_))
	{
		mlog("Fail to create Bitmap for Initial loading\n");
		return false;
	}
	return true;
}
void ZInitialLoading::DrawGrade(float blendFactor)
{
	if (!mBitmap_Grade) return;
	if (!mBitmap_Grade[0].m_pTexture)  return;

	float ftw = (float)mBitmap_Grade[0].m_pTexture->GetWidth();
	float fth = (float)mBitmap_Grade[0].m_pTexture->GetHeight();
	float msw2 = msw;
	float msh2 = msh;

	D3DFORMAT d3dformat = mBitmap_Grade[0].m_pTexture->GetFormat();

	if (d3dformat == D3DFMT_DXT1 ||
		d3dformat == D3DFMT_DXT2 ||
		d3dformat == D3DFMT_DXT3 ||
		d3dformat == D3DFMT_DXT4 ||
		d3dformat == D3DFMT_DXT5)
	{
		msw2 = (float)Floorer2PowerSize((int)msw);
		msh2 = (float)Floorer2PowerSize((int)msh);
	}

	CUSTOMVERTEX Sprite[4] = {
		{mx - ADJUST_SIZE , my - ADJUST_SIZE , 0, 1.0f, (msx) / ftw       , (msy) / fth },
		{mx + mw - ADJUST_SIZE2, my - ADJUST_SIZE , 0, 1.0f, (msx + msw2) / ftw, (msy) / fth },
		{mx + mw - ADJUST_SIZE2, my + mh - ADJUST_SIZE2, 0, 1.0f, (msx + msw2) / ftw, (msy + msh2) / fth },
		{mx - ADJUST_SIZE , my + mh - ADJUST_SIZE2, 0, 1.0f, (msx) / ftw       , (msy + msh2) / fth},
	};
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	RGetDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, (DWORD)((BYTE)(0xFF * (blendFactor))) << 24);

	RGetDevice()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

	RGetDevice()->SetFVF(D3DFVF_CUSTOMVERTEX);
	RGetDevice()->SetTexture(0, mBitmap_Grade[0].m_pTexture->GetTexture());
	RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Sprite, sizeof(CUSTOMVERTEX));
}

ZInitialLoading::ZInitialLoading()
{
	mbUseEnable = false;
}

ZInitialLoading::~ZInitialLoading()
{
	Release();
}

void	ZInitialLoading::Release()
{
	if (!mbUseEnable) return;

	mbUseEnable = false;
	mbBitmapBar = false;
	mbBitmap = false;

	SAFE_DELETE_ARRAY(mBitmaps);
	SAFE_DELETE_ARRAY(mBitmap_Bar);
	SAFE_DELETE_ARRAY(mBitmap_Grade);

	mpDC = NULL;
	mpStr = NULL;
}

void	ZInitialLoading::SetText(MFontR2* pDc, float x, float y, char* str)
{
	mpDC = pDc;
	mtx = x;
	mty = y;
	mpStr = str;

	if (pDc == NULL || str == NULL)
	{
		mbText = false;
	}
	else
	{
		mbText = true;
	}
}

void	ZInitialLoading::DrawText()
{
	if (mpDC != 0)
	{
		mpDC->m_Font.DrawText(mtx, mty, mpStr, 0xFFCCCCCC);
	}
}

extern MFontR2* g_pDefFont;
extern MDrawContextR2* g_pDC;

ZLoadingProgress::ZLoadingProgress(const char* szName, ZLoadingProgress* pParent, float fRatio)
{
	strcpy_safe(m_szName, szName);
	m_pParent = pParent;
	m_fThisAmount = pParent ? pParent->m_fThisAmount * fRatio : fRatio;
	m_fTotalProgressStart = pParent ? pParent->m_fTotalProgressStart + pParent->m_fCurrentProgress : 0;
	m_fProgressStart = pParent ? pParent->m_fCurrentProgress : 0;
	m_fCurrentProgress = 0;
}

void ZLoadingProgress::Update(float fProgress)
{
	m_fCurrentProgress = fProgress;
	if (m_pParent)
		m_pParent->Update(m_fProgressStart + m_fCurrentProgress * m_fThisAmount);
}

void ZLoadingProgress::Draw()
{
	float fTotalProgress = m_fTotalProgressStart + m_fThisAmount * m_fCurrentProgress;

	ZGetInitialLoading()->SetLoadingStr(m_szName);
	ZGetInitialLoading()->SetPercentage(fTotalProgress * 100.f);
	ZGetInitialLoading()->Draw(MODE_DEFAULT, 0, true);
}