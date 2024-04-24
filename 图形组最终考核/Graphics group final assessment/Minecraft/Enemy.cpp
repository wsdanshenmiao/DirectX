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

void Enemy::DrawEnemy(ID3D11Device* device, ID3D11DeviceContext* deviceContext, BasicEffect& effect)
{
	m_Entity.Draw(deviceContext, effect);

    // 初始化树纹理资源
    HR(CreateTexture2DArrayFromFile(
        device,
        deviceContext,
        std::vector<std::wstring>{
        L"..\\Texture\\Tree\\tree0.dds",
            L"..\\Texture\\Tree\\tree1.dds",
            L"..\\Texture\\Tree\\tree2.dds",
            L"..\\Texture\\Tree\\tree3.dds"},
        nullptr,
        m_LifebarTexArray.GetAddressOf()));
    effect.SetTextureArray(m_LifebarTexArray.Get());


    srand((unsigned)time(nullptr));
    VertexPosSize vertexes[16];
    float theta = 0.0f;
    for (int i = 0; i < 16; ++i)
    {
        // 取20-50的半径放置随机的树
        float radius = (float)(rand() % 31 + 20);
        float randomRad = rand() % 256 / 256.0f * XM_2PI / 16;
        vertexes[i].pos = XMFLOAT3(radius * cosf(theta + randomRad), 36.0f, radius * sinf(theta + randomRad));
        vertexes[i].size = XMFLOAT2(30.0f, 30.0f);
        theta += XM_2PI / 16;
    }

    // 设置顶点缓冲区描述
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));
    vbd.Usage = D3D11_USAGE_IMMUTABLE;	// 数据不可修改
    vbd.ByteWidth = sizeof(vertexes);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    // 新建顶点缓冲区
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertexes;
    HR(device->CreateBuffer(&vbd, &InitData, m_PointSpritesBuffer.GetAddressOf()));

    effect.SetRenderBillboard(deviceContext, true);
    UINT stride = sizeof(VertexPosSize);
    UINT offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, m_PointSpritesBuffer.GetAddressOf(), &stride, &offset);
    effect.Apply(deviceContext);
    deviceContext->Draw(16, 0);

}





}