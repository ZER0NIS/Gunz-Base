/////////////////////////////////////////////////
// MBMatchAsyncJob_GetCpuInfo.h
// 머신의 CPU 정보를 알아온다.
/////////////////////////////////////////////////
#include "MAsyncDBJob.h"

class MBMatchAsyncJob_GetCpuInfo : public MAsyncJob
{
protected:
	string m_strProtocolID;
	string m_strTaskName;
	UINT m_iCpuUsage;

public:
	MBMatchAsyncJob_GetCpuInfo(string strProtocolID, string strTaskName) : MAsyncJob(MASYNCJOB_GETSYSTEMINFO,MUID(0,0))
	{
		m_strProtocolID = strProtocolID;
		m_strTaskName = strTaskName;
		m_iCpuUsage = 0;
	}
	virtual ~MBMatchAsyncJob_GetCpuInfo()		{}

	UINT GetCpuUsage()							{ return m_iCpuUsage; }
	string GetProtocolID()						{ return m_strProtocolID; }
	string GetTaskName()						{ return m_strTaskName; }

	virtual void Run(void* pContext);
};
