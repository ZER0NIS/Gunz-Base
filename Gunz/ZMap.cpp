#include "stdafx.h"
#include "ZMap.h"
#include "ZApplication.h"
#include "MComboBox.h"
#include "ZChannelRule.h"

#include "ZGameClient.h"

void ZGetCurrMapPath(char* outPath)
{

	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
	{
		strcpy(outPath, PATH_QUEST_MAPS);
		return;
	}

	strcpy(outPath, PATH_GAME_MAPS);
}

bool InitMaps(MWidget *pWidget)
{
	if(!pWidget) return false;

	MComboBox* pCombo=(MComboBox*)pWidget;
	pCombo->RemoveAll();

	// ����Ʈ ���� scenario.xml�� ����� �ʵ鸸 �о���δ�.
	if ((ZGetGameClient()) && ( ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())))
	{
		LIST_SCENARIONAME* pSenarioDesc = ZApplication::GetStageInterface()->GetSenarioDesc();

		// �Ϲ� �ó������� �о�´�. 1000������ ����
		for ( int i = 1000;  ; i++)
		{
			LIST_SCENARIONAME::iterator itr = pSenarioDesc->find( i);


			// ���̸� ����
			if ( itr == pSenarioDesc->end())
				return true;


			// ������ �޺� �ڽ��� �̹� �����ϴ��� �˻�.
			MSenarioList Scenario = (*itr).second;
			bool bFind = false;

			for ( int k = 0;  k < pCombo->GetCount();  k++)
			{
				if ( !strcmp( Scenario.m_szMapSet, pCombo->GetString( k)))
				{
					bFind = true;
					break;
				}
			}

			// �޺� �ڽ��� ������ �޺� �ڽ��� �߰�.
			if ( !bFind)
				pCombo->Add( Scenario.m_szMapSet);

		}

		return true;
	}

	MChannelRule* pRule = ZGetChannelRuleMgr()->GetCurrentRule();
	if (pRule == NULL) {
		mlog("Init maps : no Current ChannelRule \n");
		return false;
	}

	bool bRelayMapInsert = false;
	int nMapCount = MGetMapDescMgr()->GetMapCount();
	for( int i=0 ; i< nMapCount ; i++)
	{

//#ifdef _DEBUG	// ����׽� ���� ������
//		pCombo->Add(MGetMapDescMgr()->GetMapName(i));
//		continue;
//#endif

		bool bDuelMode = false;
		bool bCTFMode = false;
		if ( ZGetGameClient() && (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUEL))
			bDuelMode = true;
		if ( ZGetGameClient() && (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_CTF))
			bCTFMode = true;

		bool UseThisMap = true;
		// �����̸��� ������ �־��ش�.
		if(!bRelayMapInsert)
		{
			for(int j=0 ; j< MGetMapDescMgr()->GetMapCount() ; j++)
			{
				if(strcmp(MMATCH_MAPNAME_RELAYMAP, MGetMapDescMgr()->GetMapName(j)) == 0)
				{
					pCombo->Add(MGetMapDescMgr()->GetMapName(j));
					bRelayMapInsert = true;
					//--nMapCount;	// �����̸��� �߰������� MapCount�� �ٿ��ش�.
					continue;
				}
			}
		}

		if(bCTFMode) 
		{
			if ( pRule->CheckCTFMap( MGetMapDescMgr()->GetMapName(i)))
			{
				if(strcmp(MMATCH_MAPNAME_RELAYMAP, MGetMapDescMgr()->GetMapName(i)) != 0)
					pCombo->Add(MGetMapDescMgr()->GetMapName(i));
			}
		}

		if ( pRule->CheckMap( MGetMapDescMgr()->GetMapName(i), bDuelMode) && !bCTFMode)
			if(strcmp(MMATCH_MAPNAME_RELAYMAP, MGetMapDescMgr()->GetMapName(i)) != 0)
				pCombo->Add(MGetMapDescMgr()->GetMapName(i));
	}

	return true;
}
