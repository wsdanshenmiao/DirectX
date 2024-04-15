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
	int m_Highest = 256;			// ������߸߶�
	DirectX::XMFLOAT2 m_Positon;	// ��������
	bool m_Loading = false;			// �����Ƿ��ڼ���
	Block m_Block;					// �����еķ���
};

#endif // !__CHUNK__H__
