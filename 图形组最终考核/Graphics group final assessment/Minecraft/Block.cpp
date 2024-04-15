#include "Block.h"

using namespace DirectX;

Block::Block(const std::vector<GameObject>& blockData, const BlockId& id)
	:m_BlockData(blockData), m_Id(id) {}

std::vector<GameObject>& Block::GetBlock()
{
	return m_BlockData;
}

BlockId& Block::GetId()
{
	return m_Id;
}

BlockModel& Block::GetBlockModel()
{
	return m_BlockModel;
}

void Block::SetId(const BlockId& id)
{
	m_Id = id;
}
