#ifndef MMULTICOLLISTBOX_H
#define MMULTICOLLISTBOX_H

#include "MWidget.h"
#include "MScrollBar.h"
#include "MColorTable.h"
#include "MListBox.h"
#include <list>

// multi column listbox

class MMultiColListBox;
class MMultiColListBoxLook
{
	MAlignmentMode	m_ItemTextAlignmentMode;

public:
	MMultiColListBoxLook();

	void OnDraw(MMultiColListBox* pListBox, MDrawContext* pDC);
	MRECT GetClientRect(MMultiColListBox* pListBox, MRECT& r);
};

// 아이템은 상속해서 구현하도록 얼개만 준비
class MMultiColListItem
{
public:
	virtual ~MMultiColListItem() {}
	virtual void OnDraw(MRECT& r, MDrawContext* pDC, bool bSelected, bool bMouseOver);
	virtual const char* GetString() { return ""; }

	// 드래그 & 드롭을 했을때 전송될 스트링
	virtual bool GetDragItem(MBitmap** ppDragBitmap, char* szDragString, char* szDragItemString){
		return false;
	}

	virtual int GetSortHint() { return 0; }
};

typedef std::list<MMultiColListItem*>		ListMultiColListItem;
typedef list<MMultiColListItem*>::iterator	ItorMultiColListItem;

class MMultiColListBox : public MWidget
{
	ListMultiColListItem m_items;

	int m_numColumn;		// 컬럼 갯수
	int m_desiredNumRow;	// 한번에 보여주고 싶은 행수
	int m_itemHeight;

	int m_maxRowCanShow; // 한 화면에 보여질 수 있는 최대 행 수

	MScrollBar* m_pScrollBar;

	int	m_nOverItem;			// 커서에 의해 가리켜진 아이템
	int m_nSelItem;
	bool m_bDragAndDrop;

	ZCB_ONDROP m_pOnDropFunc;

	void SetItemHeight(int h);
	void InsertSortedPos(MMultiColListItem* pItem);

public:
	MAlignmentMode m_FontAlign;

public:
	MMultiColListBox(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~MMultiColListBox();


	ListMultiColListItem& GetItemList() { return m_items; }

	void SetDesiredNumRow(int n);
	void CalcItemHeight();

	int GetItemHeight() { return m_itemHeight; }
	int GetItemWidth() { return GetClientRect().w / m_numColumn; }

	void SetNumColumn(int n);
	int GetNumColumn() { return m_numColumn; }

	int GetNumItem() { return (int)m_items.size(); }

	MScrollBar* GetScrollBar() { return m_pScrollBar; }
	int GetMaxRowCanShow() { return m_maxRowCanShow; }

	int GetRowFirstVisible() {
		return GetScrollBar()->GetValue();
	}
	int GetRowLastVisible() {
		return GetRowFirstVisible() + GetMaxRowCanShow() - 1;
	}
	int GetItemFirstVisible() {
		return GetRowFirstVisible() * m_numColumn;
	}
	int GetItemLastVisible() {
		int idxItemLastShow = GetItemFirstVisible() + (GetMaxRowCanShow() * m_numColumn) - 1;
		return min(idxItemLastShow, GetNumItem()-1);
	}

	void SetRowFirstVisible(int n) { GetScrollBar()->SetValueAdjusted(n); }

	void Add(MMultiColListItem* pItem);
	void Remove(MMultiColListItem* pItem);
	void RemoveAll();

	bool GetItemRowCol(int idx, int& out_row, int& out_col);
	int GetSelIndex() { return m_nSelItem; }
	int GetMouseOverIndex() { return m_nOverItem; }
	MMultiColListItem* GetItemByIdx(int idx);
	MMultiColListItem* GetSelItem();
	bool SetSelIndex(int i);

	bool CalcItemRect(int idx, MRECT& out);
	int FindItem(MPOINT& p);

	void EnableDragAndDrop( bool bEnable) { m_bDragAndDrop = bEnable; }
	int FindNextItem(int i, char c);	// 문자키 입력으로 항목 찾기

	const char* GetSelItemString();

	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);
	virtual bool IsDropable(MWidget* pSender) { return m_bDragAndDrop; }

	virtual void MultiplySize(float byIDLWidth, float byIDLHeight, float byCurrWidth, float byCurrHeight);

	MMultiColListItem* Get(int i);
	const char* GetString(int i);
	bool IsSelected() { return m_nSelItem != -1; }
	void ShowItem(int i);

	void SetOnDropCallback(ZCB_ONDROP pCallback) { m_pOnDropFunc = pCallback; }
	bool OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);

	bool GetItemPos(MPOINT* p, int i) { return false; }	// 딱히 사용처가 없어 구현보류

protected:
	virtual void OnSize(int w, int h);

	void UpdateScrollBar();

public:
	DECLARE_LOOK(MMultiColListBoxLook)
	DECLARE_LOOK_CLIENT()

#define MINT_MULTICOLLISTBOX	"MultiColListBox"
	/// 클래스 이름 얻기
	virtual const char* GetClassName(void){ return MINT_MULTICOLLISTBOX; }
};

#endif
