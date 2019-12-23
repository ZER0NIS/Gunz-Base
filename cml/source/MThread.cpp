#include "stdafx.h"
#include "MThread.h"

MThread::MThread() 
{ 
	m_hThread = NULL;
	m_idThread = 0;
}

MThread::~MThread()
{
	if (m_hThread)
		Destroy();
}

void MThread::Create()
{
	m_hThread = CreateThread(NULL, 0, ThreadProc, this, 0, &m_idThread); 
	OnCreate();
}

void MThread::Destroy()
{
	OnDestroy();
	if (m_hThread) {
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

DWORD WINAPI MThread::ThreadProc(LPVOID pParam)
{
	MThread* pThread = (MThread*)pParam;

	pThread->Run();

	ExitThread(0);
	return (0);
}