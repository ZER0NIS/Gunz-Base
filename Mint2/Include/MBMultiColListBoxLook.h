#ifndef MBMULTICOLLISTBOXLOOK_H
#define MBMULTICOLLISTBOXLOOK_H

#include "MMultiColListBox.h"

class MBMultiColListBoxLook : public MMultiColListBoxLook{
public:
	MBitmap*	m_pFrameBitmaps[9];
	MFont*		m_pFont;

protected:
	virtual void OnFrameDraw(MMultiColListBox* pListBox, MDrawContext* pDC);

public:
	MBMultiColListBoxLook(void);

	virtual MRECT GetClientRect(MMultiColListBox* pListBox, MRECT& r);
	virtual void OnDraw(MMultiColListBox* pListBox, MDrawContext* pDC);
};

#endif