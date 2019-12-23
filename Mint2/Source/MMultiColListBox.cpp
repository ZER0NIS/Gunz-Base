#include "stdafx.h"
#include "MMultiColListBox.h"
#include "MColorTable.h"
#include "Mint.h"

IMPLEMENT_LOOK(MMultiColListBox, MMultiColListBoxLook)

MMultiColListBox::MMultiColListBox(const char* szName, MWidget* pParent, MListener* pListener)
: MWidget(szName, pParent, pListener)
, m_numColumn(1)
, m_desiredNumRow(4)
, m_itemHeight(16)
, m_maxRowCanShow(0)
, m_nSelItem(-1)
, m_nOverItem(-1)
, m_bDragAndDrop(false)
{
	LOOK_IN_CONSTRUCTOR();

	m_pScrollBar = new MScrollBar(this, this);

	m_FontAlign = MAM_NOTALIGN;
	m_pOnDropFunc = NULL;
	
	SetFocusEnable(true);
}

MMultiColListBox::~MMultiColListBox()
{
	RemoveAll();
	delete m_pScrollBar;
}

void MMultiColListBox::SetDesiredNumRow(int n)
{
	m_desiredNumRow = n;

	CalcItemHeight();
}

void MMultiColListBox::CalcItemHeight()
{
	m_itemHeight = GetInitialClientRect().h / m_desiredNumRow;
}

void MMultiColListBox::SetNumColumn(int n)
{
	m_numColumn = n;
	if (m_numColumn < 1)
	{
		_ASSERT(0);
		m_numColumn = 1;
	}

	UpdateScrollBar();
}

void MMultiColListBox::SetItemHeight(int h)
{
	m_itemHeight = h;
	if (m_itemHeight < 1)
	{
		_ASSERT(0);	
		m_itemHeight = 1;
	}

	UpdateScrollBar();
}

void MMultiColListBox::InsertSortedPos(MMultiColListItem* pItem)
{
	// 새 항목을 정렬된 위치에 삽입한다.
	// 현재 항목들에서 정렬을 위한 값을 얻어서 대소비교하여 위치를 찾는다.
	// 만약 같은 값이 이미 존재하면 기존 같은 값들의 뒤에 넣는다. (상점아이템의 종류별 정렬을 위한 것)
	int nSortHint = pItem->GetSortHint();
	for (ListMultiColListItem::iterator it=m_items.begin(); it!=m_items.end(); ++it)
	{
		if ((*it)->GetSortHint() <= nSortHint)
			continue;

		m_items.insert(it, pItem);
		return;
	}

	m_items.push_back(pItem);
}

void MMultiColListBox::Add(MMultiColListItem* pItem)
{
	InsertSortedPos(pItem);

	if (m_nSelItem == -1)
		m_nSelItem = 0;

	UpdateScrollBar();
}

void MMultiColListBox::Remove(MMultiColListItem* pItem)
{
	for (ItorMultiColListItem it=m_items.begin(); it!=m_items.end(); ++it)
	{
		if ((*it) == pItem)
		{
			m_items.erase(it);
			if (m_nSelItem >= GetNumItem())
				m_nSelItem = GetNumItem()-1;
			UpdateScrollBar();
			break;
		}
	}
	return;
}

void MMultiColListBox::RemoveAll()
{
	for (ItorMultiColListItem it=m_items.begin(); it!=m_items.end(); ++it)
		delete *it;
	m_items.clear();

	m_pScrollBar->SetValue(0);
	UpdateScrollBar();
	
	m_nSelItem = -1;
	m_nOverItem = -1;
}

void MMultiColListBox::UpdateScrollBar()
{
	//// 스크롤의 범위를 지정
	MRECT& r = GetClientRect();

	// 한 화면에서 보여줄 수 있는 행 수 계산
	m_maxRowCanShow = r.h / m_itemHeight;
	
	// 아이템들의 전체 행 수 계산
	int numRow = ((int)m_items.size() / m_numColumn);
	if (((int)m_items.size() % m_numColumn) > 0)
		numRow += 1;
	
	// 언제나 그렇듯 스크롤바 최대범위는 한화면 분량을 빼줘야..
	int scrollmax = numRow - m_maxRowCanShow;
	if (scrollmax < 0)
		scrollmax = 0;

	m_pScrollBar->SetMinMax(0, scrollmax);

	// 한 화면에서 보여줄 수 있는 행 수에 짤려서 보여지는 것도 포함시킨다 (렌더링을 위해서)
	if (r.h % m_itemHeight > 0)
		m_maxRowCanShow += 1;
}

void MMultiColListBox::OnSize(int w, int h)
{
	MRECT cr = GetInitialClientRect();
	m_pScrollBar->SetBounds(MRECT(cr.x+cr.w-m_pScrollBar->GetDefaultBreadth(), cr.y+1, m_pScrollBar->GetDefaultBreadth(), cr.h-1));

	CalcItemHeight();

	UpdateScrollBar();
}

MMultiColListItem* MMultiColListBox::GetItemByIdx(int idx)
{
	if (idx < 0) return NULL;
	if (idx >= GetNumItem()) return NULL;
	int i=0;
	for (ItorMultiColListItem it=m_items.begin(); it!=m_items.end(); ++it, ++i)
	{
		if (i == idx)
			return *it;
	}
	return NULL;
}

MMultiColListItem* MMultiColListBox::GetSelItem()
{
	return GetItemByIdx(m_nSelItem);
}

bool MMultiColListBox::SetSelIndex(int i)
{
	if (i < 0 || GetNumItem() <= i) return false;

	m_nSelItem = i;
	return true;
}

const char* MMultiColListBox::GetSelItemString()
{
	MMultiColListItem* pItem = GetItemByIdx(m_nSelItem);
	if (pItem)
		return pItem->GetString();
	return NULL;
}

int MMultiColListBox::FindItem(MPOINT& p)
{
	if (!GetClientRect().InPoint(p)) return -1;

	int idxItemFirstVisible = GetItemFirstVisible();
	int idxItemLastVisible = GetItemLastVisible();

	MRECT r;
	for (int idx=idxItemFirstVisible; idx<=idxItemLastVisible; ++idx)
	{
		CalcItemRect(idx, r);
		if (r.InPoint(p))
			return idx;
	}
	return -1;
}

int MMultiColListBox::FindNextItem(int i, char c)
{
	for(int s=0; s<GetNumItem()-1; s++){
		int idx = (i+s+1)%GetNumItem();
		const char* szItem = GetString(idx);

		if (szItem != NULL)
		{
			char a = (char)towupper(szItem[0]);
			char b = (char)towupper(c);
			if(a==b) return idx;
		}
	}
	return -1;
}


bool MMultiColListBox::CalcItemRect(int idx, MRECT& out)
{
	if (idx < 0 || GetNumItem() <= idx) return false;

	int idxItemFirstShow = GetItemFirstVisible();
	int idxItemLastShow = GetItemLastVisible();

	// 화면에 출력되지 않는 항목이면 그냥 리턴
	if (idx < idxItemFirstShow || idxItemLastShow < idx) return false;

	MRECT r = GetClientRect();
	int widthItem = r.w / m_numColumn;
	int heightItem = GetItemHeight();

	int col = idx % m_numColumn;
	int row = idx / m_numColumn;

	int xposItem = r.x + (widthItem * col);
	int yposItem = r.y + (heightItem * (row-GetRowFirstVisible()) );

	out.Set(xposItem+2, yposItem+2, widthItem-2, heightItem-2);
	if (out.y >= r.y + r.h) return false;

	return true;
}

bool MMultiColListBox::GetItemRowCol(int idx, int& out_row, int& out_col)
{
	if (m_numColumn == 0) return false;
	out_col = (idx % m_numColumn);
	out_row = (idx / m_numColumn);
	return true;
}

bool MMultiColListBox::OnEvent(MEvent* pEvent, MListener* pListener)
{
	MRECT r = GetClientRect();
	if(pEvent->nMessage==MWM_MOUSEMOVE){
		if(r.InPoint(pEvent->Pos)==false) return false;
		m_nOverItem = FindItem(pEvent->Pos);
	}
	else if(pEvent->nMessage==MWM_LBUTTONDOWN){
		if(r.InPoint(pEvent->Pos)==false)
		{
			if (pListener)
				pListener->OnCommand(this,MLB_ITEM_CLICKOUT);
			return false;
		}

		if(m_nDebugType==2){
			int k =0;
		}

		int nSelItem = FindItem(pEvent->Pos);
		if(nSelItem==-1) return true;
		SetSelIndex(nSelItem);

		if(pListener!=NULL) pListener->OnCommand(this, MLB_ITEM_SEL);

		// 드래그 & 드롭
		if ( m_bDragAndDrop)
		{
			MMultiColListItem* pItem = GetSelItem();
			if(pItem!=NULL)
			{
				MBitmap* pDragBitmap = NULL;
				char szDragString[256] = "";
				char szDragItemString[256] = "";
				if(pItem->GetDragItem(&pDragBitmap, szDragString, szDragItemString)==true){
					Mint::GetInstance()->SetDragObject(this, pDragBitmap, szDragString, szDragItemString);
				}
			}
		}

		return true;
	}
	else if(pEvent->nMessage==MWM_RBUTTONDOWN){
		if(r.InPoint(pEvent->Pos)==false)
		{
			pListener->OnCommand(this,MLB_ITEM_CLICKOUT);
			return false;
		}
		int nSelItem = FindItem(pEvent->Pos);
		if(nSelItem==-1) return true;
		SetSelIndex(nSelItem);

		if(m_nSelItem!=-1){
			if(pListener!=NULL) pListener->OnCommand(this, MLB_ITEM_SEL2);
			return true;
		}
	}
	else if(pEvent->nMessage==MWM_KEYDOWN){
		if(pEvent->nKey==VK_DELETE){
			if(pListener!=NULL) pListener->OnCommand(this, MLB_ITEM_DEL);
		}
		else if(pEvent->nKey==VK_UP){
			if(GetSelIndex()>0){
				SetSelIndex(GetSelIndex()-1);
				ShowItem(GetSelIndex());
				if(pListener!=NULL){
					if(m_nSelItem!=-1) pListener->OnCommand(this, MLB_ITEM_SEL);
					else pListener->OnCommand(this, MLB_ITEM_SELLOST);
				}
			}
		}
		else if(pEvent->nKey==VK_DOWN){
			if(GetSelIndex()<GetNumItem()-1){
				SetSelIndex(GetSelIndex()+1);
				ShowItem(GetSelIndex());
				if(pListener!=NULL){
					if(m_nSelItem!=-1) pListener->OnCommand(this, MLB_ITEM_SEL);
					else pListener->OnCommand(this, MLB_ITEM_SELLOST);
				}
			}
		}
	}
	else if(pEvent->nMessage==MWM_CHAR){
		int nIndex = FindNextItem(GetSelIndex(), pEvent->nKey);
		if(nIndex>=0){
			SetSelIndex(nIndex);
			ShowItem(nIndex);
			if(pListener!=NULL){
				if(m_nSelItem!=-1) pListener->OnCommand(this, MLB_ITEM_SEL);
				else pListener->OnCommand(this, MLB_ITEM_SELLOST);
			}
		}
	}
	else if(pEvent->nMessage==MWM_LBUTTONDBLCLK){
		if(r.InPoint(pEvent->Pos)==false) return false;
		int nSelItem = FindItem(pEvent->Pos);
		if(nSelItem==-1) return true;
		m_nSelItem = nSelItem;
		if(pListener!=NULL){
			if(m_nSelItem!=-1) pListener->OnCommand(this, MLB_ITEM_SEL);
			else pListener->OnCommand(this, MLB_ITEM_SELLOST);
			pListener->OnCommand(this, MLB_ITEM_DBLCLK);
		}
		return true;
	}
	else if(pEvent->nMessage==MWM_MOUSEWHEEL){
		if(r.InPoint(pEvent->Pos)==false) return false;
#define MAX_WHEEL_RANGE	2
		int newPos = m_pScrollBar->GetValue() + min(max(-pEvent->nDelta, -MAX_WHEEL_RANGE), MAX_WHEEL_RANGE);
		m_pScrollBar->SetValueAdjusted(newPos);
		return true;
	}
	return false;
}

void MMultiColListBox::MultiplySize( float byIDLWidth, float byIDLHeight, float byCurrWidth, float byCurrHeight )
{
	MWidget::MultiplySize( byIDLWidth, byIDLHeight, byCurrWidth, byCurrHeight );

	//SetItemHeight(int(GetItemHeight() * byCurrHeight));
}

MMultiColListItem* MMultiColListBox::Get( int i )
{
	if (i < 0) return NULL;
	if ((int)m_items.size() <= i) return NULL;

	ItorMultiColListItem it=m_items.begin();
	for (int k=0; k<i; ++k)
		++it;
	return *it;

	return NULL;
}

const char* MMultiColListBox::GetString( int i )
{
	MMultiColListItem* pItem = Get(i);
	if (pItem)
		return pItem->GetString();
	return NULL;
}

void MMultiColListBox::ShowItem( int i )
{
	int row, col;
	if (!GetItemRowCol(i, row, col)) return;

	int rowFirstVisible = GetRowFirstVisible();
	int rowLastVisible = GetRowLastVisible();

	if (rowFirstVisible <= row && row <= rowLastVisible) return;

	m_pScrollBar->SetValueAdjusted(row);
}

bool MMultiColListBox::OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	if ( m_pOnDropFunc != NULL)
	{
		m_pOnDropFunc(this, pSender, pBitmap, szString, szItemString);
	}

	return true;
}

void MMultiColListBoxLook::OnDraw(MMultiColListBox* pListBox, MDrawContext* pDC)
{
	m_ItemTextAlignmentMode = pListBox->m_FontAlign;

	MRECT rcClient = pListBox->GetInitialClientRect();
	pDC->SetColor(MCOLOR(19,19,19,255));
	pDC->FillRectangle(rcClient);
	pDC->SetColor(MCOLOR(128,128,128,255));
	pDC->Rectangle(rcClient);

	int rowFirstVisible = pListBox->GetRowFirstVisible();
	int rowLastVisible = pListBox->GetRowLastVisible();

	MRECT r;
	int idx=0, col=0, row=0;
	ListMultiColListItem& items = pListBox->GetItemList();

	for (ItorMultiColListItem itor=items.begin(); itor!=items.end(); ++itor, ++idx)
	{
		if (false == pListBox->GetItemRowCol(idx, row, col)) continue;

		if (row < rowFirstVisible) continue;
		if (row > rowLastVisible) break;

		if (pListBox->CalcItemRect(idx, r))
			(*itor)->OnDraw(r, pDC, idx==pListBox->GetSelIndex(), idx==pListBox->GetMouseOverIndex());
	}
}

MRECT MMultiColListBoxLook::GetClientRect(MMultiColListBox* pListBox, MRECT& r)
{
	return MRECT(r.x+1, r.y+1, (r.w - pListBox->GetScrollBar()->GetClientRect().w-2), r.h-2);
}

MMultiColListBoxLook::MMultiColListBoxLook()
{
	m_ItemTextAlignmentMode	= MAM_NOTALIGN;
}

void MMultiColListItem::OnDraw(MRECT& r, MDrawContext* pDC, bool bSelected, bool bMouseOver)
{
	pDC->SetColor(MCOLOR(DEFCOLOR_MLIST_TEXT));
	pDC->Rectangle(r);
	char sz[32];
	sprintf(sz, "%d", (DWORD)this);
	pDC->Text(r, sz, MAM_HCENTER|MAM_VCENTER);
}