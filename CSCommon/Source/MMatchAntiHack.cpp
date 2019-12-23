#include "stdafx.h"
#include "MMatchConfig.h"

///////////////////// client file list

list<unsigned int>	MMatchAntiHack::m_clientFileListCRC;

void MMatchAntiHack::InitClientFileList()
{
	FILE* fp = fopen("filelistcrc.txt", "r");
	if (fp == NULL) return;

	char str[256];

	while (fgets(str, 256, fp) != NULL)
	{
		unsigned int crc;
		sscanf(str, "%u", &crc);
		m_clientFileListCRC.push_back(crc);
	}

	fclose(fp);

	mlog("Inited client file list (%d)\n", (int)m_clientFileListCRC.size());
}

bool MMatchAntiHack::CheckClientFileListCRC( unsigned int crc, const MUID& uidUser )
{
	bool bFound = m_clientFileListCRC.end() != find(m_clientFileListCRC.begin(),m_clientFileListCRC.end(),crc);
	return bFound;
}


//size_t MMatchAntiHack::GetHashMapSize() 
//{ 
//	return m_ClientHashValueList.size(); 
//}


size_t MMatchAntiHack::GetFielCRCSize() 
{ 
	return m_clientFileListCRC.size(); 
}