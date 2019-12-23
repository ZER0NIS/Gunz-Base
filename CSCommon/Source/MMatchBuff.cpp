#include "stdafx.h"
#include "MMatchBuff.h"
#include "MMatchCRC32XORCache.h"

MMatchBuffDesc::MMatchBuffDesc() : m_nBuffID(0)
{
	m_pBuffInfo = new MProtectValue<MMatchBuffInfo>;
	memset(&m_pBuffInfo->Ref(), 0, sizeof(MMatchBuffInfo));

	m_pBuffInfo->Ref().fSpeed_Ratio = 1.0f;
	m_pBuffInfo->MakeCrc();

	m_nBuffEffectType.Set_MakeCrc(MMBET_NORMAL);
	m_nBuffPeriodType.Set_MakeCrc(MMBPT_NONE);
	m_nBuffPeriod.Set_MakeCrc(0);

	memset(m_szBuffName, 0, sizeof(m_szBuffName));
	memset(m_szBuffDesc, 0, sizeof(m_szBuffDesc));
	memset(m_szBuffIconName, 0, sizeof(m_szBuffIconName));
}

MMatchBuffDesc::~MMatchBuffDesc()
{
	delete m_pBuffInfo;
}

void MMatchBuffDesc::CacheCRC32( MMatchCRC32XORCache& crc )
{
	crc.CRC32XOR( m_nBuffID );

	crc.CRC32XOR( m_nBuffPeriodType.Ref() );
	crc.CRC32XOR( m_nBuffPeriod.Ref() );
}

void MMatchBuffDesc::ShiftFugitiveValues()
{
	m_nBuffEffectType.ShiftHeapPos_CheckCrc();
	m_nBuffPeriodType.ShiftHeapPos_CheckCrc();
	m_nBuffPeriod.ShiftHeapPos_CheckCrc();
	m_pBuffInfo->ShiftHeapPos_CheckCrc();
}

///////////////////////////////////////////////////////////////////////////////
// MMatchItemDescMgr //////////////////////////////////////////////////////////
MMatchBuffDescMgr::MMatchBuffDescMgr() : m_nChecksum(0)
{

}

MMatchBuffDescMgr::~MMatchBuffDescMgr()
{
	Clear();
}

bool MMatchBuffDescMgr::ReadXml(const char* szFileName)
{
	m_nChecksum = MGetMZFileChecksum(szFileName);

	MXmlDocument xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(szFileName))
	{
		m_nChecksum = 0;
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MICTOK_BUFF))
		{
			if( !ParseItem(chrElement) ) {
				Clear();
				return false;
			}
		}
	}

	xmlIniData.Destroy();
	return true;
}


bool MMatchBuffDescMgr::ReadXml(MZFileSystem* pFileSystem, const char* szFileName)
{
	MXmlDocument xmlIniData;
	xmlIniData.Create();

	char *buffer;
	MZFile mzf;

	if(pFileSystem) 
	{
		if(!mzf.Open(szFileName,pFileSystem)) 
		{
			if(!mzf.Open(szFileName)) 
			{
				xmlIniData.Destroy();
				return false;
			}
		}
	} 
	else 
	{
		if(!mzf.Open(szFileName))
		{
			xmlIniData.Destroy();
			return false;
		}
	}

	buffer = new char[mzf.GetLength()+1];
	buffer[mzf.GetLength()] = 0;
	memset( buffer, 0, mzf.GetLength()+1 );

	mzf.Read(buffer,mzf.GetLength());

	m_nChecksum = MGetMemoryChecksum(buffer,mzf.GetLength());

	if(!xmlIniData.LoadFromMemory(buffer))
	{
		m_nChecksum = 0;
		xmlIniData.Destroy();
		return false;
	}
	delete[] buffer;
	mzf.Close();

	//	<------------------


	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MICTOK_BUFF))
		{
			if( !ParseItem(chrElement) ) {
				Clear();
				return false;
			}
		}
	}

	xmlIniData.Destroy();
	return true;
}

void MMatchBuffDescMgr::Clear()
{
	while(!empty())
	{
		MMatchBuffDesc* pBuffDesc = (*begin()).second;
		delete pBuffDesc;
		erase(begin());
	}
}

MMatchBuffDesc* MMatchBuffDescMgr::GetBuffDesc(unsigned long int nBuffID)
{
	iterator itor = find(nBuffID);
	if (itor != end()) {
		return (*itor).second;
	}

	//_ASSERT(0);
	return NULL;
}

bool MMatchBuffDescMgr::ParseItem(MXmlElement& element)
{
	MMatchBuffDesc* pNewDesc = new MMatchBuffDesc;

	int n = 0;
	char szAttrValue[256];
	char szAttrName[64];

	int nAttrCount = element.GetAttributeCount();
	for (int i = 0; i < nAttrCount; i++)
	{
		memset( szAttrValue, 0, 256 );
		memset( szAttrName, 0, 64 );
		element.GetAttribute(i, szAttrName, szAttrValue);

		if (!_stricmp(szAttrName, MICTOK_ATTRNAME_BUFF_ID))
		{
			pNewDesc->m_nBuffID = atoi(szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_ATTRNAME_BUFF_ICON_NAME))
		{
			strcpy(pNewDesc->m_szBuffIconName, szAttrValue);
		}
		else if (!_stricmp(szAttrName, MICTOK_ATTRNAME_BUFF_PERIOD_TYPE))
		{
			if (!_stricmp(szAttrValue, MICTOK_ATTRVAL_PERIOD_TYPE_LONG))			pNewDesc->m_nBuffPeriodType.Set(MMBPT_LONG);
			else if (!_stricmp(szAttrValue, MICTOK_ATTRVAL_PERIOD_TYPE_SHORT))	pNewDesc->m_nBuffPeriodType.Set(MMBPT_SHORT);
		}
		else if (!_stricmp(szAttrName, MICTOK_ATTRNAME_BUFF_PERIOD))
		{			
			pNewDesc->m_nBuffPeriod.Set_MakeCrc(atoi(szAttrValue));
		}
		else if (!_stricmp(szAttrName, MICTOK_ATTRNAME_BUFF_EFFECT_TYPE))
		{			
			if (!_stricmp(szAttrValue, MICTOK_ATTRVAL_BUFF_EFFECT_TYPE_NORMAL))		pNewDesc->m_nBuffEffectType.Set(MMBET_NORMAL);
			else if (!_stricmp(szAttrValue, MICTOK_ATTRVAL_BUFF_EFFECT_TYPE_DOTE))	pNewDesc->m_nBuffEffectType.Set(MMBET_DOTE);
		}		
		else if (!_stricmp(szAttrName, MICTOK_ATTRNAME_BUFF_HP_VALUE))
		{
			pNewDesc->m_pBuffInfo->Ref().nHP = atoi(szAttrValue);
			pNewDesc->m_pBuffInfo->MakeCrc();
		}
		else if (!_stricmp(szAttrName, MICTOK_ATTRNAME_BUFF_AP_VALUE))
		{
			pNewDesc->m_pBuffInfo->Ref().nAP = atoi(szAttrValue);
			pNewDesc->m_pBuffInfo->MakeCrc();
		}
		else if (!_stricmp(szAttrName, MICTOK_ATTRNAME_BUFF_RESPAWN_DECREASE_TIME))
		{
			pNewDesc->m_pBuffInfo->Ref().nRespawnDecTime = atoi(szAttrValue);
			pNewDesc->m_pBuffInfo->MakeCrc();
		}
		else 
		{
			//_ASSERT(0);

			delete pNewDesc;
			return false;
		}
	}

	iterator tempitor = find(pNewDesc->m_nBuffID);
	if (tempitor != end()) {
		//_ASSERT(0);		///< 같은 ID의 아이템이 존재한다.
		delete pNewDesc;
		return false;
	}

	insert(value_type(pNewDesc->m_nBuffID, pNewDesc));
	return true;
}

bool MMatchBuffDescMgr::SetBuffName(MMatchItemDescMgr* pItemDescMgr)
{
	MMatchItemDescMgr::iterator iter = pItemDescMgr->begin();
	for( ; iter != pItemDescMgr->end(); iter++) 
	{
		MMatchItemDesc* pItemDesc = iter->second;

		if( pItemDesc->m_nSpendType.Ref() == MMCT_LONGBUFF || pItemDesc->m_nSpendType.Ref() == MMCT_SHORTBUFF ) 
		{
			MMatchBuffDesc* pBuffDesc = MGetMatchBuffDescMgr()->GetBuffDesc(pItemDesc->m_nID);
			if( pBuffDesc == NULL) {
				//_ASSERT(0);
				return false;
			}

			if( pBuffDesc->m_nBuffPeriodType.Ref() == MMBPT_LONG && pItemDesc->m_nSpendType.Ref() != MMCT_LONGBUFF ) {
				//_ASSERT(0);
				return false;
			}

			if( pBuffDesc->m_nBuffPeriodType.Ref() == MMBPT_SHORT && pItemDesc->m_nSpendType.Ref() != MMCT_SHORTBUFF ) {
				//_ASSERT(0);
				return false;
			}

			strcpy(pBuffDesc->m_szBuffName, pItemDesc->m_pMItemName->Ref().m_szItemName);
			strcpy(pBuffDesc->m_szBuffDesc, pItemDesc->m_szDesc);
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// MMatchBuff
void MMatchBuff::Reset()
{
	m_uidBuff = MUID(0, 0);

	m_nBuffID				= 0;
	m_nRegTime				= 0;
	m_nBuffPeriodRemainder	= 0;

	m_pBuffDesc = NULL;
}

bool MMatchBuff::Set(MUID& uidBuff, int nBuffID, int nRegTime, int nBuffPeriodRemainder)
{
	m_pBuffDesc = MGetMatchBuffDescMgr()->GetBuffDesc(nBuffID);
	if( m_pBuffDesc == NULL ) return false;

	m_uidBuff				= uidBuff;

	m_nBuffID				= nBuffID;
	m_nRegTime				= nRegTime;
	m_nBuffPeriodRemainder	= nBuffPeriodRemainder;

	return true;
}

bool MMatchBuff::IsExpired(int nGlobalTick)
{
	if( GetBuffPeriodRemainder(nGlobalTick) <= 0 )	return true;
	else											return false;
}

int MMatchBuff::GetBuffPeriodRemainder(int nGlobalTick)
{ 
	if( m_pBuffDesc == NULL ) {
		return 0;
	}

	int nTick = (m_nRegTime + m_nBuffPeriodRemainder) - nGlobalTick;

	if( nTick < 0 ) return 0;
	else			return nTick;
}


/////////////////////////////////////////////////////////////////////////////
// MMatchShortBuffMap
void MMatchShortBuffMap::Clear() 
{
	while(!empty()) {
		delete (MMatchShortBuff*)begin()->second;
		erase(begin());
	}
}

void MMatchShortBuffMap::Remove(MUID& uidBuff)
{
	iterator iter = find(uidBuff);
	if( iter != end() ) {
		delete (MMatchShortBuff*)iter->second;
		erase(iter);		
	}
}

bool MMatchShortBuffMap::Insert(MUID& uidBuff, MMatchShortBuff* pBuff)
{
	iterator iter = find(uidBuff);
	if( iter != end() ) { return false; }
	insert(pair<MUID, MMatchShortBuff*>(uidBuff, pBuff));
	return true;
}

MMatchShortBuff* MMatchShortBuffMap::GetShortBuffByBuffID(int nBuffID)
{
	for(iterator iter = begin(); iter != end(); iter++) {
		MMatchShortBuff* pInfo = iter->second;
		if(pInfo->GetBuffID() == nBuffID) return pInfo;
	}
	return NULL;
}
