/*#ifndef _MBMATCHMONITORXML
#define _MBMATCHMONITORXML


#include "MMonitorXmlInterface.h"
#include "MXml.h"



class MBMatchMonitorXmlElement : public MMonitorXmlElement
{
private :
	MXmlElement m_XmlElement;

public :
	MBMatchMonitorXmlElement();
	~MBMatchMonitorXmlElement();

	void			SetElement( const MXmlElement& XmlElement );

	virtual bool	GetChildNode( const DWORD dwIndex, MMonitorXmlElement*& pOutXmlElement );

	virtual void	GetAttribute( const DWORD dwIndex, char* szoutAttrName, char* szoutAttrValue );

	virtual DWORD	GetChildNodeCount();
	virtual DWORD	GetAttributeCount();

	virtual void	GetTagName( char* sOutTagName );

	virtual void	GetContents( char* szOutStr );
};


class MBMatchMonitorXmlDocument : public MMonitorXmlDocument
{
private :
	MXmlDocument m_XmlDocument;

public :
	MBMatchMonitorXmlDocument();
	~MBMatchMonitorXmlDocument();

	virtual bool	Create();
	virtual bool	Destroy();

	virtual bool	LoadFromMemory( const char* szBuffer, const LANGID lanid = LANG_KOREAN );
	virtual bool	LoadFromFile( const char* szFileName );

	virtual bool	GetDocumentElement( MMonitorXmlElement*& pOutXmlElement );
};



#endif*/