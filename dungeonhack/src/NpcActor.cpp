#include "DungeonHack.h"
#include <iostream>
#include "NpcActor.h"
#include "GameManager.h"
#include "PythonManager.h"
#include "EntityManager.h"
#include "SoundManager.h"
#include "CharacterController.h"

NpcActor::NpcActor()
{
    m_EntityType = "NpcActor";
    entityGroup = 2; //Pawn group

    m_TimeSinceLastSeen = 10000;

    isActive = true;

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
    m_turnSpeed = 0.037;
    m_mass = 56;    //660 is a good mass

    m_walkSpeed = 40;

    m_walkAnimScaler = 0.6;
    m_runAnimScaler = 1;

    attack1HitFrame = 3.0;

    m_curAttackWait = 0;
    m_Target = 0;

    m_wasWaitingForDoor = false;
    m_waitForDoor = false;

    m_Height = 50; // TODO: Fix scale
}

NpcActor::~NpcActor()
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

void NpcActor::Init(Vector3 Position, Vector3 Scale, String Mesh, String attackanim, String walkanim, String idleanim, String hurtsound, String diesound, String alertsound)
{
    m_displayRepresentation = new NpcVisualRepresentation();
    m_displayRepresentation->loadMesh(Mesh, Vector3(5,5,5));
    m_charControl = new CharacterController(BtOgre::Convert::toBullet(Position),
        m_Height, m_Radius, this);

    m_UpdateFrequency = 0;
    m_UpdateFreqCounter = 0;

    //Get the sound file strings into a format FMOD likes
    m_soundHurt = hurtsound;
    m_soundDie = diesound;
    m_soundAlert = alertsound;

    Ogre::SceneManager * mSceneMgr = GameManager::getSingletonPtr()->getSceneManager();

    try
    {
        //Setup LODs
        Ogre::Mesh::LodDistanceList ldl;
        ldl.push_back(800);
        Ogre::MeshPtr mesh = m_displayRepresentation->m_displayEntity->getMesh();
        mesh->generateLodLevels(ldl,
            Ogre::ProgressiveMesh::VRQ_CONSTANT,
                90);

        NpcVisualRepresentation* display = static_cast<NpcVisualRepresentation*>(
            m_displayRepresentation);

        //Load and attach test shirt to the base human skeleton
        Ogre::Entity * testShirtEntity = 0;
        testShirtEntity = mSceneMgr->createEntity(this->m_displayRepresentation
            ->m_displaySceneNode->getName() + "_npc_shirt", "HUMAN_MALE_SHIRT_1.mesh");
        display->attachEntityToSkeleton(testShirtEntity, -1);

        //Load and attach test pants to the base human skeleton
        Ogre::Entity * testPantsEntity = 0;
        testPantsEntity = mSceneMgr->createEntity(this->m_displayRepresentation
            ->m_displaySceneNode->getName() + "_npc_pant", "HUMAN_MALE_TIGHTS_1.mesh");
        display->attachEntityToSkeleton(testPantsEntity, -1);

        //Load and attach test shoes to the base human skeleton
        Ogre::Entity * testShoesEntity = 0;
        testShoesEntity = mSceneMgr->createEntity(this->m_displayRepresentation
            ->m_displaySceneNode->getName() + "_npc_shoes", "HUMAN_MALE_SHOES_1.mesh");
        display->attachEntityToSkeleton(testShoesEntity, -1);

        //Load and attach test head to the base human skeleton
        Ogre::Entity * testHeadEntity = 0;
        testHeadEntity = mSceneMgr->createEntity(this->m_displayRepresentation
            ->m_displaySceneNode->getName() + "_npc_head", "HUMAN_MALE_HEAD_1.mesh");
        display->attachEntityToSkeleton(testHeadEntity, -1);

        //Load and attach test hair to the base human skeleton
        Ogre::Entity * testHairEntity = 0;
        testHairEntity = mSceneMgr->createEntity(this->m_displayRepresentation
                ->m_displaySceneNode->getName() + "_npc_hair", "HUMAN_MALE_HAIR_1.mesh");
        display->attachEntityToSkeleton(testHairEntity, -1);
    }
    catch(Exception e)
    {
        std::cout << "Error creating character clothes mesh" << std::endl;
    }

    isIdle = true;

    try
    {
        this->mIdleAnimation = m_displayRepresentation->m_displayEntity->getAnimationState( idleanim );
        this->mIdleAnimation->setLoop( true );
        this->mIdleAnimation->setTimePosition(0);
        this->mIdleAnimation->setEnabled( true );
    }
    catch(Exception e)
    {
        //Error loading idle animation
        std::cout << "Error loading idle animation" << std::endl;
        this->mIdleAnimation = 0;
    }

    try
    {
        this->mWalkAnimation = m_displayRepresentation->m_displayEntity->getAnimationState( walkanim );
        this->mWalkAnimation->setLoop( true );
        this->mWalkAnimation->setTimePosition(0);
        this->mWalkAnimation->setEnabled( false );
    }
    catch(Exception e)
    {
        //Error loading walk animation
        std::cout << "Error loading walk animation" << std:: endl;
        this->mWalkAnimation = 0;
    }

    isAttacking = false;

    try
    {
        this->mAttackAnimation = m_displayRepresentation->m_displayEntity->getAnimationState( attackanim );
        this->mAttackAnimation->setLoop( false );
        this->mAttackAnimation->setTimePosition(0);
        this->mAttackAnimation->setEnabled( false );
    }
    catch(Exception e)
    {
        //error loading attack animation
        std::cout << "Error loading attack animation" << std::endl;
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

void NpcActor::setAttackWait(float minWait, float maxWait)
{
    m_maxAttackDelay = maxWait;
    m_minAttackDelay = minWait;
    m_nextAttackWait = m_minAttackDelay + (rand() % (int)(m_maxAttackDelay - m_minAttackDelay));
}


void NpcActor::Update(float MoveFactor)
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
            //check here for a new target
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
        else
        {
            isAttacking = false;
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
            this->mIdleAnimation->addTime(0.015 * MoveFactor);
        }
    }

    // TODO FIXME: Movement broken and disabled
    //Update position
    Ogre::Vector3 bodyVel;// = this->m_collisionObject->m_PhysicsBody->getVelocity();
    Ogre::Vector3 newVel = bodyVel;

    Ogre::Vector3 TranslateVector = this->m_velocity * 1000;
    Ogre::Vector3 heading(0,0,0);

    Vector3 DesiredVel = m_velocity;

    Quaternion quat = getOrientation();

    // get the charater mass
    Real mass = 1.0f;
    Vector3 inertia;
    //this->m_collisionObject->m_PhysicsBody->getMassMatrix(mass, inertia);

    Vector3 force = Vector3(0.0f, mass * 988.0, 0.0f);

    // Get the velocity vector
    Vector3 currentVel;// = this->m_collisionObject->m_PhysicsBody->getVelocity();

    force += mass*(quat*DesiredVel - currentVel) / 0.025; /// timestep;

    newVel.x = m_velocity.x;
    newVel.z = m_velocity.z;

    newVel.y = bodyVel.y;

    Vector3 trans;
    trans *= 10;
    trans = quat * newVel;

    trans.y = bodyVel.y;

    Ogre::Vector3 force2(0,-9.8 *100,0);
    force2 *= mass;

    //this->m_collisionObject->m_PhysicsBody->addForce( force2 );

    Real moveSpeed = bodyVel.normalise();

    if(mWalkAnimation != 0)
    {
        mWalkAnimation->addTime((MoveFactor * (moveSpeed / 200.0)) * m_walkAnimScaler);
    }

    if(!isAlive())
    {
        //TODO: Play death animation here. Just delete ourselves for now
        onDie();
        Destroy();
    }
}

void NpcActor::checkMeleeAttackHit()
{

}

void NpcActor::startMoveForward()
{
    m_velocity.z = m_walkSpeed;
    doMoveForward = true;

    this->isWalking = true;

    if(this->mWalkAnimation != 0)
    {
        this->mWalkAnimation->setEnabled( true );
    }
}

void NpcActor::endMoveForward()
{
    m_velocity.z = 0;
    doMoveForward = false;

    this->isWalking = false;

    if(this->mWalkAnimation != 0)
    {
        this->mWalkAnimation->setEnabled( false );
    }
}

void NpcActor::startAttack()
{
    if(mAttackAnimation != 0)
    {
        this->isAttacking = true;

        this->mAttackAnimation->setTimePosition(0);
        this->mAttackAnimation->setEnabled( true );

        this->isIdle = false;
        this->mIdleAnimation->setEnabled(false);

        checkedAttack = false;
    }
}

void NpcActor::endAttack()
{
    m_curAttackWait = 0;
    m_nextAttackWait = m_minAttackDelay + (rand() % (int)(m_maxAttackDelay - m_minAttackDelay));

    this->isAttacking = false;

    this->mAttackAnimation->setTimePosition(0);
    this->mAttackAnimation->setEnabled( false );

    this->isIdle = true;
    this->mIdleAnimation->setEnabled(true);

    checkedAttack = false;
}

void NpcActor::onHurt()
{
    SoundManager *sm = SoundManager::getSingletonPtr();
    sm->manageSound(sm->playPositionedSound(m_soundHurt.c_str(),
        getPosition(), 1.5, 3000, false));

    if(m_isQuestActor)
    {
        callQuestFunction(onHurtQuestFunction);
    }
}

void NpcActor::onDie()
{
    SoundManager *sm = SoundManager::getSingletonPtr();
    sm->manageSound(sm->playPositionedSound(m_soundDie.c_str(),
        getPosition(), 1.5, 3000, false));

    if(m_isQuestActor)
    {
        callQuestFunction(onDieQuestFunction);
    }
}

void NpcActor::onAggro()
{
    SoundManager *sm = SoundManager::getSingletonPtr();
    sm->manageSound(sm->playPositionedSound(m_soundAlert.c_str(),
        getPosition(), 1.0, 3000, false));

    if(m_isQuestActor)
    {
        callQuestFunction(onAggroQuestFunction);
    }
}

//NPC events
void NpcActor::onUse(GameEntity * theinstigator)
{
    if(m_onActivateScript != "")
    {
        PythonManager::getSingletonPtr()->callFunction(m_onActivateScript,"OnActivate");
    }
}
