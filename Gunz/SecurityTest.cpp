#include "stdafx.h"
#include "SecurityTest.h"
#include "CheckReturnCallStack.h"



void testCallstack()
{
	mlog( "Call testCallstack.\n" );
	CHECK_RETURN_CALLSTACK(testCallstack);
}