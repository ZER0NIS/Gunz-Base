/***********************************************************************
  ZMonsterBookInterface.h
  
  용  도 : 몬스터 도감 인터페이스
  작성일 : 29, MAR, 2004
  작성자 : 임동환
************************************************************************/


#ifndef _ZMONSTERBOOKINTERFACE_H
#define _ZMONSTERBOOKINTERFACE_H


#include <map>
using namespace std;


// 드롭 아이템 정보
struct ZDropItemInfo
{
	string			m_strName;									// 이름
	MBitmap*		m_pIcon;									// 아이콘 비트맵

	ZDropItemInfo()
	{
		m_pIcon		= NULL;
	}
};


// 몬스터 도감 페이지 정보
class ZMonsterBookPageInfo
{
public:
	int						m_nID;								// ID
	string					m_strName;							// 이름
	int						m_nGrade;							// 등급
	string					m_strDesc;							// 설명
	int						m_nHP;								// HP
	list<string>			m_Skill;							// Skill
	list<ZDropItemInfo*>	m_DropItem;							// Drop item
	float					m_fCompleteRate;					// 달성률

	ZMonsterBookPageInfo()
	{
		m_nID		= 0;
		m_nGrade	= 0;
		m_nHP		= 0;
		m_fCompleteRate = 0.0f;
	}

	virtual ~ZMonsterBookPageInfo()
	{
		m_Skill.clear();

		while ( !m_DropItem.empty())
		{
			delete *m_DropItem.begin();
			m_DropItem.pop_front();
		}
	}
};

typedef map<int,ZMonsterBookPageInfo*>		ZMonsterBookPage;
typedef ZMonsterBookPage::iterator			ZMonsterBookPageItr;




// Class : ZMonsterBookInterface
class ZMonsterBookInterface
{
// protected varialbes
protected:
	MBitmapR2*			m_pBookBgImg;							// 배경 책 이미지
	MBitmapR2*			m_pIllustImg;							// 일러스트 비트맵 이미지

	ZMonsterBookPage	m_mapMonsterBookPage;					// 몬스터 도감 페이지 리스트

	int					m_nTotalPageNum;						// 페이지 수
	int					m_nCurrentPageNum;						// 현재 보고있는 페이지 번호를 기록

	float				m_fCompleteRate;						// 전체 달성률


// Functions
protected:
	void DrawPage( void);										// 페이지를 그린다
	bool LoadMonsterBookInfo( void);							// 페이지 정보를 로드한다

public:
	ZMonsterBookInterface( void);								// Constructor
	virtual ~ZMonsterBookInterface( void);						// Destructor

	void OnCreate( void);										// On Create
	void OnDestroy( void);										// On destroy

	void OnPrevPage( void);										// 이전 페이지 넘기기 버튼을 눌렀을 때
	void OnNextPage( void);										// 다음 페이지 넘기기 버튼을 눌렀을 때
};


#endif
