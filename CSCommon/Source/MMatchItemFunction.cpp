#include "stdafx.h"
#include "MMatchItemFunction.h"


// nBountyPrice		: 상점 구입 가격
// nRentHourPeriod	: 최대 사용 기간(시간단위) DB에서 가져온다. (ZItem.xml의 기간정보랑 다른다.)
// nRemainHour		: 남은 사용 기간(시간단위) DB에서 가져온다. (ZItem.xml의 기간정보랑 다르다.)
// bIsChangedToRent : 무제한 아이템 인지 여부
// 
// 겜블을 통해서 나온 아이템은 퍼블리셔의 재량에 의해 기간 정보가 ZItem.xml 과 다를수 있기 때문에
// DB에 저장된 값으로 수식이 동작을 해야 가격의 차이가 생기지 않는다.
const int CalculateRentItemSellBounty( const int nBountyPrice
									  , const int nRentHourPeriod
									  , const int nRemainHour
									  , bool bIsChangedToRent )	
{
	// 무제한 아이템이 여기로 들어 오면 안된다.
	if( RENT_PERIOD_UNLIMITED == nRentHourPeriod )
	{
		//_ASSERT( 0 );
		return 0;
	}

	int iSellPrice = 0;
	// 상점에서 무제한으로 팔고 있는 아이템이면 구입가격에 변화가 생긴다.
	// 상점 구입 가격 = 상점에서 판매하는 기간제 아이템의 구입 가격 / 90 * (기간(날짜단위)) 로 변경 //
	if (bIsChangedToRent)
	{
		iSellPrice = (nBountyPrice / 90) * (nRentHourPeriod / 24);
	}
	else
	{
		iSellPrice = nBountyPrice;
	}
	// 상점 판매가격 = 상점 구입가격 * (현재 남은 사용시간 / 최대 사용 시간) * 0.5 (일의 자리수 버림)
	return ((int)(iSellPrice * ((double)nRemainHour / (double)nRentHourPeriod) * 0.5) / 10) * 10;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 겜블 아이템 - 개당 무조건 500원
// 소모성 아이템 - 개당 일괄 10바운티
const int CalculateCashItemToBounty(CASHITEM_TYPE nType)
{
	if( nType == CASHITEM_GAMBLE )
	{
		return 500;
	}
	else if( nType == CASHITEM_SPENDABLE ) 
	{
		return 10;
	}

	//_ASSERT(0);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 비소모성 일반 아이템 : 10 * 기간수정치 + (레벨제한 * 기간수정치 * 종류수정치)
// 
// 기간수정치 : 남은 1일당 1배, 최대 30배(무제한 50배)
// 종류수정치 : 무기 5배, 신발/장갑/머리 0.5배, 그외 1배
const int CalculateCashItemToBounty(CASHITEM_TYPE nType, int nResLevel, int nRentMinuteRemainder, MMatchItemSlotType slotType)
{
	if( nType == CASHITEM_GAMBLE || nType == CASHITEM_SPENDABLE ) {
		return CalculateCashItemToBounty(nType);
	}

	int nRentRatio;
	int nLevelRatio;
	float fSlotTypeRatio;

	if( RENT_MINUTE_PERIOD_UNLIMITED == nRentMinuteRemainder) {
		nRentRatio = 50;
	} else {
		int nRentDayPeriod = nRentMinuteRemainder / (60 * 24);
		nRentRatio = ( nRentDayPeriod > 30 ? 30 : nRentDayPeriod );
	}

	nLevelRatio  = ( nResLevel == 0 ? 1 : nResLevel );

	switch (slotType)
	{
	case MMIST_MELEE:
	case MMIST_RANGE:
		fSlotTypeRatio = 5;
		break;
	case MMIST_HEAD:
	case MMIST_HANDS:
	case MMIST_FEET:
		fSlotTypeRatio = 0.5f;
		break;
	default:
		fSlotTypeRatio = 1;
	}

	return (10 * nRentRatio) + int(nLevelRatio * nRentRatio * fSlotTypeRatio);
}
