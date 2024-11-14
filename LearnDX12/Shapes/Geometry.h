#pragma once
#ifndef __GEOMETRY__H__
#define __GEOMETRY__H__

#include "../Common/D3DUtil.h"
#include "../Common/Singleton.h"

namespace DSM {
	namespace Geometry {

		struct Vertex
		{
			DirectX::XMFLOAT3 m_Position;
			DirectX::XMFLOAT3 m_Normal;
			DirectX::XMFLOAT4 m_Tangent;
			DirectX::XMFLOAT2 m_TexCoord;
		};

		struct GeometryMesh
		{
			std::vector<Vertex> m_Vertexs;
			std::vector<std::uint32_t> m_Indices32;

			std::vector<std::uint16_t>& GetIndices16() {
				if (m_Indices16.empty()) {
					m_Indices16.reserve(m_Indices32.size());
					for (std::size_t i = 0; i < m_Indices32.size(); ++i) {
						m_Indices16[i] = static_cast<std::uint16_t>(m_Indices32[i]);
					}
				}
				return m_Indices16;
			}

		private:
			std::vector<std::uint16_t> m_Indices16;
		};

		class GeometryGenerator
		{
		public:
			static GeometryMesh CreateBox(
				float width,
				float height,
				float depth,
				std::uint32_t subdivision) noexcept;
		};

	}


}

#endif