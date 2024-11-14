#include "Transform.h"

using namespace DirectX;

namespace DSM {
	Transform::Transform(const DirectX::XMFLOAT3& scale,
		const DirectX::XMFLOAT3& rotation,
		const DirectX::XMFLOAT3& position)
		:m_Scale(scale), m_Position(position)
	{
		SetRotate(rotation);
	}

	void Transform::SetRotate(const DirectX::XMFLOAT3& rotation)
	{
		auto quat = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));
		XMStoreFloat4(&m_Rotation, quat);
	}

	void Transform::SetPosition(const DirectX::XMFLOAT3& position)
	{
		m_Position = position;
	}

	void Transform::SetScale(const DirectX::XMFLOAT3& scale)
	{
		m_Scale = scale;
	}

	DirectX::XMMATRIX Transform::GetWorldMatrix() const
	{
		DirectX::XMMATRIX world = XMMatrixAffineTransformation(
			XMLoadFloat3(&m_Scale),
			g_XMZero,
			XMLoadFloat4(&m_Rotation),
			XMLoadFloat3(&m_Position));
		return world;
	}


}