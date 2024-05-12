#ifndef __CHUNK__H__
#define __CHUNK__H__

#include <fstream>
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
#include "InstrumentorMacro.h"
#include "InstrumentorTimer.h"

namespace DSM {

#define CHUNKHIGHEST 128	// 区块最高高度
#define SEALEVEL 64			// 海平面
#define CHUNKSIZE 16		// 区块长度
#define DIRTTHICKNESS 10	// 泥土层的厚度
#define BLOCKRANDOM 3		// 区块随机值
#define HIGHAMPLITUDE 30	// 低频噪声振幅
#define	NORMALAMPLITUDE 15	// 中频噪声振幅
#define LOWAMPLITUDE 3		// 高频噪声振幅

class Chunk
{
public:
	Chunk() = default;
	Chunk(DirectX::XMINT2 position);
	Chunk(int x, int y);

	void SetPosition(DirectX::XMINT2 position);
	void SetPosition(int x, int y);
	bool OutOfChunk(int x, int y, int z);

	DirectX::XMINT2 GetPositon();
	bool GetState();
	BlockId GetBlock(int x, int y, int z);

	std::vector<BasicEffect::InstancedData>& GetDirtInstancedData();
	std::vector<BasicEffect::InstancedData>& GetStoneInstancedData();
	std::vector<BasicEffect::InstancedData>& GetBedRockInstancedData();
	std::vector<BasicEffect::InstancedData>& GetGressInstancedData();

	std::vector<Transform>& GetDirtTransform();
	std::vector<Transform>& GetStoneTransform();
	std::vector<Transform>& GetBedRockTransform();
	std::vector<Transform>& GetGressTransform();
	std::vector<DirectX::BoundingBox> & GetBlockBox();

	void SetBlock(int x, int y, int z, Block& block, TextureManager& tManager, ModelManager& mManager);

	void LoadChunk();
	bool UnloadChunk(const DirectX::XMINT2& centerChunk);

	void FrustumCulling(std::shared_ptr<FirstPersonCamera> camera);
	void DrawChunk(ID3D11Device* device, ID3D11DeviceContext* deviceContext, BasicEffect& effect, std::shared_ptr<FirstPersonCamera> camera);

	bool InitFromFile();
	void SaveToFile();

	static float GetNoice(int x, int z);

public:
	static bool m_EnableFrustumCulling;				// 视锥体裁剪关闭
	static int m_Seed;								// 世界种子
	static int m_StoreChunkRadius;					// 超过此半径的区块被卸载

	static void InitBlock(TextureManager& tManager, ModelManager& mManager);
	static Block m_Block[5];						// 分别为空气，泥土，石头，基岩, 草方块

private:
	DirectX::XMINT2 m_Positon;				// 区块坐标
	bool m_Loading = false;					// 区块是否在加载

	std::vector<BasicEffect::InstancedData> m_AcceptedData[4];					// 上传到实例缓冲区的数据

	std::vector<BasicEffect::InstancedData> m_BlockInstancedData[4];			// 方块的实例数据
	std::vector<Transform> m_BlockTransforms[4];								// 方块的变换
	std::shared_ptr<Buffer> m_pInstancedBuffer[4];							    // 实例缓冲区

	std::vector<DirectX::BoundingBox> m_BlockBox;										// 记录方块位置
};

}

#endif // !__CHUNK__H__
