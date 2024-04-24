#ifndef __MYCAMERACONTROL__H__
#define __MYCAMERACONTROL__H__

#include "Camera.h"
#include "Chunk.h"
#include "CpuTimer.h"
#include <vector>

namespace DSM {

class CameraController
{
public:
    CameraController() = default;
    CameraController& operator=(const CameraController&) = delete;
    virtual ~CameraController() {}
    virtual void InitCamera(Camera* pCamera) = delete;
    virtual void Update(float deltaTime) = 0;

protected:
    float m_MoveSpeed = 1.0f;
    float m_MouseSensitivityX = 0.005f;
    float m_MouseSensitivityY = 0.005f;
};



class FreeCameraController : public CameraController
{
public:

    ~FreeCameraController() override {};
    void Update(float deltaTime) override;

    void InitCamera(FirstPersonCamera* pCamera);

    void SetMouseSensitivity(float x, float y);
    void SetMoveSpeed(float speed);

private:
    FirstPersonCamera* m_pCamera = nullptr;

    float m_CurrentYaw = 0.0f;
    float m_CurrentPitch = 0.0f;

    DirectX::XMFLOAT3 m_MoveDir{};
    float m_MoveVelocity = 0.0f;
    float m_VelocityDrag = 0.0f;
    float m_TotalDragTimeToZero = 0.25f;
    float m_DragTimer = 0.0f;
};



class FirstPersonCameraController : public CameraController
{
public:
    ~FirstPersonCameraController() override {};
    void Update(float deltaTime) override {};
    void Update(float deltaTime, std::vector<DSM::BlockId> containBlock);

    void InitCamera(FirstPersonCamera* pCamera);

    void SetMouseSensitivity(float x, float y);
    void SetMoveSpeed(float speed);

private:
    FirstPersonCamera* m_pCamera = nullptr;
};



class ThirdCameraController : public CameraController
{
public:
    ~ThirdCameraController() override {};
    void Update(float deltaTime) override;

    void InitCamera(ThirdPersonCamera* pCamera);

    void SetMouseSensitivity(float x, float y);
    void SetMoveSpeed(float speed);

private:
    ThirdPersonCamera* m_pCamera = nullptr;
};


}


#endif // !__MYCAMERACONTROL__H__
