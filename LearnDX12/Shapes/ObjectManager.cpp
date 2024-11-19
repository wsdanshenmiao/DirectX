#include "ObjectManager.h"
#include "../Common/Vertex.h"

using namespace DSM::Geometry;

namespace DSM {

	bool ObjectManager::IsChange() const noexcept
	{
		return m_IsChange;
	}

	void ObjectManager::AddObject(std::shared_ptr<Object> obj)
	{
		m_Objects.push_back(obj);
	}

	std::size_t ObjectManager::GetObjectSize() const noexcept
	{
		return m_Objects.size();
	}

	std::shared_ptr<Object> ObjectManager::GetObjectByName(const std::string& name)
	{
		return *std::find_if(m_Objects.begin(), m_Objects.end(), [&name](const auto& obj) {
			return obj->m_Name == name;
			});
	}

	std::shared_ptr<Object> ObjectManager::GetObjectByIndex(const std::size_t& index)
	{
		return m_Objects[index];
	}

	Geometry::GeometryMesh ObjectManager::GetAllObjectMesh() const
	{
		GeometryMesh mesh{};

		UINT baseIndex = 0;
		for (const auto& obj : m_Objects)
		{
			auto& objMesh = obj->GetGeometryMesh();
			if (!objMesh)continue;
			auto& objMeshV = objMesh->m_Vertices;
			auto& objMeshI = objMesh->m_Indices32;
			std::for_each(objMeshI.begin(), objMeshI.end(), [baseIndex](auto& index) {
				index += baseIndex;
				});
			mesh.m_Vertices.insert(mesh.m_Vertices.end(), objMeshV.begin(), objMeshV.end());
			mesh.m_Indices32.insert(mesh.m_Indices32.end(), objMeshI.begin(), objMeshI.end());
			//baseIndex += (UINT)mesh.m_Indices32.size();
		}

		return mesh;
	}


}
