#ifndef __PLAYER__H__
#define __PLAYER__H__

#include "Entity.h"

class Player :public Entity
{
public:
    Player() = default;
    Player(GameObject entity, DirectX::XMFLOAT3 speed);
    Player(const Player& entity) = default;
    Player(Player&& entity) = default;

    bool GetFlyState();
    bool GetSneakState();
    bool GetGroundState();
    void SetFlyState(bool flyState);
    void SetSneakState(bool sneakState);
    void SetGroundState(bool grouneState);
    void SetModel(std::shared_ptr<FirstPersonCamera> pCamera, ModelManager& modelManager);

private:
    bool m_OnGround = true;                //在地上
    bool m_Flying = false;                 //飞行
    bool m_Sneak = false;                  //潜行
};

#endif // !__PLAYER__H__
