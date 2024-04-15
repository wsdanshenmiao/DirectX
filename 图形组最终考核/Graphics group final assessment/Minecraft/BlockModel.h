#ifndef __BLOCKTEXTURE__H__
#define __BLOCKTEXTURE__H__

#include "ModelManager.h"
#include "TextureManager.h"
#include <DirectXMath.h>
#include <d3d11.h>

// ����ģ����
class BlockModel
{
public:
	Model* GetBedRockModel(TextureManager& textureManager, ModelManager& modelManager);	// ��ȡ���Ҳ���
	Model* GetDirtModel(TextureManager& textureManager, ModelManager& modelManager);	// ��ȡ��������
	Model* GetStoneModel(TextureManager& textureManager, ModelManager& modelManager);	// ��ȡʯͷ����

};

#endif // !__BLOCKTEXTURE__H__

