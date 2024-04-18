#include "Player.h"

using namespace DirectX;

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
	Model* pModel = modelManager.CreateFromFile("..\\Model\\house.obj");
	pModel->SetDebugObjectName("house");
	pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\block\\bedrock.jpg");
	pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
	pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
	pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
	pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
	pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
	pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());

	m_Entity.SetModel(pModel);

}