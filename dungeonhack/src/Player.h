#ifndef Player_H
#define Player_H

#include <OgreSceneManager.h>
using namespace Ogre;

#include "GameActor.h"


class GamePlayer : public GameActor
{
public:
    GamePlayer();

    Ogre::Camera * m_PlayerCamera;
    Ogre::SceneNode * m_PlayerHeadNode;
    Ogre::SceneNode * m_PlayerVertRotNode;

    //Player weapon
    SceneNode * leftHandNode;
    SceneNode * rightHandNode;
    SceneNode * weaponHolderNode;
    AnimationState* mWeaponAnimState;
    Entity *m_weaponEntity;
    Entity * m_Weapon;

    string m_equippedWeaponIcon;

    bool m_isLeftPressed;
    bool m_isRightPressed;
    bool m_isDownPressed;
    bool m_isUpPressed;
    
    bool m_isAttackPressed;
    bool m_isBlockPressed;

    bool m_isJumpPressed;
    bool m_isCrouchPressed;

    bool m_isUseMagicPressed;

    bool m_isRunPressed;

    bool m_isMenuPressed; //For inventory

    //WeaponManager m_WeaponManager;

    //Physics
    /*
     * Toggle noclip mode flying camera
     * \param noClip 1 for on, 0 for off, -1 or unspecified to invert state
     */
    void setNoClip(int noClip = -1);
    void resetMovementEvents();

    //Player event callbacks
    virtual void startJumpEvent();
    virtual void endJumpEvent();

    virtual void startUseMagicEvent();
    virtual void endUseMagicEvent();

    virtual void startCrouchEvent();
    virtual void endCrouchEvent();

    virtual void startLeftEvent();
    virtual void endLeftEvent();

    virtual void startRightEvent();
    virtual void endRightEvent();

    virtual void startUpEvent();
    virtual void endUpEvent();

    virtual void startDownEvent();
    virtual void endDownEvent();

    virtual void startAttackEvent();
    virtual void endAttackEvent();

    virtual void startBlockEvent();
    virtual void endBlockEvent();

    virtual void startUseEvent();
    virtual void endUseEvent();

    virtual void startRunEvent();
    virtual void endRunEvent();

    virtual void startWieldSwitchEvent();
    virtual void endWieldSwitchEvent();

    virtual void equipItem(Item * theItem);

    virtual void wieldWeapon();
    virtual void sheathWeapon();

    virtual void Delete() {delete this;};
    virtual void Update(float MoveFactor);
    virtual void UpdateStats(float MoveFactor);

    void initArms();
    void showArms();
    void hideArms();

    /// Returns combined yaw, pitch in negative z direction
    inline Ogre::Vector3 getViewDirection() const
    {
        return m_PlayerHeadNode->getOrientation() * m_PlayerCamera->getOrientation()
            * -Vector3::UNIT_Z;
    }

private:
    bool m_noClip;
};

#endif
