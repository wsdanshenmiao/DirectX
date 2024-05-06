#include "CherryTree.h"

using namespace DirectX;

namespace DSM {

bool CherryTree::m_EnableTreeFC = true;

void CherryTree::CreateRandomTree(const DirectX::XMINT4& range, ModelManager& mManager, TextureManager& tManager)
{
	CreateRandomTree(range.x, range.y, range.z, range.w, mManager, tManager);
}

void CherryTree::CreateRandomTree(const int& minx, const int& maxx, const int& minz, const int& maxz, ModelManager& mManager, TextureManager& tManager)
{
	BlockModel bModel;
	m_CherryTreeWood.GetBlock().SetModel(bModel.GetCherryBlossomLogModel(tManager, mManager));
	m_CherryTreeWood.SetId(DSM::BlockId::CherryBlossomLog);
	m_CherryTreeLeave.GetBlock().SetModel(bModel.GetCherryBlossomLeavesModel(tManager, mManager));
	m_CherryTreeLeave.SetId(DSM::BlockId::CherryBlossomLeaves);
	int rangeX = maxx - minx;
	int rangeZ = maxz - minz;
	XMINT2 center((maxx + minx) / 2, (maxz + minz) / 2);
	int TreeNum = (rangeX + rangeZ) / 4;
	std::mt19937 random;	// 梅森旋转
	random.seed(DSM::Chunk::m_Seed);	// 设置种子

	m_CherryTreeWoodTransforms.resize(TreeNum);
	m_WoodInstancedData.resize(TreeNum);
	m_CherryTreeLeaveTransforms.resize(TreeNum);
	m_LeaveInstancedData.resize(TreeNum);

	for (int i = 0, pos = 0; i < TreeNum; ++i, ++pos) {
		std::uniform_real<float> randomDistX(-rangeX / 2, rangeX / 2);
		std::uniform_real<float> randomDistZ(-rangeZ / 2, rangeZ / 2);
		Transform transform;
		int X = center.x + (int)randomDistX(random);
		int Z = center.y + (int)randomDistZ(random);
		int Y = SEALEVEL + (int)(DSM::Chunk::GetNoice(X, Z)) + 2;
		transform.SetPosition(X, Y, Z);
		m_CherryTreeWoodTransforms[pos] = transform;
		XMMATRIX W = transform.GetLocalToWorldMatrixXM();
		XMStoreFloat4x4(&m_WoodInstancedData[pos].world, XMMatrixTranspose(W));
		XMStoreFloat4x4(&m_WoodInstancedData[pos].worldInvTranspose, XMMatrixTranspose(XMath::InverseTranspose(W)));

		transform.SetPosition(X, Y + 4, Z);
		m_CherryTreeLeaveTransforms[pos] = transform;
		W = transform.GetLocalToWorldMatrixXM();
		XMStoreFloat4x4(&m_LeaveInstancedData[pos].world, XMMatrixTranspose(W));
		XMStoreFloat4x4(&m_LeaveInstancedData[pos].worldInvTranspose, XMMatrixTranspose(XMath::InverseTranspose(W)));
	}
}

void CherryTree::FrustumCulling(std::shared_ptr<FirstPersonCamera> camera)
{
	m_WoodAcceptedData.clear();
	m_LeaveAcceptedData.clear();
	BoundingFrustum frustum;
	BoundingFrustum::CreateFromMatrix(frustum, camera->GetProjMatrixXM());
	XMMATRIX V = camera->GetViewMatrixXM();
	BoundingOrientedBox localOrientedBox, orientedBox;
	BoundingOrientedBox::CreateFromBoundingBox(localOrientedBox, m_CherryTreeWood.GetBlock().GetBoundingBox());
	for (size_t i = 0; i < m_WoodInstancedData.size(); ++i) {
		// 将有向包围盒从局部坐标系变换到视锥体所在的局部坐标系(观察坐标系)中
		localOrientedBox.Transform(orientedBox, m_CherryTreeWoodTransforms[i].GetLocalToWorldMatrixXM() * V);
		// 相交检测
		if (frustum.Intersects(orientedBox)) {
			m_WoodAcceptedData.push_back(m_WoodInstancedData[i]);
		}
	}
	BoundingOrientedBox::CreateFromBoundingBox(localOrientedBox, m_CherryTreeLeave.GetBlock().GetBoundingBox());
	for (size_t i = 0; i < m_LeaveInstancedData.size(); ++i) {
		// 将有向包围盒从局部坐标系变换到视锥体所在的局部坐标系(观察坐标系)中
		localOrientedBox.Transform(orientedBox, m_CherryTreeLeaveTransforms[i].GetLocalToWorldMatrixXM() * V);
		// 相交检测
		if (frustum.Intersects(orientedBox)) {
			m_LeaveAcceptedData.push_back(m_LeaveInstancedData[i]);
		}
	}
}

void CherryTree::DrawTree(ID3D11Device* device, ID3D11DeviceContext* deviceContext, BasicEffect& effect, std::shared_ptr<FirstPersonCamera> camera)
{
	if (m_EnableTreeFC) {
		FrustumCulling(camera);
	}
	const auto& refData1 = m_EnableTreeFC ? m_WoodAcceptedData : m_WoodInstancedData;
	m_pWoodInstancedBuffer = std::make_unique<Buffer>(device,
		CD3D11_BUFFER_DESC(sizeof(BasicEffect::InstancedData) * refData1.size(), D3D11_BIND_VERTEX_BUFFER,
			D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE));
	if (m_pWoodInstancedBuffer->GetBuffer()) {
		m_pWoodInstancedBuffer->SetDebugObjectName("InstancedBuffer");
		// 硬件实例化绘制
		// 上传实例数据
		memcpy_s(m_pWoodInstancedBuffer->MapDiscard(deviceContext),
			m_pWoodInstancedBuffer->GetByteWidth(), refData1.data(), refData1.size() * sizeof(BasicEffect::InstancedData));
		m_pWoodInstancedBuffer->Unmap(deviceContext);
		effect.DrawInstanced(deviceContext, *m_pWoodInstancedBuffer, m_CherryTreeWood.GetBlock(), (uint32_t)refData1.size());
	}
	const auto& refData2 = m_EnableTreeFC ? m_LeaveAcceptedData : m_LeaveInstancedData;
	m_pLeaveInstancedBuffer = std::make_unique<Buffer>(device,
		CD3D11_BUFFER_DESC(sizeof(BasicEffect::InstancedData) * refData2.size(), D3D11_BIND_VERTEX_BUFFER,
			D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE));
	if (m_pLeaveInstancedBuffer->GetBuffer()) {
		m_pLeaveInstancedBuffer->SetDebugObjectName("InstancedBuffer");
		// 硬件实例化绘制
		// 上传实例数据
		memcpy_s(m_pLeaveInstancedBuffer->MapDiscard(deviceContext),
			m_pLeaveInstancedBuffer->GetByteWidth(), refData2.data(), refData2.size() * sizeof(BasicEffect::InstancedData));
		m_pLeaveInstancedBuffer->Unmap(deviceContext);
		effect.DrawInstanced(deviceContext, *m_pLeaveInstancedBuffer, m_CherryTreeLeave.GetBlock(), (uint32_t)refData2.size());
	}
}

}