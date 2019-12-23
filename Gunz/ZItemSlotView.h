#ifndef ZITEMSLOTVIEW_H
#define ZITEMSLOTVIEW_H

#include "ZPrerequisites.h"
#include "MWidget.h"
#include "RMesh.h"
#include "RVisualMeshMgr.h"
#include "ZMeshView.h"
#include "MButton.h"
#include "MMatchItem.h"

using namespace RealSpace2;


class ZItemSlotView : public MButton{
protected:
	MBitmap*				m_pBackBitmap;

	// 1) m_nItemID에 적당한 아이템아이디가 세팅된 경우 직접 지정된 item을 출력하고,
	// 2) m_nItemID==-1이면 내 캐릭터의 아이템 정보를 가져다가 출력한다.

	unsigned long int		m_nItemID;
	unsigned int			m_nItemCount;

	MMatchCharItemParts		m_nParts;

	bool					m_bSelectBox;			// 셀렉트 박스 출력 여부
	bool					m_bDragAndDrop;			// 드래그 앤 드롭 가능 여부
	bool					m_bKindable;
	bool					m_bHorizonalInverse;	// 좌우 반전해서 그릴것이냐 (아이콘이 오른쪽)

	virtual void OnDraw(MDrawContext* pDC);
	virtual bool IsDropable(MWidget* pSender);
	virtual bool OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);

	void SetDefaultText(MMatchCharItemParts nParts);
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);
	bool IsEquipableItem(unsigned long int nItemID, int nPlayerLevel, MMatchSex nPlayerSex);

	virtual void OnMouseIn(void);
	virtual void OnMouseOut(void);

	const char* GetItemDescriptionWidgetName();

public:
	char					m_szItemSlotPlace[128];

	ZItemSlotView(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZItemSlotView(void);
	MMatchCharItemParts GetParts() { return m_nParts; }
	void SetParts(MMatchCharItemParts nParts);

	void SetBackBitmap(MBitmap* pBitmap);
	void SetIConBitmap(MBitmap* pBitmap);

	void EnableDragAndDrop( bool bEnable);

	void SetKindable( bool bKindable);

	void SetItemID(unsigned long int id) { m_nItemID = id; }
	void SetItemCount(unsigned long int nCnt) { m_nItemCount = nCnt; }
	void SetHorizontalInverse(bool b) { m_bHorizonalInverse = b; }


#define MINT_ITEMSLOTVIEW	"ItemSlotView"
	virtual const char* GetClassName(void){ return MINT_ITEMSLOTVIEW; }
};



#endif