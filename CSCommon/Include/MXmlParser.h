#pragma once

class MXmlElement;
class MZFileSystem;

class MXmlParser
{
protected:
	virtual void ParseRoot(const char* szTagName, MXmlElement* pElement) = 0;
public:
	MXmlParser() {}
	virtual ~MXmlParser() {}
	bool ReadXml(const char* szFileName);							
	bool ReadXml(MZFileSystem* pFileSystem, const char* szFileName);	
};


