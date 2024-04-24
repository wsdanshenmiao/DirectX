#ifndef __ENEMY__H__
#define __ENEMY__H__

#include "Entity.h"
#include "direct.h"
#include "Player.h"
#include "Chunk.h"


namespace DSM {

class Enemy :public Entity
{
public:
	Enemy();
	~Enemy() = default;
	DirectX::XMFLOAT3& GetAzimuthTrack();	// 敌人与玩家的相对位置
	DirectX::XMFLOAT3 GetPosition();

	void SetPosition(const DirectX::XMFLOAT3& position);
	void SetModel(ModelManager& modelManager);
	void FindPlayer(DirectX::XMFLOAT3 playerPosition);
	void DrawEnemy(ID3D11DeviceContext* deviceContext, IEffect& effect);

private:
	DirectX::XMFLOAT3 m_AzimuthTrack;					// 敌人与玩家的相对位置

	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	ComPtr<ID3D11Buffer> mPointSpritesBuffer;				    // 点精灵顶点缓冲区
	ComPtr<ID3D11ShaderResourceView> mTreeTexArray;			    // 血条的纹理数组
};


}

#endif // !__ENEMY__H__
