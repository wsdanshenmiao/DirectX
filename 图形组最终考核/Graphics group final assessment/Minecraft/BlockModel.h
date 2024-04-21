#ifndef __BLOCKTEXTURE__H__
#define __BLOCKTEXTURE__H__

#include "ModelManager.h"
#include "TextureManager.h"
#include <DirectXMath.h>
#include <d3d11.h>

namespace DSM {

// 方块模型类
class BlockModel
{
public:
	Model* GetBedRockModel(TextureManager& textureManager, ModelManager& modelManager);	// 获取基岩材质
	Model* GetDirtModel(TextureManager& textureManager, ModelManager& modelManager);	// 获取泥土材质
	Model* GetStoneModel(TextureManager& textureManager, ModelManager& modelManager);	// 获取石头材质
	Model* GetGressModel(TextureManager& textureManager, ModelManager& modelManager);	// 获取草方块材质
	Model* GetCherryBlossomLogModel(TextureManager& tManager, ModelManager& mManager);	// 获取樱花木材质
	Model* GetCherryBlossomLeavesModel(TextureManager& tManager, ModelManager& mManager);// 获取樱花树叶材质
};

}

#endif // !__BLOCKTEXTURE__H__

