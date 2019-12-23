#include "stdafx.h"
/*#include "MBMatchMonitorXml.h"



MBMatchMonitorXmlElement::MBMatchMonitorXmlElement()
{
}


MBMatchMonitorXmlElement::~MBMatchMonitorXmlElement()
{
}


void MBMatchMonitorXmlElement::SetElement( const MXmlElement& XmlElement )
{
	m_XmlElement = XmlElement;
}


bool MBMatchMonitorXmlElement::GetChildNode( const DWORD dwIndex, MMonitorXmlElement*& pOutXmlElement )
{
	if( NULL != pOutXmlElement )
	{
		_ASSERT( 0 );
		return false;
	}

	MBMatchMonitorXmlElement* pXmlElement = new MBMatchMonitorXmlElement;
	if( NULL == pXmlElement )
		return false;

	MXmlElement xmlElement = m_XmlElement.GetChildNode( dwIndex );

	pXmlElement->SetElement( xmlElement );
	pOutXmlElement = pXmlElement;

	return true;
}

void MBMatchMonitorXmlElement::GetAttribute( const DWORD dwIndex, char* szoutAttrName, char* szoutAttrValue )
{
	m_XmlElement.GetAttribute( dwIndex, szoutAttrName, szoutAttrValue );
}


DWORD MBMatchMonitorXmlElement::GetChildNodeCount()
{
	return m_XmlElement.GetChildNodeCount();
}


DWORD MBMatchMonitorXmlElement::GetAttributeCount()
{
	return m_XmlElement.GetAttributeCount();
}


void MBMatchMonitorXmlElement::GetTagName( char* sOutTagName )
{
	m_XmlElement.GetTagName( sOutTagName );
}


void MBMatchMonitorXmlElement::GetContents( char* szOutStr )
{
	m_XmlElement.GetContents( szOutStr );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

MBMatchMonitorXmlDocument ::MBMatchMonitorXmlDocument ()
{
}


MBMatchMonitorXmlDocument ::~MBMatchMonitorXmlDocument ()
{
}


bool MBMatchMonitorXmlDocument ::Create()
{
	return m_XmlDocument.Create();
}


bool MBMatchMonitorXmlDocument ::Destroy()
{
	if( NULL == m_XmlDocument.IsInitialized() )
		return true;

	return m_XmlDocument.Destroy();
}


bool MBMatchMonitorXmlDocument ::LoadFromMemory( const char* szBuffer, const LANGID lanid )
{
	return m_XmlDocument.LoadFromMemory( const_cast<char*>(szBuffer), lanid );
}


bool MBMatchMonitorXmlDocument ::LoadFromFile( const char* szFileName )
{
	return m_XmlDocument.LoadFromFile( szFileName );
}


bool MBMatchMonitorXmlDocument ::GetDocumentElement( MMonitorXmlElement*& pOutXmlElement )
{
	if( NULL != pOutXmlElement )
		return false;

	MBMatchMonitorXmlElement* pXmlElement = new MBMatchMonitorXmlElement;
	if( NULL == pXmlElement )
		return false;

	MXmlElement xmlElement = m_XmlDocument.GetDocumentElement();
    
	pXmlElement->SetElement( xmlElement );
	pOutXmlElement = pXmlElement;

	return true;
}*/