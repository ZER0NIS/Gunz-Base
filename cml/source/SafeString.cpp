#include "stdafx.h"
#include <cassert>

void(*SafeStringOnOverflowFunc)() = [] { assert(false); };