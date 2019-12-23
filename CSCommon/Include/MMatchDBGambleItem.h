#ifndef _MMATCHDBGAMBLEITEM_H
#define _MMATCHDBGAMBLEITEM_H


#include "MMatchDBBaseGambleItem.h"


#include <map>
#include <vector>


using std::map;
using std::vector;


class MMatchGambleRewardItem
{
private :
	DWORD	m_dwGambleRewardItemID;
	DWORD	m_dwGambleItemID;
	DWORD	m_dwItemIDMale;
	DWORD	m_dwItemIDFemale;
	DWORD	m_dwItemCnt;
	WORD	m_wRentHourPeriod;
	WORD	m_wRate;
	WORD	m_wRateRange;	// 이 값은 MMatchGambleItem에 AddGambleRewardItem으로 추가될때 계산되고, 
							//  Gamble을 할때 사용 된다.

private :
	MMatchGambleRewardItem() {}

public :
	MMatchGambleRewardItem( const DWORD dwGambleRewardItemID
		, const DWORD dwGambleItemID
		, const DWORD dwItemIDMale
		, const DWORD dwItemIDFemale
		, const DWORD dwItemCnt
		, const WORD wRentHourPeriod
		, const WORD wRate )
	{
		m_dwGambleRewardItemID	= dwGambleRewardItemID;
		m_dwGambleItemID		= dwGambleItemID;
		m_dwItemIDMale			= dwItemIDMale;
		m_dwItemIDFemale		= dwItemIDFemale;
		m_dwItemCnt				= dwItemCnt;
		m_wRentHourPeriod		= wRentHourPeriod;
		m_wRate					= wRate;
		m_wRateRange			= 0;
	}

	~MMatchGambleRewardItem() {}

	const DWORD GetGambleRewardItemID() const			{ return m_dwGambleRewardItemID; }
	const DWORD GetGambleItemID() const					{ return m_dwGambleItemID; }
	const DWORD GetItemIDMale() const					{ return m_dwItemIDMale; }
	const DWORD GetItemIDFemale() const					{ return m_dwItemIDFemale; }
	const DWORD GetItemCnt() const						{ return m_dwItemCnt; }
	const WORD	GetRentHourPeriod() const				{ return m_wRentHourPeriod; }
	const WORD	GetRate() const							{ return m_wRate; }
	const WORD	GetRateRange() const					{ return m_wRateRange; }

	void		SetRateRange( const WORD wRateRange )	{ m_wRateRange = wRateRange; }
};


class MMatchGambleItem : public MMatchDBBaseGambleItem
{
private :
	vector< MMatchGambleRewardItem* > m_RewardItemList;

	// 현제 시간을 기준으로의 차이.
    int		m_nStartTimeMin;
	int		m_nLifeTimeMin;
	bool	m_bIsOpened;		// 이값이 false이면 사용할 수 없음. 클라이언트에 보내는 리스트에도 제외됨.
	bool	m_bIsNoTimeLimit;	// 이값이 true이면 기간이 없음.

	WORD	m_dwTotalRate;

private :
	MMatchGambleItem() {}

public :
	MMatchGambleItem( const DWORD dwGambleItemID
		, const string& strName
		, const string& strDescription
		, const int nStartTimeMin
		, const int nLifeTimeMin
		, const DWORD dwPrice
		, const bool bIsCash
		, const bool bIsOpened );
	~MMatchGambleItem();

	bool							AddGambleRewardItem( MMatchGambleRewardItem* pGRItem );

	void							Release();

	const int						GetStartTimeMin() const		{ return m_nStartTimeMin; }
	const int						GetLifeTimeMin() const		{ return m_nLifeTimeMin; }
	const int						GetEndTimeMin() const		{ return m_nStartTimeMin + m_nLifeTimeMin; }
	const bool						IsOpened() const			{ return m_bIsOpened; }
	const bool						IsNoTimeLimitItem() const	{ return m_bIsNoTimeLimit; }
	const WORD						GetTotalRate() const		{ return m_dwTotalRate; }


	const MMatchGambleRewardItem*	GetGambleRewardItemByRate( const WORD wRate ) const;
};


#endif