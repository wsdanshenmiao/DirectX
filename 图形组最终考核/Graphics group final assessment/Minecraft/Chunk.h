#ifndef __CHUNK__H__
#define __CHUNK__H__

#include <dcomp.h>
#include <DirectXMath.h>
#include <vector>
#include <d3d11.h>
#include "Block.h"
#include "TextureManager.h"
#include "ModelManager.h"

#define CHUNKHIGHEST 256	// 区块最高高度
#define CHUNKSIZE 16		// 区块长度
#define SEALEVEL 64			// 海平面



class Chunk
{
public:
	Chunk() = default;
	Chunk(DirectX::XMINT2 position);
	Chunk(int x, int y);

	DirectX::XMINT2 GetPositon();
	void SetPosition(DirectX::XMINT2 position);
	void SetPosition(int x, int y);
	bool OutOfChunk(int x, int y, int z);
	BlockId GetBlock(int x, int y, int z);
	void SetBlock(int x, int y, int z, Block& block, TextureManager& tManager, ModelManager& mManager);
	void LoadChunk(TextureManager& tManager, ModelManager& mManager);
	void DrawChunk(ID3D11DeviceContext* deviceContext, IEffect& effect);


private:


private:
	DirectX::XMINT2 m_Positon;	// 区块坐标
	bool m_Loading = false;			// 区块是否在加载
	std::vector<Block> m_Block;		// 区块中的方块
};

#endif // !__CHUNK__H__
