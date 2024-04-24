#include "MyCameraControl.h"
#include <imgui.h>

using namespace DirectX;

namespace DSM {

void FreeCameraController::Update(float deltaTime)
{
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




void FirstPersonCameraController::Update(float deltaTime, std::vector<DSM::BlockId> containBlock)
{
    static int jump = 0;
    bool isHit = false;
    bool onGround = false;
    XMFLOAT3 cameraPosition = m_pCamera->GetPosition();    
    if (!containBlock.size() || containBlock[(int)(cameraPosition.y) * CHUNKSIZE * CHUNKSIZE +
        ((int)cameraPosition.z + 1) * CHUNKSIZE + (int)cameraPosition.x] != DSM::BlockId::Air ||
        containBlock[(int)(cameraPosition.y) * CHUNKSIZE * CHUNKSIZE +
        ((int)cameraPosition.z - 1) * CHUNKSIZE + (int)cameraPosition.x] != DSM::BlockId::Air ||
        containBlock[(int)(cameraPosition.y) * CHUNKSIZE * CHUNKSIZE +
        (int)cameraPosition.z * CHUNKSIZE + ((int)cameraPosition.x + 1)] != DSM::BlockId::Air ||
        containBlock[(int)(cameraPosition.y) * CHUNKSIZE * CHUNKSIZE +
        (int)cameraPosition.z * CHUNKSIZE + ((int)cameraPosition.x - 1)] != DSM::BlockId::Air ||
        containBlock[(int)(cameraPosition.y) * CHUNKSIZE * CHUNKSIZE +
        ((int)cameraPosition.z + 1) * CHUNKSIZE + ((int)cameraPosition.x + 1)] != DSM::BlockId::Air ||
        containBlock[(int)(cameraPosition.y) * CHUNKSIZE * CHUNKSIZE +
        ((int)cameraPosition.z - 1) * CHUNKSIZE + ((int)cameraPosition.x - 1)] != DSM::BlockId::Air ||
        containBlock[(int)(cameraPosition.y) * CHUNKSIZE * CHUNKSIZE +
        ((int)cameraPosition.z - 1) * CHUNKSIZE + ((int)cameraPosition.x + 1)] != DSM::BlockId::Air ||
        containBlock[(int)(cameraPosition.y) * CHUNKSIZE * CHUNKSIZE +
        ((int)cameraPosition.z + 1) * CHUNKSIZE + ((int)cameraPosition.x - 1)] != DSM::BlockId::Air ||
        containBlock[(int)(cameraPosition.y) * CHUNKSIZE * CHUNKSIZE +
        (int)cameraPosition.z * CHUNKSIZE + (int)cameraPosition.x] != DSM::BlockId::Air) {
        onGround = true;
    }
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
    m_pCamera->Walk(forward * m_MoveSpeed * 0.016);
    m_pCamera->Strafe(strafe * m_MoveSpeed * 0.016);
    if (ImGui::IsKeyDown(ImGuiKey_Space) && onGround) {
        jump = 50;
    }
    if (!onGround && jump <= 0) {
        m_pCamera->Translate(XMFLOAT3(0.0f, -1.0f, 0.0f), 0.04f);
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