#include "Enemy.h"

using namespace DirectX;

namespace DSM {

Enemy::Enemy(const XMFLOAT3& position)
{
	SetPosition(position);
}

XMFLOAT3& Enemy::GetAzimuthTrack()
{
	return m_AzimuthTrack;
}

DirectX::XMFLOAT3 Enemy::GetPosition()
{
	return m_Entity.GetTransform().GetPosition();
}

void Enemy::SetPosition(const XMFLOAT3& position)
{
	m_Entity.GetTransform().SetPosition(position);
}

void Enemy::SetModel(ModelManager& modelManager)
{
	Model* pModel = modelManager.CreateFromFile("..\\Model\\Enemy.obj");
	pModel->SetDebugObjectName("Enemy");
	m_Entity.SetModel(pModel);
	m_Entity.GetTransform().SetScale(0.125f, 0.125f, 0.125f);
	m_Entity.GetTransform().SetPosition(0.0f, 34.0f, 0.0f);
}

void Enemy::FindPlayer(DirectX::XMFLOAT3 playerPosition)
{
	Transform& enemyTransform = m_Entity.GetTransform();
	XMFLOAT3 float3(XMFLOAT3(playerPosition.x - GetPosition().x, playerPosition.y - GetPosition().y, playerPosition.z - GetPosition().z));
	XMVECTOR directionVec = XMVector3Normalize(XMLoadFloat3(&float3));
	XMStoreFloat3(&m_AzimuthTrack, directionVec);
	enemyTransform.LookTo(XMFLOAT3(-m_AzimuthTrack.x, -m_AzimuthTrack.y, -m_AzimuthTrack.z));
	enemyTransform.Translate(m_AzimuthTrack, 0.03f);
}

}