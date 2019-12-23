#pragma once


#include <windows.h>

#define _MPROCESS_CONTROLLER

#ifdef _MPROCESS_CONTROLLER
///////////////////////////

#include <Tlhelp32.h>

class MProcessController {
public:
	static bool FindProcessByName(const char* pszProcessName, PROCESSENTRY32* pOutPE32); 
	static HANDLE OpenProcessHandleByFilePath(const char* pszFilePath);	
	static HANDLE OpenProcessHandleByName(const char* pszFilePath);
	static bool StartProcess(const char* pszProcessPath, const BOOL bInheritHandles = TRUE);
	static bool StopProcess(HANDLE hProcess);
};

int MGetCurrProcessMemory();

#endif
