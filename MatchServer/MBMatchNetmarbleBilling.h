#pragma once

class MBMatchNetmarbleBillingRequest
{
public:
	MBMatchNetmarbleBillingRequest(){}
	~MBMatchNetmarbleBillingRequest(){}
};

class MBMatchNetmarbleBilling
{
protected:
	typedef std::list<MBMatchNetmarbleBillingRequest*>	MBMatchNetmarbleBillingList;
	typedef MBMatchNetmarbleBillingList::iterator		MBMatchNetmarbleBillingListIterator;

	MBMatchNetmarbleBillingList m_RequestList;

	void AddRequest(MBMatchNetmarbleBillingRequest* pRequest);
	MBMatchNetmarbleBillingRequest* GetRequest();

	CRITICAL_SECTION m_cs;
	HANDLE m_hThread;

	static DWORD WINAPI WorkerThread(LPVOID pContext);

public:
	MBMatchNetmarbleBilling();
	~MBMatchNetmarbleBilling();

	bool Create();
	void Destroy();

	void RequestAuth();
	
	static MBMatchNetmarbleBilling& GetInstance();
};

MBMatchNetmarbleBilling& MGetNetmarbleBilling(); 
