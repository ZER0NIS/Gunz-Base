#include "stdafx.h"

#include <winsock2.h>
#include "MErrorTable.h"
#include "ZConfiguration.h"
#include "ZGameClient.h"
#include "MSharedCommandTable.h"
#include "ZConsole.h"
#include "MCommandLogFrame.h"
#include "ZIDLResource.h"
#include "MBlobArray.h"
#include "ZInterface.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "MMatchChannel.h"
#include "MMatchStage.h"
#include "ZPost.h"
#include "MMatchNotify.h"
#include "ZMatch.h"
#include "MComboBox.h"
#include "MTextArea.h"
#include "ZCharacterViewList.h"
#include "ZCharacterView.h"
#include "MDebug.h"
#include "ZScreenEffectManager.h"
#include "ZRoomListBox.h"
#include "ZPlayerListBox.h"
#include "ZChat.h"
#include "ZWorldItem.h"
#include "ZChannelRule.h"
#include "ZMyInfo.h"
#include "MToolTip.h"
#include "ZColorTable.h"
#include "ZNetRepository.h"
#include "ZCountDown.h"
#include "ZBmNumLabel.h"
#include "ZClanListBox.h"

#include "ZLanguageConf.h"

#define CLAN_CREATING_AGREEMENT_TIMEOUT			(1000 * 30)		// 30�� Ÿ�Ӿƿ�
#define CLAN_JOINING_AGREEMENT_TIMEOUT			(1000 * 30)

void ShowClanSponsorAgreeWaitFrame(bool bVisible);
void ShowClanJoinerAgreeWaitFrame(bool bVisible);

void OnTimerDialogTimeOut(void* pParam);

struct Clan_Sponsors_Ticket
{
	int		nRequestID;
	char	szClanName[256];
	char	szSponsorCharName[CLAN_SPONSORS_COUNT][256];
	bool	bAnswered[CLAN_SPONSORS_COUNT];
	bool	bAgreed[CLAN_SPONSORS_COUNT];
} ClanSponsorsTicket;

/// Ŭ������ ������ ��ٸ��� �ִ����� ����
bool IsWaitingClanCreatingAgree()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanSponsorAgreementWait");
	if (pWidget != NULL)
	{
		return pWidget->IsVisible();
	}
	return false;
}

/// Ŭ������ ������ ��ٸ��� �ִ����� ����
bool IsWaitingClanJoiningAgree()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanJoinerAgreementWait");
	if (pWidget != NULL)
	{
		return pWidget->IsVisible();
	}
	return false;
}

void ShowClanSponsorAgreeWaitFrame_OnExpire()
{
	ZChatOutput(ZMsg(MSG_CANCELED));
}

void ShowClanSponsorAgreeWaitFrame(bool bVisible)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanSponsorAgreementWait");
	if (pWidget != NULL)
	{
		if (bVisible)
		{
			static ZCOUNTDOWN countDown = { 30,"ClanSponsorAgreementWait_Remain",
				"ClanSponsorAgreementWait",ShowClanSponsorAgreeWaitFrame_OnExpire };
			countDown.nSeconds = 30;	// static �̹Ƿ� �缳��
			ZApplication::GetTimer()->SetTimerEvent(0, &OnTimer_CountDown, &countDown, true);

			pWidget->Show(true, true);
		}
		else
		{
			pWidget->Show(false);
		}
	}
}

void ShowClanJoinerAgreeWaitFrame_OnExpire()
{
	ZChatOutput(ZMsg(MSG_CANCELED));
}

void ShowClanJoinerAgreeWaitFrame(bool bVisible)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanJoinerAgreementWait");
	if (pWidget != NULL)
	{
		if (bVisible)
		{
			static ZCOUNTDOWN countDown = { 30,"ClanJoinerAgreementWait_Remain",
				"ClanJoinerAgreementWait",ShowClanJoinerAgreeWaitFrame_OnExpire };
			countDown.nSeconds = 30;	// static �̹Ƿ� �缳��
			ZApplication::GetTimer()->SetTimerEvent(0, &OnTimer_CountDown, &countDown, true);

			pWidget->Show(true, true);
		}
		else
		{
			pWidget->Show(false);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void ZGameClient::OnResponseCreateClan(const int nResult, const int nRequestID)
{
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_LOBBY)
	{
		return;
	}

	if (nResult == MOK)
	{
		// Ŭ������������� �����ҵ��� ��ٸ���� �޼���â�� ����.
		ShowClanSponsorAgreeWaitFrame(true);
	}
	else
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(nResult);
	}
}

void ZGameClient::OnResponseAgreedCreateClan(const int nResult)
{
	// Ŭ������������� �����ҵ��� ��ٸ���� �޼���â�� ���ش�.
	ShowClanSponsorAgreeWaitFrame(false);

	if (nResult == MOK)
	{
		ZApplication::GetGameInterface()->ShowMessage(MSG_CLAN_CREATED);
	}
	else
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(nResult);
	}
}

void OnClanAskSponsorAgreement_OnExpire()
{
	ZChatOutput(ZMsg(MSG_CANCELED));
	ZGetGameClient()->AnswerSponsorAgreement(false);
}

void ZGameClient::OnClanAskSponsorAgreement(const int nRequestID, const char* szClanName, MUID& uidMasterObject, const char* szMasterName)
{
	// ������ ���� ��Ȳ�̸� ����
	if (!ZGetGameInterface()->IsReadyToPropose()) return;

	m_nRequestID = nRequestID;
	m_uidRequestPlayer = uidMasterObject;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MTextArea* pTextEdit = (MTextArea*)pResource->FindWidget("ClanSponsorAgreementConfirm_Textarea");
	if (pTextEdit)
	{
		char szTemp[256];
		ZTransMsg(szTemp, MSG_CLAN_SPONSOR_AGREEMENT_LABEL, 3, szMasterName, ZGetMyInfo()->GetCharName(), szClanName);
		pTextEdit->SetText(szTemp);
	}

	MWidget* pWidget = pResource->FindWidget("ClanSponsorAgreementConfirm");
	if (pWidget != NULL)
	{
		static ZCOUNTDOWN countDown = { 30,"ClanSponsorAgreementConfirm_Remain",
			"ClanSponsorAgreementConfirm",OnClanAskSponsorAgreement_OnExpire };
		countDown.nSeconds = 30;	// static �̹Ƿ� �缳��
		ZApplication::GetTimer()->SetTimerEvent(0, &OnTimer_CountDown, &countDown, true);

		pWidget->Show(true, true);
	}
}

void ZGameClient::OnClanAnswerSponsorAgreement(const int nRequestID, const MUID& uidClanMaster, char* szSponsorCharName, const bool bAnswer)
{
	if (!IsWaitingClanCreatingAgree()) return;

	// ������ request���� ��� �����Ѵ�.
	if ((ClanSponsorsTicket.nRequestID != nRequestID) || (ClanSponsorsTicket.nRequestID == 0))
	{
		return;
	}

	for (int i = 0; i < CLAN_SPONSORS_COUNT; i++)
	{
		if ((strlen(ClanSponsorsTicket.szSponsorCharName[i])) > 0)
		{
			if (!strcmp(ClanSponsorsTicket.szSponsorCharName[i], szSponsorCharName))
			{
				ClanSponsorsTicket.bAgreed[i] = bAnswer;
				ClanSponsorsTicket.bAnswered[i] = true;
				break;
			}
		}
	}

	bool bAllAgreed = true;
	bool bAllAnswered = true;

	for (int i = 0; i < CLAN_SPONSORS_COUNT; i++)
	{
		if ((ClanSponsorsTicket.bAgreed[i] == false) || ((strlen(ClanSponsorsTicket.szSponsorCharName[i])) <= 0))
		{
			bAllAgreed = false;
		}

		if (ClanSponsorsTicket.bAnswered[i] == false)
		{
			bAllAnswered = false;
		}
	}

	// ��� �����������
	if (bAllAgreed)
	{
		// ���⼭ ������ Ŭ�� ���� ��û
		if (strlen(ClanSponsorsTicket.szClanName) >= 4)
		{
			char* ppSponsorCharName[CLAN_SPONSORS_COUNT];
			for (int i = 0; i < CLAN_SPONSORS_COUNT; i++) ppSponsorCharName[i] = ClanSponsorsTicket.szSponsorCharName[i];

			ZPostRequestAgreedCreateClan(GetPlayerUID(), ClanSponsorsTicket.szClanName, ppSponsorCharName, CLAN_SPONSORS_COUNT);
		}

		memset(&ClanSponsorsTicket, 0, sizeof(Clan_Sponsors_Ticket));
	}

	// ��� ��������� ������ �Ѹ��̶� �������
	if ((bAllAnswered) && (!bAllAgreed))
	{
		ShowClanSponsorAgreeWaitFrame(false);

		char temp[256];
		ZTransMsg(temp, MSG_CLAN_SPONSOR_AGREEMENT_REJECT, 1, szSponsorCharName);
		ZApplication::GetGameInterface()->ShowMessage(temp, NULL, MSG_CLAN_SPONSOR_AGREEMENT_REJECT);
	}
}

void ZGameClient::AnswerSponsorAgreement(bool bAnswer)
{
	char szCharName[256];
	sprintf(szCharName, ZGetMyInfo()->GetCharName());
	ZPostAnswerSponsorAgreement(m_nRequestID, m_uidRequestPlayer, szCharName, bAnswer);
}

void ZGameClient::AnswerJoinerAgreement(bool bAnswer)
{
	char szCharName[256];
	sprintf(szCharName, ZGetMyInfo()->GetCharName());

	ZPostAnswerJoinAgreement(m_uidRequestPlayer, szCharName, bAnswer);
}

void ZGameClient::RequestCreateClan(char* szClanName, char** ppMemberCharNames)
{
	// Ŭ���� �̹� ���ԵǾ� ������ �ȵȴ�.
	if (ZGetMyInfo()->IsClanJoined())
	{
		ZChatOutput(
			ZMsg(MSG_CLAN_JOINED_ALREADY),
			ZChat::CMT_SYSTEM);
		return;
	}

	// �κ񿡼��� ��û�� �� �ִ�.
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_LOBBY)
	{
		ZChatOutput(ZMsg(MSG_MUST_EXECUTE_LOBBY));
		return;
	}

	m_nRequestID++;

	memset(&ClanSponsorsTicket, 0, sizeof(Clan_Sponsors_Ticket));
	ClanSponsorsTicket.nRequestID = m_nRequestID;
	strcpy(ClanSponsorsTicket.szClanName, szClanName);

	for (int i = 0; i < CLAN_SPONSORS_COUNT; i++)
	{
		strcpy(ClanSponsorsTicket.szSponsorCharName[i], ppMemberCharNames[i]);
	}

	ZPostRequestCreateClan(GetPlayerUID(), m_nRequestID, szClanName, ppMemberCharNames, CLAN_SPONSORS_COUNT);
}

void ZGameClient::OnClanResponseCloseClan(const int nResult)
{
	if (nResult == MOK)
	{
		char szDay[4] = "";
		char szText[256] = "";

		sprintf(szDay, "%u", DAY_OF_DELETE_CLAN);
		ZTransMsg(szText, MSG_CLAN_CLOSE_RESERVED, 1, szDay);

		ZApplication::GetGameInterface()->ShowMessage(szText);
	}
	else
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(nResult);
	}
}

void ZGameClient::OnClanResponseJoinClan(const int nResult)
{
	if (nResult == MOK)
	{
		// ���â�� ����.
		ShowClanJoinerAgreeWaitFrame(true);
	}
	else
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(nResult);
	}
}

void OnClanAskJoinAgreement_OnExpire()
{
	ZChatOutput(ZMsg(MSG_CANCELED), ZChat::CMT_SYSTEM);
	ZGetGameClient()->AnswerJoinerAgreement(false);
}

void ZGameClient::OnClanAskJoinAgreement(const char* szClanName, MUID& uidClanAdmin, const char* szClanAdmin)
{
	// ������ ���� ��Ȳ�̸� ����
	if (!ZGetGameInterface()->IsReadyToPropose()) return;

	m_uidRequestPlayer = uidClanAdmin;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MTextArea* pTextEdit = (MTextArea*)pResource->FindWidget("ClanJoinerAgreementConfirm_Textarea");
	if (pTextEdit)
	{
		char szTemp[256];
		//		ZTransMsg(szTemp, MSG_CLAN_JOINER_AGREEMENT_LABEL, 1, szClanName);
		ZTransMsg(szTemp, MSG_CLAN_JOINER_AGREEMENT_LABEL, 1, szClanName);
		pTextEdit->SetText(szTemp);
	}

	MWidget* pWidget = pResource->FindWidget("ClanJoinerAgreementConfirm");
	if (pWidget != NULL)
	{
		static ZCOUNTDOWN countDown = { 30,"ClanJoinerAgreementConfirm_Remain",
			"ClanJoinerAgreementConfirm",OnClanAskJoinAgreement_OnExpire };
		countDown.nSeconds = 30;	// static �̹Ƿ� �缳��
		ZApplication::GetTimer()->SetTimerEvent(0, &OnTimer_CountDown, &countDown, true);

		pWidget->Show(true, true);
	}
}

void ZGameClient::OnClanAnswerJoinAgreement(const MUID& uidClanAdmin, const char* szJoiner, const bool bAnswer)
{
	if (!IsWaitingClanJoiningAgree()) return;

	if (ZGetGameClient()->GetPlayerUID() != uidClanAdmin) return;

	if (bAnswer)
	{
		if (IsUpperClanGrade(ZGetMyInfo()->GetClanGrade(), MCG_ADMIN))
		{
			char szClanName[256];
			sprintf(szClanName, ZGetMyInfo()->GetClanName());
			ZPostRequestAgreedJoinClan(uidClanAdmin, szClanName, (char*)szJoiner);
		}
	}
	else
	{
		// ����â ������� �Ѵ�.
		ShowClanJoinerAgreeWaitFrame(false);

		m_uidRequestPlayer = MUID(0, 0);

		ZApplication::GetGameInterface()->ShowMessage(MSG_CLAN_JOINER_AGREEMENT_REJECT);
	}
}

void ZGameClient::OnClanResponseAgreedJoinClan(const int nResult)
{
	if (!IsWaitingClanJoiningAgree()) return;

	// ���â�� ���־��Ѵ�.
	ShowClanJoinerAgreeWaitFrame(false);
	m_uidRequestPlayer = MUID(0, 0);

	if (nResult == MOK)
	{
		// Ŭ�� ������
		ZApplication::GetGameInterface()->ShowMessage(MSG_CLAN_JOINED_NEW_MEMBER);
	}
	else
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(nResult);
	}
}

ZPlayerListBox* GetProperClanListOutput()
{
	ZIDLResource* pIDLResource = ZApplication::GetGameInterface()->GetIDLResource();

	GunzState nState = ZApplication::GetGameInterface()->GetState();
	switch (nState) {
	case GUNZ_LOBBY:
	{
		ZPlayerListBox* pList = (ZPlayerListBox*)pIDLResource->FindWidget("LobbyChannelPlayerList");
		if (pList && pList->GetMode() == ZPlayerListBox::PLAYERLISTMODE_CHANNEL_CLAN)
			return pList;
		else
			return NULL;
	}
	break;
	case GUNZ_STAGE:
	{
		ZPlayerListBox* pList = (ZPlayerListBox*)pIDLResource->FindWidget("StagePlayerList_");
		if (pList && pList->GetMode() == ZPlayerListBox::PLAYERLISTMODE_STAGE_CLAN)
			return pList;
		else
			return NULL;
	}
	break;
	};
	return NULL;
}

void ZGameClient::OnClanUpdateCharClanInfo(void* pBlob)
{
	int nCount = MGetBlobArrayCount(pBlob);
	if (nCount != 1) return;

	MTD_CharClanInfo* pClanInfoNode = (MTD_CharClanInfo*)MGetBlobArrayElement(pBlob, 0);

	ZGetMyInfo()->SetClanInfo(pClanInfoNode->szClanName, pClanInfoNode->nGrade);
	ZGetMyInfo()->Serialize();

	// Ŭ�������� ����Ǿ���.
	if (ZGetGameClient()->IsConnected())
		ZPostRequestClanMemberList(ZGetGameClient()->GetPlayerUID());

	ZPlayerListBox* pList = GetProperClanListOutput();
	if (pList) {
		if (!ZGetMyInfo()->IsClanJoined())
		{
			ZPlayerListBox* pWidget = (ZPlayerListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
			if (pWidget) {
				pWidget->RemoveAll();
			}
		}

		pList->RefreshUI();
	}
}

void ZGameClient::OnClanResponseLeaveClan(const int nResult)
{
	if (nResult == MOK)
	{
		ZApplication::GetGameInterface()->ShowMessage(MSG_CLAN_LEAVED);
	}
	else
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(nResult);
	}
}

void ZGameClient::OnClanResponseChangeGrade(const int nResult)
{
	if (nResult == MOK)
	{
		ZApplication::GetGameInterface()->ShowMessage(MSG_CLAN_CHANGED_GRADE);
	}
	else
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(nResult);
	}
}

void ZGameClient::OnClanResponseExpelMember(const int nResult)
{
	if (nResult == MOK)
	{
		ZApplication::GetGameInterface()->ShowMessage(MSG_CLAN_EXPEL_MEMBER);
	}
	else
	{
		ZApplication::GetGameInterface()->ShowErrorMessage(nResult);
	}
}

void ZGameClient::OnClanMsg(const char* szSenderName, const char* szMsg)
{
	char szText[512];
	sprintf(szText, "%s(%s) : %s", ZMsg(MSG_CHARINFO_CLAN), szSenderName, szMsg);

	if (ZGetGame())
	{
		if ((ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL) && !ZGetGame()->m_pMyCharacter->IsDie())
			sprintf(szText, "%s(%s) : %s", ZMsg(MSG_CHARINFO_CLAN), szSenderName, ". . . . .");
	}

	ZChatOutput(MCOLOR(ZCOLOR_CHAT_CLANMSG), szText);
}

void ZGameClient::OnClanMemberList(void* pBlob)
{
	ZPlayerListBox* pPlayerListBox = GetProperClanListOutput();
	if (!pPlayerListBox) return;

	MUID selUID = pPlayerListBox->GetSelectedPlayerUID();

	int nStartIndex = pPlayerListBox->GetStartItem();

	int nCount = MGetBlobArrayCount(pBlob);

	if (nCount) {
		pPlayerListBox->RemoveAll();
	}
	else {//�ƹ����뵵 ���ٸ�~
		return;
	}

	for (int i = 0; i < nCount; i++)
	{
		MTD_ClanMemberListNode* pNode = (MTD_ClanMemberListNode*)MGetBlobArrayElement(pBlob, i);

		ePlayerState state;
		switch (pNode->nPlace)
		{
		case MMP_LOBBY: state = PS_LOBBY; break;
		case MMP_STAGE: state = PS_WAIT; break;
		case MMP_BATTLE: state = PS_FIGHT; break;
		default: state = PS_LOBBY;
		};

		pPlayerListBox->AddPlayer(pNode->uidPlayer, state, pNode->szName, pNode->nLevel, pNode->nClanGrade);
	}

	pPlayerListBox->SetStartItem(nStartIndex);
	pPlayerListBox->SelectPlayer(selUID);
}

void ZGameClient::OnClanResponseClanInfo(void* pBlob)
{
	int nCount = MGetBlobArrayCount(pBlob);
	if (nCount != 1) return;

	MTD_ClanInfo* pClanInfo = (MTD_ClanInfo*)MGetBlobArrayElement(pBlob, 0);

	// �̹� emblem�� ������ �־����� emblem interface �� �뺸���ش�
	int nOldClanID = ZGetNetRepository()->GetClanInfo()->nCLID;

	// repository�� Ŭ�������� �����Ѵ�
	memcpy(ZGetNetRepository()->GetClanInfo(), pClanInfo, sizeof(MTD_ClanInfo));

	//	mlog("OnClanResponseClanInfo : ");

		// emblem interface �� �뺸�Ѵ�
	ZGetEmblemInterface()->AddClanInfo(pClanInfo->nCLID);

	if (nOldClanID != 0) {
		ZGetEmblemInterface()->DeleteClanInfo(nOldClanID);
	}

	ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();

	MPicture* pPicture = (MPicture*)pRes->FindWidget("Lobby_ClanInfoEmblem");
	if (pPicture)
		pPicture->SetBitmap(ZGetEmblemInterface()->GetClanEmblem2(pClanInfo->nCLID));

	// Ŭ�� �̸�
	MLabel* pLabel = (MLabel*)pRes->FindWidget("Lobby_ClanInfoName");
	pLabel->SetText(ZGetNetRepository()->GetClanInfo()->szClanName);

	// ���ӵ� �����
	char szCount[16];
	sprintf(szCount, "%d", ZGetNetRepository()->GetClanInfo()->nConnedMember);

	char szOutput[256];
	//	ZTranslateMessage(szOutput,MSG_LOBBY_CLAN_DETAIL,2,
	//		ZGetNetRepository()->GetClanInfo()->szMaster,szCount);
	ZTransMsg(szOutput, MSG_LOBBY_CLAN_DETAIL, 2,
		ZGetNetRepository()->GetClanInfo()->szMaster, szCount);

	pLabel = (MLabel*)pRes->FindWidget("Lobby_ClanInfoDetail");
	pLabel->SetText(szOutput);

	sprintf(szOutput, "%d/%d", ZGetNetRepository()->GetClanInfo()->nWins, ZGetNetRepository()->GetClanInfo()->nLosses);
	ZBmNumLabel* pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_ClanInfoWinLose");
	pNumLabel->SetText(szOutput);

	sprintf(szOutput, "%d", ZGetNetRepository()->GetClanInfo()->nPoint);
	pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_ClanInfoPoints");
	pNumLabel->SetText(szOutput);

	pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_ClanInfoTotalPoints");
	//		sprintf(szOutput,"%d",ZGetNetRepository()->GetClanInfo()->nWins,ZGetNetRepository()->GetClanInfo()->nXP);
	//		pNumLabel->SetText(szOutput);
	pNumLabel->SetNumber(ZGetNetRepository()->GetClanInfo()->nTotalPoint, true);

	int nRanking = pClanInfo->nRanking;

	pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_ClanInfoRanking");
	pNumLabel->SetIndexOffset(16);	// �Ʒ��� ���ٸ� �۾��� ��´�
	MWidget* pUnranked = pRes->FindWidget("Lobby_ClanInfoUnranked");
	if (nRanking == 0) {
		pNumLabel->Show(false);
		if (pUnranked)
			pUnranked->Show(true);
	}
	else
	{
		pNumLabel->Show(true);
		pNumLabel->SetNumber(nRanking);
		if (pUnranked)
			pUnranked->Show(false);
	}

	/*
	// UI�� ������� ������ ������ �غ� �ȵǾ��ִ� ����� ä��â�� �Ѹ���.

	char szText[256];
	sprintf(szText, "Ŭ����: %s", pClanInfo->szClanName);
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_CLANMSG), szText);

	sprintf(szText, "����: %d", pClanInfo->nLevel);
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_CLANMSG), szText);

	sprintf(szText, "����ġ: %d", pClanInfo->nXP);
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_CLANMSG), szText);

	sprintf(szText, "����Ʈ: %d", pClanInfo->nPoint);
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_CLANMSG), szText);

	sprintf(szText, "������: %s", pClanInfo->szMaster);
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_CLANMSG), szText);

	sprintf(szText, "����: %d�� %d��", pClanInfo->nWins, pClanInfo->nLoses);
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_CLANMSG), szText);

	sprintf(szText, "Ŭ��������: �� %d���� %d�� ������", pClanInfo->nTotalMemberCount, pClanInfo->nConnedMember);
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_CLANMSG), szText);
	*/
}

void ZGameClient::OnClanStandbyClanList(int nPrevStageCount, int nNextStageCount, void* pBlob)
{
	int nCount = MGetBlobArrayCount(pBlob);

	ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
	ZClanListBox* pListBox = (ZClanListBox*)pRes->FindWidget("Lobby_ClanList");
	if (!pListBox) return;

	pListBox->ClearAll();
	vector< int > vecClanID;

	for (int i = 0; i < nCount; i++)
	{
		MTD_StandbyClanList* pNode = (MTD_StandbyClanList*)MGetBlobArrayElement(pBlob, i);

		//_ASSERT(i<4);
		pListBox->SetInfo(i, pNode->nCLID, pNode->szClanName, pNode->nPlayers);
		//ProcessEmblem(pNode->nCLID, pNode->nEmblemChecksum);

		// Emblem // Ŭ�� URL�� ������ Vector�� �״´�
		if (m_EmblemMgr.CheckEmblem(pNode->nCLID, pNode->nEmblemChecksum)) {
			// Begin Draw
		}
		else /*if (pNode->nEmblemChecksum != 0)*/ {
			vecClanID.push_back(pNode->nCLID);
		}
	}

	//// Emblem // Ŭ�� URL�� ���� vector�� ������ ������.
	if (vecClanID.size() > 0)
	{
		void* pBlob = MMakeBlobArray(sizeof(int), (int)vecClanID.size()); /// nOneBlobSize��ŭ nBlobCount������ŭ �迭�� �� �����
		int nCount = 0;
		for (vector<int>::iterator it = vecClanID.begin(); it != vecClanID.end(); it++, nCount++)
		{
			int* nClanID = (int*)MGetBlobArrayElement(pBlob, nCount);
			*nClanID = *it;
		}

		ZPostRequestEmblemURL(pBlob);
		MEraseBlobArray(pBlob);
		vecClanID.clear();
	}

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pBtn = pResource->FindWidget("StageBeforeBtn");
	if (nPrevStageCount != -1)
	{
		if (nPrevStageCount == 0)
		{
			if (pBtn) pBtn->Enable(false);
		}
		else
		{
			if (pBtn) pBtn->Enable(true);
		}
	}

	pBtn = pResource->FindWidget("StageAfterBtn");
	if (nNextStageCount != -1)
	{
		if (nNextStageCount == 0)
		{
			if (pBtn) pBtn->Enable(false);
		}
		else
		{
			if (pBtn) pBtn->Enable(true);
		}
	}
}

void ZGameClient::OnClanMemberConnected(const char* szMember)
{
	if (!strcmp(ZGetMyInfo()->GetCharName(), szMember)) return;

	char szMsg[256];

	ZTransMsg(szMsg, MSG_CLAN_MEMBER_CONNECTED, 1, szMember);
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_CLANMSG), szMsg);
}

void ZGameClient::OnBroadcastClanRenewVictories(const char* pszWinnerClanName, const char* pszLoserClanName, int nVictories)
{
	char szText[256];
	char szVic[32];

	int nStringCode = MSG_CLANBATTLE_BROADCAST_RENEW_VICTORIES_3;

	if (nVictories < 5) nStringCode = MSG_CLANBATTLE_BROADCAST_RENEW_VICTORIES_3;
	else if (nVictories < 7) nStringCode = MSG_CLANBATTLE_BROADCAST_RENEW_VICTORIES_5;
	else if (nVictories < 10) nStringCode = MSG_CLANBATTLE_BROADCAST_RENEW_VICTORIES_7;
	else if ((nVictories % 10) == 0) nStringCode = MSG_CLANBATTLE_BROADCAST_RENEW_VICTORIES_10;
	else nStringCode = MSG_CLANBATTLE_BROADCAST_RENEW_VICTORIES_11;

	sprintf(szVic, "%d", nVictories);

	ZTransMsg(szText, nStringCode, 3, pszWinnerClanName, pszLoserClanName, szVic);

	//	ZChatOutput(szText, ZChat::CMT_BROADCAST);

	switch (nStringCode)
	{
	case MSG_CLANBATTLE_BROADCAST_RENEW_VICTORIES_3:
		ZChatOutput(ZCOLOR_CHAT_CLANVICTORY1, szText);
		break;
	case MSG_CLANBATTLE_BROADCAST_RENEW_VICTORIES_5:
		ZChatOutput(ZCOLOR_CHAT_CLANVICTORY2, szText);
		break;
	case MSG_CLANBATTLE_BROADCAST_RENEW_VICTORIES_7:
		ZChatOutput(ZCOLOR_CHAT_CLANVICTORY3, szText);
		break;
	case MSG_CLANBATTLE_BROADCAST_RENEW_VICTORIES_10:
		ZChatOutput(ZCOLOR_CHAT_CLANVICTORY4, szText);
		break;
	case MSG_CLANBATTLE_BROADCAST_RENEW_VICTORIES_11:
		ZChatOutput(ZCOLOR_CHAT_CLANVICTORY5, szText);
		break;
	default:
		ZChatOutput(szText, ZChat::CMT_BROADCAST);
		break;
	}
}

void ZGameClient::OnBroadcastClanInterruptVictories(const char* pszWinnerClanName, const char* pszLoserClanName, int nVictories)
{
	char szText[256];
	char szVic[32];
	sprintf(szVic, "%d", nVictories);
	ZTransMsg(szText, MSG_CLANBATTLE_BROADCAST_INTERRUPT_VICTORIES, 3, pszWinnerClanName, pszLoserClanName, szVic);

	ZChatOutput(szText, ZChat::CMT_BROADCAST);
}