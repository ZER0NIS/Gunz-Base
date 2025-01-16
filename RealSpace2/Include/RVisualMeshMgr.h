#ifndef _RVisualMeshMgr_h
#define _RVisualMeshMgr_h

#include "RVisualMesh.h"

_NAMESPACE_REALSPACE2_BEGIN

using r_vmesh_list = std::list<std::unique_ptr<RVisualMesh>>;
using r_vmesh_node = r_vmesh_list::iterator;

constexpr int MAX_VMESH_TABLE = 1000;

class RVisualMeshMgr {
public:

	RVisualMeshMgr();
	~RVisualMeshMgr();

	int		Add(RMesh* pMesh);
    int Add(std::unique_ptr<RVisualMesh> pMesh);

	void	Del(int id);
	void	Del(RVisualMesh* pMesh);

	void	DelAll();

	void	Render();
	void	Render(int id);

	void	RenderFast(int id);

	void	Frame();
	void	Frame(int id);

	RVisualMesh* GetFast(int id);

	r_vmesh_list m_list;
	int			 m_id_last;

	std::vector<RVisualMesh*> m_node_table;
};

extern bool g_bBirdRenderTest;

_NAMESPACE_REALSPACE2_END

#endif