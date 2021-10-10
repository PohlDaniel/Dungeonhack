/* -----------------------------------------------------------------------------

        This file belongs to the DungeonHack project
        http://dungeonhack.uesp.net/
        
        The source code is subject to GPL v3, please read COPYING for details.

----------------------------------------------------------------------------- */
/**
        \brief
        Common header file for most of the game objects
        
        Most of the objects of the game framework are derived from
        the class which are declared here. This includes entities
        but also special effects and game stats.

        \version
        20081212    Garvek      Added header & Doxygen documentation
*/

#ifndef GameObject_H
#define GameObject_H


/* -----------------------------------------------------------------------------
        Headers include
----------------------------------------------------------------------------- */
#include <Ogre.h>
#include <OgrePrerequisites.h>

#include "WeaponState.h"
#include "Location.h"
#include "PhysicsBody.h"

using namespace std;
using namespace Ogre;


/* -----------------------------------------------------------------------------
        Classes and structures pre-declarations
----------------------------------------------------------------------------- */
class VisualRepresentation;
class Item;
class GameEntity;

struct ActorStats;
struct ActorStatus;
struct EquippedItems;


/* -----------------------------------------------------------------------------
        Classes declarations
----------------------------------------------------------------------------- */

/**
    Base class for game entities
*/
class GameEntity
{
public:
    int m_EntityID;
    int entityGroup;
    String m_EntityType;

    String m_Name;

    bool isActive;
    bool requestDelete;
    bool m_hasPhysicsObject;

    virtual void Update(float MoveFactor) { };
    virtual void Delete() { delete this; };
    virtual void Destroy() { requestDelete = true; onDestroy(); };

    //Used for scripting and interaction
    virtual void onUse(GameEntity* instigator) { };
    virtual void onDamage(GameEntity* instigator) { };
    virtual void onLoad() { };
    virtual void onUnload() { };
    virtual void onDestroy() { };

    void setName(Ogre::String name) { m_Name = name; }
    String getName() { return m_Name; };

    virtual int SaveState(ostream the_stream) { return 0; };

    GameEntity() { m_EntityType = ""; isActive = true; entityGroup = 0; requestDelete = false; m_hasPhysicsObject = false;};
    virtual ~GameEntity() { }
};

/**
 * Base class for all entities which have a presence in the game world
 * Entities derived from this class exist in the world in the sense that they
 * have a physical presence which can potentially be "touched", or seen, or
 * heard by other entities.
 */
class WorldEntity : public GameEntity
{
public:
    // Transform functions
    virtual const Ogre::Quaternion getOrientation() = 0;
    virtual const Ogre::Vector3 getPosition() = 0;
    virtual const Ogre::Vector3 getVelocity() = 0;
    virtual void setOrientation(const Ogre::Quaternion& quat) = 0;
    virtual void setPosition(const Ogre::Vector3& position) = 0;
    virtual void setVelocity(const Ogre::Vector3& velocity) = 0;
};

/**
 * Base class for entities transparently occupying a location
 */
class PointEntity : public WorldEntity
{
public:
    virtual const Ogre::Quaternion getOrientation() { return m_rot; };
    virtual const Ogre::Vector3 getPosition() { return m_pos; };
    virtual const Ogre::Vector3 getVelocity() { return m_velocity; };
    virtual void setOrientation(const Ogre::Quaternion& quat) { m_rot = quat; };
    virtual void setPosition(const Ogre::Vector3& position) { m_pos = position; };
    virtual void setVelocity(const Ogre::Vector3& velocity) { m_velocity = velocity; };

protected:
    Vector3 m_velocity;     /// Velocity
    Vector3 m_pos;          /// Position
    Quaternion m_rot;       /// Rotation
    };

/**
    Display data associated with visible entities
    \sa class VisibleEntity
*/
class VisualRepresentation
{
public:
    char* m_displayMesh;

    Vector3 m_displayScale;
    Vector3 m_displayOffset;
    Vector3 m_displayRotation;

    SceneNode* m_displaySceneNode;
    Entity* m_displayEntity;

    /**
     * Load a mesh to be used for display
     * \param mesh The name of the mesh to use. If empty, only the SceneNode scale is set.
     * \param scale Scale of the SceneNode and thus mesh also.
     * \param generateLod Whether or not to generate LODs.
     */
    void loadMesh(String mesh, Vector3 scale = Vector3::UNIT_SCALE, bool generateLod = false);
    void unloadMesh();

    /**
     * Initializes with a new named SceneNode with optional parent
     * \param name The name of the SceneNode
     * \param parentNode The node to attach the new SceneNode to. Default is the scene's root node.
     */
    VisualRepresentation(Ogre::String name, Ogre::SceneNode* parentNode = NULL);
    virtual ~VisualRepresentation();

private:
    /// Counter that is used to give SceneNodes unique names
    static unsigned int nodeCount;
};

/**
    Entities with local sound support
*/
class GameSoundObject : public PointEntity
{
public:
    GameSoundObject();

    bool getLooping() { return m_loop; }
    void setLooping(bool l) { m_loop = l; }

    bool hasNoPosition() { return m_noPosition; }
    void setNoPosition(bool npos) { m_noPosition = npos; }

    float getRadius() { return m_radius; }
    void setRadius(float r) { m_radius = r; }

    float getGain() { return m_gain; }
    void setGain(float g) { m_gain = g; }

    int getPriority() { return m_priority; }
    void setPriority(int prio) { m_priority = prio; }

    float getDistanceSq() { return m_distance2; }
    void setDistanceSq(float d) { m_distance2 = d; }

    virtual void Stop() = 0;

    virtual void Init() = 0;
    virtual void Update(float MoveFactor) = 0;
    virtual void Delete() = 0;

    virtual void UpdateVolume() = 0;

protected:
    bool m_loop;
    bool m_noPosition;
    float m_radius;
    float m_gain;
    int m_priority;
    float m_distance2;
};

/**
    Displayable entities
    \sa class VisualRepresentation
*/
class VisibleEntity : public WorldEntity
{
public:
    VisualRepresentation* m_displayRepresentation;

    virtual void Update(float MoveFactor) { };
    virtual void Delete() { delete this; };

    VisibleEntity()
    {
        requestDelete = false;
        m_displayRepresentation = 0;
        isActive = true;
        entityGroup = 0;
        m_hasPhysicsObject = true;
        m_velocity = Vector3::ZERO;
    };

    virtual ~VisibleEntity()
    {
        if(this != 0)
        {
            if(m_displayRepresentation != 0)
                delete m_displayRepresentation;
        }
    };

    virtual const Ogre::Quaternion getOrientation()
    {
        if(m_displayRepresentation == 0) 
            return Quaternion();
        if( m_displayRepresentation->m_displaySceneNode == 0)
            return Quaternion();
        return m_displayRepresentation->m_displaySceneNode->getOrientation();
    }

    virtual const Ogre::Vector3 getPosition()
    {
        if(m_displayRepresentation == 0)
            return Vector3();
        if(m_displayRepresentation->m_displaySceneNode == 0)
            return Vector3();
        return m_displayRepresentation->m_displaySceneNode->getPosition();
    }

    virtual const Ogre::Vector3 getVelocity() { return m_velocity; }

    virtual void setOrientation(const Ogre::Quaternion& quat)
    {
        if(m_displayRepresentation != 0)
        {
            if(m_displayRepresentation->m_displaySceneNode != 0)
                m_displayRepresentation->m_displaySceneNode->setOrientation(quat);
        }
    }

    virtual void setPosition(const Ogre::Vector3& position)
    {
        if(m_displayRepresentation != 0) 
        {
            if(m_displayRepresentation->m_displaySceneNode != 0)
                m_displayRepresentation->m_displaySceneNode->setPosition(position);
        }
    }

    virtual void setVelocity(const Ogre::Vector3& velocity) { m_velocity = velocity; }

protected:
    /// SceneNodes have no concept of velocity, so we must store this here.
    /// However, anything deriving from PhysicsEntity should NOT use this, use
    /// getVelocity() instead.
    Vector3 m_velocity;
};

/**
 * Base class for entities with collision and dynamics
 */
class PhysicsEntity : public VisibleEntity
{
    protected: PhysicsEntity() {};
public:
    virtual ~PhysicsEntity(){ delete m_physicsBody; };

    /// This function is an intermediate fix until the loading code can be improved.
    /// Eventually the same functionality will be moved to the proper constructors.
    virtual void initPhysicsEntity(Ogre::String meshName,
            Ogre::Vector3 scale=Vector3::UNIT_SCALE, float mass=15.0f)
    {
        // Create SceneNode with some kind of name
        if(m_Name != "")
            m_displayRepresentation = new VisualRepresentation(m_Name);
        else
            m_displayRepresentation = new VisualRepresentation(m_EntityType);

        m_displayRepresentation->loadMesh(meshName, scale);
        m_physicsBody = new PhysicsBody(*m_displayRepresentation, mass,
                Ogre::Vector3::ZERO, Ogre::Quaternion());
        m_physicsBody->getBody().setUserPointer(this);
    }

    virtual const Ogre::Quaternion getOrientation()
    {
        return m_physicsBody->getOrientation();
    }

    virtual const Ogre::Vector3 getPosition()
    {
        return m_physicsBody->getPosition();
    }

    virtual const Ogre::Vector3 getVelocity()
    {
        return m_physicsBody->getVelocity();
    }

    virtual void setOrientation(const Ogre::Quaternion& quat)
    {
        m_physicsBody->setOrientation(quat);
    }

    virtual void setPosition(const Ogre::Vector3& position)
    {
        m_physicsBody->setPosition(position);
    }

    virtual void setVelocity(const Ogre::Vector3& velocity)
    {
        m_physicsBody->setVelocity(velocity);
    }

    virtual void setMass(float mass)
    {
        btRigidBody& body = m_physicsBody->getBody();
        btVector3 inertia;
        body.getCollisionShape()->calculateLocalInertia(mass, inertia);
        body.setMassProps(mass, inertia);
        body.updateInertiaTensor();
    }
    
    virtual PhysicsBody* getPhysicsBody()
    {
        return m_physicsBody;
    }

protected:
    PhysicsBody* m_physicsBody;
};

/**
    Door with transportation to another location
    \sa class VisibleEntity
    \sa class PhysicsEntity
    \sa class Location
*/
class TeleportDoorProp : public PhysicsEntity
{
public:
    /// Similar to other entities' Init functions, except this one was created as a loading workaround.
    void Init(Vector3 position, Quaternion orientation);
    void leaveInterior(int exitID);
    void enterInterior();

    //virtual void Init(Vector3 Position,Quaternion Rotation, Vector3 Scale, String Mesh, String name);

    virtual void onUse(GameEntity* instigator);

    Location* m_Location;

    int m_exitNum;
    int m_interiorNum;

    String m_CollisionMesh;
    String m_DisplayMesh;
    Vector3 m_DisplayScale;
    Vector3 m_CollisionScale;

    TeleportDoorProp()
    {
        m_EntityType = "TeleportDoorProp";
        entityGroup = 4;

        m_interiorNum = 0;
        m_exitNum = 0;

        requestDelete = false;
    }
};

/**
    Door with animation support (including destruction)
    \sa class VisibleEntity
    \sa class PhysicsEntity
    \sa class Location
*/
class DoorProp : public PhysicsEntity
{
public:

    virtual void Init(Vector3 Position, Quaternion Rotation, Vector3 Scale, String Mesh,
        String name, SceneNode* rootNode);

    virtual void onUse(GameEntity* instigator);
    virtual void onDamage(GameEntity* instigator);
    virtual void Update(float MoveFactor);

    Location* m_Location;

    int m_exitNum;
    int m_interiorNum;

    String m_CollisionMesh;
    String m_DisplayMesh;
    Vector3 m_DisplayScale;
    Vector3 m_CollisionScale;

    bool isClosed;
    bool isOpening;
    bool isClosing;

    Quaternion openCloseTargetRot;
    Quaternion closedRotation;
    Quaternion openedRotation;

    bool isBroken;

    DoorProp()
    {
        m_EntityType = "DoorProp";
        entityGroup = 8;

        m_interiorNum = 0;
        m_exitNum = 0;

        isClosed = true;
        isOpening = false;
        isClosing = false;

        requestDelete = false;

        isBroken = false;
    }
};

/**
    Static entities
    \sa class PhysicsEntity
*/
class GameProp : public PhysicsEntity
{
public:
    float mass;
    float health;

    bool isStatic;

    virtual void Update(float MoveFactor) { };
    virtual void Delete() { delete this; };

    virtual void onUse(GameEntity* instigator);

    String m_CollisionMesh;
    String m_DisplayMesh;
    Vector3 m_DisplayScale;
    Vector3 m_CollisionScale;
    
    GameProp()
    {
        m_EntityType = "GameProp";
        entityGroup = 3;
        requestDelete = false;

        m_DisplayScale = Vector3::UNIT_SCALE;
        m_CollisionScale = Vector3::UNIT_SCALE;
    };

    virtual ~GameProp()
    {
        if(m_displayRepresentation)
        {
            //delete m_displayRepresentation;
        }
    }
};

/**
    Projectile entities
    \sa class GameProp
*/
class GameProjectile : public GameProp
{
public:
    virtual void Update(float MoveFactor);
    virtual void Delete() { delete this; };

    virtual void onSpawn();
    virtual void onHit();

    virtual void Init(Vector3 Position, Vector3 Size, float Mass, String DisplayMesh);

    float m_timeToLive;
    float m_beenAliveTime;

    GameProjectile()
    {
        m_EntityType = "GameProjectile";
        entityGroup = 20;   //20 for projectiles
        requestDelete = false;

        m_beenAliveTime = 0;
        m_timeToLive = 0;

        m_DisplayScale = Vector3(1,1,1);
        m_CollisionScale = Vector3(1,1,1);
    };
};

/**
    Sun & Moon effects
*/
class GameCorona : public VisibleEntity
{
public:
    bool isVisible();

    ColourValue m_Color;

    virtual void Update(float MoveFactor);
    virtual void Init(Vector3 Position, Vector3 Scale, String name);
    virtual void Delete() {delete this;};

    float fadeFactor;
    float flickerAlpha;
    float flickerUpdateTime;

    float scaleSize;

    float maxViewDistance;

    BillboardSet* mCoronaBillboardSet;
    Billboard* mCoronaBillboard;
    
    GameCorona();
    virtual ~GameCorona();
};

/**
    \sa class GameCorona
*/
class GameSun : public GameCorona
{
public:
    virtual void Update(float MoveFactor);
    virtual void Init(Vector3 Position, Vector3 Scale, String name);
    virtual void Delete() { delete this; };

    void showSun(bool show);

    BillboardSet* mSunBillboardSet;
    Billboard* mSunBillboard;
    Billboard* mSunCoronaBillboard;

    GameSun()
    {
        m_EntityType = "GameSun";

        mCoronaBillboardSet = 0;
        mCoronaBillboard = 0;
        m_displayRepresentation = new VisualRepresentation(m_EntityType);
        mSunBillboard = 0;
        mSunBillboardSet = 0;
        mSunCoronaBillboard = 0;
        entityGroup = 5;

        requestDelete = false;
    };

    bool hideSun;
    bool visible;
};

/**
    \sa class GameCorona
*/
class GameMoon : public GameCorona
{
public:
    virtual void Update(float MoveFactor);
    virtual void Init(Vector3 Position, Vector3 Scale, String name);
    virtual void Delete() {delete this;};

    void showMoon(bool show);

    BillboardSet* mMoonBillboardSet;
    Billboard* mMoonBillboard;

    GameMoon()
    {
        m_EntityType = "GameMoon";

        mCoronaBillboardSet = 0;
        mCoronaBillboard = 0;
        m_displayRepresentation = new VisualRepresentation(m_EntityType);
        mMoonBillboard = 0;
        mMoonBillboardSet = 0;
        entityGroup = 5;

        requestDelete = false;
    };

    bool hideMoon;
    bool visible;
};

/**
    Entities with particle effects
    \sa class VisibleEntity
*/
class GameEmitter : public VisibleEntity
{
public:
    virtual void Update(float MoveFactor) {};
    virtual void Delete() { delete this; };

    ParticleSystem* m_Emitter;

    GameEmitter()
    {
        m_EntityType = "GameEmitter";

        m_displayRepresentation = new VisualRepresentation(m_EntityType);
        m_Emitter = 0;
        entityGroup = 6;

        requestDelete = false;
    };

    virtual ~GameEmitter()
    {
        m_Emitter->clear();
        //ParticleSystemManager::getSingleton().destroySystem(m_Emitter);
        m_Emitter = 0;
    };
};

/**
    Placeholder for gameplay stats
*/
struct GameActorStats
{
    //Final derived attributes
    float curHealth;
    float curMagic;
    float curFatigue;

    float maxHealth;
    float maxMagic;
    float maxFatigue;

    float armourRating;
    float runSpeed;
    float walkSpeed;
    float attackSpeed;
    float jumpHeight;

    //Base character attributes
    float strength;
    float intelligence;
    float willpower;
    float agility;
    float endurance;
    float personality;
    float speed;
    float luck;

    //Character attributes after modifiers
    float adj_strength;
    float adj_intelligence;
    float adj_willpower;
    float adj_agility;
    float adj_endurance;
    float adj_personality;
    float adj_speed;
    float adj_luck;
};

#endif
