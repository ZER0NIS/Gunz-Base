#pragma once

#include "stdafx.h"

class UPnP
{
public:
	UPnP(void);
	~UPnP(void);

	bool Create(WORD Port);
	void Destroy(void);
	char* GetAddress()		{ return m_Address; }
	WORD GetPort()			{ return m_Port; }

protected:
	bool GetIp(void);

	char m_Address[17];
	WORD m_Port;
};
