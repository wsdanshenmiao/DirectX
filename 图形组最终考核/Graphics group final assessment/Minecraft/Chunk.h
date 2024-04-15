#ifndef __CHUNK__H__
#define __CHUNK__H__

#include <vector>
#include <DirectXMath.h>
#include "Block.h"


class Chunk
{
public:
	Chunk() = default;
	Chunk(DirectX::XMFLOAT2 position);

	BlockId GetBlock();

private:


private:
	int m_Highest = 256;			// 区块最高高度
	DirectX::XMFLOAT2 m_Positon;	// 区块坐标
	bool m_Loading = false;			// 区块是否在加载
	Block m_Block;					// 区块中的方块
};

#endif // !__CHUNK__H__
