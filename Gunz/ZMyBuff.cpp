#include "stdafx.h"
#include "ZMyBuff.h"

////////////////////////////////////////////////////////////////////////////////////
//// ZMyShortBuff Class
////////////////////////////////////////////////////////////////////////////////////
ZMyShortBuff::ZMyShortBuff() : MMatchShortBuff() 
{

}


////////////////////////////////////////////////////////////////////////////////////
//// ZMyShortBuffMap Class
////////////////////////////////////////////////////////////////////////////////////
void ZMyShortBuffMap::Clear() 
{
	while(!empty()) {
		delete (ZMyShortBuff*)begin()->second;
		erase(begin());
	}
}

void ZMyShortBuffMap::Remove(MUID& uidBuff)
{
	iterator iter = find(uidBuff);
	if( iter != end() ) {
		erase(iter);
		delete (ZMyShortBuff*)iter->second;
	}
}

bool ZMyShortBuffMap::Insert(MUID& uidBuff, ZMyShortBuff* pBuff)
{
	iterator iter = find(uidBuff);
	if( iter != end() ) { return false; }
	insert(pair<MUID, ZMyShortBuff*>(uidBuff, pBuff));
	return true;
}

ZMyShortBuff* ZMyShortBuffMap::GetShortBuffByBuffID(int nBuffID)
{
	for(iterator iter = begin(); iter != end(); iter++) {
		ZMyShortBuff* pBuff = iter->second;
		if(pBuff->GetBuffID() == nBuffID) return pBuff;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////
//// ZMyBuffSummary Class
////////////////////////////////////////////////////////////////////////////////////
ZMyBuffSummary::ZMyBuffSummary() : MMatchBuffSummary()
{

}