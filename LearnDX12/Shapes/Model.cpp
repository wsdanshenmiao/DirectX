#include "Model.h"

namespace DSM {
	Transform& Model::GetTransform()
	{
		return m_Transform;
	}

	const Transform& Model::GetTransform() const
	{
		return m_Transform;
	}

	const std::shared_ptr<Geometry::MeshData> Model::GetMesh() const
	{
		return m_MeshData;
	}

	DirectX::BoundingBox Model::GetBouningBox() const
	{
		return m_BoundingBox;
	}

}