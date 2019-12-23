#include "stdafx.h"
#include "ZServerView.h"
#include "ZApplication.h"



ZServerView::ZServerView(const char* szName, MWidget* pParent, MListener* pListener)
: MWidget(szName, pParent, pListener)
{
	m_cServerList.clear();
	m_nSelectNum = -1;
	m_nTextOffset = 0;
	m_nSelBoxSizeX = 270;
	m_nSelBoxSizeY = 17;
	m_nSelBoxSizeW = 270; //220;

	for(int i=0; i < AGENTMAXCOUNT; i++)
		m_dAgentPing[i] = 300;
	m_pPingBitmap[0] = MBitmapManager::Get( "AgentPing01.tga" );
	m_pPingBitmap[1] = MBitmapManager::Get( "AgentPing02.tga" );
	m_pPingBitmap[2] = MBitmapManager::Get( "AgentPing03.tga" );
	m_pPingBitmap[3] = MBitmapManager::Get( "AgentPing04.tga" );
	m_pPingBitmap[4] = MBitmapManager::Get( "AgentPing05.tga" );
	m_pPingBitmap[5] = MBitmapManager::Get( "AgentPing06.tga" );
}


ZServerView::~ZServerView(void)
{
	ClearServerList();
}

void ZServerView::OnDraw(MDrawContext* pDC)
{
	for ( int nRowBlock = 0;  nRowBlock < 2/*nRow*/;  nRowBlock++)
	{
		for ( int nColBlock = 0;  nColBlock < 3/*nColBlock*/;  nColBlock++)
		{
			int nSelect = 0;
			int nColCount = 0;
			int nPingImageCount = 0;
			for ( SERVERLIST::iterator itr = m_cServerList.begin();  itr != m_cServerList.end();  itr++)
			{
				ServerInfo* pServerInfo = (*itr);
				if ( (pServerInfo->nRow != nRowBlock) || (pServerInfo->nType == 0) || (pServerInfo->nCol != nColBlock))
				{
					nSelect++;
					nPingImageCount++;
					continue;
				}

				// Set region
				float fScreenRateX = MGetWorkspaceWidth() / 800.f;
				float fScreenRateY = MGetWorkspaceHeight() / 600.f;

#if defined(LOCALE_KOREA) 
				pServerInfo->rRect.x = nRowBlock * m_nSelBoxSizeX * fScreenRateX + 10;
				if(pServerInfo->nRow == 0)
					pServerInfo->rRect.y = nColCount * m_nSelBoxSizeY * fScreenRateY + (pServerInfo->nCol * m_nSelBoxSizeY* fScreenRateY * 5.7);
				else if(pServerInfo->nRow == 1)
					pServerInfo->rRect.y = nColCount * m_nSelBoxSizeY * fScreenRateY + (pServerInfo->nCol * m_nSelBoxSizeY * fScreenRateY * 4.2);
				pServerInfo->rRect.w = m_nSelBoxSizeW;
				pServerInfo->rRect.h = m_nSelBoxSizeY - 1;
#else
				pServerInfo->rRect.x = nRowBlock * m_nSelBoxSizeX * fScreenRateX;
				pServerInfo->rRect.y = nColCount * m_nSelBoxSizeY * fScreenRateY + (pServerInfo->nCol * 100*fScreenRateY);
				pServerInfo->rRect.w = m_nSelBoxSizeW;
				pServerInfo->rRect.h = m_nSelBoxSizeY - 1;
#endif

				// Check select
				bool bSelected = false;
				if ( nSelect == m_nSelectNum)
					bSelected = true;

				IconDraw(pDC, pServerInfo->rRect, pServerInfo->nType, bSelected);
				ServerListDraw(pDC, pServerInfo->rRect, pServerInfo, bSelected);
				PingImageDraw(pDC, pServerInfo->rRect, nPingImageCount, pServerInfo->bIsLive);

				nColCount++;
				nPingImageCount++;
				nSelect++;
			}
		}
	}
}

void ZServerView::PingImageDraw(MDrawContext* pDC, MRECT rectBox, int nImageCount, bool bIsLive)
{
	if ( !bIsLive)
		return;

	if(m_dAgentPing[nImageCount] >= 200)												pDC->SetBitmap(m_pPingBitmap[0]);
	else if(m_dAgentPing[nImageCount] < 200 && m_dAgentPing[nImageCount] >= 160)		pDC->SetBitmap(m_pPingBitmap[1]);
	else if(m_dAgentPing[nImageCount] < 160 && m_dAgentPing[nImageCount] >= 120)		pDC->SetBitmap(m_pPingBitmap[2]);
	else if(m_dAgentPing[nImageCount] < 120 && m_dAgentPing[nImageCount] >=  80)		pDC->SetBitmap(m_pPingBitmap[3]);
	else if(m_dAgentPing[nImageCount] <  80 && m_dAgentPing[nImageCount] >=  40)		pDC->SetBitmap(m_pPingBitmap[4]);
	else if(m_dAgentPing[nImageCount] <  40 && m_dAgentPing[nImageCount] >=   0)		pDC->SetBitmap(m_pPingBitmap[5]);
	else pDC->SetBitmap(m_pPingBitmap[0]);
	const int nMoveImage = int(220 * MGetWorkspaceWidth() / 800.f);
	pDC->Draw(rectBox.x+nMoveImage , rectBox.y, 25 , 16);

#ifdef  _DEBUG
	char strPing[16];
	if(0 < m_dAgentPing[nImageCount] && m_dAgentPing[nImageCount] < 1000000 )
	{
		sprintf(strPing, "%d", m_dAgentPing[nImageCount]);
		rectBox.w = 210;
		pDC->Text( rectBox, strPing, MAM_RIGHT | MAM_VCENTER);
	}
#endif
}

void ZServerView::IconDraw(MDrawContext* pDC, MRECT rectBox, int nType, bool bSelected)
{
	// Draw select bar
	if ( bSelected)
	{
		MCOLOR color = MCOLOR(0xFF454545);
		color.a = ((timeGetTime() % 150) < 75) ? 255 : 210;
		pDC->SetColor( color);
		rectBox.y = rectBox.y - 1 * MGetWorkspaceHeight() / 600.f;
		rectBox.w = 230 * MGetWorkspaceWidth() / 800.f;
		rectBox.h = 15 * MGetWorkspaceHeight() / 600.f;
		pDC->FillRectangle( rectBox);
	}

	// Draw Icon shadow
	pDC->SetColor( MCOLOR(0xFF000000));
	pDC->FillRectangle( rectBox.x + 4, rectBox.y + 7, 7, 4);
	pDC->FillRectangle( rectBox.x + 5, rectBox.y + 6, 5, 6);

	switch ( nType)
	{
	case 1 :	// Debug server
		//sprintf( szText, pServerInfo->szName, nDebugCount++);
		if ( bSelected)		pDC->SetColor( MCOLOR(0xFFFF0098));
		else				pDC->SetColor( MCOLOR(0xFFAC5387));
		break;

	case 2 :	// Match server
		//sprintf( szText, pServerInfo->szName, nMatchCount++);
		if ( bSelected)		pDC->SetColor( MCOLOR(0xFF00E9FF));
		else				pDC->SetColor( MCOLOR(0xFF53A4AC));
		break;

	case 3 :	// Clan server
		//sprintf( szText, pServerInfo->szName, nClanCount++);
		if ( bSelected)		pDC->SetColor( MCOLOR(0xFFFFBE00));
		else				pDC->SetColor( MCOLOR(0xFFAC9553));
		break;

	case 4 :	// Quest server
		//sprintf( szText, pServerInfo->szName, nQuestCount++);
		if ( bSelected)		pDC->SetColor( MCOLOR(0xFF44FF00));
		else				pDC->SetColor( MCOLOR(0xFF6BAC53));
		break;

	case 5 :	// Event server
		//sprintf( szText, pServerInfo->szName, nEventCount++);
		if ( bSelected)		pDC->SetColor( MCOLOR(0xFF8800FF));
		else				pDC->SetColor( MCOLOR(0xFF8453AC));
		break;
	}

	// Draw Icon
	pDC->FillRectangle( rectBox.x + 3, rectBox.y + 6, 7, 4);
	pDC->FillRectangle( rectBox.x + 4, rectBox.y + 5, 5, 6);
}

void ZServerView::ServerListDraw(MDrawContext* pDC, MRECT rectBox, ServerInfo* pServerInfo, bool bSelected)
{
	// Set server name
	char szText[ 100];
	if ( pServerInfo->bIsLive)
	{
		if ( pServerInfo->nNumOfUser >= pServerInfo->nCapacity)
			sprintf( szText, "%s (FULL / %d)", pServerInfo->szName, pServerInfo->nCapacity);
		else
			sprintf( szText, "%s (%d / %d)", pServerInfo->szName, pServerInfo->nNumOfUser, pServerInfo->nCapacity);
	}
	else
	{
		sprintf( szText, "%s (checking)", pServerInfo->szName);

		pDC->SetColor( MCOLOR(0xFF505050));
	}

	// Draw name(아이콘 크기 만큼 띄어준다.)
	pDC->SetColor( MCOLOR(0xFF000000));
	rectBox.x += 14;
	rectBox.y += m_nTextOffset + 1;
	pDC->Text( rectBox, szText, MAM_LEFT | MAM_VCENTER);		// Shadow

	bool bFulled = false;
	if ( (pServerInfo->nNumOfUser >= pServerInfo->nCapacity) || ( !pServerInfo->bIsLive))
		bFulled = true;
	if ( pServerInfo->bIsLive)
	{
		if ( bFulled)
		{
			if ( bSelected)
				pDC->SetColor( MCOLOR(0xFFD00000));
			else
				pDC->SetColor( MCOLOR(0xFF900000));
		}
		else
		{
			if ( bSelected)
				pDC->SetColor( MCOLOR(0xFFFFFFFF));
			else
				pDC->SetColor( MCOLOR(0xFFA0A0A0));
		}
	}
	else
	{
		pDC->SetColor( MCOLOR(0xFF505050));
	}

	rectBox.x--;
	rectBox.y--;
	pDC->Text( rectBox, szText, MAM_LEFT | MAM_VCENTER);
}

bool ZServerView::OnEvent(MEvent* pEvent, MListener* pListener)
{
	bool bRet = MWidget::OnEvent(pEvent, pListener);


	// Check rect range
	MRECT r = GetClientRect();
	if ( r.InPoint( pEvent->Pos) == false)
		return bRet;


	// LButton down
	if ( pEvent->nMessage == MWM_LBUTTONDOWN)
	{
		MRECT rect;
		rect = GetClientRect();

		int nSelect = 0;
		for ( SERVERLIST::iterator itr = m_cServerList.begin();  itr != m_cServerList.end();  itr++)
		{
			ServerInfo* pServerInfo = (*itr);

			// 세팅된 서버리스트 좌표로 서버선택 처리
			if( pServerInfo->rRect.x <= pEvent->Pos.x && pEvent->Pos.x <= (pServerInfo->rRect.x + pServerInfo->rRect.w) )
			{
				if(pServerInfo->rRect.y <= pEvent->Pos.y && pEvent->Pos.y <= (pServerInfo->rRect.y + pServerInfo->rRect.h)) 
				{
					if ( pServerInfo->bIsLive && pServerInfo->nType != 0)
					{
						m_nSelectNum = nSelect;
						ZGetSoundEngine()->PlaySound("if_click");	// 사운드 출력
						return bRet;
					}
				}
			}
			++nSelect;
		}
	}

	return bRet;
}


void ZServerView::ClearServerList( void)
{
	while ( m_cServerList.size())
	{
		delete *m_cServerList.begin();
		m_cServerList.pop_front();
	}
	m_nSelectNum = -1;
}


bool ZServerView::AddServer( char* szName, char* szAddress, int nPort, int nType, int nNumOfUser, int nCapacity, bool IsLive, char* szAgentIP, int nRow, int nCol)
{
	ServerInfo* pServerNode = new ServerInfo;
	strcpy( pServerNode->szName, szName);
	strcpy( pServerNode->szAddress, szAddress);
	pServerNode->nPort = nPort;
	pServerNode->nType = nType;
	pServerNode->nNumOfUser = nNumOfUser;
	pServerNode->nCapacity = nCapacity;
	pServerNode->bIsLive = IsLive;
	pServerNode->nRow = nRow;
	pServerNode->nCol = nCol;
	if(szAgentIP != NULL)
		strcpy( pServerNode->szAgentIP, szAgentIP);

	m_cServerList.push_back( pServerNode);

	return true;
}

void ZServerView::SetAgentPing(DWORD nIP, DWORD dTimeStamp)
{
	DWORD dwTimeCount =	timeGetTime();
	int i = 0;
	for ( SERVERLIST::iterator itr = m_cServerList.begin();  itr != m_cServerList.end();)
	{
		ServerInfo* pServer = (*itr);

		sockaddr_in Addr;
		Addr.sin_addr.S_un.S_addr = nIP;
		char* pszIP = inet_ntoa(Addr.sin_addr);

		if(strcmp(pServer->szAgentIP, pszIP) == 0)
		{
			m_dAgentPing[i] = dwTimeCount - dTimeStamp;
			break;
		}
		itr++;
		i++;
	}
}

ServerInfo* ZServerView::GetSelectedServer()
{
	if ( m_nSelectNum < 0)
		return NULL;

	SERVERLIST::iterator itr = m_cServerList.begin();

	for ( int i = 0;  i < m_nSelectNum;  i++)
	{
		if ( itr == m_cServerList.end())
			return NULL;

		itr++;			// 아  map을 썻어야 했는데... -ㅇ-;
	}

	return (*itr);
}


void ZServerView::SetCurrSel( int nNumber)
{
	if ( nNumber > (int)m_cServerList.size())
		return;

	if ( nNumber == -1)
		m_nSelectNum = -1;


	SERVERLIST::iterator itr = m_cServerList.begin();
	for ( int i = 0;  i < nNumber;  i++)
		itr++;			// 아 map을 썻어야 했는데...일단 배째~ -ㅇ-; 님아 그러시면 골룸~


//	if ( ( (*itr)->nType > 0) && ( (*itr)->nNumOfUser < (*itr)->nCapacity) && (*itr)->bIsLive)
	if ( (itr!=m_cServerList.end()) && ( (*itr)->nType > 0) && (*itr)->bIsLive)
		m_nSelectNum = nNumber;
	else
#ifdef	_DEBUG
		m_nSelectNum = -1;
#else
		m_nSelectNum = 0;
#endif
}

int ZServerView::GetCurrSel()
{
	if( m_cServerList.empty() ) return -1;
	SERVERLIST::iterator itr = m_cServerList.begin();
	for ( int i = 0;  i < m_nSelectNum;  i++)
		itr++;


	int bRet = -1;
	int nType = (*itr)->nType;
	int nNumOfUser = (*itr)->nNumOfUser;
	int nCapacity = (*itr)->nCapacity;
	bool nLive = (*itr)->bIsLive;


	if ( ( (*itr)->nType > 0) && ( (*itr)->nNumOfUser < (*itr)->nCapacity) && (*itr)->bIsLive)
		bRet = m_nSelectNum;

	return bRet;
}
