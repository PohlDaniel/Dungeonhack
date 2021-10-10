#include "DungeonHack.h"

#include "GameObject.h"
#include "GameManager.h"
#include "SoundManager.h"


void DoorProp::Init(Vector3 Position, Quaternion Rotation, Vector3 Scale, String Mesh, String Name, SceneNode * rootNode)
{
    initPhysicsEntity(Mesh, Scale * 60);

    SceneNode * thisWorldNode = 0;
    Ogre::Entity * thisEntity = 0;

    Quaternion initial(Radian(1.57079633),Vector3(-1,0,0));
    Rotation = initial * Rotation;

    if(thisEntity != 0)
    {
        thisEntity->setNormaliseNormals(true);
        thisEntity->setCastShadows(false);

        thisWorldNode = rootNode->createChildSceneNode();
        thisWorldNode->attachObject(thisEntity);

        Vector3 HingeOffset = Vector3(60 * 0.05,-60 * 0.44,0);
        HingeOffset * Scale;

        HingeOffset = Rotation * HingeOffset;

        setPosition(Position);
        setOrientation(Rotation);

       
        /* // TODO FIXME -- make Bullet hinge
        doorHinge = new OgreNewt::BasicJoints::Hinge(PhysicsManager::getSingletonPtr()->getWorld(),m_collisionObject->m_PhysicsBody, NULL,Position+HingeOffset,Vector3(0,1,0));
        doorHinge->setCallback(DoorProp::_hingeCallback);
        doorHinge->setUserData(this);
        */

        closedRotation = Rotation;
        openedRotation = Rotation * Quaternion(Radian(Degree(105)),Vector3(0,0,1));
        openCloseTargetRot = closedRotation;
    }
}

void DoorProp::onUse(GameEntity * instigator)
{
    if(!isBroken)
    {
        Vector3 Force = Vector3(1,0,0);

        if(isClosed)
        {
            SoundManager *sm = SoundManager::getSingletonPtr();
            sm->manageSound(sm->playPositionedSound("DoorOpen.wav",
                    getPosition(), 0.5, 1500, false));
        }
        if(isClosed || isClosing)
        {
            isClosed = false;
            isClosing = false;
            isOpening = true;
        }
        else
        {
            Force = Vector3(-1.4,0,0);
            isOpening = false;
            isClosing = true;
            isClosed = false;
        }

        // Next line is untested, but *should* do the right thing
        btVector3 ForceDirection = quatRotate(m_physicsBody->getBtOrientation(),
                BtOgre::Convert::toBullet(Force));

        m_physicsBody->getBody().activate(true);
        m_physicsBody->getBody().applyImpulse(ForceDirection * 10, btVector3());
    }
}

void DoorProp::onDamage(GameEntity * instigator)
{
    /* // TODO FIXME: hinge
    if(doorHinge != 0 && isClosed)
    {
        float doBreak = rand() % 1000;

        SoundManager *sm = SoundManager::getSingletonPtr();
        if(doBreak >= 800)  //Don't break right away, put strength check stuff in here for door bashing
        {
            delete doorHinge;
            doorHinge = 0;

            isBroken = true;
            isClosed = false;
            isOpening = false;
            isClosing = false;

            sm->manageSound(sm->playPositionedSound("Door_Bash.wav",
                    getPosition(), 0.5, 1500, false));
        }
        else
        {
            sm->manageSound(sm->playPositionedSound("DoorClose.wav",
                    getPosition(), 0.5, 1500, false));
        }
    }
    */

    /*Vector3 Force = Vector3(-300,0,0);
    Vector3 Offset = Vector3(0,0,0);

    Vector3 Position;
    Quaternion Rotation;

    Force = Rotation * Force;
    Offset = Rotation * Offset;

    m_collisionObject->m_PhysicsBody->getPositionOrientation(Position,Rotation);

    m_collisionObject->m_PhysicsBody->unFreeze();
    m_collisionObject->m_PhysicsBody->addImpulse(Force,Offset);*/
}

void DoorProp::Update(float MoveFactor)
{
    /*if(isOpening || isClosing)
    {
        doorHinge->
        Vector3 Position;
        Quaternion Rotation;
        m_collisionObject->m_PhysicsBody->getPositionOrientation(Position,Rotation);

        Vector3 src = Rotation * Vector3::UNIT_Z;

        Quaternion last = Rotation;
        Quaternion toTargetQuat = this->openCloseTargetRot;

        Rotation = Rotation * toTargetQuat;

        float m_turnSpeed = 0.017;
        Rotation = Quaternion::nlerp(m_turnSpeed * MoveFactor,last,Rotation,true);

        m_collisionObject->m_PhysicsBody->setPositionOrientation(Position,Rotation);

        if(Rotation == openCloseTargetRot)
        {
            if(isOpening)
            {
                this->isClosed = false;
                this->isOpening = false;
            }
            else
            {
                this->isClosed = true;
                this->isClosing = true;
            }
        }
    }*/
}

/* // TODO FIXME: hinge
void _CDECL DoorProp::_hingeCallback( OgreNewt::BasicJoints::Hinge* me )
{
    DoorProp* obj = (DoorProp*)me->getUserData();

    Ogre::Radian angle = me->getJointAngle();
    Ogre::Degree lim1(  -95 );
    Ogre::Degree lim2( 10 );

    if(obj != 0)
    {
        if(obj->isOpening)
        {
            if(angle.valueDegrees() <= 80)
            {
                obj->isClosed = false;
                obj->isOpening = false;
            }
        }
        else if(obj->isClosed)
        {
            lim1 = 0;
            lim2 = 0;
        }

        if(angle.valueDegrees() > 0)
        {
            if(!obj->isClosed && !obj->isOpening && obj->isClosing)
            {
                obj->isClosed = true;
                obj->isClosing = false;
                obj->isOpening = false;

                //Latch door here
                SoundManager *sm = SoundManager::getSingletonPtr();
                sm->manageSound(sm->playPositionedSound("DoorClose.wav",
                        obj->getPosition(), 0.5, 1500, false));
                lim1 = 0;
            }
        }

        if(angle.valueDegrees() > 1)
        {
            if(!obj->isClosed)
            {
                obj->isClosed = true;
                obj->isClosing = false;
                obj->isOpening = false;

                //Latch door here
                SoundManager *sm = SoundManager::getSingletonPtr();
                sm->manageSound(sm->playPositionedSound("DoorClose.wav",
                        obj->getPosition(), 0.5, 1500, false));
                lim1 = 0;
            }
        }

        if (angle.valueRadians() < lim1.valueRadians())
        {
            Ogre::Real accel = me->calculateStopAlpha(Radian( lim1.valueRadians() ) );
            me->setCallbackAccel( accel );
        }

        if (angle.valueRadians() > lim2.valueRadians() && (obj->isClosing || obj->isClosed) )
        {
            Ogre::Real accel = me->calculateStopAlpha( Radian( lim2.valueRadians() ) );
            me->setCallbackAccel( accel );
        }
    }
} 
*/
