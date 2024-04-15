#ifndef __ENTITY__H__
#define __ENTITY__H__

#include "GameObject.h"

class Entity
{
public:
	Entity() = default;
	Entity(GameObject entity, DirectX::XMFLOAT3 speed);

	GameObject& GetEntity();
	DirectX::XMFLOAT3 GetSpeed();

	void SetSpeed(DirectX::XMFLOAT3 speed);

private:
	GameObject m_Entity;
	DirectX::XMFLOAT3 m_Speed;
};

#endif // !__Entity__H__
