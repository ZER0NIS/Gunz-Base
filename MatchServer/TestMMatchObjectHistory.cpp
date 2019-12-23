#include "stdafx.h"
#include "../../sdk/UnitTest++/src/UnitTest++.h"
#include "MMatchObjectCommandHistory.h"

TEST(MatchObjectCommandHistrory)
{
	MMatchObjectCommandHistory aCommandHistory;

	// 3명분 command 100ms 간격으로 40개를 밀어넣어본다 
	for(int i = 0 ; i < 60 ; ++i )
	{
		aCommandHistory.PushCommand( MUID(0,1) , i, i*100 );
		aCommandHistory.PushCommand( MUID(0,2) , i, i*100 );
		aCommandHistory.PushCommand( MUID(0,3) , i, i*100 );
	}

	// 3분 시차를 두고 한명 더 더해준다 
	for(int i = 0 ; i < 50 ; ++i )
	{
		aCommandHistory.PushCommand( MUID(0,4) , i, i*100 + 3 * 60 * 1000 );
	}

	// 4명의 데이터가 존재하나 테스트
	CHECK_EQUAL( 4, aCommandHistory.GetObjectCount() );

	// 데이터 하나를 받아와 본다
	MOBJECTCOMMANDHISTORY* pOutput = aCommandHistory.GetCommandHistory( MUID(0,2) );

	// MAX_COUNT 넘어가지 않는지 테스트
	CHECK_EQUAL( MAX_COMMAND_HISTORY_COUNT, (unsigned int)pOutput->m_commands.size() );

	// 앞의 10개는 없어져야한다
	CHECK_EQUAL ( 10, (int)pOutput->m_commands.front().first );

	// 11분을 흘려보낸다
	aCommandHistory.Update( 11 * 60 * 1000 );

	// 앞의 3명이 사라져야한다
	CHECK_EQUAL( 1 , aCommandHistory.GetObjectCount());

	// 데이터도 없어야한다
	MOBJECTCOMMANDHISTORY* pOutput2 = aCommandHistory.GetCommandHistory( MUID(0,2) );
	CHECK( NULL==pOutput2);

	// 4번째 사람의 데이터가 있다
	MOBJECTCOMMANDHISTORY* pOutput3 = aCommandHistory.GetCommandHistory( MUID(0,4) );
	CHECK( NULL!=pOutput3 );

	// 시간을 더 보내면 다 없어져야 한다
	aCommandHistory.Update( 40 * 60 * 1000 );

	CHECK_EQUAL( 0, aCommandHistory.GetObjectCount() );

	// 50분 뒤 한명 더 더해준다 
	for(int i = 0 ; i < 30 ; ++i )
	{
		aCommandHistory.PushCommand( MUID(0,5) , i, i*100 + 50 * 60 * 1000 );
	}
	CHECK_EQUAL( 1, aCommandHistory.GetObjectCount() );

	// time get time 이 한바퀴 돌아 0이되면 전체가 다 지워진다
	aCommandHistory.Update( 0 );
	CHECK_EQUAL( 0, aCommandHistory.GetObjectCount() );


	// flooding test .. 비슷한 시간에 많이 밀어넣으면 플러딩으로 취급
	bool bFloodingSuspect;
	for(int i=0;i<100;i++)
	{
		aCommandHistory.PushCommand( MUID(0,6) , 1, i, &bFloodingSuspect );
	}
	CHECK( bFloodingSuspect);

}


