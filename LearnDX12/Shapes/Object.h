#pragma once
#ifndef __MODEL__H__
#define __MODEL__H__

#include <memory>
#include <string>
#include <unordered_map>
#include <DirectXCollision.h>
#include "Transform.h"
#include "Geometry.h"
#include "../Common/Mesh.h"

namespace DSM {

	class Object
	{
	public:
		Object(const std::string& name) noexcept;
		~Object();
		Transform& GetTransform() noexcept;
		const Transform& GetTransform() const noexcept;
		DirectX::BoundingBox GetBouningBox() const noexcept;
		const std::shared_ptr<Geometry::GeometryMesh> GetGeometryMesh() const noexcept;

		void SetParent(Object* parent) noexcept;
		void SetGeometryMesh(std::shared_ptr<Geometry::GeometryMesh> pMesh) noexcept;

		void AddChild(Object* child) noexcept;


	public:
		std::string m_Name;

	private:
		Transform m_Transform;
		std::shared_ptr<Geometry::GeometryMesh> m_GeometryMesh = nullptr;
		std::unordered_map<std::string, Object*> m_ChildObject;
		Object* m_Parent = nullptr;
		DirectX::BoundingBox m_BoundingBox;
	};
}

#endif // !__MODEL__H__
