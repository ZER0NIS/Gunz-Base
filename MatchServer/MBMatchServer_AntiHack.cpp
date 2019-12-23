#include "stdafx.h"
#include "MBMatchServer.h"

#ifdef _GAMEGUARD
#include "MBMatchGameGuard.h"
#include "MBMatchGGSvrRef.h"
#endif

#ifdef _XTRAP
#include "MBMatchXTrapCC.h"
#endif

void MBMatchServer::OnResponseGameguardAuth( const MUID& uidUser, const DWORD dwIndex, const DWORD dwValue1, const DWORD dwValue2, const DWORD dwValue3 )
{
#ifdef _GAMEGUARD
	MMatchObject* pObj = GetObject( uidUser );
	if( 0 == pObj )
		return;

	MBMatchGameguard* pGameguard = GetGameguard( uidUser );
	if( 0 == pGameguard )
	{
		// ggtest
		LOG(LOG_PROG,  "GAMEGUARD ERR :gameguard obj is null point (2). AID(%u)\n"
			, pObj->GetAccountInfo()->m_nAID );

//		pObj->SetGameguardHackerDisconnectWaitInfo();
		pObj->DisconnectHacker(MMHT_GAMEGUARD_HACKER);

		return;
	}

#ifdef _DEBUG
	mlog( "GAMEGUARD : receivce from client : index(%u), v1(%u), v2(%u), v3(%u)\n",
		dwIndex, dwValue1, dwValue2, dwValue3 );
#endif

	pGameguard->SetAuthAnswer( dwIndex, dwValue1, dwValue2, dwValue3 );

	if( !pGameguard->CheckAuthAnswer(GetGlobalClockCount()) )
	{
		// ggtest
		LOG(LOG_PROG,  "GAMEGUARD ERR : response gameguard auth : errcode(%u), AID(%u), CreateAuthCount(%u)\n"
			, pGameguard->GetLastError()
			, pObj->GetAccountInfo()->m_nAID
			, pGameguard->GetCreateAuthCount() );

//		pObj->SetGameguardHackerDisconnectWaitInfo();
		pObj->DisconnectHacker(MMHT_GAMEGUARD_HACKER);
		return;
	}

	pGameguard->UpdateLastCheckTime( GetGlobalClockCount() );

#else
	mlog( "this server version isn't use gameguard... why call me!!\n" );
#endif
}


void MBMatchServer::OnResponseFirstGameguardAuth( const MUID& uidUser, const DWORD dwIndex, const DWORD dwValue1, const DWORD dwValue2, const DWORD dwValue3 )
{
#ifdef _GAMEGUARD
	MMatchObject* pObj = GetObject( uidUser );
	if( 0 == pObj )
		return;

#ifdef _DEBUG
	mlog( "GAMEGUARD : first receivce from client : index(%u), v1(%u), v2(%u), v3(%u)\n",
		dwIndex, dwValue1, dwValue2, dwValue3 );
#endif

	MBMatchGameguard* pGameguard = GetGameguard( uidUser );
	if( 0 == pGameguard )
	{
		// ggtest
		LOG(LOG_PROG,  "GAMEGUARD ERR :gameguard obj is null point (2). AID(%u)\n"
			, pObj->GetAccountInfo()->m_nAID );

//		pObj->SetGameguardHackerDisconnectWaitInfo();
		pObj->DisconnectHacker(MMHT_GAMEGUARD_HACKER);

		return;
	}

	pGameguard->SetAuthAnswer( dwIndex, dwValue1, dwValue2, dwValue3 );

	if( !pGameguard->CheckAuthAnswer(GetGlobalClockCount()) )
	{
//		pObj->SetGameguardHackerDisconnectWaitInfo();
		pObj->DisconnectHacker(MMHT_GAMEGUARD_HACKER);

#ifdef _DEBUG
		// ggtest
		LOG(LOG_PROG,  "GAMEGUARD ERR :first gameguard auth : errcode(%u), AID(%u), CreateAuthCount(%u)\n"
			, pGameguard->GetLastError()
			, pObj->GetAccountInfo()->m_nAID
			, pGameguard->GetCreateAuthCount() );
#endif

		return;
	}

	// 처음 응답을 받으면 바로 한번더 확인을 해줘야 한다.
	if( pGameguard->CreateAuthQuery() )
	{
		const GG_AUTH_DATA& AuthData = pGameguard->GetServerAuth()->GetAuthQuery();
		RequestGameguardAuth( uidUser
			, AuthData.dwIndex
			, AuthData.dwValue1
			, AuthData.dwValue2
			, AuthData.dwValue3 );
	}
	else
	{
//		pObj->SetGameguardHackerDisconnectWaitInfo();
		pObj->DisconnectHacker(MMHT_GAMEGUARD_HACKER);

		LOG(LOG_PROG,  "GAMEGUARD ERR : create second auth key : ErrCode(%d), AID(%u), CreateAuthCount(%u)\n"
			, pGameguard->GetLastError()
			, pObj->GetAccountInfo()->m_nAID
			, pGameguard->GetCreateAuthCount() );

		return;
	}

	// NHN은 이걸 설정해 줘야 이후에 모든 커맨드 요청이 허락된다.
	pObj->FirstGameguardResponseIsRecved();

	// NHN은 초기 gameguard인증이 완료되어야 캐릭터 리스트를 보내준다.
	OnRequestAccountCharList( pObj->GetUID(), NULL );

#else
	mlog( "this server version isn't use gameguard... why call me!!\n" );
#endif
}

void MBMatchServer::OnResponseXTrapSeedKey(const MUID &uidUser, unsigned char *pComBuf)
{
#ifdef _XTRAP
	MMatchObject *pObj = GetObject(uidUser);
	if (pObj == NULL)
	{
		return;
	}

	if (pObj->GetDisconnStatusInfo().GetStatus() != MMDS_CONNECTED)
	{
		return;
	}

	MBMatchXTrapCC *pXTrapCC = GetXTrapCC(uidUser);
	if (pXTrapCC == NULL)
	{
        return;
	}

	DWORD retVal = pXTrapCC->XTrapCheckHashData(pComBuf);
    if (retVal != 0)
	{
		// 해당 유저 종료 처리
//		pObj->SetXTrapHackerDisconnectWaitInfo();
		pObj->DisconnectHacker( MMHT_XTRAP_HACKER );
	}
#endif
}