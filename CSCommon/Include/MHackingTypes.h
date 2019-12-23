#pragma once

#include "MSingleton.h"
#include "MUtil.h"

// 여기에 하나 추가 하시면 아래 응징 테이블도 하나 추가하셔야 합니다
enum MMatchHackingType
{
	MMHT_NO = 0,	
	MMHT_XTRAP_HACKER,
	MMHT_HSHIELD_HACKER,
	MMHT_BADFILECRC,
	MMHT_BADUSER,
	MMHT_GAMEGUARD_HACKER,
	MMHT_GIVE_ONESELF_UP_DLLINJECTION,
	MMHT_INVALIDSTAGESETTING,
	MMHT_COMMAND_FLOODING,
	MMHT_COMMAND_BLOCK_BY_ADMIN,
	MMHT_SLEEP_ACCOUNT = 10,					///< 일단은 NHN 요청에 따른 휴명 계정 처리이다. (절대로 10이어야 한다. 바뀌어선 안된다)
	MMHT_END,
};

struct PUNISH_TABLE_ITEM {
	DWORD				dwMessageID;
	const char*			szComment;

	u_short				nDay;
	u_short				nHour;
	u_short				nMin;
	
	MMatchBlockLevel	eLevel;
};

class MPunishTable {
	static const PUNISH_TABLE_ITEM PUNISH_TABLE[MMHT_END];
public:
	static const PUNISH_TABLE_ITEM& GetPunish( MMatchHackingType eType );
};