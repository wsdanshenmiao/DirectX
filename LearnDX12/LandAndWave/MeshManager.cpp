#include "MeshManager.h"
#include "../Common/Vertex.h"

using namespace DSM::Geometry;
using namespace DirectX;

namespace DSM {

	void MeshManager::AddMesh(
		const std::string& name,
		GeometryMesh&& mesh,
		DirectX::BoundingBox box)
	{
		m_GeometryMesh.insert(std::make_pair(name, std::make_pair(mesh, box)));
	}

	std::size_t MeshManager::GetMeshSize() const noexcept
	{
		return m_GeometryMesh.size();
	}

	std::pair<GeometryMesh, BoundingBox> MeshManager::GetMesh(const std::string& name)
	{
		return m_GeometryMesh[name];
	}

}
