#include "stdafx.h"
#include "MDrawContext.h"
#include "MWidget.h"
#include <crtdbg.h>
#include "Mint.h"
#include <math.h>

MDrawContext::MDrawContext(void)
	: m_Color(0, 255, 0), m_HighlightColor(255, 255, 0)
{
#ifdef _DEBUG
	m_nTypeID = MINT_BASE_CLASS_TYPE;
#endif
	m_Color = 0x00000000;
	m_BitmapColor = 0xFFFFFFFF;
	m_ColorKey = MCOLOR(0xFF, 0, 0xFF);
	m_pBitmap = 0;
	m_pFont = 0;
	m_Clip.x = 0;
	m_Clip.y = 0;
	m_Clip.w = MGetWorkspaceWidth();
	m_Clip.h = MGetWorkspaceHeight();
	m_Origin.x = 0;
	m_Origin.y = 0;
	m_nOpacity = 0xFF;
	m_Effect = MDE_NORMAL;
}

MDrawContext::~MDrawContext()
{
}

MCOLOR MDrawContext::SetBitmapColor(MCOLOR& color)
{
	m_BitmapColor = color;
	return m_BitmapColor;
}
MCOLOR MDrawContext::SetBitmapColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	return SetBitmapColor(MCOLOR(r, g, b, a));
}

MCOLOR MDrawContext::GetBitmapColor()
{
	return m_BitmapColor;
}

MCOLOR MDrawContext::SetColor(const MCOLOR& color)
{
	MCOLOR temp = m_Color;
	if (m_nOpacity != 0xFF)
	{
		unsigned char a = (unsigned char)(color.a * (float)(m_nOpacity / 255.0f));
		m_Color = MCOLOR(color.r, color.g, color.b, a);
	}
	else m_Color = color;
	return temp;
}

MCOLOR MDrawContext::SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	if (m_nOpacity != 0xFF) a = (unsigned char)(a * (float)(m_nOpacity / 255.0f));
	return SetColor(MCOLOR(r, g, b, a));
}

MCOLOR MDrawContext::GetColor(void)
{
	return m_Color;
}

MCOLOR MDrawContext::SetHighlightColor(MCOLOR& color)
{
	MCOLOR temp = m_HighlightColor;
	if (m_nOpacity != 0xFF)
	{
		unsigned char a = (unsigned char)(color.a * (float)(m_nOpacity / 255.0f));
		m_HighlightColor = MCOLOR(color.r, color.g, color.b, a);
	}
	else m_HighlightColor = color;
	return temp;
}

MCOLOR MDrawContext::SetHighlightColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	if (m_nOpacity != 0xFF) a = (unsigned char)(a * (float)(m_nOpacity / 255.0f));
	return SetHighlightColor(MCOLOR(r, g, b, a));
}

MCOLOR MDrawContext::GetHighlightColor(void)
{
	return m_HighlightColor;
}

MCOLOR MDrawContext::SetColorKey(MCOLOR& color)
{
	MCOLOR temp = m_ColorKey;
	m_ColorKey = color;
	return temp;
}

MCOLOR MDrawContext::SetColorKey(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	return SetColorKey(MCOLOR(r, g, b, a));
}

MCOLOR MDrawContext::GetColorKey(void)
{
	return m_ColorKey;
}

MBitmap* MDrawContext::SetBitmap(MBitmap* pBitmap)
{
	MBitmap* temp = m_pBitmap;
	m_pBitmap = pBitmap;
	return temp;
}

MFont* MDrawContext::SetFont(MFont* pFont)
{
	MFont* temp = m_pFont;
	m_pFont = pFont;
	if (m_pFont == NULL) {
		m_pFont = MFontManager::Get(0);
	}
	return temp;
}

void MDrawContext::SetClipRect(int x, int y, int w, int h)
{
	SetClipRect(MRECT(x, y, w, h));
}

void MDrawContext::SetClipRect(MRECT& r)
{
	memcpy(&m_Clip, &r, sizeof(MRECT));

	if (m_Clip.x < 0) m_Clip.x = 0;
	if (m_Clip.y < 0) m_Clip.y = 0;
	if (m_Clip.x + m_Clip.w > MGetWorkspaceWidth()) m_Clip.w = max(MGetWorkspaceWidth() - m_Clip.x, 0);
	if (m_Clip.y + m_Clip.h > MGetWorkspaceHeight()) m_Clip.h = max(MGetWorkspaceHeight() - m_Clip.y, 0);
}

void MDrawContext::Rectangle(int x, int y, int cx, int cy)
{
	HLine(x, y, cx);
	HLine(x, y + cy, cx);
	VLine(x, y, cy);
	VLine(x + cx, y, cy);
}

void MDrawContext::Rectangle(const MRECT& r)
{
	Rectangle(r.x, r.y, r.w, r.h);
}

void MDrawContext::SetOrigin(int x, int y)
{
	m_Origin.x = x;
	m_Origin.y = y;
}

void MDrawContext::SetOrigin(MPOINT& p)
{
	m_Origin = p;
}

MPOINT MDrawContext::GetOrigin(void)
{
	return m_Origin;
}

void MDrawContext::SetEffect(MDrawEffect effect)
{
	m_Effect = effect;
}

void MDrawContext::FillRectangle(const MRECT& r)
{
	FillRectangle(r.x, r.y, r.w, r.h);
}

int MDrawContext::TextWithHighlight(int x, int y, const char* szText)
{
	char szFront[MWIDGET_NAME_LENGTH];
	char szBack[MWIDGET_NAME_LENGTH];
	char cHighlight;

	int nPos = RemoveAnd(szFront, &cHighlight, szBack, szText);
	if (nPos == -1) {
		return Text(x, y, szText);
	}
	else {
		if (m_pFont != NULL) {
			char temp[2] = { cHighlight, 0 };
			int nFrontWidth = m_pFont->GetWidth(szFront);
			int nHighlightWidth = m_pFont->GetWidth(temp);
			Text(x, y, szFront);
			MCOLOR tmpColor = m_Color;
			SetColor(m_HighlightColor);
			Text(x + nFrontWidth, y, temp);
			SetColor(tmpColor);
			return Text(x + nFrontWidth + nHighlightWidth, y, szBack);
		}
		else {
			return Text(x, y, szText);
		}
	}
}

void MDrawContext::GetPositionOfAlignment(MPOINT* p, MRECT& r, const char* szText, MAlignmentMode am, bool bAndInclude)
{
	if (m_pFont != NULL) {
		int w;
		if (bAndInclude == true) w = m_pFont->GetWidth(szText);
		else w = m_pFont->GetWidthWithoutAmpersand(szText);
		int h = m_pFont->GetHeight();

#define DEFAULT_ALIGN_MARGIN	2
		p->x = r.x;
		p->y = r.y;
		if (am & MAM_LEFT) {
			p->x = r.x + DEFAULT_ALIGN_MARGIN;
			p->y = r.y + (r.h - h) / 2;
		}
		if (am & MAM_RIGHT) {
			p->x = r.x + (r.w - w - DEFAULT_ALIGN_MARGIN);
			p->y = r.y + (r.h - h) / 2;
		}
		if (am & MAM_EDIT) {
			if (w + DEFAULT_ALIGN_MARGIN < r.w) p->x = r.x + DEFAULT_ALIGN_MARGIN;
			else p->x = r.x + (r.w - w);
			p->y = r.y + (r.h - h) / 2;
		}
		if (am & MAM_HCENTER) {
			p->x = r.x + (r.w - w) / 2;
		}
		if (am & MAM_VCENTER) {
			p->y = r.y + (r.h - h) / 2;
		}
		if (am & MAM_TOP) {
			p->y = r.y;
		}
		if (am & MAM_BOTTOM) {
			p->y = r.y + r.h - h;
		}
	}
}

int MDrawContext::Text(MRECT& r, const char* szText, MAlignmentMode am)
{
	MPOINT p;
	GetPositionOfAlignment(&p, r, szText, am);
	return Text(p.x, p.y, szText);
}

int MDrawContext::TextWithHighlight(MRECT& r, const char* szText, MAlignmentMode am)
{
	char szFront[MWIDGET_NAME_LENGTH];
	char szBack[MWIDGET_NAME_LENGTH];
	char cHighlight;

	int nPos = RemoveAnd(szFront, &cHighlight, szBack, szText);
	if (nPos == -1) {
		return Text(r, szText, am);
	}
	else {
		MPOINT p;
		char szTextWithoutAnd[MWIDGET_NAME_LENGTH];
		RemoveAnd(szTextWithoutAnd, szText);
		GetPositionOfAlignment(&p, r, szTextWithoutAnd, am);
		if (m_pFont != NULL) {
			char temp[2] = { cHighlight, 0 };
			int nFrontWidth = m_pFont->GetWidth(szFront);
			int nHighlightWidth = m_pFont->GetWidth(temp);
			Text(p.x, p.y, szFront);
			MCOLOR tmpColor = m_Color;
			SetColor(m_HighlightColor);
			Text(p.x + nFrontWidth, p.y, temp);
			SetColor(tmpColor);
			return Text(p.x + nFrontWidth + nHighlightWidth, p.y, szBack);
		}
	}
	return -1;
}

MRECT MDrawContext::GetClipRect(void)
{
	return m_Clip;
}

void MDrawContext::TextMC(MRECT& r, const char* szText, MAlignmentMode am)
{
	MPOINT p;
	char* pPText;

	if (GetFont() == NULL) return;

	if ((pPText = GetPureText(szText)) == NULL) return;
	GetPositionOfAlignment(&p, r, pPText, am);
	free(pPText);

	TextMC(p.x, p.y, szText);
}

u32 MMColorSet[] = {
	0xFF808080,
	0xFFFF0000,
	0xFF00FF00,
	0xFF0000FF,
	0xFFFFFF00,
	0xFF800000,
	0xFF008000,
	0xFF000080,
	0xFF808000,
	0xFFFFFFFF,
};

static bool TestDigit(int c)
{
	if (c >= 48 && c <= 57) {
		return true;
	}
	else {
		return false;
	}
}

void MDrawContext::TextMC(int x, int y, const char* szText)
{
	unsigned int nPos = 0, nLen, nOffset = 0;
	const char* pSrc = szText;
	char* pText;

	if (GetFont() == NULL) return;

	while (true) {
		nPos = strcspn(pSrc, "^");
		pText = (char*)malloc(nPos + 1);
		if (pText) {
			strncpy(pText, pSrc, nPos);
			pText[nPos] = '\0';
			Text(x + nOffset, y, pText);
			nOffset += GetFont()->GetWidth(pText);
			free(pText);
		}

		nLen = strlen(pSrc);

		if (nPos + 1 < strlen(pSrc)) {
			if (TestDigit(pSrc[nPos + 1])) {
				SetColor(MCOLOR(MMColorSet[pSrc[nPos + 1] - '0']));
				pSrc = pSrc + nPos + 2;
			}
			else {
				Text(x + nOffset, y, "^");
				nOffset += GetFont()->GetWidth("^");
				pSrc = pSrc + nPos + 1;
			}
		}
		else {
			if (nPos + 1 == nLen && TestDigit(pSrc[nPos]) == false) {
				Text(x + nOffset, y, "^");
				nOffset += GetFont()->GetWidth("^");
			}
			if (nPos == nLen && (pSrc[nPos - 1] == '^')) {
				Text(x + nOffset, y, "^");
				nOffset += GetFont()->GetWidth("^");
			}
			break;
		}
	}
}

char* MDrawContext::GetPureText(const char* szText)
{
	unsigned int nPos = 0, nLen;
	const char* pSrc = szText;
	char* pText;

	nLen = strlen(szText);
	pText = (char*)malloc(nLen + 1);
	memset(pText, 0, nLen + 1);

	while (true) {
		nPos = strcspn(pSrc, "^");

		strncat(pText, pSrc, nPos);
		nLen = strlen(pSrc);

		if (nPos + 1 < strlen(pSrc)) {
			if (TestDigit(pSrc[nPos + 1])) {
				pSrc = pSrc + nPos + 2;
			}
			else {
				pSrc = pSrc + nPos + 1;
				strcat(pText, "^");
			}
		}
		else {
			if (nPos + 1 == nLen && TestDigit(pSrc[nPos]) == false) {
				strcat(pText, "^");
			}
			if (nPos == nLen && (pSrc[nPos - 1] == '^')) {
				strcat(pText, "^");
			}
			break;
		}
	}
	return pText;
}

#define MAX_CHAR_A_LINE		255
#include "MDebug.h"
int MDrawContext::TextMultiLine(MRECT& r, const char* szText, int nLineGap, bool bAutoNextLine, int nIndentation, int nSkipLine, MPOINT* pPositions)
{
	bool bColorSupport = true;

	MBeginProfile(99, "MDrawContext::TextMultiLine");

	int nLine = 0;
	MFont* pFont = GetFont();

	int nLength = strlen(szText);

	int y = r.y;
	const char* szCurrent = szText;
	MPOINT* pCurrentPos = pPositions;
	do {
		int nX = nLine == 0 ? 0 : nIndentation;

		int nOriginalCharCount = MMGetNextLinePos(pFont, szCurrent, r.w - nX, bAutoNextLine, true);

		if (nSkipLine <= nLine)
		{
			int nCharCount = min(nOriginalCharCount, MAX_CHAR_A_LINE);
			char buffer[256];
			if (bColorSupport) {
#define FLUSHPOS(_Pos)		if(pCurrentPos!=NULL){	\
								for(int i=0; buffer[i]!=NULL; i++){	\
									pCurrentPos[i+szCurrent-szText].x = _Pos+pFont->GetWidth(buffer, i);	\
									pCurrentPos[i+szCurrent-szText].y = y;	\
								}	\
							}

#define FLUSH				if(buffer[0]) { Text(r.x+nLastX, y, buffer); FLUSHPOS(r.x+nLastX); nLastX=nX; buffer[0]=0;pcurbuf=buffer; }

				int nLastX = nX;

				buffer[0] = 0;
				char* pcurbuf = buffer;
				for (int i = 0; i < nCharCount; i++) {
					unsigned char c = szCurrent[i], cc = szCurrent[i + 1];

					if (c == '^' && ('0' <= cc) && (cc <= '9'))
					{
						FLUSH;
						SetColor(MCOLOR(MMColorSet[cc - '0']));
						i++;
						continue;
					}

					int w;

					*(pcurbuf++) = c;
					if (c > 127 && i < nCharCount) {
						*(pcurbuf++) = cc;
						w = pFont->GetWidth(szCurrent + i, 2);
						i++;
					}
					else w = pFont->GetWidth(szCurrent + i, 1);

					*pcurbuf = 0;

					nX += w;
				}

				FLUSH;
			}
			else
			{
				strncpy(buffer, szCurrent, nCharCount);
				buffer[nCharCount] = 0;
				Text(r.x + nX, y, buffer);
				FLUSHPOS(r.x + nX);
			}
			y += pFont->GetHeight() + nLineGap;
		}

		szCurrent += nOriginalCharCount;
		nLine++;
		if (y >= r.y + r.h) break;
	} while (szCurrent < szText + nLength);

	MEndProfile(99);
	return nLine - nSkipLine;
}

int MDrawContext::TextMultiLine2(MRECT& r, const char* szText, int nLineGap, bool bAutoNextLine, MAlignmentMode am)
{
	MFont* pFont = GetFont();
	int nHeight = pFont->GetHeight() + nLineGap;
	int nWidth = pFont->GetWidth(szText);

	int nLine = MMGetLineCount(pFont, szText, r.w, bAutoNextLine);
	int nStrLen = strlen(szText);
	int nX = 0;
	int nCurrLine = 0;
	int IncY = nHeight;

#define		MAX_WIDGET_LINE_LENGTH	1024
	int clip = 0;
	char	TempStr[MAX_WIDGET_LINE_LENGTH];

	MRECT rTemp;

	int i;
	for (i = 0; i < nStrLen; i++)
	{
		char temp[3] = { 0, 0, 0 };
		temp[0] = szText[i];
		unsigned char c = temp[0];

		if (c == '\n') {
			nX = 0;

			rTemp.x = r.x;
			rTemp.y = r.y + IncY * nCurrLine;
			rTemp.w = r.w;
			rTemp.h = IncY;

			strncpy(TempStr, szText + clip, i - clip);
			TempStr[i - clip] = '\0';
			Text(rTemp, TempStr, am);

			clip = i + 1;
			++nCurrLine;

			continue;
		}

		if (c == '^' && ('0' <= szText[i + 1]) && (szText[i + 1] <= '9'))
		{
			SetColor(MCOLOR(MMColorSet[szText[i + 1] - '0']));
			i++;
			continue;
		}

		if (c > 127)
		{
			i++;
			if (i < nStrLen) temp[1] = szText[i];
		}

		int w = pFont->GetWidth(temp);
		if (nX + w > r.w&& bAutoNextLine)
		{
			nX = 0;

			rTemp.x = r.x;
			rTemp.y = r.y + IncY * nCurrLine;
			rTemp.w = r.w;
			rTemp.h = IncY;
			++nCurrLine;

			strncpy(TempStr, szText + clip, i - clip);
			TempStr[i - clip] = '\0';
			Text(rTemp, TempStr, am);

			clip = c > 127 ? i - 1 : i;

			continue;
		}
		nX += w;
	}

	rTemp.x = r.x;
	rTemp.y = r.y + IncY * nCurrLine;
	rTemp.w = r.w;
	rTemp.h = IncY;

	if (nCurrLine == 0)
		rTemp.h = r.h;

	strncpy(TempStr, szText + clip, i - clip);
	TempStr[i - clip] = '\0';
	Text(rTemp, TempStr, am);

	return nLine;
}

unsigned char MDrawContext::SetOpacity(unsigned char nOpacity)
{
	unsigned char nPrevOpacity = m_nOpacity;
	m_nOpacity = nOpacity;
	return nPrevOpacity;
}

unsigned char MDrawContext::GetOpacity()
{
	return m_nOpacity;
}

void MDrawContext::Draw(int x, int y)
{
	if (m_pBitmap == NULL) return;
	int w = m_pBitmap->GetWidth();
	int h = m_pBitmap->GetHeight();
	Draw(x, y, w, h, 0, 0, w, h);
}

void MDrawContext::Draw(int x, int y, int w, int h)
{
	if (m_pBitmap == NULL) return;
	int bw = m_pBitmap->GetWidth();
	int bh = m_pBitmap->GetHeight();
	Draw(x, y, w, h, 0, 0, bw, bh);
}

void MDrawContext::DrawInverse(int x, int y, int w, int h, bool bMirrorX, bool bMirrorY)
{
	if (m_pBitmap == NULL) return;
	int bw = m_pBitmap->GetWidth();
	int bh = m_pBitmap->GetHeight();
	DrawInverse(x, y, w, h, 0, 0, bw, bh, bMirrorX, bMirrorY);
}

void MDrawContext::DrawInverse(int x, int y, int w, int h, int sx, int sy, int sw, int sh, bool bMirrorX, bool bMirrorY)
{
	Draw(m_pBitmap->GetSourceBitmap(), x, y, w, h, sx + m_pBitmap->GetX(), sy + m_pBitmap->GetY(), sw, sh, bMirrorX, bMirrorY);
}
void MDrawContext::Draw(int x, int y, int sx, int sy, int sw, int sh)
{
	if (m_pBitmap == NULL) return;
	int w = m_pBitmap->GetWidth();
	int h = m_pBitmap->GetHeight();
	Draw(x, y, w, h, sx, sy, sw, sh);
}

void MDrawContext::Draw(MPOINT& p)
{
	Draw(p.x, p.y);
}

void MDrawContext::Draw(MRECT& r)
{
	Draw(r.x, r.y, r.w, r.h);
}

void MDrawContext::Draw(MRECT& d, MRECT& s)
{
	Draw(d.x, d.y, d.w, d.h, s.x, s.y, s.w, s.h);
}

void MDrawContext::Draw(int x, int y, MRECT& s)
{
	Draw(x, y, s.x, s.y, s.w, s.h);
}

void MDrawContext::DrawEx(int tx1, int ty1, int tx2, int ty2,
	int tx3, int ty3, int tx4, int ty4)
{
}

void MDrawContext::Draw(int x, int y, int w, int h, int sx, int sy, int sw, int sh)
{
	Draw(m_pBitmap->GetSourceBitmap(), x, y, w, h, sx + m_pBitmap->GetX(), sy + m_pBitmap->GetY(), sw, sh);
}

int MMGetLinePos(MFont* pFont, const char* szText, int nWidth, bool bAutoNextLine, bool bColorSupport, int nLine, int nIndentation)
{
	int nInd = nIndentation >= nWidth ? 0 : nIndentation;

	int nStrLen = strlen(szText);
	if (nStrLen == 0) return 0;

	int nX = 0;
	int nCurLine = 0;

	int nThisChar = 0;

	for (int i = 0; i < nStrLen; i++) {
		if (nCurLine == nLine)
			return nThisChar;

		nThisChar = i;

		char temp[3] = { 0, 0, 0 };
		temp[0] = szText[i];
		unsigned char c = temp[0];

		if (c == '\n') {
			nCurLine++;
			nThisChar++;
			continue;
		}

		if (bColorSupport)
		{
			if (c == '^' && ('0' <= szText[i + 1]) && (szText[i + 1] <= '9'))
			{
				i++;
				continue;
			}
		}

		if (c > 127) {
			i++;
			if (i < nStrLen) temp[1] = szText[i];
		}

		if (bAutoNextLine) {
			int w = pFont->GetWidth(temp);
			if (nX + w > nWidth) {
				nCurLine++;
				nX = nIndentation;
			}
			nX += w;
		}
	}

	if (nCurLine == nLine)
		return nThisChar;

	return nStrLen;
}

int MMGetNextLinePos(MFont* pFont, const char* szText, int nWidth, bool bAutoNextLine, bool bColorSupport)
{
	return MMGetLinePos(pFont, szText, nWidth, bAutoNextLine, bColorSupport);
}

int MMGetWidth(MFont* pFont, const char* szText, int nSize, bool bColorSupport)
{
	int nStrLen = min((int)strlen(szText), nSize);

	int nX = 0;

	for (int i = 0; i < nStrLen; i++) {
		char temp[3] = { 0, 0, 0 };
		temp[0] = szText[i];
		unsigned char c = temp[0];

		if (c == '\n') {
			return nX;
		}

		if (bColorSupport)
		{
			if (c == '^' && ('0' <= szText[i + 1]) && (szText[i + 1] <= '9'))
			{
				i++;
				continue;
			}
		}

		if (c > 127) {
			i++;
			if (i < nStrLen) temp[1] = szText[i];
		}

		int w = pFont->GetWidth(temp);
		nX += w;
	}

	return nX;
}

int MMGetLineCount(MFont* pFont, const char* szText, int nWidth, bool bAutoNextLine, bool bColorSupport, int nIndentation)
{
	int nLine = 0;
	int nLength = strlen(szText);

	int nCurPos = 0;
	do {
		int nRealWidth = (nLine == 0 ? nWidth : nWidth - nIndentation);
		int nOriginalCharCount = MMGetNextLinePos(pFont, szText + nCurPos, nRealWidth, bAutoNextLine, bColorSupport);
		if (nOriginalCharCount == 0 && szText[nCurPos] != 0) return -1;
		nCurPos += nOriginalCharCount;
		nLine++;
	} while (nCurPos < nLength);

	return nLine;
}