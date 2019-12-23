#ifndef _MBASEITEM_H
#define _MBASEITEM_H


// #define MAX_ITEM_COUNT					100		// 한사람이 최대로 갖을 수 있는 아이템 개수
#define RENT_PERIOD_UNLIMITED			(0)			// 기간제 아이이 무제한이면 최대 사용 시간은 0으로 설정이 된다. 
#define RENT_MINUTE_PERIOD_UNLIMITED	(525600)	// 클라이언트한테는 기간제 아이템 기간을 minute단위로 보낸다. 525600이면 무제한(1년)


// 아이템. 서버, 클라이언트 공통의 부모 클래스
// 서버는 MMatchItem
// 클라이언트는 ZMyItemNode
class MBaseItem
{
protected:
	bool				m_bIsRentItem;						///< 기간제 아이템인지 여부
	int					m_nRentMinutePeriodRemainder;		///< 기간제 남은기간(분단위)
	int					m_nCount;							///< 수량. 
	WORD				m_wRentHourPeriod;					///< 기간제 아이템일경우 최대 사용 시간.

public:
	MBaseItem(): m_bIsRentItem(false), m_nRentMinutePeriodRemainder(RENT_MINUTE_PERIOD_UNLIMITED), m_nCount(0), m_wRentHourPeriod(RENT_PERIOD_UNLIMITED)  { }
	virtual	~MBaseItem() {}
	
	int			GetRentMinutePeriodRemainder() const	{ return ((IsRentItem()) ? m_nRentMinutePeriodRemainder : RENT_MINUTE_PERIOD_UNLIMITED); }
	const WORD	GetRentHourPeriod() const				{ return m_wRentHourPeriod; }

	void		SetRentHourPeriod( const WORD wHour )	{ m_wRentHourPeriod = wHour; m_bIsRentItem = RENT_PERIOD_UNLIMITED != wHour; }

	bool		IsRentItem() const						{ return m_bIsRentItem; }

	void SetRentItem(int nRentMinutePeriodRemainder)	{ m_nRentMinutePeriodRemainder=nRentMinutePeriodRemainder; }
	
	int			GetItemCount() const					{ return m_nCount; }
	void		SetItemCount(int nVal)					{ m_nCount = nVal; }
};


#endif