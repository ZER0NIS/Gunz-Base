#include "stdafx.h"
#include "RAStar.h"
#include <crtdbg.h>

RAStarNode::RAStarNode()
{
	m_fCostFromStart = 0.0f;
	m_fCostToGoal = 0.0f;
	m_pParent = NULL;
	m_nSessionID = 0;
	m_pData = NULL;

	m_fWeight = 1.0f;
}

RAStarNode::~RAStarNode()
{
}

RAStar::RAStar() : m_nPathSession(0)
{
}

RAStar::~RAStar()
{
}

void RAStar::PushOnOpenList(RAStarNode* pStartNode)
{
	m_OpenList.push_back(pStartNode);
}

bool RAStar::IsOpenListEmpty()
{
	return m_OpenList.empty();
}

RAStarNode* RAStar::PopLowestCostFromOpenList()
{
	RAStarNode* pOutNode = NULL;
	list<RAStarNode*>::iterator itorErase = m_OpenList.end();

	for (list<RAStarNode*>::iterator itor = m_OpenList.begin(); itor != m_OpenList.end(); ++itor)
	{
		RAStarNode* pNode = (*itor);
		if (pOutNode == NULL)
		{
			pOutNode = pNode;
			itorErase = itor;
		}
		else
		{
			if (pOutNode->GetTotalCost() > pNode->GetTotalCost())
			{
				pOutNode = pNode;
				itorErase = itor;
			}
		}
	}

	m_OpenList.erase(itorErase);
	return pOutNode;
}

#include "RNavigationNode.h"

void RAStar::PushToShortestPath(RAStarNode* pNode)
{
	RNavigationNode* pNNode = (RNavigationNode*)pNode;
	m_ShortestPath.push_back(pNode);
}

bool RAStar::Search(RAStarNode* pStartNode, RAStarNode* pGoalNode)
{
	m_OpenList.clear();
	m_nPathSession++;

	pStartNode->m_fCostFromStart = 0.0f;
	pStartNode->m_fCostToGoal = pStartNode->GetHeuristicCost(pGoalNode);
	pStartNode->m_pParent = NULL;

	PushOnOpenList(pStartNode);

	while (!IsOpenListEmpty())
	{
		RAStarNode* pNode = PopLowestCostFromOpenList();

		if (pNode == pGoalNode)
		{
			m_ShortestPath.clear();
			RAStarNode* pShortestNode = pNode;
			while (pShortestNode != pStartNode)
			{
				PushToShortestPath(pShortestNode);
				pShortestNode = pShortestNode->m_pParent;
			}

			return true;
		}

		for (int i = 0; i < pNode->GetSuccessorCount(); i++)
		{
			RAStarNode* pSuccessor = pNode->GetSuccessor(i);
			if (pSuccessor == NULL) continue;
			if (pSuccessor == pStartNode) continue;
			if (pNode->m_pParent == pSuccessor) continue;

			float fNewCostFromStart = pNode->GetSuccessorCostFromStart(pSuccessor);

			if (pSuccessor->GetSessionID() == m_nPathSession)
			{
				if (pSuccessor->m_fCostFromStart <= fNewCostFromStart) continue;
			}

			pSuccessor->m_pParent = pNode;
			pSuccessor->m_fCostFromStart = fNewCostFromStart;
			pSuccessor->m_fCostToGoal = pSuccessor->GetHeuristicCost(pGoalNode);
			pSuccessor->OnSetData(i, pNode);

			if (pSuccessor->GetSessionID() != m_nPathSession)
			{
				pSuccessor->m_nSessionID = m_nPathSession;
				PushOnOpenList(pSuccessor);
			}
		}
	}

	return false;
}