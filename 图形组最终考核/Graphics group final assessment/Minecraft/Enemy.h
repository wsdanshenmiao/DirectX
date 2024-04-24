#ifndef __ENEMY__H__
#define __ENEMY__H__

#include "Entity.h"
#include "direct.h"
#include "Player.h"


namespace DSM {

class Enemy :public Entity
{
public:
	Enemy() = default;
	Enemy(const DirectX::XMFLOAT3& position);
	~Enemy() = default;
	DirectX::XMFLOAT3& GetAzimuthTrack();	// 敌人与玩家的相对位置
	DirectX::XMFLOAT3 GetPosition();

	void SetPosition(const DirectX::XMFLOAT3& position);
	void SetModel(ModelManager& modelManager);
	void FindPlayer(DirectX::XMFLOAT3 playerPosition);

private:
	DirectX::XMFLOAT3 m_AzimuthTrack;					// 敌人与玩家的相对位置

};


}

#endif // !__ENEMY__H__
