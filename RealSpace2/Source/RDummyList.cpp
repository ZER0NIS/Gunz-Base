#include "stdafx.h"
#include "MXml.h"
#include "RDummyList.h"

_NAMESPACE_REALSPACE2_BEGIN

RDummyList::~RDummyList()
{
	Clear();
}

void RDummyList::Clear()
{
	for (iterator itor = begin(); itor != end(); ++itor)
		delete* itor;

	clear();
}

bool RDummyList::Open(MXmlElement* pElement)
{
	MXmlElement	aDummyNode, aChild;
	int nCount = pElement->GetChildNodeCount();

	char szTagName[256], szContents[256];
	for (int i = 0; i < nCount; i++)
	{
		aDummyNode = pElement->GetChildNode(i);
		aDummyNode.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;

		if (stricmp(szTagName, RTOK_DUMMY) == 0)
		{
			RDummy* pNewDummy = new RDummy;
			aDummyNode.GetAttribute(szContents, RTOK_NAME);
			pNewDummy->szName = szContents;

			int nChildCount = aDummyNode.GetChildNodeCount();
			for (int j = 0; j < nChildCount; j++)
			{
				aChild = aDummyNode.GetChildNode(j);
				aChild.GetTagName(szTagName);
				aChild.GetContents(szContents);

				auto ReadVector = [&]() {
					rvector vec;
					if (sscanf(szContents, "%f %f %f", &vec.x, &vec.y, &vec.z) != 3)
						vec = { 0, 0, 0 };
					return vec;
				};

				if (_stricmp(szTagName, RTOK_POSITION) == 0)
					pNewDummy->Position = ReadVector();
				else if (_stricmp(szTagName, RTOK_DIRECTION) == 0)
					pNewDummy->Direction = ReadVector();
			}

			push_back(pNewDummy);
		}
	}

	return true;
}

bool RDummyList::Save(MXmlElement* pElement)
{
	MXmlElement	aDummyListElement = pElement->CreateChildElement(RTOK_DUMMYLIST);

	for (RDummyList::iterator itor = begin(); itor != end(); ++itor)
	{
		aDummyListElement.AppendText("\n\t\t");

		RDummy* pDummy = *itor;
		char buffer[256];

		MXmlElement		aElement, aChild;
		aElement = aDummyListElement.CreateChildElement(RTOK_DUMMY);

		aElement.AddAttribute(RTOK_NAME, pDummy->szName.c_str());

		aElement.AppendText("\n\t\t\t");
		aChild = aElement.CreateChildElement(RTOK_POSITION);
		aChild.SetContents(Format(buffer, pDummy->Position));

		aElement.AppendText("\n\t\t\t");
		aChild = aElement.CreateChildElement(RTOK_DIRECTION);
		aChild.SetContents(Format(buffer, pDummy->Direction));

		aElement.AppendText("\n\t\t");
	}
	aDummyListElement.AppendText("\n\t");

	return true;
}

_NAMESPACE_REALSPACE2_END