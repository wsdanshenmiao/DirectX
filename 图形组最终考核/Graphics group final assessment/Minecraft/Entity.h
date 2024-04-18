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
	Entity(GameObject entity, DirectX::XMFLOAT3 speed);
	Entity(const Entity& entity) = default;
	Entity(Entity&& entity) = default;

	GameObject& GetEntity();
	DirectX::XMFLOAT3 GetSpeed();

	void SetSpeed(DirectX::XMFLOAT3 speed);
	void SetModel(std::shared_ptr<FirstPersonCamera> pCamera, ModelManager& modelManager, TextureManager& textureManager) = delete;

protected:
	GameObject m_Entity;
	DirectX::XMFLOAT3 m_Speed = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
};

#endif // !__Entity__H__
