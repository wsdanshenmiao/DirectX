#pragma once
#ifndef __MODEL__H__
#define __MODEL__H__

#include "../Common/Mesh.h"
#include "Geometry.h"
#include "Material.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

namespace DSM {

	struct ModelMsh
	{
		Geometry::GeometryMesh m_MeshData;
		DirectX::BoundingBox m_BoundingBox;
		UINT m_MaterialIndex;
	};

	class Model
	{
	public:
		Model(const std::string& filename);

	private:
		bool LoadModelFromFile(const std::string& filename);
		void ProcessNode(aiNode* node, const aiScene* scene);
		ModelMsh ProcessMesh(aiMesh* mesh, const aiScene* scene);

	private:
		std::string m_Name;
		std::vector<ModelMsh> m_Meshs;
		std::vector<Material> m_Materials;
	};

}

#endif // !__MODEL__H__
