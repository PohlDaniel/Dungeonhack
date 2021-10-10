#include "DungeonHack.h"

#include "Player.h"
#include "GameManager.h"
#include "PlayState.h"
#include "EntityManager.h"
#include "Projectiles.h"
#include "SoundManager.h"
#include "PhysicsManager.h"
#include "Raycast.h"
#include "CharacterController.h"

GamePlayer::GamePlayer()
{
    entityGroup = 2; //Pawn group
    this->m_EntityType = "GamePlayer";

    this->m_SpeedMod = 1;
    m_isUpPressed = false;
    m_isDownPressed = false;
    m_isLeftPressed = false;
    m_isRightPressed = false;
    m_isUseMagicPressed = false;

    m_weaponOffset = Vector3(0,m_Height - 3,0);

    m_PlayerHeadNode = NULL;
    m_PlayerCamera = NULL;

    isRunning = false;
    m_WeaponManager.m_Owner = this;

    m_isWieldingWeapon = false;

    m_Weapon = 0;
    m_noClip = false;

    m_charControl = new CharacterController(BtOgre::Convert::toBullet(Vector3::ZERO),
        m_Height, m_Radius, this);

    // temp give the player a name
    m_Name = "McSconeluvin";

    m_displayRepresentation = new VisualRepresentation(m_Name);
}

void GamePlayer::Update(float MoveFactor)
{
    Quaternion quat = m_PlayerHeadNode->getOrientation();
    Vector3 force = Vector3::ZERO;

    m_charControl->setOrientation(quat); // Set our current turn angle

    // Update force vector based on view orientation
    if(m_noClip)
    {
        // Calculate in pitch
        quat = quat * m_PlayerCamera->getOrientation();
        force = quat * m_velocity;
    }
    else
    {
        // Bullet "gravity" for CharacterController stepping seems to have no acceleration
        // component (intended for stepping down sloped terrain?), let's make our own
        m_velocity.y -= 7.2f;
        if(m_velocity.y < 0 && m_charControl->onGround())
            m_velocity.y = 0;
        force = quat * m_velocity;
    }

    // Update player head position, leaving your head behind can be disorienting
    if(m_PlayerHeadNode != NULL)
    {
        Vector3 position;
        m_charControl->setWalkDirection(BtOgre::Convert::toBullet(force));
        position = m_charControl->getPosition(); // We may be blocked, get the real position

        // Set view position above player location
        position.y = position.y + m_Height - 5;
        m_PlayerHeadNode->setPosition(position);
    }

    // Update vertical node as well to keep 'view model' in view
    if(m_PlayerVertRotNode != 0)
    {
        m_PlayerVertRotNode->setOrientation(m_PlayerCamera->getOrientation());
    }

    m_WeaponManager.update(MoveFactor);
    UpdateStats(MoveFactor);
}

void GamePlayer::setNoClip(int noClip)
{
    if(noClip < 0)
        m_noClip = !m_noClip;
    else if(noClip > 0)
        m_noClip = true;
    else
        m_noClip = false;

    if(m_noClip)
    {
        m_charControl->setMovementFlags(MF_NO_COLLIDE | MF_FLY);
        m_actorStats.runSpeed = 80; // Allow very fast movement in noclip mode
    }
    else
    {
        m_charControl->setMovementFlags(0);
        m_actorStats.runSpeed = 15; // TODO: save this constant somewhere else!
    }
}

void GamePlayer::UpdateStats(float MoveFactor)
{
    //Update stats here based on... stuff? :P
    //The Daggerfall Chronicles should be the best place to get this information

    setFatigue(getFatigue() + (0.1 * MoveFactor));  //Catch your breath
    setMagicka(getMagicka() + (0.01 * MoveFactor)); //Recharge Mana

    if(this->isRunning)
    {
        if(m_isUpPressed || m_isDownPressed || m_isLeftPressed || m_isRightPressed)
        {
            setFatigue(getFatigue() - (0.15 * MoveFactor));
        }
    }
}

//Attack Event callbacks
void GamePlayer::startAttackEvent()
{
    m_isAttackPressed = true;
    
    this->m_WeaponManager.pressedAttack();
}

void GamePlayer::endAttackEvent()
{
    m_isAttackPressed = false;

    this->m_WeaponManager.releasedAttack();
}

//Cast Magic callbacks
void GamePlayer::startUseMagicEvent()
{
    m_isUseMagicPressed = true;
    Vector3 direction = getViewDirection();

    if(this->getMagicka() >= 8)
    {
        SoundManager *sm = SoundManager::getSingletonPtr();
        sm->manageSound(sm->playPositionedSound("Magic_Fire_Cast.wav",
            getPosition(), 0.5, 2000, false));

        GameEmitterProjectile* newFireball = new GameEmitterProjectile();
        EntityManager::getSingletonPtr()->addLocalEntity(newFireball);
        float forwardOffset = m_Radius * 3;
        Vector3 startPos = m_PlayerCamera->getWorldPosition() + direction * forwardOffset;
        startPos.y -= 7; // Cast from about arm height
        newFireball->Init(startPos, direction, 600, Vector3(15, 15, 15), 25, "Magic/FireBall");

        this->setMagicka(this->getMagicka() - 8);
    }
    else
    {
        PlayState* theState = static_cast<PlayState*>(GameManager::getSingletonPtr()
            ->getCurrentState());
        theState->addStringToHUDInfo("Not enough magicka to cast spell");
    }
}

void GamePlayer::endUseMagicEvent()
{
    m_isUseMagicPressed = false;
}

//Block Event callbacks
void GamePlayer::startBlockEvent()
{
    m_isBlockPressed = true;
}

void GamePlayer::endBlockEvent()
{
    m_isBlockPressed = false;
}

//Crouch Event callbacks
void GamePlayer::startCrouchEvent()
{
    m_isCrouchPressed = true;
    m_velocity.y = -m_actorStats.walkSpeed * m_SpeedMod;
}

void GamePlayer::endCrouchEvent()
{
    m_velocity.y = 0;
    m_isCrouchPressed = false;
}

void GamePlayer::startRunEvent()
{
    this->startRunning();

    if(this->m_isUpPressed)
    {
        startUpEvent();
    }
    if(this->m_isDownPressed)
    {
        startDownEvent();
    }
    if(this->m_isLeftPressed)
    {
        startLeftEvent();
    }
    if(this->m_isRightPressed)
    {
        startRightEvent();
    }
}

void GamePlayer::endRunEvent()
{
    this->stopRunning();

    if(this->m_isUpPressed)
    {
        startUpEvent();
    }
    if(this->m_isDownPressed)
    {
        startDownEvent();
    }
    if(this->m_isLeftPressed)
    {
        startLeftEvent();
    }
    if(this->m_isRightPressed)
    {
        startRightEvent();
    }
}

//Jump Event callbacks
void GamePlayer::startJumpEvent()
{
    m_isJumpPressed = true;
    float fatigueCost = 40.0f;

    if(!m_charControl->onGround())
        return;

    // Make sure we are not too tired to jump
    if(getFatigue() - fatigueCost > 0)
    {
        if(isRunning)
            m_velocity.y = 50 * m_SpeedMod;
        else
            m_velocity.y = 40 * m_SpeedMod;

        // Reduce Fatigue. Jumping is hard in all that plate mail.
        setFatigue(getFatigue() - fatigueCost);
    }
    else
    {
        PlayState* theState = static_cast<PlayState*>(GameManager::getSingletonPtr()
            ->getCurrentState());
        theState->addStringToHUDInfo("You are fatigued");
    }
}

void GamePlayer::endJumpEvent()
{
    m_isJumpPressed = false;
}

//Down Event callbacks
void GamePlayer::startDownEvent()
{
    if(!isRunning)
        m_velocity.z = (m_actorStats.walkSpeed * 0.5) * m_SpeedMod;
    else
        m_velocity.z = (m_actorStats.walkSpeed * 0.75) * m_SpeedMod;

    m_isDownPressed = true;
}

void GamePlayer::endDownEvent()
{
    m_velocity.z = 0;
    m_isDownPressed = false;
}

//Up Event callbacks
void GamePlayer::startUpEvent()
{
    if(!isRunning)
        m_velocity.z = -m_actorStats.walkSpeed * m_SpeedMod;
    else
        m_velocity.z = -m_actorStats.runSpeed * m_SpeedMod;

    m_isUpPressed = true;
}

void GamePlayer::endUpEvent()
{
    m_velocity.z = 0;
    m_isUpPressed = false;
}

//Left Event callbacks
void GamePlayer::startLeftEvent()
{
    if(!isRunning)
        m_velocity.x = (-m_actorStats.walkSpeed * 0.85) * m_SpeedMod;
    else
        m_velocity.x = (-m_actorStats.runSpeed * 0.85) * m_SpeedMod;

    m_isLeftPressed = true;
}

void GamePlayer::endLeftEvent()
{
    m_velocity.x = 0;
    m_isLeftPressed = false;
}

//Right Event callbacks
void GamePlayer::startRightEvent()
{
    if(!isRunning)
        m_velocity.x = (m_actorStats.walkSpeed * 0.85) * m_SpeedMod;
    else
        m_velocity.x = (m_actorStats.runSpeed * 0.85) * m_SpeedMod;

    m_isRightPressed = true;
}

void GamePlayer::endRightEvent()
{
    m_velocity.x = 0;
    m_isRightPressed = false;
}

void GamePlayer::startUseEvent()
{
    Vector3 start = m_PlayerCamera->getWorldPosition();
    Vector3 direction = getViewDirection();

    float rayLength = 150;

    Vector3 end = (rayLength * direction) + start;

    EntityRayResultCallback rayCallback;

    PhysicsManager::getSingletonPtr()->getWorld()->rayTest(
            BtOgre::Convert::toBullet(start),
            BtOgre::Convert::toBullet(end), rayCallback);

    if(rayCallback.hasHit())
    {
        PhysicsEntity* entity = static_cast<PhysicsEntity*>(
            rayCallback.m_collisionObject->getUserPointer());
        if(entity)
            entity->onUse(this);
    }
}

void GamePlayer::endUseEvent()
{

}

void GamePlayer::startWieldSwitchEvent()
{
    if(m_isWieldingWeapon == false)
    {
        wieldWeapon();
    }
    else
    {
        sheathWeapon();
    }
}

void GamePlayer::endWieldSwitchEvent()
{

}

void GamePlayer::wieldWeapon()
{
    Ogre::SceneManager * theScene = GameManager::getSingletonPtr()->getSceneManager();

    if(this->m_equippedWeapon != 0)
    {
        m_isWieldingWeapon = true;
        showArms();

        m_WeaponManager.start(new BladeState(),m_weaponEntity);

        if(m_Weapon == 0)
        {
            m_Weapon = theScene->createEntity( "SwordEntity2", m_equippedWeapon->m_displayMesh );
        }
        else
        {

            m_weaponEntity->detachObjectFromBone(m_Weapon);
            theScene->destroyEntity(m_Weapon);
            m_Weapon = 0;

            m_Weapon = theScene->createEntity( "SwordEntity2", m_equippedWeapon->m_displayMesh );
        }

        m_weaponEntity->attachObjectToBone("Holder",m_Weapon,Quaternion(Radian(0.25),Vector3(0,1,-0.25)),Vector3(0.75,0.75,0));
        //m_SwordEntity->attachObjectToBone("Holder",m_Weapon,Quaternion(Ogre::Radian(Degree(-90)),Vector3(1,0,0)) * Quaternion(Radian(0.20),Vector3(0,-0.1,1.0)),Vector3(0.0,0.75,0));

        m_Weapon->setNormaliseNormals(true);

        //Play unsheath sound
        SoundManager *sm = SoundManager::getSingletonPtr();
        sm->manageSound(sm->playPositionedSound("UnsheathSword.wav",
            getPosition(), 0.5, 2000, false));

        //showArms();
    }
}

void GamePlayer::sheathWeapon()
{
    m_isWieldingWeapon = false;
    hideArms();
}

void GamePlayer::initArms()
{
    Ogre::SceneManager * theScene = GameManager::getSingletonPtr()->getSceneManager();

    m_PlayerVertRotNode = m_PlayerHeadNode->createChildSceneNode("PlayerNode");

    rightHandNode = m_PlayerVertRotNode->createChildSceneNode("Player_Right_Hand");
    leftHandNode = m_PlayerVertRotNode->createChildSceneNode("Player_Left_Hand");

    m_weaponEntity = theScene->createEntity( "playerWeaponEntity", "Arm.mesh" );
    m_weaponEntity->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);
    m_weaponEntity->setNormaliseNormals(true);

    weaponHolderNode = rightHandNode->createChildSceneNode("WeaponH");
    weaponHolderNode->attachObject(m_weaponEntity);
    weaponHolderNode->scale(0.9,0.9,0.9);

    rightHandNode->setPosition(15,-8,-8);
    leftHandNode->setPosition(-4.5,-8,-16.5);

    rightHandNode->yaw(Radian(3.14159265));
    rightHandNode->roll(Radian(-0.1));

    leftHandNode->yaw(Radian(-1.0));

    rightHandNode->setVisible(false);
    leftHandNode->setVisible(false);

    //m_PlayerHeadNode->removeChild(rightHandNode);
    //m_PlayerHeadNode->removeChild(leftHandNode);
}

void GamePlayer::showArms()
{
    rightHandNode->setVisible(true);
    leftHandNode->setVisible(true);

    //m_PlayerHeadNode->addChild(rightHandNode);
    //m_PlayerHeadNode->addChild(leftHandNode);
}

void GamePlayer::hideArms()
{
    rightHandNode->setVisible(false);
    leftHandNode->setVisible(false);

    //m_PlayerHeadNode->removeChild(rightHandNode);
    //m_PlayerHeadNode->removeChild(leftHandNode);
}

void GamePlayer::equipItem(Item * theItem)
{
    PlayState * theState = static_cast<PlayState *>(GameManager::getSingletonPtr()->getCurrentState());

    if(theItem->m_ItemType == "weapon")
    {
        this->m_equippedWeapon = theItem;
        this->m_equippedWeaponIcon = theItem->m_displayIcon;
        theState->switchHUDEquippedWeaponIcon(theItem->m_displayIcon);

        wieldWeapon();

        theState->addStringToHUDInfo("Equipped " + theItem->m_ItemName);
    }
}

void GamePlayer::resetMovementEvents()
{
    this->endDownEvent();
    this->endUpEvent();
    this->endRightEvent();
    this->endLeftEvent();
    this->endRunEvent();
}

