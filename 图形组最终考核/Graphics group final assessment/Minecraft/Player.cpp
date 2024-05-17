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

void Player::SetPosition(const XMFLOAT3& position)
{
	m_Entity.GetTransform().SetPosition(position);
}

XMFLOAT3 Player::GetPosition()
{
	return m_Entity.GetTransform().GetPosition();
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


void Player::SaveToFile()
{
	std::string fileName = "World\\" + std::to_string(DSM::Chunk::m_Seed);

	std::ofstream ofs;
	ofs.open(fileName + "\\Player.dat", std::ios::out | std::ios::binary | std::ios::trunc);
	
	Transform& transform = m_Entity.GetTransform();
	ofs.write((char*)&GetPosition(), sizeof(XMFLOAT3));
	ofs.write((char*)&transform.GetRotation(), sizeof(XMFLOAT3));

	ofs.write((char*)&GetSpeed(), sizeof(float));

	ofs.close();
}
	
bool Player::InitFromFile()
{
	std::string seedName = "World\\" + std::to_string(DSM::Chunk::m_Seed);

	std::ifstream ifs;
	ifs.open(seedName + "\\Player.dat", std::ios::in | std::ios::binary);
	if (!ifs.is_open()) {
		return false;
	}

	XMFLOAT3 stream;
	Transform& transform = m_Entity.GetTransform();
	ifs.read((char*)&stream, sizeof(XMFLOAT3));
	SetPosition(stream);

	ifs.read((char*)&stream, sizeof(XMFLOAT3));
	transform.SetRotation(stream);

	float speed;
	ifs.read((char*)&speed, sizeof(float));
	SetSpeed(speed);

	ifs.close();
}



}


