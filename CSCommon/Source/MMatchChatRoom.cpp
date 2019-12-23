#include "stdafx.h"
#include "MMatchChatRoom.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"



MMatchChatRoom::MMatchChatRoom(const MUID& uidRoom, const MUID& uidMaster, const char* pszName)
{
	m_uidChatRoom = uidRoom;
	m_uidMaster = uidMaster;

	const size_t NameLen = strlen( pszName) + 1;

	strncpy(m_szName, pszName
		, NameLen > MAX_CHATROOMNAME_STRING_LEN ? MAX_CHATROOMNAME_STRING_LEN : NameLen );
}

MMatchChatRoom::~MMatchChatRoom() 
{
}

bool MMatchChatRoom::AddPlayer(const MUID& uidPlayer)
{
	MUIDRefCache::iterator i = m_PlayerList.find(uidPlayer);
	if (i != m_PlayerList.end())
		return false;

	MMatchServer* pServer = MMatchServer::GetInstance();
	MMatchObject* pPlayer = pServer->GetObject(uidPlayer);
	if( !IsEnabledObject(pPlayer) )
		return false;

	pPlayer->SetChatRoomUID(GetUID());

	m_PlayerList.Insert(uidPlayer, pPlayer);
	return true;
}

bool MMatchChatRoom::IsFindPlayer(const MUID& uidPlayer)	// 해당 플레이어가 있는지 확인한다
{
	MUIDRefCache::iterator i = m_PlayerList.find(uidPlayer);
	if (i == m_PlayerList.end())
		return false;
	return true;										// 플레이어가 있으면 참 반환
}

void MMatchChatRoom::RemovePlayer(const MUID& uidPlayer)
{
	MUIDRefCache::iterator i = m_PlayerList.find(uidPlayer);
	if (i != m_PlayerList.end()) {
		m_PlayerList.erase(i);
	
		// 2008.08.28 채팅방에 탈퇴후 /채팅 할말을 하면 채팅방에 참가되어있는 사람들에게 채팅말 보임현상 처리
		// 채팅방 탈퇴시 플레이어 채팅룸 ID초기화 http://dev:8181/projects/gunz/ticket/158
		MMatchServer* pServer = MMatchServer::GetInstance();
		MMatchObject* pPlayer = pServer->GetObject(uidPlayer);
		if( !IsEnabledObject(pPlayer) )
			return;
		pPlayer->SetChatRoomUID(MUID(0,0));
	}
}

void MMatchChatRoom::RouteChat(const MUID& uidSender, char* pszMessage)
{
	if( (0 == pszMessage) || (256 < strlen(pszMessage)) )
		return;

	MMatchServer* pServer = MMatchServer::GetInstance();
	MMatchObject* pSenderObj = pServer->GetObject(uidSender);
	if (pSenderObj == NULL)
		return;

	for (MUIDRefCache::iterator i=m_PlayerList.begin(); i!=m_PlayerList.end(); i++) {
		MUID uidTarget = (*i).first;
		MMatchObject* pTargetObj = pServer->GetObject(uidTarget);
		if (pTargetObj) {
			MCommand* pCmd = pServer->CreateCommand(MC_MATCH_CHATROOM_CHAT, MUID(0,0));
			pCmd->AddParameter(new MCmdParamStr( const_cast<char*>(GetName()) ));
			pCmd->AddParameter(new MCmdParamStr(pSenderObj->GetName()));
			pCmd->AddParameter(new MCmdParamStr(pszMessage));
			pServer->RouteToListener(pTargetObj, pCmd);
		}
	}
}

void MMatchChatRoom::RouteInfo(const MUID& uidReceiver)
{
	
}

void MMatchChatRoom::RouteCommand(const MCommand* pCommand)
{
	if( 0 == pCommand )
		return;

	MMatchServer* pServer = MMatchServer::GetInstance();
	for (MUIDRefCache::iterator i=m_PlayerList.begin(); i!=m_PlayerList.end(); i++) {
		MUID uidTarget = (*i).first;
		MMatchObject* pTargetObj = pServer->GetObject(uidTarget);
		if (pTargetObj) {
			MCommand* pRouteCmd = pCommand->Clone();
			pServer->RouteToListener(pTargetObj, pRouteCmd);
		}
	}
	delete pCommand;
}


MMatchChatRoomMgr::MMatchChatRoomMgr()
{
}

MMatchChatRoomMgr::~MMatchChatRoomMgr()
{
}

MMatchChatRoom* MMatchChatRoomMgr::AddChatRoom(const MUID& uidMaster, const char* pszName)
{
	if( (0 == pszName) || (128 < strlen(pszName)) )
		return 0;

	if (FindChatRoomByName(pszName) != NULL)
		return NULL;

	MMatchObject* pMaster = MMatchServer::GetInstance()->GetObject( uidMaster );
	if( !IsEnabledObject(pMaster) )
		return 0;

	MMatchChatRoom* pRoom = new MMatchChatRoom(m_RoomMap.UseUID(), uidMaster, pszName);
	if( 0 == pRoom )
		return 0;

	m_RoomMap.Insert(pRoom->GetUID(), pRoom);

	string strName = pszName;
	m_RoomStringSubMap.insert( 
		MMatchChatRoomStringSubMap::value_type(strName, pRoom->GetUID()) 
	);

	return pRoom;
}

void MMatchChatRoomMgr::RemoveChatRoom(const MUID& uidChatRoom)
{
	MMatchChatRoomMap::iterator i = m_RoomMap.find(uidChatRoom);
	if (i!=m_RoomMap.end()) {
		// Remove Sub Map
		MMatchChatRoom* pRoom = (*i).second;
		MMatchChatRoomStringSubMap::iterator itorSub = 
			m_RoomStringSubMap.find(pRoom->GetName());
		if (itorSub != m_RoomStringSubMap.end()) {
			m_RoomStringSubMap.erase(itorSub);
		}

		// Remove from Map
		m_RoomMap.erase(i);
	}
}

MMatchChatRoom* MMatchChatRoomMgr::FindChatRoom(const MUID& uidChatRoom)
{
	MMatchChatRoomMap::iterator i = m_RoomMap.find(uidChatRoom);
	if (i!=m_RoomMap.end())
		return (*i).second;
	return NULL;
}

MMatchChatRoom* MMatchChatRoomMgr::FindChatRoomByName(const char* pszName)
{
	if( (0 == pszName) || (128 < strlen(pszName)) )
		return 0;

	MMatchChatRoomStringSubMap::iterator itorSub = 
		m_RoomStringSubMap.find(pszName);
	if (itorSub != m_RoomStringSubMap.end()) {
		MUID uidRoom = (*itorSub).second;
		return FindChatRoom(uidRoom);
	}
	return NULL;
}

void MMatchChatRoomMgr::Update()
{
}
