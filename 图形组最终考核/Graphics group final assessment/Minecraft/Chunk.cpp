#include "Chunk.h"

Chunk::Chunk(DirectX::XMINT2 position)
	:m_Positon(position) {}

Chunk::Chunk(int x, int y)
	:m_Positon(DirectX::XMINT2(x, y)) {}


DirectX::XMINT2 Chunk::GetPositon()
{
	return m_Positon;
}

void Chunk::SetPosition(DirectX::XMINT2 position)
{
	m_Positon = position;
}

void Chunk::SetPosition(int x, int y)
{
	m_Positon = DirectX::XMINT2(x, y);
}

// 判断是否在区块中
bool Chunk::OutOfChunk(int x, int y, int z)
{
	if (y<0 || y>CHUNKHIGHEST) {
		return true;
	}
	if (x - GetPositon().x < 0) {
		return true;
	}
	if (z - GetPositon().y < 0) {
		return true;
	}
	return false;
}

// 获取方块种类
BlockId Chunk::GetBlock(int x, int y, int z)
{
	//创建随机数引擎
	std::random_device rd;
	std::mt19937 gen(rd());
	//指定随机数的类型和范围
	std::uniform_int_distribution<size_t> dis(0, 3);
	FastNoiseLite noice;
	noice.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	noice.SetFrequency(1);
	if (OutOfChunk(x, y, z)) {
		return BlockId::Air;
	}
	if (y < 1 + dis(gen)) {
		return BlockId::BedRock;
	}
	else if (y < 8 + dis(gen)) {
		return BlockId::Stone;
	}
	else if (y < 12 + 10 * noice.GetNoise((float)x, (float)z)) {
		return BlockId::Dirt;
	}
	else {
		return BlockId::Air;
	}
}

// 设置方块种类
void Chunk::SetBlock(int x, int y, int z, Block& block, TextureManager& tManager, ModelManager& mManager)
{
	BlockModel bModel = block.GetBlockModel();
	if (OutOfChunk(x, y, z)) {
		return;
	}
	block.SetId(GetBlock(x, y, z));	//获取Id
	switch (block.GetId())
	{
	case BlockId::Air:
		block.GetBlock().SetModel(nullptr); break;
	case BlockId::Dirt:
		block.GetBlock().SetModel(bModel.GetDirtModel(tManager, mManager)); break;
	case BlockId::Stone:
		block.GetBlock().SetModel(bModel.GetStoneModel(tManager, mManager)); break;
	case BlockId::BedRock:
		block.GetBlock().SetModel(bModel.GetBedRockModel(tManager, mManager)); break;
	default:
		break;
	}
}

void Chunk::LoadChunk(TextureManager& tManager, ModelManager& mManager)
{
	m_Loading = true;
	m_Block.resize(CHUNKSIZE * CHUNKSIZE * CHUNKHIGHEST);
	for (int y = 0; y < CHUNKHIGHEST; y++) {	//每一层
		for (int z = 0; z < CHUNKSIZE; z++) {
			for (int x = 0; x < CHUNKSIZE; x++) {
				int mx = m_Positon.x;
				int mz = m_Positon.y;
				int ly = y * CHUNKSIZE * CHUNKSIZE;
				int lz = z * CHUNKSIZE;
				SetBlock(mx + x, y, mz + z, m_Block[ly + lz + x] , tManager, mManager);
				m_Block[ly + lz + x].GetBlock().GetTransform().SetPosition(mx + x, y, mz + z);
			}
		}
	}
}

void Chunk::DrawChunk(ID3D11DeviceContext* deviceContext, IEffect& effect)
{
	for (auto block : m_Block) {
		if (block.GetBlock().GetModel()) {
			block.GetBlock().Draw(deviceContext, effect);
		}
	}
}
