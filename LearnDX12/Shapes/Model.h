#pragma once
#ifndef __MODEL__H__
#define __MODEL__H__

#include "Transform.h"
#include "../Common/Mesh.h"

namespace DSM {
	class Model
	{
	public:
		Transform& GetTransform();
		const Transform& GetTransform() const;
		const std::shared_ptr<Geometry::MeshData> GetMesh() const;
		DirectX::BoundingBox GetBouningBox() const;

	private:
		Transform m_Transform;
		std::shared_ptr<Geometry::MeshData> m_MeshData;
		DirectX::BoundingBox m_BoundingBox;
	};
}

#endif // !__MODEL__H__
