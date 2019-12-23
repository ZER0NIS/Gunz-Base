#ifndef ZTOOLTIP_H
#define ZTOOLTIP_H

#include "MWidget.h"
#include "MToolTip.h"

#define MAX_TOOLTIP_LINE_STRING 40

class ZToolTip : public MToolTip
{
public:
	ZToolTip(const char* szName, MWidget* pParent, MAlignmentMode align=MAM_LEFT|MAM_TOP);
	~ZToolTip();
	virtual void OnDraw(MDrawContext* pDC);
	virtual void SetBounds(void);

private:
	void GetPosAlignedWithParent(int& x, int& y, int nTextPixelWidth, int nTextPixelHeight);

	//	MTextArea* m_pTextArea;
	MBitmap* m_pBitmap1;
	MBitmap* m_pBitmap2;

	MAlignmentMode m_alignMode;		// 부모 기준 툴팁 위치
};

#endif//ZTOOLTIP_H
