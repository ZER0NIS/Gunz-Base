#ifndef _ZDUELTOURNAMENTRANKINGLISTBOX_H
#define _ZDUELTOURNAMENTRANKINGLISTBOX_H

#define NUM_DISPLAY_DUELTOURNAMENT_RANKING 5
#define INDEX_DUELTOURNAMENT_MY_RANKING 2			// 랭킹 목록에서 내 랭킹 아이템의 인덱스

struct ZDUELTOURNAMENTRANKINGITEM {
	char szCharName[MAX_CHARNAME+1];
	int nWins;
	int nLosses;

	int nFluctuation;		// 순위 변동
    int nRank;				// 현재 순위
	
	int nWinners;			// 우승횟수
	int nPoint;				// 토너먼트 포인트

	int nGrade;				// 등급 [1~10]

	bool bEmptyItem;

	ZDUELTOURNAMENTRANKINGITEM() : nWins(0), nLosses(0), nFluctuation(0), nRank(0), nWinners(0), nPoint(0), nGrade(0), bEmptyItem(true) {
		szCharName[0] = 0;
	}
};

class ZDuelTournamentRankingListBox : public MWidget {

	ZDUELTOURNAMENTRANKINGITEM m_rankingList[NUM_DISPLAY_DUELTOURNAMENT_RANKING];

	MBitmapR2* m_pBmpRankingItemBg;
	MBitmapR2* m_pBmpArrowUp;
	MBitmapR2* m_pBmpArrowDown;
	MBitmapR2* m_pBmpArrowBar;

	int m_nMyRankIndex;			// 내 랭킹 표시 인덱스

protected:
	virtual void	OnDraw( MDrawContext* pDC );

public:
	ZDuelTournamentRankingListBox(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	~ZDuelTournamentRankingListBox();

	void ClearAll();
	void SetRankInfo(unsigned int nIndex, const ZDUELTOURNAMENTRANKINGITEM& rankingItem);

	void LoadInterfaceImgs();
	void UnloadInterfaceImgs();

	void SetMyRankIndex(int myRankIndex) { m_nMyRankIndex = myRankIndex; }

};



#endif