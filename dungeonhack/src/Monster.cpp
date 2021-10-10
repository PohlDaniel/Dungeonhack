#include "DungeonHack.h"
#include "Monster.h"
#include "GameManager.h"
#include "EntityManager.h"
#include "SoundManager.h"
#include "CharacterController.h"
#include "PhysicsManager.h"
#include "Raycast.h"

Monster::Monster(Ogre::String type)
{
    entityGroup = 2; //Pawn group

    m_TimeSinceLastSeen = 10000;

    isActive = true;
    m_wasWaitingForDoor = false;

    this->m_Target = 0;

    mIdleAnimation = 0;
    mWalkAnimation = 0;
    mAttackAnimation = 0;
    rotateProgress = 0;
    doMoveForward = false;

    m_isAlert = false;
    m_SearchTime = 3000;
    this->m_sightRange = 1500;
    this->m_meleeRange = 80;
    this->m_stopRange = 65;
    m_mass = 56;    //660 is a good mass

    attack1HitFrame = 3.0;
    m_waitForDoor = false;

    m_curAttackWait = 0;

    // Simple differentiation of monster types
    if(type == "Zombie")
    {
        m_EntityType = "MonsterZombie";
        m_turnSpeed = 0.025;
        m_walkSpeed = 2.5;
        m_walkAnimScaler = 1;
        m_runAnimScaler = 1;
        m_idleTime = 0.015;
        m_Height = 53;
        m_Radius = 10;
        scale = 4;
    }
    else
    {
        m_EntityType = "Monster";
        m_turnSpeed = 0.037;
        m_walkSpeed = 3.5;
        m_walkAnimScaler = 0.6;
        m_runAnimScaler = 1;
        m_idleTime = 0.0075;
        m_Height = 7;
        m_Radius = 20;
        scale = 6;
    }

    m_Name = m_EntityType;
}

Monster::~Monster()
{
    if(this->m_displayRepresentation != 0)
    {
        //delete m_displayRepresentation;
        if(m_displayRepresentation->m_displaySceneNode)
        {
            //m_displayRepresentation->m_displaySceneNode->removeAndDestroyAllChildren();
            m_displayRepresentation->m_displaySceneNode->detachAllObjects();
            GameManager::getSingletonPtr()->getSceneManager()->destroySceneNode(m_displayRepresentation->m_displaySceneNode->getName());
            //m_displayRepresentation->m_displaySceneNode->setVisible(false);
        }

        //delete m_displayRepresentation;
        m_displayRepresentation = 0;
    }
}

void Monster::Init(Vector3 Position, Vector3 Scale, String Mesh, String attackanim, String walkanim, String idleanim, String hurtsound, String diesound, String alertsound)
{
    m_displayRepresentation = new VisualRepresentation(m_EntityType);
    m_displayRepresentation->loadMesh(Mesh, Vector3::UNIT_SCALE * scale);
    m_charControl = new CharacterController(BtOgre::Convert::toBullet(Position),
        m_Height, m_Radius, this);

    m_UpdateFrequency = 0;
    m_UpdateFreqCounter = 0;

    //Get the sound file strings into a format FMOD likes
    m_soundHurt = hurtsound;
    m_soundDie = diesound;
    m_soundAlert = alertsound;

    setPosition(Position);

    Ogre::Entity* thisEntity = m_displayRepresentation->m_displayEntity;

    //Setup LODs
    Ogre::Mesh::LodDistanceList ldl;
    ldl.push_back(800);
    thisEntity->getMesh()->generateLodLevels(ldl,
            Ogre::ProgressiveMesh::VRQ_CONSTANT,
            90);

    isIdle = true;
    try
    {
        this->mIdleAnimation = thisEntity->getAnimationState( idleanim );
        this->mIdleAnimation->setLoop( true );
        this->mIdleAnimation->setTimePosition(0);
        this->mIdleAnimation->setEnabled( true );
    }
    catch(Exception e)
    {
        //Error loading idle animation
        this->mIdleAnimation = 0;
    }

    try
    {
        this->mWalkAnimation = thisEntity->getAnimationState( walkanim );
        this->mWalkAnimation->setLoop( true );
        this->mWalkAnimation->setTimePosition(0);
        this->mWalkAnimation->setEnabled( false );
    }
    catch(Exception e)
    {
        //Error loading walk animation
        this->mWalkAnimation = 0;
    }

    isAttacking = false;

    try
    {
        this->mAttackAnimation = thisEntity->getAnimationState( attackanim );
        this->mAttackAnimation->setLoop( false );
        this->mAttackAnimation->setTimePosition(0);
        this->mAttackAnimation->setEnabled( false );
    }
    catch(Exception e)
    {
        //error loading attack animation
        this->mAttackAnimation = 0;
    }

    //MatMoving = new OgreNewt::MaterialID(m_World);
    //MatStill = new OgreNewt::MaterialID(m_World);
    
    //OgreNewt::MaterialPair * MovingMat = new OgreNewt::MaterialPair(m_World, MatDefault, MatMoving);
    //OgreNewt::MaterialPair * StillMat = new OgreNewt::MaterialPair(m_World, MatDefault, MatStill);
    //OgreNewt::MaterialPair * DefaultMat = new OgreNewt::MaterialPair(m_World, MatDefault, MatDefault);

    //m_Player->m_collisionObject->mMatPairDefault = new OgreNewt::MaterialPair(m_World, MatDefault, MatPlayer);

    /*MovingMat->setDefaultElasticity(0);
    MovingMat->setDefaultFriction(0.15, 0.15);

    StillMat->setDefaultElasticity(0);
    StillMat->setDefaultFriction(0.87, 0.87);

    DefaultMat->setDefaultFriction(0.6, 0.6);
    DefaultMat->setDefaultElasticity(0.2);*/

    //setup stats
    setTotalHealth(40);
    setTotalMagicka(20);
    setTotalFatigue(25);

    setHealth(getTotalHealth());
    setFatigue(getTotalFatigue());
    setMagicka(getTotalMagicka());

    setAttackWait(40,100);
}

void Monster::setAttackWait(float minWait, float maxWait)
{
    m_maxAttackDelay = maxWait;
    m_minAttackDelay = minWait;
    m_nextAttackWait = m_minAttackDelay + (rand() % (int)(m_maxAttackDelay - m_minAttackDelay));
}


void Monster::Update(float MoveFactor)
{
    m_UpdateFreqCounter++;
    if(m_UpdateFreqCounter >= m_UpdateFrequency)
    {
        m_UpdateFreqCounter = 0;

        Vector3 StartPos = getPosition();
        StartPos.y += 3;
        Vector3 Direction = getOrientation() * Vector3(0,0,1);

        //Get distance to player to find out how often we should update. Don't want to waste much time doing updates when we're miles away :)
        if(EntityManager::getSingletonPtr()->getPlayer() != 0)
        {
            Vector3 distToPlayerVector = StartPos 
                    - EntityManager::getSingletonPtr()->getPlayer()->getPosition();
            float distToPlayer = distToPlayerVector.normalise();

            if(distToPlayer > 3000)
            {
                m_UpdateFrequency = 10;
            }
            else
            {
                m_UpdateFrequency = 0;
            }
        }

        float rayLength = 39;

        Vector3 RayStart = StartPos;

        Ogre::Ray camray(RayStart,Direction);

        Vector3 start = camray.getOrigin();
        Vector3 end = camray.getPoint(rayLength);

        /* // TODO FIXME: raycast
        OgreNewt::BasicRaycast* ray = new OgreNewt::BasicRaycast( m_World, start, end );
        OgreNewt::BasicRaycast::BasicRaycastInfo info = ray->getFirstHit();

        bool gotOne = false;
        if (ray->getHitCount() > 0)
        {
            for(int i = 0; i < ray->getHitCount() && gotOne == false; i++)
            {
                OgreNewt::BasicRaycast::BasicRaycastInfo info = ray->getInfoAt(i);
                //OgreNewt::BasicRaycast::BasicRaycastInfo info = ray->getFirstHit();
                defaultGameBody * thisBody = (defaultGameBody *)info.mBody;

                    if(thisBody->m_ParentGameEntity != NULL)
                    {
                        if(thisBody->m_ParentGameEntity->entityGroup == 8)  //Is this a Pawn/Actor entity?
                        {
                            DoorProp * thisDoor = (DoorProp*)thisBody->m_ParentGameEntity;
                            if(thisDoor->isClosed || thisDoor->isBroken)
                            {
                                //thisDoor->onUse();
                                if(thisDoor->isClosed || thisDoor->isBroken)
                                {
                                    if(!this->isAttacking)
                                    {
                                        this->startAttack();
                                    }
                                }
                            }

                            if(thisDoor->isClosed || thisDoor->isOpening && !thisDoor->isBroken)
                            {
                                m_wasWaitingForDoor = true;
                                m_waitForDoor = true;
                            }
                            else
                            {
                                m_waitForDoor = false;
                            }
                        }
                        else
                        {
                            m_waitForDoor = false;
                        }
                    }
                }
            }
        else
        {
            m_waitForDoor = false;
        }
        delete ray;
        */

        if(m_wasWaitingForDoor)
        {
            if(!m_waitForDoor)
            {
                m_wasWaitingForDoor = false;
                this->startMoveForward();
            }
        }

        if(this->m_Target == 0)
        {
            if(EntityManager::getSingletonPtr()->getPlayer() != 0)
            {
                if(CanSee(EntityManager::getSingletonPtr()->getPlayer()))
                {
                    //SoundManager *sm = SoundManager::getSingletonPtr();
                    //sm->manageSound(sm->playSound("zombie_1.wav",0.8,false));
                    //sm->manageSound(sm->playPositionedSound(m_soundAlert.c_str(),m_pos,1.0,3000,false));

                    onAggro();

                    m_TimeSinceLastSeen = 0 ;
                    m_Target = EntityManager::getSingletonPtr()->getPlayer();
                }
            }
        }
        else    //If there is a target
        {
            //Vector3 targetPos = m_Target->m_pos;
            //targetPos.y = m_pos.y;

            Vector3 toTargetDir = m_Target->getPosition() - getPosition();
            Vector3 toLastTargetDir = LastSeenPosition - getPosition();
            Vector3 src = getOrientation() * Vector3::UNIT_Z;

            Vector3 theDistance = toTargetDir;
            Vector3 theLastDistance = toLastTargetDir;
            float Distance = theDistance.normalise();
            float LastDistance = theLastDistance.normalise();

            src.y = 0;
            toTargetDir.y = 0;
            toLastTargetDir.y = 0;

            src.normalise();
            toTargetDir.normalise();    //Strip out the distance, we only need direction

            //Quaternion toTargetQuat = this->m_pos.getRotationTo(Vector3(0,5,0));

            if(Distance < this->m_sightRange)
            {
                Quaternion last = getOrientation();
                Quaternion toTargetQuat = src.getRotationTo(toTargetDir);
                Quaternion toLastTargetQuat = src.getRotationTo(toLastTargetDir);

                setOrientation(last * toLastTargetQuat);

                setOrientation(Quaternion::nlerp(m_turnSpeed * MoveFactor, last,
                    getOrientation(), true));

                if(!this->m_isAlert)
                {
                    this->m_isAlert = true;
                }

                if(LastDistance > m_stopRange)
                {
                    if(!doMoveForward && !m_waitForDoor)
                    {
                        this->startMoveForward();
                    }
                }
                else
                {
                    if(doMoveForward)
                    {
                        this->endMoveForward();
                    }
                }

                if(Distance < this->m_meleeRange)
                {
                    if(!this->isAttacking)
                    {
                        if(m_curAttackWait >= m_nextAttackWait)
                        {
                            this->startAttack();
                        }
                    }
                }
            }
            else
            {
                if(doMoveForward)
                {
                    this->endMoveForward();
                }
            }

            if(m_waitForDoor)
            {
                if(doMoveForward)
                {
                    this->endMoveForward();
                }
            }

            if(CanSee((GameActor*)this->m_Target))
            {
                if(m_TimeSinceLastSeen >= 500)
                {
                    //SoundManager::getSingletonPtr()->playSound("zombie_2.wav",0.8,false);
                    onAggro();
                }

                m_TimeSinceLastSeen = 0 ;
                LastSeenPosition = m_Target->getPosition();
            }
            else
            {
                m_TimeSinceLastSeen += 1 * MoveFactor;

                if(m_TimeSinceLastSeen >= m_SearchTime)
                {
                    m_Target = NULL;

                    if(doMoveForward)
                    {
                        this->endMoveForward();
                    }
                }
            }
        }
    }

    if(isAttacking)
    {
        if(mAttackAnimation != 0)
        {
            mAttackAnimation->addTime(0.07 * MoveFactor);

            if(this->mAttackAnimation->getTimePosition() >= this->mAttackAnimation->getLength())
            {
                this->endAttack();
            }
            else //We are still playing the animation
            {
                if(!checkedAttack)
                {
                    if(this->mAttackAnimation->getTimePosition() >= this->attack1HitFrame)
                    {
                        checkMeleeAttackHit();
                    }
                }
            }
        }
        // Deal with spider's lack of animation
        else if(!checkedAttack)
        {
            // At least play an 'attacking' sound
            SoundManager *sm = SoundManager::getSingletonPtr();
            sm->manageSound(sm->playPositionedSound(m_soundAlert.c_str(),getPosition(),1.0,3000,false));
            checkMeleeAttackHit();
        }
        else
        {
            isAttacking = false;
            endAttack();
        }
    }
    else
    {
        m_curAttackWait += MoveFactor;
    }
    
    if(isIdle)
    {
        if(mIdleAnimation != 0)
        {
            this->mIdleAnimation->addTime(m_idleTime * MoveFactor);
        }
    }

    //Update position
    Vector3 newVel = getOrientation() * m_velocity;
    m_charControl->setWalkDirection(BtOgre::Convert::toBullet(newVel));
    Real moveSpeed = newVel.normalise();

    if(mWalkAnimation != 0)
    {
        mWalkAnimation->addTime((MoveFactor * (moveSpeed / 20.0)) * m_walkAnimScaler);
    }

    if(!isAlive())
    {
        //TODO: Play death animation here. Just delete ourselves for now
        onDie();
        Destroy();
    }
}

void Monster::checkMeleeAttackHit()
{
    checkedAttack = true;

    Vector3 StartPos = getPosition();
    StartPos.y += 3;
    Vector3 Direction = getOrientation() * Vector3(0,0,1);

    //TODO: Switch to use the attack's hit reach
    float rayLength = 60;

    Vector3 start = StartPos;
    Vector3 end = (rayLength * Direction) + start;

    NotThisEntityRayResultCallback ray(this); // Stop hitting yourself!
    PhysicsManager::getSingletonPtr()->getWorld()->rayTest(
        BtOgre::Convert::toBullet(start), BtOgre::Convert::toBullet(end), ray);

    if(ray.hasHit())
    {
        PhysicsEntity* entity = static_cast<PhysicsEntity*>(
            ray.m_collisionObject->getUserPointer());
        if(entity)
            entity->onDamage(this);
    }
}

void Monster::startMoveForward()
{
    m_velocity.z = m_walkSpeed;
    doMoveForward = true;

    this->isWalking = true;
    this->mWalkAnimation->setEnabled( true );
}

void Monster::endMoveForward()
{
    m_velocity.z = 0;
    doMoveForward = false;

    this->isWalking = false;
    this->mWalkAnimation->setEnabled( false );
}

void Monster::startAttack()
{
    isAttacking = true;
    checkedAttack = false;
    isIdle = false;

    if(mAttackAnimation != 0)
    {
        this->mAttackAnimation->setTimePosition(0);
        this->mAttackAnimation->setEnabled( true );

        this->mIdleAnimation->setEnabled(false);
    }
}

void Monster::endAttack()
{
    m_curAttackWait = 0;
    m_nextAttackWait = m_minAttackDelay + (rand() % (int)(m_maxAttackDelay - m_minAttackDelay));

    this->isAttacking = false;
    this->isIdle = true;
    checkedAttack = false;

    if(mAttackAnimation != 0)
    {
        mAttackAnimation->setTimePosition(0);
        mAttackAnimation->setEnabled( false );
        mIdleAnimation->setEnabled(true);
    }
}

void Monster::onHurt()
{
    SoundManager *sm = SoundManager::getSingletonPtr();
    sm->manageSound(sm->playPositionedSound(m_soundHurt.c_str(),getPosition(),1.5,3000,false));

    if(m_isQuestActor)
    {
        callQuestFunction(onHurtQuestFunction);
    }
}

void Monster::onDie()
{
    SoundManager *sm = SoundManager::getSingletonPtr();
    sm->manageSound(sm->playPositionedSound(m_soundDie.c_str(),getPosition(),1.5,3000,false));

    if(m_isQuestActor)
    {
        callQuestFunction(onDieQuestFunction);
    }
}

void Monster::onAggro()
{
    SoundManager *sm = SoundManager::getSingletonPtr();
    sm->manageSound(sm->playPositionedSound(m_soundAlert.c_str(),getPosition(),1.0,3000,false));

    if(m_isQuestActor)
    {
        callQuestFunction(onAggroQuestFunction);
    }
}

