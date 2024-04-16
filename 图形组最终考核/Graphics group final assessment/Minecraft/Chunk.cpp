#include "Chunk.h"

using namespace DirectX;

Chunk::Chunk(DirectX::XMINT2 position, ID3D11Device* device)
	:m_Positon(position) {}

Chunk::Chunk(int x, int y, ID3D11Device* device)
	:m_Positon(DirectX::XMINT2(x, y)) {}

void Chunk::InitChunk(ID3D11Device* device)
{
	m_AcceptedData.reserve(16384);
	m_AcceptedIndices.reserve(16384);
	m_pInstancedBuffer = std::make_unique<Buffer>(device,
		CD3D11_BUFFER_DESC(sizeof(BasicEffect::InstancedData) * 16384, D3D11_BIND_VERTEX_BUFFER,
			D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE));
	m_pInstancedBuffer->SetDebugObjectName("InstancedBuffer");
}

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
	if (OutOfChunk(x, y, z)) {
		return BlockId::Air;
	}
	//创建随机数引擎
	std::random_device rd;
	std::mt19937 gen(rd());
	//指定随机数的类型和范围
	std::uniform_int_distribution<size_t> dis(0, 2);
	// 生成柏林噪声
	FastNoiseLite noice;
	noice.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	noice.SetFrequency(0.05);
	if (y < 1 + dis(gen)) {
		return BlockId::BedRock;
	}
	else if (y < SEALEVEL - 10 + 10 * noice.GetNoise((float)x, (float)z) + dis(gen)) {
		return BlockId::Stone;
	}
	else if (y < SEALEVEL + 10 * noice.GetNoise((float)x, (float)z)) {
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

#if 0
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

#else

void Chunk::LoadChunk(TextureManager& tManager, ModelManager& mManager)
{
	m_Loading = true;
	// 初始化方块
	Model* pModel = m_Block.GetBlockModel().GetStoneModel(tManager, mManager);
	m_Block.GetBlock().SetModel(pModel);
	m_Block.SetId(BlockId::Stone);
	pModel->SetDebugObjectName("Block");

	// 生成16384个方块
	m_BlockInstancedData.resize(16384);
	m_BlockTransforms.resize(16384);

	Transform transform;
	size_t pos = 0;

	for (int y = 0; y < CHUNKHIGHEST; y++) {	//每一层 16 * 16
		for (int z = 0; z < CHUNKSIZE; z++) {
			for (int x = 0; x < CHUNKSIZE; x++) {
				int mx = m_Positon.x;
				int mz = m_Positon.y;
				transform.SetPosition(mx + x, y, mz + z);
				m_BlockTransforms[pos] = transform;
				XMMATRIX W = transform.GetLocalToWorldMatrixXM();
				XMMATRIX WInvT = XMath::InverseTranspose(W);
				XMStoreFloat4x4(&m_BlockInstancedData[pos].world, XMMatrixTranspose(W));
				XMStoreFloat4x4(&m_BlockInstancedData[pos].worldInvTranspose, XMMatrixTranspose(WInvT));
				m_BlockInstancedData[pos++].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			}
		}
	}
}

void Chunk::DrawChunk(ID3D11DeviceContext* deviceContext, BasicEffect& effect)
{
	// 硬件实例化绘制
	const auto& refData =  m_BlockInstancedData;
	// 上传实例数据
	memcpy_s(m_pInstancedBuffer->MapDiscard(deviceContext),
		m_pInstancedBuffer->GetByteWidth(), refData.data(), refData.size() * sizeof(BasicEffect::InstancedData));
	m_pInstancedBuffer->Unmap(deviceContext);
	effect.DrawInstanced(deviceContext, *m_pInstancedBuffer, m_Block.GetBlock(), (uint32_t)refData.size());
}


#endif
