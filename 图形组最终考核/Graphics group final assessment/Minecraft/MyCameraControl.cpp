#include "MyCameraControl.h"
#include <imgui.h>

using namespace DirectX;

namespace DSM{

void PlaceDestroyBlocks(FirstPersonCamera* pCamera, DSM::Chunk& inChunk, const DSM::BlockId& blockType, Model* model)
{
    std::vector<Transform>* transform;
    std::vector<BasicEffect::InstancedData>* data;
    std::vector<BoundingBox>& blockBox = inChunk.GetBlockBox();

    switch (blockType)
    {
    case DSM::BlockId::Dirt: {
        transform = &inChunk.GetDirtTransform();
        data = &inChunk.GetDirtInstancedData();
        break;
    }
    case DSM::BlockId::Stone: {
        transform = &inChunk.GetStoneTransform();
        data = &inChunk.GetStoneInstancedData();
        break;
    }
    case DSM::BlockId::Gress: {
        transform = &inChunk.GetGressTransform();
        data = &inChunk.GetGressInstancedData();
        break;
    }
    case DSM::BlockId::BedRock: {
        transform = &inChunk.GetBedRockTransform();
        data = &inChunk.GetBedRockInstancedData();
        break;
    }
    default:
        break;
    }

    //获取鼠标
    ImVec2 mousePos = ImGui::GetMousePos();
    // 将射线设置在鼠标处
    Ray ray = Ray::ScreenToRay(*pCamera, mousePos.x, mousePos.y);

    //放置方块
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
#if 0
        bool Placing = true;
        GameObject tmpObject;
        tmpObject.SetModel(DSM::BlockModel().GetDirtModel(m_TextureManager.Get(), m_ModelManager.Get()));
        DirectX::XMFLOAT3 position = m_pFCamera->GetPosition();
        DirectX::XMFLOAT3 lookAxis = m_pFCamera->GetLookAxis();
        tmpObject.GetTransform().SetPosition(position.x + lookAxis.x * 4, position.y + lookAxis.y * 4, position.z + lookAxis.z * 4);
        for (size_t i = 0; i < m_Dirt.size(); i++) {
            if (m_Dirt[i].GetBlock().GetModel() && m_Dirt[i].GetBlock().GetBoundingBox().Intersects(tmpObject.GetBoundingBox())) {
                Placing = false;
                break;
            }
        }
        if (Placing) {
            m_Dirt[create] = (DSM::Block(tmpObject, DSM::BlockId::Dirt));
            create++;
        }
    }
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        for (size_t i = 0; i < m_SoilNum; i++) {
            if (ray.Hit(m_Dirt[i].GetBlock().GetBoundingBox(), nullptr, RAYRANGE)) {
                m_Dirt[i].GetBlock().SetModel(nullptr);
                create = i;
            }
        }
    }
#else
        // 射线打中的方块
        GameObject tmpObject;
        tmpObject.SetModel(model);

        std::vector<BoundingBox> hitBlock;
        XMFLOAT3 extents = XMFLOAT3(0.5f, 0.5f, 0.5f);
        XMFLOAT3 position = pCamera->GetPosition();
        XMINT2 chunkPosition = inChunk.GetPositon();
        // 筛选射线打中的方块
        for (int Y = (int)position.y - RAYRANGE; 0 <= Y && Y < CHUNKHIGHEST && Y < (int)position.y + RAYRANGE; ++Y) {
            for (int z = 0; z < CHUNKSIZE; ++z) {
                for (int x = 0; x < CHUNKSIZE; ++x) {
                    if (ray.Hit(blockBox[Y * CHUNKSIZE * CHUNKSIZE + z * CHUNKSIZE + x], nullptr, RAYRANGE)) {
                        float X = x + 0.5f;
                        float Z = z + 0.5f;
                        hitBlock.emplace_back(XMFLOAT3(X, Y + 0.5f, Z), extents);
                    }
                }
            }
        }

        // 选取最近的方块
        float distance;
        int pos = 0;
        for (int i = 0; i < hitBlock.size(); ++i) {
            // XMVECTOR vDistance = XMVectorSubtract(XMLoadFloat3(&hitBlock[i].Center), XMLoadFloat3(&position));
            // vDistance = XMVectorMultiply(vDistance, vDistance);
            // float fDistance = (XMVectorGetX(vDistance) + XMVectorGetY(vDistance) + XMVectorGetZ(vDistance));
            XMFLOAT3 vector(hitBlock[i].Center.x - position.x, hitBlock[i].Center.y - position.y, hitBlock[i].Center.z - position.z);
            float fDistance = pow(vector.x, 2) + pow(vector.y, 2) + pow(vector.z, 2);
            if (i == 0) {
                distance = fDistance;
            }
            else if (distance > fDistance) {
                distance = fDistance;
                pos = i;
            }
        }

        if (hitBlock.size() > 0) {
            XMFLOAT3 localPosition = XMFLOAT3(hitBlock[pos].Center.x - 0.5f, hitBlock[pos].Center.y + 0.5f, hitBlock[pos].Center.z - 0.5f);
            XMFLOAT3 worldPosition = XMFLOAT3(localPosition.x + chunkPosition.x, localPosition.y, localPosition.z + chunkPosition.y);
            Transform objectTransform = tmpObject.GetTransform();
            objectTransform.SetPosition(worldPosition);

            BasicEffect::InstancedData instanceData;
            XMMATRIX W(objectTransform.GetLocalToWorldMatrixXM());
            XMStoreFloat4x4(&instanceData.world, XMMatrixTranspose(W));
            XMStoreFloat4x4(&instanceData.worldInvTranspose, XMMatrixTranspose(XMath::InverseTranspose(W)));

            int pos = localPosition.y * CHUNKSIZE * CHUNKSIZE + localPosition.z * CHUNKSIZE + localPosition.x;
            if (blockBox[pos].Extents.x == 0.0f) {  // 判断是否有方块
                (*data).push_back(instanceData);
                (*transform).push_back(objectTransform);
                blockBox[pos] = BoundingBox(XMFLOAT3(worldPosition.x + 0.5f, worldPosition.y + 0.5f, worldPosition.z + 0.5f), extents);

                std::string seedName = "World\\" + std::to_string(DSM::Chunk::m_Seed);
                if (!std::filesystem::exists(seedName) || !std::filesystem::is_directory(seedName)) {
                    // 创建文件夹
                    std::filesystem::create_directory(seedName);
                }
                // 将放置的方块存入文件
                std::ofstream ofs;
                std::string fileName = std::string(seedName + "\\") + 'x' + std::to_string(chunkPosition.x) + 'y' + std::to_string(chunkPosition.y);
                // 以区块坐标命名文件
                ofs.open(fileName, std::ios::out | std::ios::binary | std::ios::app);
                
                // 写入方块种类
                ofs.write((char*)&blockType, sizeof(DSM::BlockId));
                // 写入坐标
                ofs.write((char*)&worldPosition, sizeof(XMFLOAT3));

                ofs.close();
            }
        }
}
#endif
}

void FreeCameraController::Update(float deltaTime, DSM::Chunk& inChunk, const DSM::BlockId& blockType, Model* model)
{
    PlaceDestroyBlocks(m_pCamera, inChunk, blockType, model);

    ImGuiIO& io = ImGui::GetIO();

    float yaw = 0.0f, pitch = 0.0f;
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)){
        yaw += io.MouseDelta.x * m_MouseSensitivityX;
        pitch += io.MouseDelta.y * m_MouseSensitivityY;
    }

    int forward = (
        (ImGui::IsKeyDown(ImGuiKey_W) ? 1 : 0) +
        (ImGui::IsKeyDown(ImGuiKey_S) ? -1 : 0)
        );
    int strafe = (
        (ImGui::IsKeyDown(ImGuiKey_A) ? -1 : 0) +
        (ImGui::IsKeyDown(ImGuiKey_D) ? 1 : 0)
        );

    if (forward || strafe){
        XMVECTOR dir = m_pCamera->GetLookAxisXM() * (float)forward + m_pCamera->GetRightAxisXM() * (float)strafe;
        XMStoreFloat3(&m_MoveDir, dir);
        m_MoveVelocity = m_MoveSpeed * 5;
        m_DragTimer = m_TotalDragTimeToZero;
        m_VelocityDrag = m_MoveSpeed / m_DragTimer;
    }
    else{
        if (m_DragTimer > 0.0f){
            m_DragTimer -= deltaTime;
            m_MoveVelocity -= m_VelocityDrag * deltaTime;
        }
        else{
            m_MoveVelocity = 0.0f;
        }
    }

    m_pCamera->RotateY(yaw);
    m_pCamera->Pitch(pitch);

    m_pCamera->Translate(m_MoveDir, m_MoveVelocity * deltaTime);
}

void FreeCameraController::InitCamera(FirstPersonCamera* pCamera)
{
    m_pCamera = pCamera;
}

void FreeCameraController::SetMouseSensitivity(float x, float y)
{
    m_MouseSensitivityX = x;
    m_MouseSensitivityY = y;
}

void FreeCameraController::SetMoveSpeed(float speed)
{
    m_MoveSpeed = speed;
}





void FirstPersonCameraController::Update(float deltaTime, DSM::Chunk& inChunk, const DSM::BlockId& blockType, Model* model)
{
    PlaceDestroyBlocks(m_pCamera, inChunk, blockType, model);

    static int jump = 0;
    bool isHit = false;
    bool onGround = false;
    std::vector<BoundingBox>& containBlock = inChunk.GetBlockBox();
    XMFLOAT3 cameraPosition = m_pCamera->GetPosition();    
    //if (!containBlock.size() || containBlock[(int)(cameraPosition.y) * CHUNKSIZE * CHUNKSIZE +
    //    ((int)cameraPosition.z + 1) * CHUNKSIZE + (int)cameraPosition.x] != DSM::BlockId::Air ||
    //    containBlock[(int)(cameraPosition.y) * CHUNKSIZE * CHUNKSIZE +
    //    ((int)cameraPosition.z - 1) * CHUNKSIZE + (int)cameraPosition.x] != DSM::BlockId::Air ||
    //    containBlock[(int)(cameraPosition.y) * CHUNKSIZE * CHUNKSIZE +
    //    (int)cameraPosition.z * CHUNKSIZE + ((int)cameraPosition.x + 1)] != DSM::BlockId::Air ||
    //    containBlock[(int)(cameraPosition.y) * CHUNKSIZE * CHUNKSIZE +
    //    (int)cameraPosition.z * CHUNKSIZE + ((int)cameraPosition.x - 1)] != DSM::BlockId::Air ||
    //    containBlock[(int)(cameraPosition.y) * CHUNKSIZE * CHUNKSIZE +
    //    ((int)cameraPosition.z + 1) * CHUNKSIZE + ((int)cameraPosition.x + 1)] != DSM::BlockId::Air ||
    //    containBlock[(int)(cameraPosition.y) * CHUNKSIZE * CHUNKSIZE +
    //    ((int)cameraPosition.z - 1) * CHUNKSIZE + ((int)cameraPosition.x - 1)] != DSM::BlockId::Air ||
    //    containBlock[(int)(cameraPosition.y) * CHUNKSIZE * CHUNKSIZE +
    //    ((int)cameraPosition.z - 1) * CHUNKSIZE + ((int)cameraPosition.x + 1)] != DSM::BlockId::Air ||
    //    containBlock[(int)(cameraPosition.y) * CHUNKSIZE * CHUNKSIZE +
    //    ((int)cameraPosition.z + 1) * CHUNKSIZE + ((int)cameraPosition.x - 1)] != DSM::BlockId::Air ||
    //    containBlock[(int)(cameraPosition.y) * CHUNKSIZE * CHUNKSIZE +
    //    (int)cameraPosition.z * CHUNKSIZE + (int)cameraPosition.x] != DSM::BlockId::Air) {
    //    onGround = true;
    //}
    ImGuiIO& io = ImGui::GetIO();
    float yaw = 0.0f, pitch = 0.0f;
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)){
        yaw += io.MouseDelta.x * m_MouseSensitivityX;
        pitch += io.MouseDelta.y * m_MouseSensitivityY;
    }
    m_pCamera->RotateY(yaw);
    m_pCamera->Pitch(pitch);
    int forward = (
        (ImGui::IsKeyDown(ImGuiKey_W) ? 1 : 0) +
        (ImGui::IsKeyDown(ImGuiKey_S) ? -1 : 0)
        );
    int strafe = (
        (ImGui::IsKeyDown(ImGuiKey_A) ? -1 : 0) +
        (ImGui::IsKeyDown(ImGuiKey_D) ? 1 : 0)
        );
    m_pCamera->Walk(forward * m_MoveSpeed * 0.016f);
    m_pCamera->Strafe(strafe * m_MoveSpeed * 0.016f);
    if (ImGui::IsKeyDown(ImGuiKey_Space) && onGround) {
        jump = 50;
    }
    if (!onGround && jump <= 0) {
        m_pCamera->Translate(XMFLOAT3(0.0f, -1.0f, 0.0f), 0.06f);
    }
    if (jump > 0) {
        m_pCamera->Translate(XMFLOAT3(0.0f, 1.0f, 0.0f), 0.04f);
        jump--;
    }
}

void FirstPersonCameraController::InitCamera(FirstPersonCamera* pCamera)
{
    m_pCamera = pCamera;
}

void FirstPersonCameraController::SetMouseSensitivity(float x, float y)
{
    m_MouseSensitivityX = x;
    m_MouseSensitivityY = y;
}

void FirstPersonCameraController::SetMoveSpeed(float speed)
{
    m_MoveSpeed = speed;
}




void ThirdCameraController::Update(float deltaTime)
{
    ImGuiIO& io = ImGui::GetIO();
    float yaw = 0.0f, pitch = 0.0f;
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)){
        yaw += io.MouseDelta.x * m_MouseSensitivityX;
        pitch += io.MouseDelta.y * m_MouseSensitivityY;
    }
    m_pCamera->RotateY(yaw);
    m_pCamera->RotateX(pitch);
}

void ThirdCameraController::InitCamera(ThirdPersonCamera* pCamera)
{
    m_pCamera = pCamera;
}

void ThirdCameraController::SetMouseSensitivity(float x, float y)
{
    m_MouseSensitivityX = x;
    m_MouseSensitivityY = y;
}

void ThirdCameraController::SetMoveSpeed(float speed)
{
    m_MoveSpeed = speed;
}


}