#ifndef __ENTITY__H__
#define __ENTITY__H__

#include "GameObject.h"
#include "Camera.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include <DirectXMath.h>

class Entity
{
public:
	Entity() = default;
	Entity(GameObject entity, float speed);
	Entity(const Entity& entity) = default;
	Entity& operator=(const Entity& other) = default;
	Entity(Entity&& entity) = default;

	GameObject& GetEntity();
	float& GetSpeed();

	void SetSpeed(float speed);
	void SetModel(std::shared_ptr<FirstPersonCamera> pCamera, ModelManager& modelManager, TextureManager& textureManager) = delete;

protected:
	GameObject m_Entity;
	float m_Speed = 2.0f;
};

#endif // !__Entity__H__
