#include "Entity.h"

Entity::Entity(GameObject entity, float speed)
	:m_Entity(entity), m_Speed(speed) {}

GameObject& Entity::GetEntity()
{
	return m_Entity;
}

float& Entity::GetSpeed()
{
	return m_Speed;
}

void Entity::SetSpeed(float speed)
{
	m_Speed = speed;
}
