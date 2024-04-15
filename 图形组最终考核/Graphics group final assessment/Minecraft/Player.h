#ifndef __PLAYER__H__
#define __PLAYER__H__

#include "Entity.h"

class Player :public Entity
{
private:
    bool m_OnGround = true;         //在地上
    bool m_Flying = false;          //飞行
    bool m_Sneak = false;           //潜行
};

#endif // !__PLAYER__H__
