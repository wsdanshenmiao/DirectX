#include "Player.h"

using namespace DirectX;

namespace DSM {

bool Player::GetFlyState()
{
	return m_Flying;
}

bool Player::GetSneakState()
{
	return m_Sneak;
}
bool Player::GetGroundState()
{
	return m_OnGround;
}

void Player::SetFlyState(bool flyState)
{
	m_Flying = flyState;
}

void Player::SetSneakState(bool sneakState)
{
	m_Sneak = sneakState;
}

void Player::SetGroundState(bool grouneState)
{
	m_OnGround = grouneState;
}

void Player::SetModel(std::shared_ptr<FirstPersonCamera> pCamera, ModelManager& modelManager)
{
	Model* pModel = modelManager.CreateFromFile("..\\Model\\Player.obj");
	pModel->SetDebugObjectName("Player");
	m_Entity.SetModel(pModel);
	m_Entity.GetTransform().SetScale(0.125f, 0.125f, 0.125f);
	XMFLOAT3 position = pCamera->GetPosition();
	m_Entity.GetTransform().SetPosition(position.x, position.y - 1.8, position.z);
	m_Entity.GetTransform().SetRotation(0.0f, pCamera->GetRotationY() - XM_PI, 0.0f);
}

}