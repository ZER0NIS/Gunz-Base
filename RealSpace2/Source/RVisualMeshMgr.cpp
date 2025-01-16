#include "stdafx.h"
#include "RMeshMgr.h"
#include "RVisualMeshMgr.h"
#include "MDebug.h"
#include "RealSpace2.h"

#include "MProfiler.h"
#include "RBspObject.h"

_USING_NAMESPACE_REALSPACE2

_NAMESPACE_REALSPACE2_BEGIN

RVisualMeshMgr::RVisualMeshMgr()
{
	m_id_last = 0;

	m_node_table.reserve(MAX_VMESH_TABLE);

	for (int i = 0; i < MAX_VMESH_TABLE; i++)
		m_node_table[i] = NULL;
}

RVisualMeshMgr::~RVisualMeshMgr()
{
	DelAll();
}

int RVisualMeshMgr::Add(RMesh* pMesh)
{
	if (!pMesh) {
		mlog("VisualMesh Create failure (pMesh==NULL) !!!\n");
		return -1;
	}

   auto node = std::make_unique<RVisualMesh>();

	if (!node->Create(pMesh)) {
		mlog("VisualMesh Create failure !!!\n");
		return -1;
	}

	node->m_id = m_id_last;
    m_node_table.push_back(node.get());
    m_list.push_back(std::move(node));
    return m_id_last++;
}

int RVisualMeshMgr::Add(std::unique_ptr<RVisualMesh> node)
{
	if (!node) {
		mlog("VisualMesh Create failure (pMesh==NULL) !!!\n");
		return -1;
	}

    node->m_id = m_id_last;
    m_node_table.push_back(node.get());
    m_list.push_back(std::move(node));
    return m_id_last++;
}

void RVisualMeshMgr::Del(int id)
{
	if (m_list.empty())
		return;

    for (auto it = m_list.begin(); it != m_list.end();)
    {
        if ((*it)->m_id == id)
        {
            m_node_table.erase(std::remove(m_node_table.begin(), m_node_table.end(), it->get()), m_node_table.end());
            it = m_list.erase(it); // `erase` devuelve el siguiente iterador
        }
        else
        {
            ++it;
        }
    }
}

void RVisualMeshMgr::Del(RVisualMesh* pVMesh)
{
	if (m_list.empty())
		return;
		
    for (auto it = m_list.begin(); it != m_list.end();)
    {
        if (it->get() == pVMesh)
        {
            m_node_table.erase(std::remove(m_node_table.begin(), m_node_table.end(), pVMesh), m_node_table.end());
            it = m_list.erase(it); // `erase` devuelve el siguiente iterador
        }
        else
        {
            ++it;
        }
    }
}

void RVisualMeshMgr::DelAll()
{
	if (m_list.empty())
		return;

	m_node_table.clear();

	m_id_last = 0;
}

void RVisualMeshMgr::Render()
{
	if (m_list.empty())
		return;

	for (auto node = m_list.begin(); node != m_list.end(); ++node)
	{
		(*node)->Render();
	}
}

void RVisualMeshMgr::Render(int id)
{
	if (m_list.empty())
		return;

	for (auto node = m_list.begin(); node != m_list.end();) {
		if ((*node)->m_id == id) {
			(*node)->Render();
			return;
		}
		else ++node;
	}
}

void RVisualMeshMgr::RenderFast(int id)
{
	if (id == -1)
		return;

	m_node_table[id]->Render();
}

void RVisualMeshMgr::Frame()
{
	if (m_list.empty())
		return;

	for (auto node = m_list.begin(); node != m_list.end(); ++node) {
		(*node)->Frame();
	}
}

void RVisualMeshMgr::Frame(int id)
{
	if (m_list.empty())
		return;

	for (auto node = m_list.begin(); node != m_list.end();)
	{
		if ((*node)->m_id == id)
		{
			(*node)->Frame();
			return;
		}
		else ++node;
	}
}

RVisualMesh* RVisualMeshMgr::GetFast(int id)
{
	// Custom: Fix empty node table
    if (id < 0 || id >= static_cast<int>(m_node_table.size()))
        return nullptr;
    return m_node_table[id];
}

_NAMESPACE_REALSPACE2_END