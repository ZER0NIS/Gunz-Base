#include "stdafx.h"
#include "MMatchBRMachine.h"
#include "MMath.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MMatchBRDescription::MMatchBRDescription(int nBRID, string strName, string strResetDesc, 
										 int nBRTID, int nRewardMinutePeriod, int nRewardCount, int nRewardKillCount)
{
	m_nBRID = nBRID;
	m_nBRTID = nBRTID;

	m_strName = strName;
	m_strResetDesc = strResetDesc;

	m_nRewardMinutePeriod = nRewardMinutePeriod;
	m_nRewardCount = nRewardCount;
	m_nRewardKillCount = nRewardKillCount;

	m_nTotalRate = 0;
}

MMatchBRDescription::~MMatchBRDescription()
{
	vector< MMatchBRItem* >::iterator iter = m_RewardItemList.begin();
	while( iter != m_RewardItemList.end() )
	{
		delete (*iter);
		iter = m_RewardItemList.erase(iter);
	}
}

void MMatchBRDescription::AddRewardItem(MMatchBRItem* pRewardItem)
{
	if( pRewardItem == NULL ) { return; }
	if( GetBRID() != pRewardItem->GetBRID() ) { return; }
	if( m_nTotalRate + pRewardItem->GetRate() > 1000 ) { return; }

	m_nTotalRate += pRewardItem->GetRate();
	pRewardItem->SetRateRange(m_nTotalRate);

	m_RewardItemList.push_back(pRewardItem);
}

MMatchBRItem* MMatchBRDescription::GetRewardItem()
{
	int nRateRage = RandomNumber(0, MAX_REWARD_ITEM_RATE);

	vector< MMatchBRItem* >::const_iterator it, end;
	for( it = m_RewardItemList.begin(); it != m_RewardItemList.end(); ++it ) 
	{
		if( nRateRage < (*it)->GetRateRange() )
			return (*it);
	}

	return NULL;
}

DWORD MMatchBRDescription::GetCRC32()
{
	MMatchCRC32XORCache CRC32;

	CRC32.Reset();

	CRC32.CRC32XOR(GetBRID());
	CRC32.CRC32XOR(GetBRTID());
	CRC32.CRC32XOR(GetRewardMinutePeriod());
	CRC32.CRC32XOR(GetRewardCount());
	CRC32.CRC32XOR(GetRewardkillCount());

	for(int i = 0; i < (int)m_RewardItemList.size(); i++)
	{
		CRC32.CRC32XOR(m_RewardItemList[i]->GetBRID());
		CRC32.CRC32XOR(m_RewardItemList[i]->GetBRIID());
		CRC32.CRC32XOR(m_RewardItemList[i]->GetItemIDMale());
		CRC32.CRC32XOR(m_RewardItemList[i]->GetItemIDFemale());
		CRC32.CRC32XOR(m_RewardItemList[i]->GetItemCnt());
		CRC32.CRC32XOR(m_RewardItemList[i]->GetRentHourPeriod());		
		CRC32.CRC32XOR(m_RewardItemList[i]->GetRate());
	}	

	return CRC32.GetCRC32();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MMatchBRDescriptionMap::MMatchBRDescriptionMap()
{
	MakeCRC32();	
}

void MMatchBRDescriptionMap::Clear()
{
	iterator iter = begin();
	while( iter != end() ) {
		delete iter->second;
		iter = erase(iter);
	}
}

void MMatchBRDescriptionMap::MakeCRC32()
{
	m_CRC32.Reset();

	for( iterator iter = begin(); iter != end(); iter++)
	{
		MMatchBRDescription* pDesc = iter->second;
		m_CRC32.CRC32XOR(pDesc->GetCRC32());
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MMatchBRMachine::MMatchBRMachine()
{
}

MMatchBRMachine::~MMatchBRMachine()
{
	m_RewardDescription.Clear();
}

void MMatchBRMachine::SetBattleTimeRewardMachine(MMatchBRDescriptionMap DescriptionMap)
{
	if( m_RewardDescription.GetCRC32() != DescriptionMap.GetCRC32() )
	{
		m_RewardDescription.Clear();

		mlog("=======================================================================\n");
		MMatchBRDescriptionMap::iterator iter = DescriptionMap.begin();
		for( ; iter != DescriptionMap.end(); iter++) 
		{
			MMatchBRDescription *pDesc = iter->second;

			mlog(" BRID(%d), RewardMinutePeriod(%d), RewardCount(%d)\n", 
				pDesc->GetBRID(), pDesc->GetRewardMinutePeriod(), pDesc->GetRewardCount());

			vector<MMatchBRItem*> v = pDesc->GetBattleRewardItemList();

			for(int i = 0; i < (int)v.size(); i++)
			{
				mlog("    BRIID(%d), ItemID(M=%d, F=%d), RentHourPeriod(%d), Rate(%d), RateRange(%d)\n", 
					v[i]->GetBRIID(), v[i]->GetItemIDMale(), v[i]->GetItemIDFemale(), v[i]->GetRentHourPeriod(), v[i]->GetRate(), v[i]->GetRateRange());
			}

			
			m_RewardDescription.insert(pair<int, MMatchBRDescription*>(iter->first, iter->second));
		}
		m_RewardDescription.MakeCRC32();

		mlog("=======================================================================\n");
	}
	else
	{
		DescriptionMap.Clear();
	}
}

MMatchBRDescription* MMatchBRMachine::GetBattleTimeRewardDescription(int nBRID)
{
	MMatchBRDescriptionMap::iterator iter = m_RewardDescription.find(nBRID);
	if( iter != m_RewardDescription.end() ) return iter->second;

	return NULL;
}
