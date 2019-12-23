#ifndef _MMATCH_ANTIHACK_H
#define _MMATCH_ANTIHACK_H


#include "MUID.h"

#include <list>
#include <string>
using namespace std;

class MMatchAntiHack
{
private:
	static list<unsigned int>	m_clientFileListCRC;

public:
	MMatchAntiHack()		{}
	~MMatchAntiHack()		{}

	static size_t			GetFielCRCSize();
	static void				ClearClientFileList() { m_clientFileListCRC.clear(); }
	static void				InitClientFileList();
	static bool				CheckClientFileListCRC(unsigned int crc, const MUID& uidUser );
};

#endif
