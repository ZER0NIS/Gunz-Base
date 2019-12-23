#include "stdafx.h"
#include "MMemoryProxy.h"

void (*g_fpOnCrcFail)() = 0;

MCrc32Container g_crc32Container;


MCrc32Container* GetCrcContainer()
{
	return &g_crc32Container;
}

