#include "Entity.h"

Entity::Entity(GameObject entity, DirectX::XMFLOAT3 speed)
	:m_Entity(entity), m_Speed(speed) {}

GameObject& Entity::GetEntity()
{
	return m_Entity;
}

DirectX::XMFLOAT3 Entity::GetSpeed()
{
	return m_Speed;
}

void Entity::SetSpeed(DirectX::XMFLOAT3 speed)
{
	m_Speed = speed;
}
