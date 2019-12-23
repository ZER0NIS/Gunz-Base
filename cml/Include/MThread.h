#pragma once

#include "MTime.h"
#include "GlobalTypes.h"
#include <thread>
#include "optional.h"

#pragma comment(lib, "winmm.lib")


class MThread {
protected:
	HANDLE		m_hThread;
	DWORD		m_idThread;

public:
	MThread();
	virtual ~MThread();
	void Create();
	void Destroy();

	HANDLE GetThreadHandle()	{ return m_hThread; }
	DWORD GetThreadID()			{ return m_idThread; }

	static DWORD WINAPI ThreadProc(LPVOID lpParameter);

	virtual void OnCreate()		{}
	virtual void OnDestroy()	{}
	virtual void Run()			{}
};
