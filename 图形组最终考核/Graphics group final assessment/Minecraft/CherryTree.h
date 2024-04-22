#ifndef __CHERRYTREE__H__
#define __CHERRYTREE__H__

#include "random"
#include "Block.h"
#include "Effects.h"
#include "Camera.h"
#include "Chunk.h"
#include "BlockModel.h"

namespace DSM {

class CherryTree
{
public:
	CherryTree() = default;
	CherryTree(const CherryTree& other) = default;
	CherryTree(CherryTree&& other) = default;

	void CreateRandomTree(const DirectX::XMINT4& range, ModelManager& mManager, TextureManager& tManager);
	void CreateRandomTree(const int& minx, const int& maxx, const int& minz, const int& maxz, ModelManager& mManager, TextureManager& tManager);
	void FrustumCulling(std::shared_ptr<FirstPersonCamera> camera);
	void DrawTree(ID3D11Device* device, ID3D11DeviceContext* deviceContext, BasicEffect& effect, std::shared_ptr<FirstPersonCamera> camera);

public:
	static bool m_EnableTreeFC;                                      // 树的视锥体裁剪

private:
	DSM::Block m_CherryTreeLeave;						// 樱花树叶
	DSM::Block m_CherryTreeWood;						// 樱花原木

	std::vector<Transform> m_CherryTreeLeaveTransforms;
	std::vector<BasicEffect::InstancedData> m_WoodInstancedData;		// 原木的实例数据
	std::vector<BasicEffect::InstancedData> m_WoodAcceptedData;         // 上传到实例缓冲区的数据
	std::unique_ptr<Buffer> m_pWoodInstancedBuffer;						// 实例缓冲区

	std::vector<Transform> m_CherryTreeWoodTransforms;
	std::vector<BasicEffect::InstancedData> m_LeaveInstancedData;		// 树叶的实例数据
	std::vector<BasicEffect::InstancedData> m_LeaveAcceptedData;        // 上传到实例缓冲区的数据
	std::unique_ptr<Buffer> m_pLeaveInstancedBuffer;						// 实例缓冲区

};


}
#endif // !__CHERRYTREE__H__
