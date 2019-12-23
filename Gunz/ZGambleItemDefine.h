#ifndef _ZMYGAMBLEITEMDEFINE_H
#define _ZMYGAMBLEITEMDEFINE_H


#include "MMatchDBBaseGambleItem.h"


#include <map>

using std::map;


class ZGambleItemDefine : public MMatchDBBaseGambleItem
{
private :
	
public :
	ZGambleItemDefine( const DWORD dwGIID
		, const string& strName
		, const string& strDesc
		, const DWORD dwPrice
		, const bool bIsCash ) 
		: MMatchDBBaseGambleItem( dwGIID
								, strName
								, strDesc
								, dwPrice
								, bIsCash )
	{
	}

	const string& GetFullDescription() const
	{
		return GetDescription();
	}
};


class ZGambleItemDefineManager
{
private :
	map< DWORD, ZGambleItemDefine* > m_GambleItemDefineList;

	bool m_bIsRecvUpdateCommand;	// 서버가 겜블 아이템 목록이 업데이트 되면은 클라이언트에 알려주고 
									// 다음 리스트 정보가 필요하면은 이 플레그를 검사해서 업데이트 한다.

public :
	ZGambleItemDefineManager() {}
	~ZGambleItemDefineManager() { Release(); }

	static ZGambleItemDefineManager& GetInstance()
	{
		static ZGambleItemDefineManager Instance;
        return Instance;
	}

	const ZGambleItemDefine* GetGambleItemDefine( const DWORD dwGambleItemID ) const
	{
		map< DWORD, ZGambleItemDefine* >::const_iterator itFind
			= m_GambleItemDefineList.find( dwGambleItemID );
		if( m_GambleItemDefineList.end() == itFind )
		{
#ifdef _GAMBLEITEM_TEST_LOG
			mlog("Character GAMBLE GetGambleItemDefine() Fail [ID:%d]\n", dwGambleItemID);
#endif
			return NULL;
		}

		return itFind->second;
	}

	const bool AddGambleItemDefine( ZGambleItemDefine* pGItemDef )
	{
		if( NULL == pGItemDef )
		{
			_ASSERT( 0 );
			return false;
		}

		if( NULL != GetGambleItemDefine(pGItemDef->GetGambleItemID()) )
		{
			_ASSERT( 0 && "중복된 GambleItemID" );
#ifdef _GAMBLEITEM_TEST_LOG
			mlog("AddGambleItemDefine() Fail 중복된 GambleItemID \n");
#endif
			return false;
		}

		m_GambleItemDefineList.insert( 
			map<DWORD, ZGambleItemDefine*>::value_type(pGItemDef->GetGambleItemID(), pGItemDef) );

		return true;
	}

	void Release()
	{
		map< DWORD, ZGambleItemDefine* >::iterator it, end;
		end = m_GambleItemDefineList.end();
		for( it = m_GambleItemDefineList.begin(); it != end; ++it )
			delete it->second;
		m_GambleItemDefineList.clear();
	}
};


static ZGambleItemDefineManager& ZGetGambleItemDefineMgr()
{
	return ZGambleItemDefineManager::GetInstance();
}

#endif