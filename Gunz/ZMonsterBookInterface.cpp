/***********************************************************************
  ZMonsterBookInterface.cpp
  
  용  도 : 몬스터 도감 인터페이스
  작성일 : 29, MAR, 2004
  작성자 : 임동환
************************************************************************/


#include "stdafx.h"							// Include stdafx.h
#include "ZMonsterBookInterface.h"			// Include ZMonsterBookInterface.h
#include "ZQuest.h"							// Include ZQuest.h
#include "ZGameInterface.h"
#include "ZApplication.h"
#include "ZItemIconBitmap.h"
#include "ZModule_Skills.h"



/***********************************************************************
  ZMonsterBookInterface : public
  
  desc : 생성자
************************************************************************/
ZMonsterBookInterface::ZMonsterBookInterface( void)
{
	m_pBookBgImg = NULL;
	m_pIllustImg = NULL;
	m_fCompleteRate = 0.0f;
}


/***********************************************************************
  ~ZMonsterBookInterface : public
  
  desc : 소멸자
************************************************************************/
ZMonsterBookInterface::~ZMonsterBookInterface( void)
{
	OnDestroy();
}


/***********************************************************************
  OnCreate : public
  
  desc : 몬스터 도감 보이기
  arg  : none
  ret  : none
************************************************************************/
void ZMonsterBookInterface::OnCreate( void)
{
	// Get resource pointer
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();


	// 변수 초기화
	m_nTotalPageNum = 0;
	m_nCurrentPageNum = 0;


	// 퀘스트에 필요한 정보 로딩
	LoadMonsterBookInfo();


	// 로비 UI 감추기
	MWidget* pWidget = pResource->FindWidget( "Lobby");
	if ( pWidget)
		pWidget->Show( false);


	// 페이지를 그린다
	DrawPage();


	// 몬스터 도감 보이기
	pWidget = pResource->FindWidget( "MonsterBook");
	if ( pWidget)
		pWidget->Show( true);

	
	// 스트립 이미지 애니메이션
	MPicture* pPicture = (MPicture*)pResource->FindWidget( "MonsterBook_StripBottom");
 	if( pPicture)
		pPicture->SetAnimation( 0, 1000.0f);
	pPicture = (MPicture*)pResource->FindWidget( "MonsterBook_StripTop");
	if( pPicture)
		pPicture->SetAnimation( 1, 1000.0f);
}


/***********************************************************************
  OnDestroy : public
  
  desc : 몬스터 도감 감추기
  arg  : none
  ret  : none
************************************************************************/
void ZMonsterBookInterface::OnDestroy( void)
{
	// Get resource pointer
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();


	// 몬스터 도감 감추기
	MWidget* pWidget = pResource->FindWidget( "MonsterBook");
	if ( pWidget)
		pWidget->Show( false);


	// 배경 책 이미지를 메모리로부터 삭제한다
	if ( m_pBookBgImg != NULL)
	{
		// 배경 책 이미지를 보여주는 위젯의 비트맵 이미지 포인터를 리셋한다
		MPicture* pPicture = (MPicture*)pResource->FindWidget( "MonsterBook_BookBG");
		if ( pPicture)
			pPicture->SetBitmap( NULL);
	
		delete m_pBookBgImg;
		m_pBookBgImg = NULL;
	}


	// 해당 몬스터의 일러스트 이미지 삭제
	if ( m_pIllustImg)
	{
		MPicture* pPicture = (MPicture*)pResource->FindWidget( "MonsterBook_MonsterIllust");
		if ( pPicture)
			pPicture->SetBitmap( NULL);
	
		delete m_pIllustImg;
		m_pIllustImg = NULL;
	}


	// 몬스터 도감 페이지 정보 리스트 삭제
	if ( !m_mapMonsterBookPage.empty())
	{
		for ( ZMonsterBookPageItr itr = m_mapMonsterBookPage.begin();  itr != m_mapMonsterBookPage.end();  itr++)
			delete (*itr).second;

		m_mapMonsterBookPage.clear();
	}


	// 로비 UI 보이기
	pWidget = pResource->FindWidget( "Lobby");
	if ( pWidget)
		pWidget->Show( true);
}


/***********************************************************************
  OnPrevPage : public
  
  desc : 이전 페이지 넘기기 버튼을 눌렀을 때
  arg  : none
  ret  : none
************************************************************************/
void ZMonsterBookInterface::OnPrevPage( void)
{
	// 보여줄 페이지 번호를 구한다
	if ( m_nCurrentPageNum == 0)
		m_nCurrentPageNum = m_nTotalPageNum;
	else
		m_nCurrentPageNum--;


	// 페이지를 그린다
	DrawPage();
}


/***********************************************************************
  OnNextPage : public
  
  desc : 다음 페이지 넘기기 버튼을 눌렀을 때
  arg  : none
  ret  : none
************************************************************************/
void ZMonsterBookInterface::OnNextPage( void)
{
	// 보여줄 페이지 번호를 구한다
	if ( m_nCurrentPageNum == m_nTotalPageNum)
		m_nCurrentPageNum = 0;
	else
		m_nCurrentPageNum++;


	// 페이지를 그린다
	DrawPage();
}


/***********************************************************************
  DrawPage : protected
  
  desc : 페이지를 그린다
  arg  : none
  ret  : none
************************************************************************/
void ZMonsterBookInterface::DrawPage( void)
{
	// 페이지 정보를 가져온다
	ZMonsterBookPageItr itr = m_mapMonsterBookPage.find( m_nCurrentPageNum);
	ZMonsterBookPageInfo* pPageInfo;
	if ( itr != m_mapMonsterBookPage.end())
		pPageInfo = (*itr).second;
	else
		pPageInfo = new ZMonsterBookPageInfo;


	// Get resource pointer
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();


	// 배경 이미지 로딩
	if ( m_pBookBgImg != NULL)
	{
		delete m_pBookBgImg;
		m_pBookBgImg = NULL;
	}

	m_pBookBgImg = new MBitmapR2;
	if ( m_nCurrentPageNum == 0)
		((MBitmapR2*)m_pBookBgImg)->Create( "monsterIllust.png", RGetDevice(), "interface/MonsterIllust/book_firstbg.jpg");
	else
		((MBitmapR2*)m_pBookBgImg)->Create( "monsterIllust.png", RGetDevice(), "interface/MonsterIllust/book_bg.jpg");

	if ( m_pBookBgImg)
	{
		// 읽어온 비트맵 이미지 포인터를 해당 위젯에 넘겨줘서 표시한다
		MPicture* pPicture = (MPicture*)pResource->FindWidget( "MonsterBook_BookBG");
		if ( pPicture)
			pPicture->SetBitmap( m_pBookBgImg->GetSourceBitmap());
	}


	// 해당 몬스터의 일러스트 이미지를 업데이트 한다
	if ( m_pIllustImg)
	{
		delete m_pIllustImg;
		m_pIllustImg = NULL;
	}
	MPicture* pPicture = (MPicture*)pResource->FindWidget( "MonsterBook_MonsterIllust");
	if ( pPicture)
		pPicture->SetBitmap( NULL);


	m_pIllustImg = new MBitmapR2;
	char szFileName[ 256];
	sprintf( szFileName, "interface/MonsterIllust/monster_Illust%02d.jpg", pPageInfo->m_nID);
	((MBitmapR2*)m_pIllustImg)->Create( "monsterIllust.png", RGetDevice(), szFileName);
	if ( m_pIllustImg)
	{
		MPicture* pPicture = (MPicture*)pResource->FindWidget( "MonsterBook_MonsterIllust");
		if ( pPicture)
			pPicture->SetBitmap( m_pIllustImg->GetSourceBitmap());
	}


	// 해당 몬스터의 이름을 업데이트 한다
	MLabel* pLabel = (MLabel*)pResource->FindWidget( "MonsterBook_MonsterName");
	if (pLabel)
	{
		pLabel->SetText( m_nCurrentPageNum ? pPageInfo->m_strName.data() : "");
		pLabel->Show( true);
	}


	// 해당 몬스터의 등급을 업데이트 한다
	pLabel = (MLabel*)pResource->FindWidget( "MonsterBook_MonsterGrade");
	if ( pLabel)
	{
		char szGrade[ 64] = { 0, };

		if ( m_nCurrentPageNum)
		{
			sprintf( szGrade, "%s : ", ZMsg(MSG_WORD_GRADE));

			switch ( pPageInfo->m_nGrade)
			{
				case NPC_GRADE_REGULAR :
					strcat( szGrade, ZMsg(MSG_WORD_REGULAR));
					break;

				case NPC_GRADE_LEGENDARY :
					strcat( szGrade, ZMsg(MSG_WORD_LEGENDARY));
					break;

				case NPC_GRADE_BOSS :
					strcat( szGrade, ZMsg(MSG_WORD_BOSS));
					break;

				case NPC_GRADE_ELITE :
					strcat( szGrade, ZMsg(MSG_WORD_ELITE));
					break;

				case NPC_GRADE_VETERAN :
					strcat( szGrade, ZMsg(MSG_WORD_VETERAN));
					break;
			}
		}

		pLabel->SetText( szGrade);
		pLabel->Show( true);
	}


	// 해당 몬스터의 설명을 업데이트 한다
	MTextArea* pTextArea = (MTextArea*)pResource->FindWidget( "MonsterBook_MonsterDesc");
	if ( pTextArea)
	{
		pTextArea->Clear();
		pTextArea->AddText( m_nCurrentPageNum ? pPageInfo->m_strDesc.data() : "", MCOLOR( 0xFF321E00));
		pTextArea->Show();
	}


	// 해당 몬스터의 HP를 업데이트 한다
	pLabel = (MLabel*)pResource->FindWidget( "MonsterBook_MonsterHP");
	if ( pLabel)
	{
		char szHP[ 128] = { 0, };

		if ( m_nCurrentPageNum)
		{
			strcpy( szHP, "HP : ");

			if ( pPageInfo->m_nHP > 200)
				strcat( szHP, ZMsg(MSG_WORD_VERYHARD));
			else if ( pPageInfo->m_nHP > 120)
				strcat( szHP, ZMsg(MSG_WORD_HARD));
			else if ( pPageInfo->m_nHP > 80)
				strcat( szHP, ZMsg(MSG_WORD_NORAML));
			else if ( pPageInfo->m_nHP > 30)
				strcat( szHP, ZMsg(MSG_WORD_WEAK));
			else
				strcat( szHP, ZMsg(MSG_WORD_VERYWEAK));
		}

		pLabel->SetText( szHP);
		pLabel->Show( true);
	}


	// 해당 몬스터의 특수기를 업데이트 한다
	pTextArea = (MTextArea*)pResource->FindWidget( "MonsterBook_Attacks");
	if ( pTextArea)
	{
		pTextArea->Clear();

		for ( list<string>::iterator itrSkill = pPageInfo->m_Skill.begin();  itrSkill != pPageInfo->m_Skill.end();  itrSkill++)
			pTextArea->AddText( (*itrSkill).data(), MCOLOR( 0xFF321E00));
	}


	// 드롭 아이템 업데이트
	list<ZDropItemInfo*>::iterator  itrDropItem = pPageInfo->m_DropItem.begin();
	for ( int i = 0;  i < 10;  i++)
	{
		char szWidgetName[ 50];
		sprintf( szWidgetName, "MonsterBook_DropItem%d", i);
		MPicture* pPicture = (MPicture*)pResource->FindWidget( szWidgetName);
		if ( pPicture)
		{
			if ( itrDropItem != pPageInfo->m_DropItem.end())
			{
				pPicture->AttachToolTip( (*itrDropItem)->m_strName.data());
				pPicture->SetBitmap( (*itrDropItem)->m_pIcon);
				pPicture->Show( true);

				itrDropItem++;
			}
			else
			{
				pPicture->Show( false);
			}
		}
	}


	// 페이지 번호를 업데이트 한다
	pLabel = (MLabel*)pResource->FindWidget( "MonsterBook_PageNumber");
	if ( pLabel)
	{
		char szPageNum[ 20] = { 0, };

		if ( m_nCurrentPageNum)
			sprintf( szPageNum, "- %d -", m_nCurrentPageNum);

		pLabel->SetText( szPageNum);
	}


	// 달성률을 표시한다
	pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "MonsterBook_Complete");
	if ( pLabel)
	{
		char szComplete[ 128] = { 0, };

		if ( m_nCurrentPageNum == 0)
			sprintf( szComplete, "%s : %.1f%%", ZMsg(MSG_WORD_RATE), m_fCompleteRate);

		pLabel->SetText( szComplete);
	}


	// 이전 페이지
	MWidget* pWidget = pResource->FindWidget( "MonsterBook_PrevPageButton");
	if ( pWidget)
		pWidget->Show( (m_nCurrentPageNum > 0) ? true : false);


	// 다음 페이지
	pWidget = pResource->FindWidget( "MonsterBook_NextPageButton");
	if ( pWidget)
		pWidget->Show( (m_nCurrentPageNum < m_nTotalPageNum) ? true : false);


	// 메모리 삭제
	if ( itr == m_mapMonsterBookPage.end())
		delete pPageInfo;
}


// LoadMonsterBookInfo
bool ZMonsterBookInterface::LoadMonsterBookInfo( void)
{
	// 몬스터 도감 페이지 정보 리스트 삭제
	if ( !m_mapMonsterBookPage.empty())
	{
		for ( ZMonsterBookPageItr itr = m_mapMonsterBookPage.begin();  itr != m_mapMonsterBookPage.end();  itr++)
			delete (*itr).second;

		m_mapMonsterBookPage.clear();
	}


	// NPC 정보를 구함
	ZGetQuest()->Load();
	MQuestNPCCatalogue* pNPCCatalogue = ZGetQuest()->GetNPCCatalogue();
	if ( pNPCCatalogue == NULL)
		return false;


	int nTotalItemCount = 0;
	int nTotalCollectItemCount = 0;
	m_nTotalPageNum = 0;


	for ( map<MQUEST_NPC, MQuestNPCInfo*>::iterator itr = pNPCCatalogue->begin();  itr != pNPCCatalogue->end();  itr++)
	{
		// 하드 코드 : 이벤트용 NPC인 경우엔 빼준다
		if ( (*itr).first >= 100)
			continue;


		// 리스트 노드 생성
		MQuestNPCInfo* pNPCInfo = (*itr).second;
		ZMonsterBookPageInfo* pMonsterBookPageInfo = new ZMonsterBookPageInfo;

		pMonsterBookPageInfo->m_nID			= (*itr).first;
		pMonsterBookPageInfo->m_strName		= pNPCInfo->szName;
		pMonsterBookPageInfo->m_nGrade		= (int)pNPCInfo->nGrade;
		pMonsterBookPageInfo->m_strDesc		= pNPCInfo->szDesc;
		pMonsterBookPageInfo->m_nHP			= pNPCInfo->nMaxHP;


		// 스킬 정보를 구함
		for ( int i = 0;  i < pNPCInfo->nSkills;  i++)
		{
			ZSkillManager* pSkillMgr = ZGetApplication()->GetSkillManager();
			map<int,ZSkillDesc*>::iterator itrSkill = pSkillMgr->find( pNPCInfo->nSkillIDs[ i]);
			if ( itrSkill != pSkillMgr->end())
			{
				string strSkill = (*itrSkill).second->szName;

				pMonsterBookPageInfo->m_Skill.push_back( strSkill);
			}
		}


		// 드롭 아이템 목록을 구함
		MQuestDropSet* pDropItem = ZGetQuest()->GetDropTable()->Find( pNPCInfo->nDropTableID);
		if ( pDropItem)
		{
			// 드롭 아이템 갯수
			int nDropItemCount = (int)pDropItem->GetQuestItems().size();
			nTotalItemCount += nDropItemCount;


			// 수집한 아이템 갯수
			int nCollectItemCount = 0;


			for ( set<int>::iterator itrItem = pDropItem->GetQuestItems().begin();  itrItem != pDropItem->GetQuestItems().end();  itrItem++)
			{
				ZDropItemInfo* pItemNode = new ZDropItemInfo;

				// 아이템을 획득한적이 있는지 검사
				ZMyQuestItemMap::iterator itr = ZGetMyInfo()->GetItemList()->GetQuestItemMap().find( *itrItem);
				if ( itr != ZGetMyInfo()->GetItemList()->GetQuestItemMap().end())
				{
					// 획득한 아이템 갯수 추가
					nCollectItemCount++;


					// 아이템 정보를 구함
					MQuestItemDesc* pQItemDesc = GetQuestItemDescMgr().FindQItemDesc( *itrItem);
					if ( pQItemDesc)
					{
						pItemNode->m_strName	= pQItemDesc->m_szQuestItemName;
						pItemNode->m_pIcon		= ZGetGameInterface()->GetQuestItemIcon( *itrItem, false);
					}
					else
					{
						MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc( *itrItem);
						if ( pItemDesc)
						{
							pItemNode->m_strName	= pItemDesc->m_pMItemName->Ref().m_szItemName;
							pItemNode->m_pIcon		= GetItemIconBitmap( pItemDesc);
						}
					}
				}
				// 획득한 적이 없으면 ???로 표시
				else
				{
					pItemNode->m_strName	= "?????";
					pItemNode->m_pIcon		= MBitmapManager::Get( "slot_icon_unknown.tga");
				}


				// 노드 추가
				pMonsterBookPageInfo->m_DropItem.push_back( pItemNode);
			}


			// 전체 수집한 아이템 갯수 추가
			nTotalCollectItemCount += nCollectItemCount;
	
			
			// 페이지 달성률 구함
			if ( nDropItemCount > 0)
				pMonsterBookPageInfo->m_fCompleteRate = (float)nCollectItemCount / (float)nDropItemCount * 100.0f;
		}


		// 노드 추가
		m_mapMonsterBookPage.insert( ZMonsterBookPage::value_type( m_nTotalPageNum + 1, pMonsterBookPageInfo));
		m_nTotalPageNum++;
	}


	// 전체 달성률
	if ( nTotalItemCount > 0)
		m_fCompleteRate	= (float)nTotalCollectItemCount / (float)nTotalItemCount * 100.0f;


	return true;
}
