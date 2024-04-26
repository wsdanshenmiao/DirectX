#ifndef __CHUNK__H__
#define __CHUNK__H__

#include <DirectXMath.h>
#include <vector>
#include <random>
#include <memory>
#include "Effects.h"
#include "FastNoiseLite.h"
#include "Block.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Camera.h"

namespace DSM {

#define CHUNKHIGHEST 64		// 区块最高高度
#define SEALEVEL 32			// 海平面
#define CHUNKSIZE 16			// 区块长度
#define CHUNKRANGE 15		// 柏林噪声应用的幅度
#define DIRTTHICKNESS 10	// 泥土层的厚度
#define BLOCKRANDOM 3		// 区块随机值


class Chunk
{
public:
	Chunk() = default;
	Chunk(DirectX::XMINT2 position, ID3D11Device* device);
	Chunk(int x, int y, ID3D11Device* device);

	DirectX::XMINT2 GetPositon();
	void SetPosition(DirectX::XMINT2 position);
	void SetPosition(int x, int y);
	bool OutOfChunk(int x, int y, int z);

	std::vector<BasicEffect::InstancedData>& GetDirtInstancedData();
	std::vector<BasicEffect::InstancedData>& GetStoneInstancedData();
	std::vector<BasicEffect::InstancedData>& GetBedRockInstancedData();
	std::vector<BasicEffect::InstancedData>& GetGressInstancedData();
	std::vector<Transform>& GetDirtTranform();
	std::vector<Transform>& GetStoneTranform();
	std::vector<Transform>& GetBedRockTranform();
	std::vector<Transform>& GetGressTranform();
	std::vector<DSM::BlockId>& GetBlockId();
	static float GetNoice(int x, int z);
	BlockId GetBlock(int x, int y, int z);

	void SetBlock(int x, int y, int z, Block& block, TextureManager& tManager, ModelManager& mManager);
	void LoadChunk(TextureManager& tManager, ModelManager& mManager);
	void FrustumCulling(std::shared_ptr<FirstPersonCamera> camera);
	
	//void DrawChunk(ID3D11DeviceContext* deviceContext, IEffect& effect);
	void DrawChunk(ID3D11Device* device, ID3D11DeviceContext* deviceContext, BasicEffect& effect, std::shared_ptr<FirstPersonCamera> camera);

public:
	static bool m_EnableFrustumCulling;				// 视锥体裁剪关闭
	static int m_Seed;

private:
	DirectX::XMINT2 m_Positon;				// 区块坐标
	bool m_Loading = false;					// 区块是否在加载
	Block m_Block[5];						// 分别为空气，泥土，石头，基岩, 草方块
	//std::vector<Block> m_Block;			// 区块中的方块

	std::vector<BasicEffect::InstancedData> m_AcceptedData[4];					// 上传到实例缓冲区的数据
	std::vector<BasicEffect::InstancedData> m_BlockInstancedData[4];			// 方块的实例数据
	std::vector<Transform> m_BlockTransforms[4];								// 方块的变换
	std::unique_ptr<Buffer> m_pInstancedBuffer[4];							    // 实例缓冲区

	std::vector<DSM::BlockId> m_BlockId;										// 记录方块位置
};

}

#endif // !__CHUNK__H__
