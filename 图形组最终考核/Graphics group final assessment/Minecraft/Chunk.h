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

#define CHUNKHIGHEST 64	// 区块最高高度
#define SEALEVEL 32			// 海平面
#define CHUNKSIZE 16		// 区块长度



class Chunk
{
public:
	Chunk() = default;
	Chunk(DirectX::XMINT2 position, ID3D11Device* device);
	Chunk(int x, int y, ID3D11Device* device);

	void InitChunk(ID3D11Device* device);
	DirectX::XMINT2 GetPositon();
	void SetPosition(DirectX::XMINT2 position);
	void SetPosition(int x, int y);
	bool OutOfChunk(int x, int y, int z);
	BlockId GetBlock(int x, int y, int z);
	void SetBlock(int x, int y, int z, Block& block, TextureManager& tManager, ModelManager& mManager);
	void LoadChunk(TextureManager& tManager, ModelManager& mManager);
	
	//void DrawChunk(ID3D11DeviceContext* deviceContext, IEffect& effect);
	void DrawChunk(ID3D11DeviceContext* deviceContext, BasicEffect& effect, std::shared_ptr<FirstPersonCamera> camera);


private:


private:
	DirectX::XMINT2 m_Positon;		// 区块坐标
	bool m_Loading = false;			// 区块是否在加载
	//std::vector<Block> m_Block;		// 区块中的方块
	Block m_Block;

	std::vector<Transform> m_BlockTransforms;							// 方块的变换
	std::vector<BasicEffect::InstancedData> m_BlockInstancedData;		// 方块的实例数据
	std::vector<size_t> m_AcceptedIndices;                              // 通过视锥体裁剪的实例索引
	std::vector<BasicEffect::InstancedData> m_AcceptedData;             // 上传到实例缓冲区的数据
	std::unique_ptr<Buffer> m_pInstancedBuffer;                         // 实例缓冲区

};

#endif // !__CHUNK__H__
