#pragma once


#include "TestCRC32Resource.h"
#include "TestCRCPlayer.h"
#include "TestCRCStage.h"
#include "TestCRCNetModule.h"


class TestCRCServer
{
private :
	MUID				m_UIDCache;
	T_ItemDescMap		m_ItemDescMap;
	TestCRCPlayerMap	m_PlayerMap;
	TestCRCStageMap		m_StageMap;
	TestCRCNetModule	m_NetModule;


public :
	TestCRCServer();
	~TestCRCServer();

	const MUID			CreateNewUID();
	const bool			AddItemDesc( const int nItemID, MMatchItemDesc* pItemDesc );
	const bool			AddPlayer( TestCRCPlayer* pPlayer );
	const bool			AddStage( TestCRCStage* pStage );
	
	
	MMatchItemDesc*		GetItemDesc( const int nItemID );
	TestCRCPlayer*		GetPlayer( const MUID uidPlayer );
	TestCRCStage*		GetStage( const MUID uidStage );
	TestCRCNetModule&	GetTestNetModule()					{ return m_NetModule; }


	const bool			JoinStage( const MUID uidStage, const MUID uidPlayer );


	void				ReleaseItemDescMap();
	void				ReleasePlayerMap();
	void				ReleaseStageMap();
};   