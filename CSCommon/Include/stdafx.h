#pragma once

#include "targetver.h"

#define POINTER_64 __ptr64
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifdef _WIN32
#include <afxdb.h>
#include <afxtempl.h>
#include <afxdtctl.h>

#include <Winsock2.h>
#include <mswsock.h>
#include <crtdbg.h>
#include <windows.h>
#include <stdlib.h>
#endif

// stl
#include <string.h>
#include <map>
#include <list>
#include <vector>
#include <algorithm>

#include "Config.h"

#define _QUEST_ITEM	
#define _MONSTER_BIBLE
#define _BLOCK_HACKER

#include "MLocaleDefine.h"
#include "MDebug.h"
#include "MXml.h"

#include "MUID.h"
#include "MMatchGlobal.h"
#include "MMatchUtil.h"
#include "MSharedCommandTable.h"
#include "MCommand.h"
#include "MCommandParameter.h"
#include "MCommandCommunicator.h"
#include "MErrorTable.h"
#include "MServer.h"
#include "MMatchServer.h"
#include "MMatchClient.h"
#include "MObject.h"
#include "MMatchItem.h"
#include "MMatchObjCache.h"
#include "MMatchStage.h"
#include "MMatchObject.h"
#include "MMatchChannel.h"

#include "SafeString.h"
#include "GlobalTypes.h"

#include <cassert>

#ifdef _ASSERT
#undef _ASSERT
#endif

#define _ASSERT assert