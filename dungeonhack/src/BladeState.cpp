#include "DungeonHack.h"
#include "WeaponState.h"
#include "GameManager.h"
#include "GameActor.h"
#include "PlayState.h"
#include "SoundManager.h"
#include "PhysicsManager.h"
#include "Raycast.h"

#include "Player.h" // TODO: remove when no longer player specific

using namespace Ogre;


BladeState::BladeState()
{
    this->mWeaponAttackAnimation = NULL;
    this->mWeaponIdleAnimation = NULL;

    readyFrame = 0.74;
    startHitCheckFrame = 1.39;
    endHitCheckFrame = 1.67;
}

void BladeState::enter()
{
    isWindup = false;
    isReady = false;
    isAttacking = false;
    isReturning = false;
    doNextAttack = false;

    if(this->m_WeaponEntity != NULL)
    {
        this->mWeaponAttackAnimation = this->m_WeaponEntity->getAnimationState( "Swing" );
            this->mWeaponAttackAnimation->setLoop( false );
            this->mWeaponAttackAnimation->setTimePosition(0);
            this->mWeaponAttackAnimation->setEnabled( false );

        this->mWeaponIdleAnimation = this->m_WeaponEntity->getAnimationState( "Idle" );
            this->mWeaponIdleAnimation->setLoop( true );
            this->mWeaponIdleAnimation->setTimePosition(0);
            this->mWeaponIdleAnimation->setEnabled( false );
    }

    this->playIdleAnimation();
}

void BladeState::exit()
{
    isWindup = false;
    isReady = false;
    isAttacking = false;

    this->m_WeaponManager->doneChangingState(this);
}

void BladeState::pressedAttack()
{
    if(isIdle())
    {
        StartAttack();
    }
    else if(isAttacking || isReturning)
    {
        doNextAttack = true;
    }

    isAttackPressed = true;
}

void BladeState::releasedAttack()
{
    if(isReady || isWindup)
    {
        Swing();
    }
    else
    {
        doNextAttack = true;
    }

    isAttackPressed = false;
}

void BladeState::StartAttack()
{
        playWindupAnimation();
        doNextAttack = false;
        isWindup = true;
        isReady = false;
        isAttacking = false;
        isReturning = false;
        checkHit = true;
}

void BladeState::EndAttack()
{
    playIdleAnimation();
    isWindup = false;
    isReady = false;
    isAttacking = false;
}

void BladeState::EndAttackHit()
{
    playReturnAnimation();
    isWindup = false;
    isReady = false;
    isAttacking = false;
}

void BladeState::Swing()
{
    isWindup = false;
    isReady = false;
    isAttacking = true;

    if(m_WeaponManager != 0)
    {
        GameActor * myActor = this->m_WeaponManager->m_Owner;
        if(myActor != 0)
        {
            myActor->setFatigue(myActor->getFatigue() - 11.0);
            //SoundManager *sm = SoundManager::getSingletonPtr();
            //sm->manageSound(sm->playPositionedSound("MaleSwing.wav",myActor->m_pos,0.5,2000,false));
        }
    }

    playAttackAnimation();
}

void BladeState::CheckHitFrame()
{
    m_WeaponManager->CheckHitFrameCallback();

    PlayState* theState = static_cast<PlayState*>(GameManager::getSingletonPtr()
        ->getCurrentState());

    // TODO: this only currently works for the player, fix when NPCs get weapons
    Vector3 start = theState->getCamera()->getWorldPosition();
    Vector3 direction = theState->m_Player->getViewDirection();

    float rayLength = 48;

    Vector3 end = (rayLength * direction) + start;

    NotThisEntityRayResultCallback ray(m_WeaponManager->m_Owner); // Stop hitting yourself!
    PhysicsManager::getSingletonPtr()->getWorld()->rayTest(
        BtOgre::Convert::toBullet(start), BtOgre::Convert::toBullet(end),ray);

    if (ray.hasHit())
    {
        EndAttackHit();
        checkHit = false;

        PhysicsEntity* entity = static_cast<PhysicsEntity*>(
            ray.m_collisionObject->getUserPointer());
        if(entity)
            entity->onDamage(m_WeaponManager->m_Owner);

        SoundManager* sm = SoundManager::getSingletonPtr();
        sm->manageSound(sm->playPositionedSound("Sword.wav",
            BtOgre::Convert::toOgre(ray.m_hitPointWorld), 0.5, 2000, false));

        //TODO: Add bullet physics impulses here
        /*info.mBody->unFreeze();

        Real mass;
        Ogre::Vector3 inertia;
        info.mBody->getMassMatrix(mass,inertia);

        //Get global and object-local positions of the body
        Ogre::Vector3 bodpos;
        Ogre::Quaternion bodorient;

        info.mBody->setUserData(info.mBody);
        info.mBody->getPositionOrientation( bodpos, bodorient );

        // info.mDistance is in the range [0,1].
        Ogre::Vector3 globalpt = camray.getPoint( (rayLength) * info.mDistance );
        Ogre::Vector3 localpt = bodorient.Inverse() * (globalpt - bodpos);

        defaultGameBody * newGameBody = (defaultGameBody *)info.mBody;
        if(newGameBody->m_ParentGameEntity != 0)
            newGameBody->m_ParentGameEntity->onDamage(m_WeaponManager->m_Owner);

        info.mBody->addImpulse(ForceDir * (45 * (60 / mass)),globalpt);

        if(theState && GameManager::getSingletonPtr()->isDebugMode() == true)
        {
        // TODO: draw line from StartPos to globalpt
        }
        */
    }
}

void BladeState::update(float MoveFactor)
{
    GameActor * myActor = this->m_WeaponManager->m_Owner;

    PlayState * theState = static_cast<PlayState *>(GameManager::getSingletonPtr()->getCurrentState());

    if(doNextAttack)
    {
        if(isIdle())
        {
            if(!isAttackPressed)
            {
                doNextAttack = false;
                StartAttack();
                Swing();
            }
            else
            {
                StartAttack();
            }
        }
    }

    if(theState && GameManager::getSingletonPtr()->isDebugMode() == true)
    {
        Bone * holderBone = this->m_WeaponEntity->getSkeleton()->getBone("Holder");

        if(holderBone != NULL && GameManager::getSingletonPtr()->isDebugMode())
        {
            //theState->weaponRayNode->setPosition(this->m_WeaponEntity->getParentNode()->_getDerivedPosition() +holderBone->_getDerivedPosition() - Vector3(0,0,0));
            Quaternion translateQuat = myActor->getOrientation()
                * theState->getCamera()->getOrientation();
            translateQuat = translateQuat * Quaternion(Radian(3.14159265),Vector3(0,1,0));
            //translateQuat = translateQuat * Quaternion(Radian(0.25),Vector3(1,0,0));
    
            theState->weaponRayNode->setPosition(myActor->getPosition()
                    + (translateQuat * (holderBone->_getDerivedPosition()
                    - Vector3(15,0,-2))));
            theState->weaponRayNode->setOrientation(translateQuat * holderBone->_getDerivedOrientation());
            theState->weaponRayNode->setOrientation(theState->weaponRayNode->getOrientation() * Quaternion(Radian(0.15),Vector3(0.3,0,0.6)));

            Vector3 StartPos = myActor->getPosition() + (translateQuat
                    * (holderBone->_getDerivedPosition() - Vector3(15,0,-2)));
            Quaternion temp = translateQuat * holderBone->_getDerivedOrientation();
            temp = temp * Quaternion(Radian(0.15),Vector3(0.3,0,0.6));

            Vector3 Direction = temp * (Vector3::NEGATIVE_UNIT_Z);
            //theState->weaponRayNode->setPosition(holderBone->_getFullTransform() * myActor->m_pos.In);
            //theState->weaponRayNode->setOrientation(myActor->m_rot * theState->getCamera()->getOrientation());
        }
    }

    if(mWeaponIdleAnimation != NULL && mWeaponAttackAnimation != NULL)
    {
        mWeaponIdleAnimation->addTime(this->m_idleAnimSpeed * MoveFactor);
        mWeaponAttackAnimation->addTime(this->m_attackAnimSpeed * MoveFactor);
    }

    if(isWindup == true)
    {
        if(mWeaponAttackAnimation->getTimePosition() >= readyFrame)
        {
            isWindup = false;
            isReady = true;
            isAttacking = false;

            playHoldAnimation();
        }
    }

    if(isAttacking == true)
    {
        if(mWeaponAttackAnimation != NULL)
        {
            float thisTimePos = mWeaponAttackAnimation->getTimePosition();
            float thisTimeLength = mWeaponAttackAnimation->getLength();

            if(thisTimePos >= this->startHitCheckFrame && thisTimePos <= this->endHitCheckFrame && checkHit == true)
            {
                this->CheckHitFrame();
            }

            if(thisTimePos >= thisTimeLength)
            {
                checkHit = false;
                isWindup = false;
                isReady = false;
                isAttacking = false;

                EndAttack();
            }
        }
    }

    if(isReturning == true)
    {
        if(doHitBlend)
        {
            //blendFactor += 0.04;
            blendFactor += ((blendFactor / (2.4 / MoveFactor)) + (0.0015 * MoveFactor));
        }
        else if(this->mWeaponAttackAnimation->getTimePosition() <= hitTimePosition)
        {
            m_attackAnimSpeed = 0.0;
            doHitBlend = true;
        }

        if(mWeaponIdleAnimation != NULL && mWeaponAttackAnimation != NULL)
        {
            this->mWeaponIdleAnimation->setWeight(blendFactor);
        }

        if(blendFactor >= 100)
        {
            doHitBlend = false;
            isWindup = false;
            isReady = false;
            isAttacking = false;
            isReturning = false;
            blendFactor = 0;
            playIdleAnimation();
        }
    }
}

bool BladeState::isIdle()
{
    if(isWindup == false && isReady == false && isAttacking == false && isReturning == false)
    {
        return true;
    }
    return false;
}

void BladeState::playIdleAnimation()
{
    if(mWeaponIdleAnimation != NULL)
    {
        this->mWeaponIdleAnimation->setLoop( true );
        this->mWeaponIdleAnimation->setTimePosition(0);
        this->mWeaponIdleAnimation->setEnabled( true );

        this->m_idleAnimSpeed = 0.009;
    }

    if(mWeaponAttackAnimation != NULL)
    {
        this->mWeaponAttackAnimation->setEnabled( false );
        this->m_attackAnimSpeed = 0.015;
    }
}

void BladeState::playWindupAnimation()
{
    if(mWeaponAttackAnimation != NULL)
    {
        this->mWeaponAttackAnimation->setLoop( false );
        this->mWeaponAttackAnimation->setTimePosition(0);
        this->mWeaponAttackAnimation->setEnabled( true );

        this->m_attackAnimSpeed = 0.015;
    }

    if(mWeaponIdleAnimation != NULL)
    {
        this->mWeaponIdleAnimation->setEnabled( false );
    }
}

void BladeState::playHoldAnimation()
{
    this->m_attackAnimSpeed = 0;
}

void BladeState::playAttackAnimation()
{
    this->m_attackAnimSpeed = 0.03; //0.03
}

void BladeState::playReturnAnimation()
{
    this->m_attackAnimSpeed = 0.0;
    this->m_idleAnimSpeed = 0.0;
    isWindup = false;
    isReady = false;
    isAttacking = false;
    isReturning = true;

    blendFactor = 0.0;
    doHitBlend = true;
    //hitTimePosition = this->mWeaponAttackAnimation->getTimePosition() - 0.8;
    if(mWeaponIdleAnimation != NULL)
    {
        this->mWeaponIdleAnimation->setEnabled( true );
        this->mWeaponIdleAnimation->setWeight(0.0);
    }
}
