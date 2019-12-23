/*#ifndef _MBMATCHMONITORCOMMUNICATOR_H_
#define _MBMATCHMONITORCOMMUNICATOR_H_

#include <deque>

using std::deque;

#include "MMonitorCommunicator.h"
#include "MSafeUDP.h"

class MBMatchMonitorCommunicator : public MMonitorCommunicator
{
private :
	virtual bool	Send( const DWORD dwIP, const USHORT nPort, const string& strMonitorCommand );

public :
	MBMatchMonitorCommunicator();
	~MBMatchMonitorCommunicator();

	bool			Init();
	void			Release();	
};

#endif*/