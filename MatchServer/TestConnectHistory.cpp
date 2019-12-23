#include "stdafx.h"
#include "../../sdk/UnitTest++/src/UnitTest++.h"
#include "MConnectHistory.h"

TEST(ConnectHistory)
{
	MConnectHistory history;

	// 추가
	history.Add("testClient", 10);

	history.Update( 20 );

	// 있는지 확인
	CHECK ( history.IsExist("testClient") );

	// timegettime 한바퀴 돌았을때 사라져야 한다
	history.Update( 0 );

	CHECK ( !history.IsExist("testClient") );


	// 다른넘 추가
	history.Add("testClient2", 10);
	CHECK ( history.IsExist("testClient2") );

	// 11초 지난뒤에 사라져야 한다
	history.Update( 11000 );
	CHECK ( !history.IsExist("testClient2") );
}