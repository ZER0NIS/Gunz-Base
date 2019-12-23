#pragma once

#include <list>
#include "MMatchGlobal.h"
#include "StringView.h"

#define MAX_FRIEND_COUNT	20

struct MMatchFriendNode {
	unsigned long	nFriendCID;
	unsigned short	nFavorite;
	char			szName[MATCHOBJECT_NAME_LENGTH];

	unsigned char	nState;										
	char			szDescription[MATCH_SIMPLE_DESC_LENGTH];
};

using MMatchFriendList = std::list<MMatchFriendNode*>;

class MMatchFriendInfo {
private:
	MCriticalSection	m_csFriendListLock;
public:
	MMatchFriendList	m_FriendList;
public:
	MMatchFriendInfo();
	virtual ~MMatchFriendInfo();
	bool Add(unsigned long nFriendCID, unsigned short nFavorite, const char* pszName);
	void Remove(const char* pszName);
	MMatchFriendNode* Find(unsigned long nFriendCID);
	MMatchFriendNode* Find(const char* pszName);
	void UpdateDesc();
};

#pragma pack(1)
struct MFRIENDLISTNODE {
	unsigned char	nState;
	char			szName[MATCHOBJECT_NAME_LENGTH];
	char			szDescription[MATCH_SIMPLE_DESC_LENGTH];
};
#pragma pack()
