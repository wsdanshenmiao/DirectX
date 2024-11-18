#pragma once
#ifndef __VERTEX__H__
#define __VERTEX__H__

#include "Pubh.h"

namespace DSM {

	struct VertexPosColor
	{
		static decltype(auto) GetInputLayOut()
		{
			const std::array<D3D12_INPUT_ELEMENT_DESC, 2> inputLayout = {
				D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				D3D12_INPUT_ELEMENT_DESC{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};
			return inputLayout;
		};

		DirectX::XMFLOAT3 m_Pos;
		DirectX::XMFLOAT4 m_Color;
	};
}

#endif // !__VERTEX__H__
