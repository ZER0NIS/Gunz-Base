#pragma once

#include "MMsgBox.h"

class ZShopEquipInterface;
class ZShopEquipListItem;


////////////////////////////////////////////////////////////////////////////////////
//		아이템 수량 입력 대화상자
////////////////////////////////////////////////////////////////////////////////////

struct IItemCountDlgDoneHandler
{
	virtual void OnDone(int nCount) = 0;
};

// ZItemCountDlg의 사용 용도
enum ZITEMCOUNTDLG_MODE {
	ZICD_SELL,			// 판매
	ZICD_BUY,			// 구매
	ZICD_SENDACCOUNT,	// 중앙은행에 보내기
	ZICD_BRINGACCOUNT,	// 중앙은행에서 가져오기
};

// 구입,판매,은행출납 목적으로 아이템의 수량을 입력할 수 있는 대화상자
class ZItemCountDlg
{
	ZITEMCOUNTDLG_MODE	m_mode;
	string		m_strItemName;
	MBitmap*	m_pItemIcon;
	int			m_nPrice;
	int			m_nMin, m_nMax;
	IItemCountDlgDoneHandler* m_pDoneHandler;

	int			m_nCurrCount;

public:
	void Open(ZITEMCOUNTDLG_MODE mode, const char* szItemName, MBitmap* pIcon, int price, int nMin, int nMax, IItemCountDlgDoneHandler* pHandler);
	void Close();
	void UpdateDlg();
	void AddCount(int n);
	void OnEditBoxChanged();
	void OnDlgDone();
};



////////////////////////////////////////////////////////////////////////////////////
//		간단한 확인용 대화상자 ( 메시지박스 )
////////////////////////////////////////////////////////////////////////////////////

///// 구입,판매 확인 메시지 박스의 완료 핸들러
struct ISimpleConfirmDlgDoneHandler
{
	virtual void OnDone(bool bOk) = 0;
};

// 구입,판매시 간단하게 유저 확인을 받는 메시지박스
class ZSimpleConfirmDlg : public MListener
{
	MMsgBox* m_pMsgbox;
	ISimpleConfirmDlgDoneHandler* m_pDoneHandler;

public:
	ZSimpleConfirmDlg();
	~ZSimpleConfirmDlg();

	virtual bool OnCommand(MWidget* pWidget, const char* szMessage);

	void Open(const char* szMsg, ISimpleConfirmDlgDoneHandler* pHandler);
};



////////////////////////////////////////////////////////////////////////////////////
//		캐쉬 아이템 팔기 확인용 대화상자
////////////////////////////////////////////////////////////////////////////////////

///// 확인 대화상자의 완료 핸들러
struct ISellCashItemConfirmDlgDoneHandler
{
	virtual void OnConfirmSellCashItem(bool bOk) = 0;
};

// 캐쉬 아이템을 바운티를 받고 판매할 때의 경고 대화상자
class ZSellCashItemConfirmDlg
{
	ISellCashItemConfirmDlgDoneHandler* m_pDoneHandler;
	DWORD m_nWaitActivatingOkBtnBeginTime;
	bool m_bWaitActivatingOkBtn;

public:
	ZSellCashItemConfirmDlg();
	~ZSellCashItemConfirmDlg();

	void Open(const char* szItemName, MBitmap* pIcon, int price, int count, ISellCashItemConfirmDlgDoneHandler* pHandler);
	void Update();

	void Close();
	void OnOkButton();
};



////////////////////////////////////////////////////////////////////////////////////
//		캐쉬 아이템 구매 확인용 대화상자 (기간이나 묶음 갯수 선택)
////////////////////////////////////////////////////////////////////////////////////

// 완료 핸들러
struct ICashItemConfirmDlgDoneHandler
{
	virtual void OnDone(bool bOk) = 0;
};

class ZCashItemConfirmDlg
{
public:
	ZCashItemConfirmDlg();
	~ZCashItemConfirmDlg();

	void Open(MBitmap* pItemIcon, ICashItemConfirmDlgDoneHandler* pHandler);
};