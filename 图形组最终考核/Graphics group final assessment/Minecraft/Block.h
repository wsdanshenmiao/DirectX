#ifndef __BLOCK__H__
#define __BLOCK__H__

#include <vector>
#include "GameObject.h"
#include "BlockModel.h"

namespace DSM {

// 物品Id枚举
enum class BlockId
{
	Air = 0,
	Gress = 1,
	Dirt = 2,
	Stone = 3,
	BedRock = 4,
	CherryBlossomLog = 5,
	CherryBlossomLeaves = 6,
};

// 方块类
class Block
{
public:
	Block() = default;
	Block(const GameObject& blockData, const BlockId& id);

	GameObject& GetBlock();
	BlockModel& GetBlockModel();
	BlockId& GetId();
	void SetId(const BlockId& id);

private:
	BlockId m_Id;								// 方块Id
	GameObject m_BlockData;						// 方块数据
	BlockModel m_BlockModel;					// 获取方块模型与材质
};

}

#endif // !__BLOCK__H__