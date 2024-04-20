#include "Block.h"

using namespace DirectX;

namespace DSM {

Block::Block(const GameObject& blockData, const BlockId& id)
	:m_BlockData(blockData), m_Id(id) {}

GameObject& Block::GetBlock()
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

}