#ifndef _MBMATCH_GAMEGUARD
#define _MBMATCH_GAMEGUARD



#include "MBMatchUserSecurityInfo.h"



// Error code 14 : 게임가드 버전 검사 실패이며 클라이언트쪽에서 최신버전의 게임가드를 사용하지 않을때 발생합니다.



#define GAMEGUARD_DLL_PATH		".\\"
#define ACTIVE_NUM				50
#define GAMEGUARD_CHECKTIME		(1000 * 60 * 3) // 3min
#define MAX_ELAPSEDTIME			(GAMEGUARD_CHECKTIME * 2) // 6min



class MBMatchGGSvrRef;



class MBMatchGameguard : public MBMatchUserSecurityInfo
{
public :
	MBMatchGameguard( const DWORD dwCurTime );
	~MBMatchGameguard();


	const bool Init();
	void Close();

	// server에서 client로 보내는 AuthData를 생성하는 순서
	//   CreateAuthQuery()를 호출하면 인증쿼리 데이터가 생성된다.
	//   GetAuthQuery()로 생성된 인증쿼리 데이터를 가져온다.

	// client로부터 server로 도착한 AuthData검사 순서.
	//  SetAuthAnswer()로 응답받은 인증 데이터를 저장한다.
	//  CheckAuthAnswer() 저장된 데이터를 검사한다.

	const MBMatchGGSvrRef* GetServerAuth() { return m_pServerAuth; }
	void SetAuthAnswer( const DWORD dwIndex, const DWORD dwValue1, const DWORD dwValue2, const DWORD dwValue3 );
		

	bool CreateAuthQuery();
	bool CheckAuthAnswer( const DWORD dwCurTime );


	const DWORD GetLastError()			{ return m_dwLastError; }
	const DWORD GetCreateAuthCount()	{ return m_dwCreateAuthCount; }

private :
	MBMatchGameguard() {}
	
private :
	MBMatchGGSvrRef*	m_pServerAuth;
	bool				m_bIsInit;
	DWORD				m_dwLastError;
	DWORD				m_dwCreateAuthCount;
};


const DWORD InitMBMatchGameguardAuth();
void CleanupMBMatchGameguardAuth();
void UpdateGambeguard( const DWORD dwCurTime );


#endif