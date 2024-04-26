#ifndef __ENEMY__H__
#define __ENEMY__H__

#include "DXTrace.h"
#include "Entity.h"
#include "direct.h"
#include "Player.h"
#include "Chunk.h"
#include "WICTextureLoader11.h"
#include "Vertex.h"
#include "d3dUtil.h"


namespace DSM {

class Enemy :public Entity
{
public:
	Enemy() = default;
	Enemy(const Enemy& other) = default;
	Enemy(Enemy&& other) = default;
	Enemy& operator=(const Enemy& other) = default;
	~Enemy() = default;

	DirectX::XMFLOAT3& GetAzimuthTrack();	// 敌人与玩家的相对位置
	DirectX::XMFLOAT3 GetPosition();
	int& GetHP();

	void SetPosition(const DirectX::XMFLOAT3& position);
	void SetPosition(float x, float y, float z);
	void SetModel(TextureManager& tManager, ModelManager& modelManager);
	void LoadEnemy(TextureManager& tManager, ModelManager& mManager);
	void FindPlayer(DirectX::XMFLOAT3 playerPosition);
	void DrawEnemy(ID3D11DeviceContext* deviceContext, BasicEffect& effect);

private:
	DirectX::XMFLOAT3 m_AzimuthTrack;					// 敌人与玩家的相对位置
	GameObject m_Lifebar;								// 血条
	int m_HP = 3;										// 血量
};


}

#endif // !__ENEMY__H__
