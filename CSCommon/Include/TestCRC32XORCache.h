#pragma once


// 주석처리된 함수들은 외부에서 알 필요가 없다. 



//class CRC32XORCache;
//
//
//struct MMatchItemDesc;
//struct MMatchMapsWorldItemSpawnInfoSet;



//void								T_InitItemDescMgr();
//void								T_InitEquipItemList();
//void								T_InitChecksumItemDescList();
//
//void								T_InitWItemDescMgr();
//void								T_InitSpawnWItemList();
//
//MMatchItemDesc*						T_FindItemDesc( const int nItemID );
//MMatchMapsWorldItemSpawnInfoSet*	T_FindWItemDesc( const int nWItemID );
//
//void								T_ReleaseItemDescMap();
//void								T_ReleaseWItemDescMap();
//
//const DWORD						T_BuildResourceChecksum( const DWORD dwChecksum, BYTE* pBuff, const DWORD nSize );
//void								T_BuildResourceListChecksum();
//const DWORD						T_BuildEquipResourceChecksum( const DWORD dwChecksum );
//const DWORD						T_BuildWItemResourceChecksum( const DWORD dwChecksum );
//void								T_BuildEquipListCRC32XORCache( CRC32XORCache* pCRC32XORCache );
//void								T_BuildSpawnInfoSetCRC32XORCache( CRC32XORCache* pCRC32XORCache );
//
//void								T_SameValueChecksumParam();
//void								T_RandomOrderCheckParamOutputIsAllEqual();
//void								T_ResourceCRC32MemberFunction();
//void								T_BitXORTestForCRC32XORCache();
//void								T_DeleteCRC32ForCRC32XORCache();

void								DoTestResourceCRC32Cache();