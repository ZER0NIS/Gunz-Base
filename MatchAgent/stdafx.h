#pragma once

#include "targetver.h"

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN	
#endif

#ifndef WINVER			
#define WINVER 0x0501		
#endif

#ifndef _WIN32_WINNT		
#define _WIN32_WINNT 0x0501
#endif						

#ifndef _WIN32_WINDOWS		
#define _WIN32_WINDOWS 0x0501
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
#include <afxrich.h>
#include <afxcview.h>
#endif 

#include "winsock2.h"
#include <cassert>
#define ASSERT assert

#include "SafeString.h"
#include "MDebug.h"
#pragma comment(lib,"../Binaries/Lib/cml/cml.lib")
#pragma comment(lib,"../Binaries/Lib/CSCommon/CSCommon.lib")
#pragma comment(lib,"../Binaries/Lib/MCountryFilter/MCountryFilter.lib")
#pragma comment(lib,"../Binaries/Lib/MDatabase/MDatabase.lib")
#pragma comment(lib,"../Binaries/Lib/SafeUDP/SafeUDP.lib")
#pragma comment(lib,"../Binaries/Lib/StringLiteral/StringLiteral.lib")
