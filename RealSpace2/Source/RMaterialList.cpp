#include "stdafx.h"
#include "MXml.h"
#include "MDebug.h"
#include "RMaterialList.h"

_NAMESPACE_REALSPACE2_BEGIN

RMaterialList::~RMaterialList()
{
	for (iterator i = begin(); i != end(); i++)
		delete* i;
}

bool RMaterialList::Open(MXmlElement* pElement)
{
	MXmlElement	aMaterialNode, aChild;
	int nCount = pElement->GetChildNodeCount();

	char szTagName[256], szContents[256];
	for (int i = 0; i < nCount; i++)
	{
		aMaterialNode = pElement->GetChildNode(i);
		aMaterialNode.GetTagName(szTagName);

		if (stricmp(szTagName, RTOK_MATERIAL) == 0)
		{
			RMATERIAL* pMaterial = new RMATERIAL;
			pMaterial->dwFlags = 0;
			aMaterialNode.GetAttribute(szContents, RTOK_NAME);
			pMaterial->Name = szContents;

			int nChildCount = aMaterialNode.GetChildNodeCount();
			for (int j = 0; j < nChildCount; j++)
			{
				aChild = aMaterialNode.GetChildNode(j);
				aChild.GetTagName(szTagName);
				aChild.GetContents(szContents);

#define READVECTOR(v) sscanf(szContents,"%f %f %f",&v.x,&v.y,&v.z)

				if (stricmp(szTagName, RTOK_AMBIENT) == 0)		READVECTOR(pMaterial->Ambient);
				else if (stricmp(szTagName, RTOK_DIFFUSE) == 0)		READVECTOR(pMaterial->Diffuse);
				else if (stricmp(szTagName, RTOK_SPECULAR) == 0)		READVECTOR(pMaterial->Specular);
				else if (stricmp(szTagName, RTOK_DIFFUSEMAP) == 0)	pMaterial->DiffuseMap = szContents;
				else if (stricmp(szTagName, RTOK_POWER) == 0)		sscanf(szContents, "%f", &pMaterial->Power);
				else if (stricmp(szTagName, RTOK_ADDITIVE) == 0)		pMaterial->dwFlags |= RM_FLAG_ADDITIVE;
				else if (stricmp(szTagName, RTOK_USEOPACITY) == 0)	pMaterial->dwFlags |= RM_FLAG_USEOPACITY;
				else if (stricmp(szTagName, RTOK_TWOSIDED) == 0)		pMaterial->dwFlags |= RM_FLAG_TWOSIDED;
				else if (stricmp(szTagName, RTOK_USEALPHATEST) == 0)	pMaterial->dwFlags |= RM_FLAG_USEALPHATEST;
			}

			push_back(pMaterial);
		}
	}
	return true;
}

bool RMaterialList::Save(MXmlElement* pElement)
{
	MXmlElement	aMaterialListElement = pElement->CreateChildElement(RTOK_MATERIALLIST);

	{
		for (iterator i = begin(); i != end(); i++)
		{
			aMaterialListElement.AppendText("\n\t\t");

			RMATERIAL* pMaterial = *i;

			char buffer[256];

			MXmlElement		aElement, aChild;
			aElement = aMaterialListElement.CreateChildElement(RTOK_MATERIAL);
			aElement.AddAttribute(RTOK_NAME, pMaterial->Name.c_str());

			aElement.AppendText("\n\t\t\t");
			aChild = aElement.CreateChildElement(RTOK_DIFFUSE);
			aChild.SetContents(Format(buffer, pMaterial->Diffuse));

			aElement.AppendText("\n\t\t\t");
			aChild = aElement.CreateChildElement(RTOK_AMBIENT);
			aChild.SetContents(Format(buffer, pMaterial->Ambient));

			aElement.AppendText("\n\t\t\t");
			aChild = aElement.CreateChildElement(RTOK_SPECULAR);
			aChild.SetContents(Format(buffer, pMaterial->Specular));

			aElement.AppendText("\n\t\t\t");
			aChild = aElement.CreateChildElement(RTOK_DIFFUSEMAP);
			aChild.SetContents(pMaterial->DiffuseMap.c_str());

			{
				MXmlElement aFlagElement;

				if ((pMaterial->dwFlags & RM_FLAG_ADDITIVE) != 0)
				{
					aElement.AppendText("\n\t\t\t");
					aElement.CreateChildElement(RTOK_ADDITIVE);
				}
				if ((pMaterial->dwFlags & RM_FLAG_TWOSIDED) != 0)
				{
					aElement.AppendText("\n\t\t\t");
					aElement.CreateChildElement(RTOK_TWOSIDED);
				}
				if ((pMaterial->dwFlags & RM_FLAG_USEOPACITY) != 0)
				{
					aElement.AppendText("\n\t\t\t");
					aElement.CreateChildElement(RTOK_USEOPACITY);
				}
				if ((pMaterial->dwFlags & RM_FLAG_USEALPHATEST) != 0)
				{
					aElement.AppendText("\n\t\t\t");
					aElement.CreateChildElement(RTOK_USEALPHATEST);
				}
			}
			aElement.AppendText("\n\t\t");
		}
		aMaterialListElement.AppendText("\n\t");
	}
	return true;
}

_NAMESPACE_REALSPACE2_END