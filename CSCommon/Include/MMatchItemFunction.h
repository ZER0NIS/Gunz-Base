#ifndef _MMATCHITEMFUNCTION_H
#define _MMATCHITEMFUNCTION_H



const int CalculateRentItemSellBounty( const int nBountyPrice, const int nRentHourPeriod, const int nRemainHour, bool bIsChangedToRent);


enum CASHITEM_TYPE
{
	CASHITEM_GAMBLE = 0,
	CASHITEM_SPENDABLE,
	CASHITEM_NORMAL
};

const int CalculateCashItemToBounty(CASHITEM_TYPE nType);
const int CalculateCashItemToBounty(CASHITEM_TYPE nType, int nResLevel, int nRentMinuteRemainder, MMatchItemSlotType slotType);

#endif