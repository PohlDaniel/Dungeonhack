#ifndef _COLLISION_INFO_H
#define _COLLISION_INFO_H

#include <btBulletDynamicsCommon.h>


/**
    Placeholder to store collision information for Callbacks
*/
class CollisionInfo
{
public:
    CollisionInfo() : obA(NULL) {}

    CollisionInfo(btCollisionObject* o, const btVector3& a, const btVector3& b, const btVector3& n)
        : obA(o), ptA(a), ptB(b), normalOnB(n) {}

    btCollisionObject* obA;
    btVector3 ptA;
    btVector3 ptB;
    btVector3 normalOnB;
};

#endif
