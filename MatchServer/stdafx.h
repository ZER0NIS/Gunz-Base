#pragma once

#include "targetver.h"

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "shlwapi.lib")

#define POINTER_64 __ptr64
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN	
#endif

#ifndef WINVER                       
#define WINVER 0x0600          
#endif

#ifndef _WIN32_WINNT         
#define _WIN32_WINNT 0x0600    
#endif						

#ifndef _WIN32_WINDOWS       
#define _WIN32_WINDOWS 0x0410 
#endif

#ifndef _WIN32_IE                  
#define _WIN32_IE 0x0700     
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	
#define _AFX_ALL_WARNINGS

#include <afxwin.h>        
#include <afxext.h>      
#include <afxdisp.h>       

#include <afxdtctl.h>		
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			
#endif 
#include <afxrich.h>
#include <afxcview.h>

#include "winsock2.h"
#include "windows.h"
#include <comutil.h>
#include <stdio.h>

#include "SafeString.h"

#include "MBMatchServer.h"
#include "Config.h"


//#include "StringView.h"

#include <cassert>
#define ASSERT assert

#define _QUEST_ITEM
#define _MONSTER_BIBLE 

#pragma comment(lib,"../Binaries/Lib/cml/cml.lib")
#pragma comment(lib,"../Binaries/Lib/CSCommon/CSCommon.lib")
#pragma comment(lib,"../Binaries/Lib/MCountryFilter/MCountryFilter.lib")
#pragma comment(lib,"../Binaries/Lib/MDatabase/MDatabase.lib")
#pragma comment(lib,"../Binaries/Lib/SafeUDP/SafeUDP.lib")
#pragma comment(lib,"../Binaries/Lib/StringLiteral/StringLiteral.lib")

