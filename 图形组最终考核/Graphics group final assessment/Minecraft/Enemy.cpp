#include "Enemy.h"

using namespace DirectX;

namespace DSM {

Enemy::Enemy()
{

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
	m_Entity.GetTransform().SetPosition(-5.0f, SEALEVEL + (int)(CHUNKRANGE * DSM::Chunk::GetNoice(-5, -5)) + 0.5f, -5.0f);
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

void Enemy::DrawEnemy(ID3D11DeviceContext* deviceContext, IEffect& effect)
{
	// »æÖÆÑªÌõ
	//effect.SetRenderBillboard(m_pd3dImmediateContext.Get(), m_EnableAlphaToCoverage);
	//effect.SetMaterial(m_TreeMat);
	//UINT stride = sizeof(VertexPosSize);
	//UINT offset = 0;
	//m_pd3dImmediateContext->IASetVertexBuffers(0, 1, mPointSpritesBuffer.GetAddressOf(), &stride, &offset);
	//effect.Apply(m_pd3dImmediateContext.Get());
	//m_pd3dImmediateContext->Draw(16, 0);

	m_Entity.Draw(deviceContext, effect);
}





}