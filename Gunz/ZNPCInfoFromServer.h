#pragma once


#include <map>


using std::map;


struct MTD_NPCINFO;



class ZNPCInfoFromServerManager : public map< BYTE, MTD_NPCINFO*  >
{
public :
	ZNPCInfoFromServerManager();
	~ZNPCInfoFromServerManager();

	bool CreateNPCInfo( const MTD_NPCINFO* pNPCInfo );

	const MTD_NPCINFO* GetNPCInfo( const BYTE nNPCID );

	void Clear();
};