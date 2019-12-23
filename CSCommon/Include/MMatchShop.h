#ifndef _MMATCHSHOP_H
#define _MMATCHSHOP_H

#include "winsock2.h"
#include "MXml.h"
#include "MUID.h"
#include <map>
#include <vector>
#include <algorithm>
using namespace std;

#include "MMatchItem.h"
#include "MQuestItem.h"
#include "MMatchDBGambleItem.h"

/// 상점 아이템 노드
/// - 현재 가격정보는 MMatchItem.h에, 기간정보는 여기에 있는 구조로 되어버렸음.
/// - 옮기려니 너무 대공사라 지금은 여기에 기간정보를 올리는 선에서 마침.
/// - 추후에 유지보수팀에서 MMatchItem.h에 있는 BountyPrice, BountyValue를 여기로 이사시켜주세요. - bird
/// - 건즈 개발이 종료됨으로 작업을 최소화 하기 위해서 ZItem에 그대로 유지하도록 함. - by SungE 2007-06-28
struct ShopItemNode
{
	unsigned int	nItemID;			/// 아이템 ID
	int				nItemCount;
	bool			bIsRentItem;		/// 사용 안함. 기간 아이템인지 여부(일반 아이템만 적용)
	int				nRentPeriodHour;	/// 사용 안함. 기간(1 = 1시간)
	

	ShopItemNode() : nItemID(0), nItemCount(0), bIsRentItem(false), nRentPeriodHour(0) {}
};

/// 상점
class MMatchShop
{
private:
protected:
	vector<ShopItemNode*>					m_ItemNodeVector;
	map<unsigned int, ShopItemNode*>		m_ItemNodeMap;

	void ParseSellItem(MXmlElement& element);
	bool ReadXml(const char* szFileName);


public:
	MMatchShop();
	virtual ~MMatchShop();
	bool Create(const char* szDescFileName);
	void Destroy();

	void Clear();
	int GetCount() { return static_cast< int >( m_ItemNodeVector.size() ); }
	bool IsSellItem(const unsigned long int nItemID);		// 해당 아이템을 팔고 있는지 여부
	ShopItemNode* GetSellItemByIndex(int nListIndex);			// 샵에서 팔고 있는 nListIndex번째 아이템 명세 반환
	ShopItemNode* GetSellItemByItemID(int nItemID);				// 샵에서 팔고 있는 nItemID 아이템 명세 반환
	
	static MMatchShop* GetInstance();


#ifdef _DEBUG
	void MakeShopXML();
#endif
};

inline MMatchShop* MGetMatchShop() { return MMatchShop::GetInstance(); }

#define MTOK_SELL					"SELL"
#define MTOK_SELL_ITEMID			"itemid"
#define MTOK_SELL_ITEM_COUNT		"item_Count"		
#define MTOK_SELL_RENT_PERIOD_HOUR	"rent_period"




#endif