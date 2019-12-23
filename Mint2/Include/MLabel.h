#ifndef MLABEL_H
#define MLABEL_H

#include "MWidget.h"
#include "MDrawContext.h"
#include "MLookNFeel.h"

class MLabel;

/// Label의 Draw 코드가 있는 클래스, 이 클래스를 상속받아서 커스텀룩을 가질 수 있다.
class MLabelLook{
public:
	virtual void OnDraw(MLabel* pLabel, MDrawContext* pDC);
	virtual MRECT GetClientRect(MLabel* pLabel, MRECT& r);
};

//# MLabel은 align 기능에 문제가 있다.
//# xml에서 지정한 align은 MWidget::m_BoundsAlign에 읽혀지고,
//# 실제로 draw시에는 MLabel::m_AlignmentMode로 정렬을 수행한다. (xml의 align 설정이 무시됨)
//# MLabel::m_AlignmentMode을 없애고 MWidget::m_BoundsAlign를 사용하게 바꾸면 화면크기조정 후에 위치가 엉망이 된다.
//# 고치고 싶지만 기존 MLabel들이 xml에서 align 지정해둔 것이 무시되다가 갑자기 적용되었을때 정렬문제가 생길 수도 있고
//# 아무튼 애매한 상황. 현재로서는 MLabel에 align을 설정하고 싶으면 코드에서 직접 할 수밖에 없다.

/// Label
class MLabel : public MWidget{
protected:
	MCOLOR			m_TextColor;
	MAlignmentMode	m_AlignmentMode;

	DECLARE_LOOK(MLabelLook)
	DECLARE_LOOK_CLIENT()

protected:

public:
	MLabel(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);

	/// 텍스트 컬러 지정
	void SetTextColor(MCOLOR color);
	/// 텍스트 컬러 얻기
	MCOLOR GetTextColor(void);

	/// 정렬 얻기
	MAlignmentMode GetAlignment(void);
	/// 정렬 지정
	MAlignmentMode SetAlignment(MAlignmentMode am);


#define MINT_LABEL	"Label"
	virtual const char* GetClassName(void){ return MINT_LABEL; }
};


#endif