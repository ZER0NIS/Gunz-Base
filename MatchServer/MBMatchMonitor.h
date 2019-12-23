#ifndef _MBMATCHMONITOR
#define _MBMATCHMONITOR
/*
//#include "MMonitor.h"
#include "MBMatchSystemInfo.h"

class MBMatchMonitor : public MMonitor
{
private :
	virtual	MMonitorCommandElement* OnRequestMonitorCommandElement( const MMonitorCommandElement* pCmdElement );
	virtual bool InitProtocolBuilder();
	virtual bool OnRun( const DWORD dwCurTime );

public :
	void SafePushRecvUDP( const DWORD dwIP, const WORD wPort, const char* pData, const DWORD dwDataSize );

private:
	MBMatchSystemInfo* m_pMatchSystemInfo;

public:
	MBMatchMonitor();
	virtual ~MBMatchMonitor();

	bool OnInit();
	void OnRelease();
	void OnCheckDBConnectionToCmdElement(const MMonitorCommandElement* pRequestCmdElement, MMonitorCommandElement* pResponseCmdElement);
	void OnGetProcessRunningTimeToCmdElement(const MMonitorCommandElement* pRequestCmdElement, MMonitorCommandElement* pResponseCmdElement);
	void OnGetMemoryInfoToCmdElement(const MMonitorCommandElement* pRequestCmdElement, MMonitorCommandElement* pResponseCmdElement);
	void OnGetDiskInfoToCmdElement(const MMonitorCommandElement* pRequestCmdElement, MMonitorCommandElement* pResponseCmdElement);
	void OnGetUserConnectionCountToCmdElement(const MMonitorCommandElement* pRequestCmdElement, MMonitorCommandElement* pResponseCmdElement);
	void OnPostAsyncCpuUsage(string strProtocolID, string strTaskName, UINT uiCpuUsage);	
};
*/
#endif