#include "stdafx.h"
#include "RAniEventInfo.h"

RAniNameEventSet* _RAniIDEventSet::GetAniNameEventSet(char* AnimationName)
{
	for(m_AniIDEventSetIter = m_AniIDEventSet.begin(); m_AniIDEventSetIter != m_AniIDEventSet.end(); m_AniIDEventSetIter++)
	{
		if( strcmp( (*(m_AniIDEventSetIter))->GetAnimationName(), AnimationName) == 0)
		{
			return (*(m_AniIDEventSetIter));
		}
	}
	return NULL;
}


RAniIDEventSet*	RAniEventMgr::GetAniIDEventSet(int id)
{
	for(m_AniEventMgrIter = m_AniEventMgr.begin(); m_AniEventMgrIter != m_AniEventMgr.end() ; m_AniEventMgrIter++)
	{
		if( (*(m_AniEventMgrIter))->GetID() == id)
		{
			return (*(m_AniEventMgrIter));
		}
	}
	return NULL;
}

bool RAniEventMgr::ReadXml(MZFileSystem* pFileSystem, const char* szFileName)
{
	MXmlDocument xmlIniData;
	if (!xmlIniData.LoadFromFile(szFileName, pFileSystem))
		return false;

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++) {

		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;

		if (!stricmp(szTagName, "NPC")) {
			char ID[256];

			chrElement.GetAttribute(ID, "id");
			RAniIDEventSet * pAniIDEventSet = new RAniIDEventSet();
			pAniIDEventSet->SetID(atoi(ID));
			ParseAniEvent(&chrElement, pAniIDEventSet);
			m_AniEventMgr.push_back(pAniIDEventSet);
		}
	}

	return true;
}

void RAniEventMgr::ParseAniEvent(MXmlElement* PNode, RAniIDEventSet* pAnimIdEventSet)
{
	char NodeName[256];
	char cAnimationName[256];

	int nCnt = PNode->GetChildNodeCount();

	MXmlElement Node;

	for (int i=0; i<nCnt; i++) 
	{
		Node = PNode->GetChildNode(i);

		Node.GetTagName(NodeName);

		if (NodeName[0] == '#') continue;

		if (strcmp(NodeName, "Animation")==0) 
		{
			Node.GetAttribute(cAnimationName, "name");
			//애니메이션 이름 이벤트 셋 생성
			RAniNameEventSet* pAniNameEventSet = new RAniNameEventSet();
			pAniNameEventSet->SetAnimationName(cAnimationName);

			MXmlElement ChildNode;
			int nEventChiledCnt = Node.GetChildNodeCount();	//차일드 노드 개수 읽어와서
			for(int i=0; i<nEventChiledCnt; i++)			//파싱하기
			{
				char ChildNodeName[256];
				char ChildEventType[256];
				char ChildEventFileName[256];
				char ChildEventBeginFrame[256];
				
				ChildNode = Node.GetChildNode(i);
				ChildNode.GetNodeName(ChildNodeName);
				if(strcmp(ChildNodeName, "AddAnimEvent")==0)
				{
					ChildNode.GetAttribute(ChildEventType, "eventtype");
					ChildNode.GetAttribute(ChildEventFileName, "filename");
					ChildNode.GetAttribute(ChildEventBeginFrame, "beginframe");

					//애니메이션 이벤트 생성
					RAniEventInfo* AniEventInfo = new RAniEventInfo();

					AniEventInfo->SetBeginFrame(atoi(ChildEventBeginFrame));
					AniEventInfo->SetEventType(ChildEventType);
					AniEventInfo->SetFileName(ChildEventFileName);
					//애니메이션 이벤트를 애니메이션 이름 이벤트 셋에 추가
					pAniNameEventSet->m_AniNameEventSet.push_back(AniEventInfo);
				}
			}
			//애니메이션 이름 이벤트 셋을 애니메이션 아이디 이벤트 셋에 추가
			pAnimIdEventSet->m_AniIDEventSet.push_back(pAniNameEventSet);
		}
	}
}
