#include "stdafx.h"
#include "MXml.h"
#include "MAgentConfig.h"

//#define AGENTCONFIGFILE		"AgentConfig.xml"

MAgentConfig* MAgentConfig::m_pInstance = NULL;


//MAgentConfig* MAgentConfig::GetInstance()
//{ 
//	if (m_pInstance == NULL) {
//		m_pInstance = new MAgentConfig;
//		m_pInstance->LoadFromXML(AGENTCONFIGFILE);
//	}
//	return m_pInstance; 
//}

MAgentConfig::MAgentConfig()
{ 
	//ZeroMemory(this, sizeof(MAgentConfig));
	memset(m_szName, 0, 128);
	m_nMaxClient = 0;
	memset(m_szIP, 0, 64);
	m_nTCPPort = 0;
	m_nUDPPort = 0;
	memset(m_szMatchServerIP, 0, 64);
	m_nMatchServerTCPPort = 0;
}

MAgentConfig::~MAgentConfig()
{
	//if (m_pInstance == NULL) {
	//	delete m_pInstance;
	//	m_pInstance = NULL;
	//}
}

void MAgentConfig::ReleaseInstance()
{
	//if (m_pInstance == NULL) {
	//	delete m_pInstance;
	//	m_pInstance = NULL;
	//}
	if (m_pInstance != NULL)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

bool MAgentConfig::LoadFromXML(const char* pszFileName)
{
	MXmlDocument	xmlConfig;
	MXmlElement		parentElement, childElement;
	MXmlElement		serverElement;

	xmlConfig.Create();
	if (!xmlConfig.LoadFromFile(pszFileName)) 
	{
		xmlConfig.Destroy();
		return false;
	}

	parentElement = xmlConfig.GetDocumentElement();
	int iCount = parentElement.GetChildNodeCount();

	if (!parentElement.IsEmpty())
	{
		if (parentElement.FindChildNode(ATOK_AGENTCONFIG, &serverElement))
		{
			serverElement.GetChildContents(m_szName, ATOK_NAME);
			serverElement.GetChildContents(&m_nMaxClient, ATOK_MAXCLIENT);
			if (serverElement.FindChildNode(ATOK_ADDRESS, &childElement)) {
				childElement.GetAttribute(m_szIP, ATOK_ATTR_IP);
				childElement.GetAttribute(&m_nTCPPort, ATOK_ATTR_TCPPORT);
				childElement.GetAttribute(&m_nUDPPort, ATOK_ATTR_UDPPORT);
			}
			if (serverElement.FindChildNode(ATOK_MATCHSERVERADDRESS, &childElement)){
				childElement.GetAttribute(m_szMatchServerIP, ATOK_ATTR_IP);
				childElement.GetAttribute(&m_nMatchServerTCPPort, ATOK_ATTR_TCPPORT);
			}
		}
	}

	xmlConfig.Destroy();

	return true;
}