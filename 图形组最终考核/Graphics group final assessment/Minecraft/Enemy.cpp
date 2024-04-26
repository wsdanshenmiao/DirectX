#include "Enemy.h"

using namespace DirectX;

namespace DSM {

XMFLOAT3& Enemy::GetAzimuthTrack()
{
	return m_AzimuthTrack;
}

DirectX::XMFLOAT3 Enemy::GetPosition()
{
	return m_Entity.GetTransform().GetPosition();
}

int& Enemy::GetHP()
{
	return m_HP;
}

void Enemy::SetPosition(const XMFLOAT3& position)
{
	m_Entity.GetTransform().SetPosition(position);
	// 使血条朝向玩家并在敌人头顶
	XMFLOAT3 entieyPosition = m_Entity.GetTransform().GetPosition();
	m_Lifebar.GetTransform().SetPosition(entieyPosition.x, entieyPosition.y + 2.2f, entieyPosition.z);
	XMFLOAT3 right = m_Entity.GetTransform().GetRightAxis();
	m_Lifebar.GetTransform().SetRotation(-right.x, -right.y, -right.z);
}

void Enemy::SetPosition(float x, float y, float z)
{
	SetPosition(XMFLOAT3(x, y, z));
}

void Enemy::SetModel(TextureManager& tManager, ModelManager& mManager)
{
	Model* pModel = mManager.CreateFromFile("..\\Model\\Enemy.obj");
	pModel->SetDebugObjectName("Enemy");
	m_Entity.SetModel(pModel);
	m_Entity.GetTransform().SetScale(0.125f, 0.125f, 0.125f);

	pModel = mManager.CreateFromGeometry("Lifebar1", Geometry::CreatePlane(1.5f, 0.15f));
	tManager.CreateFromFile("..\\Texture\\entity\\1.png");
	pModel->SetDebugObjectName("Lifebar1");
	pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\entity\\1.png");
	pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
	pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
	pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
	pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
	pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
	m_Lifebar.SetModel(pModel);
	XMFLOAT3 entieyPosition = m_Entity.GetTransform().GetPosition();
	m_Lifebar.GetTransform().SetPosition(entieyPosition.x, entieyPosition.y + 2.2f, entieyPosition.z);
	XMFLOAT3 right = m_Entity.GetTransform().GetRightAxis();
	m_Lifebar.GetTransform().SetRotation(-right.x, -right.y, -right.z);
}

void Enemy::FindPlayer(DirectX::XMFLOAT3 playerPosition)
{
	// 获取敌人和玩家的相对位置
	Transform& enemyTransform = m_Entity.GetTransform();
	XMFLOAT3 float3(XMFLOAT3(playerPosition.x - GetPosition().x, playerPosition.y - GetPosition().y, playerPosition.z - GetPosition().z));
	XMVECTOR directionVec = XMVector3Normalize(XMLoadFloat3(&float3));
	XMStoreFloat3(&m_AzimuthTrack, directionVec);
	// 使敌人始终朝向玩家
	enemyTransform.LookTo(XMFLOAT3(-m_AzimuthTrack.x, -m_AzimuthTrack.y, -m_AzimuthTrack.z));
	enemyTransform.Translate(m_AzimuthTrack, 0.016f * m_Speed);
	// 使血条朝向玩家并在敌人头顶
	XMFLOAT3 entieyPosition = m_Entity.GetTransform().GetPosition();
	XMFLOAT3 up = m_Entity.GetTransform().GetUpAxis();
	m_Lifebar.GetTransform().SetPosition(entieyPosition.x + up.x * 2.2f, entieyPosition.y + up.y * 2.2f, entieyPosition.z + up.z * 2.2f);
	m_Lifebar.GetTransform().LookTo(XMFLOAT3(-m_AzimuthTrack.x, -m_AzimuthTrack.y + XM_PI, -m_AzimuthTrack.z));
}

void Enemy::LoadEnemy(TextureManager& tManager, ModelManager& mManager)
{
	if (m_HP == 2) {
		Model* pModel = mManager.CreateFromGeometry("Lifebar2", Geometry::CreatePlane(1.5f, 0.15f));
		tManager.CreateFromFile("..\\Texture\\entity\\2.png");
		pModel->SetDebugObjectName("Lifebar2");
		pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\entity\\2.png");
		pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
		pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
		pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
		m_Lifebar.SetModel(pModel);
	}
	else if (m_HP == 1) {
		Model* pModel = mManager.CreateFromGeometry("Lifebar3", Geometry::CreatePlane(1.5f, 0.15f));
		tManager.CreateFromFile("..\\Texture\\entity\\3.png");
		pModel->SetDebugObjectName("Lifebar3");
		pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\entity\\3.png");
		pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
		pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
		pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
		pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
		m_Lifebar.SetModel(pModel);
	}
}

void Enemy::DrawEnemy(ID3D11DeviceContext* deviceContext, BasicEffect& effect)
{
	if (m_HP > 0) {
		m_Entity.Draw(deviceContext, effect);
		m_Lifebar.Draw(deviceContext, effect);
	}
}





}