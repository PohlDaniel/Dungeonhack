#include "DungeonHack.h"
#include <fstream>

#include "PlayState.h"
#include "PauseState.h"
#include "MessageState.h"
#include "ConsoleState.h"
#include "GuiHUD.h"
#include "HDR.h"

#include "TimeManager.h"
#include "EntityManager.h"
#include "PhysicsManager.h"
#include "UiManager.h"
#include "SoundManager.h"
#include "QuestManager.h"

#include "GameWorld.h"
#include "GameTerrain.h"
#include "Sky.h"
#include "WaterPlane.h"

#include "Location.h"
#include "PythonManager.h"
#include "GameConfig.h"

using namespace Ogre;

PlayState PlayState::mPlayState;


void PlayState::enter()
{
    int pX, pY, iX, iY;
    int terrainTileSize;
    float centerOffset;

    GameConfiguration* config = GameManager::getSingletonPtr()->getConfig();

    //Read config settings
    cfg_fogThickness = config->getFloat("fogThickness");
    TimeManager::getSingleton().setTimeFactor(config->getFloat("speed"));
    TimeManager::getSingleton().setDayScale(config->getFloat("dayScale"));
    timeToCheckUpdating = 3000; //How many ms that have to pass before we update the array of entities that are in range of the player
    lastCheckUpdatingTime = timeToCheckUpdating + 2;

    //Landscape Vars
    //Player start tile
    pX = config->getInteger("gameStartTileX");
    pY = config->getInteger("gameStartTileY");
    iX = pX;
    iY = pY;
    terrainTileSize = 512;  //256
    centerOffset = (30 * terrainTileSize) / 2.0;

    this->m_WorldTerrain = 0;   //In the begginning, there was nothing
    isOutdoor = true;
    TimeManager::getSingletonPtr()->setTimeOfDay( (LENGTH_OF_MONTH * 10) + LENGTH_OF_DAY / 14.0 );

    //Initialize noise generation
    ambientNoiseRandom = rand() % 300;
    ambientNoiseWait = 300;
    curAmbientNoiseWait = 0;

    InitOgre();
    InitPlayer();

    GameWorld::getSingletonPtr()->loadLocations("locations.xml");
    InitLandscape(iX, iY, 3);

    m_GUI = GameManager::getSingletonPtr()->getUI();
    showHUD();

    // HDR/Bloom
    mHdrListener = 0;
    if (config->getInteger("doBloom") == 1)
    {
        string tech = config->getString("bloomTechnique");
        if (tech.compare("HDR") == 0 || tech.compare("Bloom") == 0)
        {
            Ogre::CompositorInstance *instance = Ogre::CompositorManager::getSingleton().addCompositor(this->getViewport(), tech, 0);
            if (instance)
            {
                Ogre::CompositorManager::getSingleton().setCompositorEnabled(this->getViewport(), tech, true);
                if (tech == "HDR")
                {
                    mHdrListener = new HDRListener();
                    instance->addListener(mHdrListener);
                    mHdrListener->notifyViewportSize(this->getViewport()->getActualWidth(), this->getViewport()->getActualHeight());
                    mHdrListener->notifyCompositor(instance);
                }
            }
            else
            {
                config->setInteger("doBloom", 0);
            }
        }
        else
        {
            config->setInteger("doBloom", 0);
        }
    }

    SoundManager::getSingletonPtr()->playStreamingOutdoorAmbient("grassland3.ogg");
    SoundManager::getSingletonPtr()->playStreamingMusic("RobertHood_DF-TheVirtuous.ogg");

    //Create the script manager singleton
    questManager = new QuestManager();
    pythonManager = new PythonManager();
}

void PlayState::InitOgre()
{
    GameManager* m_gameMgr = GameManager::getSingletonPtr();

    m_root = Root::getSingletonPtr();
    m_root->setFrameSmoothingPeriod(0.5);

    m_sceneMgr = m_gameMgr->getSceneManager();
    m_sceneMgr->setShadowTechnique(SHADOWTYPE_NONE);

    m_camera = m_gameMgr->getMainCamera();
    m_viewport = m_gameMgr->getMainViewport();

    m_camera->setFOVy(Radian(Real(0.92)));
    m_camera->setAspectRatio(Real(m_viewport->getActualWidth()) / Real(m_viewport->getActualHeight()));
    m_PlayerCam = m_camera;
}

void PlayState::InitPlayer()
{
    // TODO: Put most of this code in the player class!

    //Create player
    m_Player = new GamePlayer();
    EntityManager::getSingletonPtr()->addGlobalEntity(m_Player);
    EntityManager::getSingletonPtr()->setPlayer(m_Player);

    m_Player->m_PlayerHeadNode = m_sceneMgr->getRootSceneNode()->createChildSceneNode( "PlayerHeadNode" );
    m_Player->m_PlayerHeadNode->attachObject(m_camera);
    m_Player->initArms();

    //Setup camera
    m_Player->m_PlayerCamera = m_camera;

    //Initialize player stats
    m_Player->m_actorStats.walkSpeed = 5;
    m_Player->m_actorStats.runSpeed = 15;

    m_Player->m_actorStats.speed = 40;
    m_Player->m_actorStats.endurance = 20;
    m_Player->m_actorStats.strength = 20;
    m_Player->m_actorStats.luck = 20;
    m_Player->m_actorStats.intelligence = 20;
    m_Player->m_actorStats.willpower = 20;
    m_Player->m_actorStats.personality = 20;

    m_Player->setTotalHealth(50);
    m_Player->setTotalMagicka(30);
    m_Player->setTotalFatigue((m_Player->getStrength() + m_Player->getEndurance()) * 64);

    m_Player->setHealth(m_Player->getTotalHealth());
    m_Player->setFatigue(m_Player->getTotalFatigue());
    m_Player->setMagicka(m_Player->getTotalMagicka());
}

void PlayState::InitLandscape(int x_tile, int y_tile, int exit_marker_num)
{
    int terrain_tile_size = 512; //TODO: Move to enum or define
    float centerOffset = (30 * terrain_tile_size) / 2.0;
    Ogre::Vector3 Offset = Vector3((x_tile + 0.5) * (64 * terrain_tile_size), 3000, (y_tile + 0.5) * (64 * terrain_tile_size));

    m_sky = new Sky();
    m_sky->Init(x_tile, y_tile, terrain_tile_size, centerOffset, m_sceneMgr, m_camera);
    m_sky->SetFogThickness(cfg_fogThickness);
    m_sky->SetCloudMaterialName("Clear");
    m_sceneMgr->addRenderQueueListener(m_sky);

    m_water = new WaterPlane();
    m_water->Init(m_sceneMgr, m_camera);
    m_water->Update(Offset);
    m_sky->m_clip_plane = m_water->getClipPlane();

    m_WorldTerrain = new GameTerrain(x_tile, y_tile, terrain_tile_size,this->m_sceneMgr, m_water);

    bool markerFound = false;
    GameConfiguration* config = GameManager::getSingletonPtr()->getConfig();
    if(config->getInteger("gameStartLocStartMarker") >= 0)
    {
        //Assume that we're outside for now
        if(m_WorldTerrain->m_CenterTile->m_Chunk->m_Location != 0)
        {
            LocationMarker * exitMarker = m_WorldTerrain->m_CenterTile->m_Chunk->m_Location->findExitMarker(exit_marker_num);
            if(exitMarker != 0 && m_Player != 0)
            {
                m_Player->setPosition(exitMarker->m_Position);
                markerFound = true;
            }
        }
    }

    if(!markerFound) // Fall back to offset
    {
        m_Player->setPosition(Offset);
    }
}

void PlayState::clearLandscape()
{
    delete m_WorldTerrain;
    delete m_sky;
    delete m_water;
    m_WorldTerrain = NULL;
    m_sky = NULL;
    m_water = NULL;
}

void PlayState::exit()
{
    if (m_hud)
    {
        hideHUD();
        delete m_hud;
        m_hud = NULL;
        m_GUI = NULL;
    }

    EntityManager::getSingletonPtr()->unloadLocalEntities();
    EntityManager::getSingletonPtr()->unloadGlobalEntities();
    clearLandscape();

    if(m_Player != NULL)
    {
        delete(m_Player);
        m_Player = NULL;
    }

    m_playerLight = NULL;
}

void PlayState::showInventory()
{
    
}

void PlayState::closeInventory()
{

}

void PlayState::pause()
{
    TimeManager::getSingleton().setTimeFactor(0.0);
    hideHUD();
}

void PlayState::resume()
{
    GameConfiguration* config = GameManager::getSingletonPtr()->getConfig();
    TimeManager::getSingleton().setTimeFactor(config->getFloat("speed"));
    showHUD();
}


void PlayState::keyPressed(const OIS::KeyEvent &e)
{
    if (e.key == OIS::KC_UP || e.key == OIS::KC_W)
    {
        if(this->m_Player != NULL)
        {
            m_Player->startUpEvent();
        }
    }

    if (e.key == OIS::KC_DOWN  || e.key == OIS::KC_S)
    {
        if(this->m_Player != NULL)
        {
            m_Player->startDownEvent();
        }
    }

    if (e.key == OIS::KC_LEFT  || e.key == OIS::KC_A)
    {
        if(this->m_Player != NULL)
        {
            m_Player->startLeftEvent();
        }
    }

    if (e.key == OIS::KC_RIGHT  || e.key == OIS::KC_D)
    {
        if(this->m_Player != NULL)
        {
            m_Player->startRightEvent();
        }
    }

    if(e.key == OIS::KC_SPACE)
    {
        if(this->m_Player != NULL)
        {
            m_Player->startUseEvent();
        }
    }

    if(e.key == OIS::KC_F)
    {
        if(this->m_Player != NULL)
        {
            m_Player->startWieldSwitchEvent();
        }
    }

    if(e.key == OIS::KC_LSHIFT)
    {
        if(this->m_Player != NULL)
        {
            m_Player->startRunEvent();
        }
    }

    if(e.key == OIS::KC_E)
    {
        if(this->m_Player != NULL)
        {
            m_Player->startJumpEvent();
        }
    }

    if(e.key == OIS::KC_R)
    {
        if(this->m_Player != NULL)
        {
            m_Player->startUseMagicEvent();
        }
    }

    if(e.key == OIS::KC_F3)
    {
    }

    if(e.key == OIS::KC_C)
    {
        m_sky->SetCloudMaterialName("");
    }
    if(e.key == OIS::KC_X)
    {
        m_sky->SetCloudMaterialName("HeavyClouds");
    }

    if(e.key == OIS::KC_V)
    {
        //cfg_fogThickness -= 0.0000025;
        //m_sky->cloud_scaler -= 0.01;
        //m_sky->SetFogThickness(cfg_fogThickness);

        float water_height = m_water->GetHeight();
        m_water->SetHeight(water_height - 100);
        m_water->Update(m_Player->getPosition());
    }
    if(e.key == OIS::KC_B)
    {
        //cfg_fogThickness += 0.0000025;
        //m_sky->cloud_scaler += 0.01;
        //m_sky->SetFogThickness(cfg_fogThickness);

        float water_height = m_water->GetHeight();
        m_water->SetHeight(water_height + 100);
        m_water->Update(m_Player->getPosition());
    }
    if(e.key == OIS::KC_G)
    {
        std::cout << "Chucking box!" << std::endl;
        // Chuck a Bullet physics box
        GameProjectile* newTestObj = new GameProjectile();
        EntityManager::getSingletonPtr()->addLocalEntity(newTestObj);

        Quaternion quat = m_Player->m_PlayerHeadNode->getOrientation();
        Vector3 startPos = m_Player->getPosition() + (quat * Vector3(0, 15.5, -10));
        newTestObj->Init(startPos, Vector3(1,1,1), 10, "barrel.mesh");
    }
    // Cycle physics debug modes
    if(e.key == OIS::KC_O)
    {
        static int dbgMode = 0;
        if(dbgMode == 0)
        {
            PhysicsManager::getSingletonPtr()->toggleDebugDraw();
            dbgMode++;
            std::cout << "Enabled physics debug draw" << std::endl;
        }
        else if(dbgMode == 1)
        {
            // TODO: tell GameTerrain to not draw
            dbgMode++;
            std::cout << "Enabled physics debug draw and disabled terrain" << std::endl;
        }
        else if(dbgMode == 2)
        {
            // TODO: tell GameTerrain to draw
            PhysicsManager::getSingletonPtr()->toggleDebugDraw();
            dbgMode = 0; // Back to start
            std::cout << "Disabled physics debug draw and enabled terrain" << std::endl;
        }
    }
    // Toggle noclip
    if(e.key == OIS::KC_L)
    {
        m_Player->setNoClip();
    }
}


void PlayState::keyReleased(const OIS::KeyEvent &e)
{
    if (e.key == OIS::KC_P)
    {
        pause();

        m_Player->resetMovementEvents();

        pushState(PauseState::getInstance());
    }

    if (e.key == OIS::KC_ESCAPE)
    {
        pause();

        m_Player->resetMovementEvents();

        pushState(PauseState::getInstance());
    }

    if (e.key == OIS::KC_UP || e.key == OIS::KC_W)
    {
        if(this->m_Player != NULL)
        {
            m_Player->endUpEvent();
        }
    }

    if (e.key == OIS::KC_DOWN  || e.key == OIS::KC_S)
    {
        if(this->m_Player != NULL)
        {
            m_Player->endDownEvent();
        }
    }

    if (e.key == OIS::KC_LEFT  || e.key == OIS::KC_A)
    {
        if(this->m_Player != NULL)
        {
            m_Player->endLeftEvent();
        }
    }

    if (e.key == OIS::KC_RIGHT  || e.key == OIS::KC_D)
    {
        if(this->m_Player != NULL)
        {
            m_Player->endRightEvent();
        }
    }

    if(e.key == OIS::KC_SPACE)
    {
        if(this->m_Player != NULL)
        {
            m_Player->endUseEvent();
        }
    }

    if(e.key == OIS::KC_F)
    {
        if(this->m_Player != NULL)
        {
            m_Player->endWieldSwitchEvent();
        }
    }

    if(e.key == OIS::KC_LSHIFT)
    {
        if(this->m_Player != NULL)
        {
            m_Player->endRunEvent();
        }
    }

    if(e.key == OIS::KC_E)
    {
        if(this->m_Player != NULL)
        {
            m_Player->endJumpEvent();
        }
    }

    if(e.key == OIS::KC_R)
    {
        if(this->m_Player != NULL)
        {
            m_Player->endUseMagicEvent();
        }
    }

    if(e.key == OIS::KC_M)
    {
        this->m_Player->m_SpeedMod = 10;
    }
    if(e.key == OIS::KC_N)
    {
        this->m_Player->m_SpeedMod = 1;
    }
    if(e.key == OIS::KC_H)
    {
        EntityManager::getSingletonPtr()->unloadLocalEntities();
    }
    if(e.key == OIS::KC_J)
    {
        if(this->m_WorldTerrain->m_CenterTile->m_Chunk->m_Location != 0)
        {
            this->m_WorldTerrain->m_CenterTile->m_Chunk->m_Location->UnloadExterior();
        }
        //TODO: Unload the current cell here, for testing
    }
    if(e.key == OIS::KC_I)
    {
        if(this->m_Player != NULL)
        {
            m_Player->m_isInvisible = !m_Player->m_isInvisible;
        }
    }

    if(e.key == OIS::KC_5)
    {
        if(m_playerLight)
        {
            m_sceneMgr->destroyLight(m_playerLight);
            m_playerLight = 0;
        }
        else
        {
            try
            {
                m_playerLight = m_sceneMgr->createLight("Light2");
            }
            catch(Exception e)
            {
                m_playerLight = m_sceneMgr->getLight("Light2");
            }

            m_playerLight->setType(Light::LT_POINT);
            m_playerLight->setDiffuseColour(ColourValue(1.0 ,0.517, 0.00) * 0.7);
            m_playerLight->setSpecularColour(1, 1, 1);
            m_playerLight->setAttenuation(500, 1, 0.05, 0);
            m_playerLight->setAttenuation(20000, 1.0 , -0.0005713425, 0.0001273547);
        }
    }

    if (e.key == OIS::KC_6)
    {
        if(m_showHUD == true)
        {
            hideHUD();
        }
        else
        {
            showHUD();
        }
    }

    if(e.key == OIS::KC_F7)
    {
        Ogre::Viewport * vp = m_viewport;
        Ogre::CompositorManager::getSingleton().addCompositor(vp,"DHBloom");
        Ogre::CompositorManager::getSingleton().setCompositorEnabled(vp,"DHBloom",true);
    }
    if(e.key == OIS::KC_F8)
    {
        Ogre::Viewport * vp = m_viewport;
        Ogre::CompositorManager::getSingleton().setCompositorEnabled(vp,"DHBloom",false);
    }

    if(e.key == OIS::KC_F5)
    {
        TimeManager::getSingleton().modDayScale(2);
    }

    if(e.key == OIS::KC_F6)
    {
        TimeManager::getSingleton().modDayScale(-2);
    }

    if(e.key == OIS::KC_F9)
    {
        Vector3 newItemPos = m_Player->getPosition();
        newItemPos.y += 10;

        Weapon* newWeapon = new Weapon();
        EntityManager::getSingletonPtr()->addLocalEntity(newWeapon);
        newWeapon->initialize("Iron Shortsword","weapon_sword.png", 10, 50, "SteelShortsword.mesh", "SteelShortsword.mesh", Vector3(1.0,1.0,1.0), Vector3(1.0,1.0,1.0), 20);
        newWeapon->initializeWeapon(0, 2, 10, 10, 10);
        newWeapon->loadLocalItem(newItemPos);
        
        //Item * newItem = static_cast<Item *>( GameManager::getSingletonPtr()->SpawnLocalEntity("GameItem",0) );
        //newItem->initialize("Iron Shortsword","SwordIcon", 10, 50, "SteelShortsword.mesh", "SteelShortsword.mesh", Vector3(1.0,1.0,1.0), Vector3(1.0,1.0,1.0), 20);
        //newItem->loadLocalItem(newItemPos);

        //newItem->m_displayMesh = "battle_axe_001.mesh";
        //newItem->m_collisionMesh = "battle_axe_001_collision.mesh";

        //string Mesh, string CollisionMesh, Vector3 displayScale = Vector3(1,1,1), Vector3 collisionScale = Vector3(1,1,1), float mass = 10.0, Vector3 displayOffset = Vector3(0,0,0), Vector3 collisionOffset = Vector3(0,0,0), Vector3 centerOfGravity = Vector3(0,0,0)

        //newItem->initialize("AxeIcon", 10, 50, "battle_axe_001.mesh", "battle_axe_001_collision.mesh", Vector3(50.0,50.0,50.0), Vector3(50.0,50.0,50.0), 20);

        //newItem->loadLocalItem("battle_axe_001.mesh", "battle_axe_001_collision.mesh", Vector3(50.0,50.0,50.0), newItemPos, 200, Vector2(3,60));
    }

    if(e.key == OIS::KC_F12)
    {
        string filename("screenshot_");
        string suffix(".png");

        bool exists = true;
        int num = 0;

        while(exists)
        {
            string fullFilename;
            char theNum[15];

            itoa(++num, theNum, 10);
            fullFilename = filename + theNum + suffix;

            FILE * checkfile;
            checkfile = fopen (fullFilename.c_str(), "r");
            if (checkfile == NULL)
            {
                GameManager::getSingletonPtr()->getMainWindow()->writeContentsToFile(fullFilename);
                exists = false;
            }
            else
            {
                fclose(checkfile);
            }

            if(num > 1000)
            {
                return;
            }
        }
    }

    if (e.key == OIS::KC_GRAVE || e.key == OIS::KC_F11)
    {
        pause();
        pushState(ConsoleState::getInstance());
    }
}


bool PlayState::frameStarted(const FrameEvent& evt)
{
    Real MoveFactor = 80.0 * evt.timeSinceLastFrame;    //The time scaler to keep time constant
    timeScaler = MoveFactor * TimeManager::getSingleton().getTimeFactor();

    grassWaveSin += timeScaler * 1.0;
    std::vector<GameEntity*>::iterator it;

    //TESTING: Update mouse position
    /*if(m_Player != NULL && mCamera != NULL)
    {
        OIS::MouseState mouse_state = InputManager::getSingletonPtr()->getMouse()->getMouseState();
        float mouse_speed_multiplier = 0.13;
        m_Player->m_PlayerHeadNode->yaw(Degree(-mouse_state.X.rel * mouse_speed_multiplier));
        mCamera->pitch(Degree(-mouse_state.Y.rel * mouse_speed_multiplier));
    }*/

    float timeOfDay = TimeManager::getSingleton().update(MoveFactor);

    if(isOutdoor)
    {
        if(m_sky != 0)
        {
            m_sky->Update(m_Player->getPosition(),timeOfDay);
        }

        //Play ambient outdoor sounds here
        curAmbientNoiseWait += 1.0 * timeScaler;
        if(curAmbientNoiseWait > ambientNoiseWait + ambientNoiseRandom)
        {
            curAmbientNoiseWait = 0;

            int twentyfour_hour = TimeManager::getSingletonPtr()->get24FormatHour();
            if( twentyfour_hour <= 6 || twentyfour_hour >= 18)
            {
                ambientNoiseRandom = rand() % 100;
                Vector3 randPos;
                randPos.x = (rand() % 10000) - 7000;
                randPos.y = (rand() % 2000);
                randPos.z = (rand() % 10000) - 7000;

                //It's the night, play night ambient sounds here
                int numSounds = 8;
                int thisRandomSound = rand() % numSounds;

                if(thisRandomSound <= 1)
                {
                    SoundManager *sm = SoundManager::getSingletonPtr();
                    sm->manageSound(sm->playPositionedSound("WolfHowl.wav", 
                        randPos + m_Player->getPosition(), 1.8, 10000, false));
                }
                else if(thisRandomSound > 1)
                {
                    SoundManager *sm = SoundManager::getSingletonPtr();
                    sm->manageSound(sm->playPositionedSound("cricket.wav",
                        randPos + m_Player->getPosition(), 1.8, 10000, false));
                }
            }
            else
            {
                
                ambientNoiseRandom = rand() % 300;
                ambientNoiseWait = 100;
                Vector3 randPos;
                randPos.x = (rand() % 10000) - 5000;
                randPos.y = (rand() % 5000);
                randPos.z = (rand() % 10000) - 5000;

                //It's the daytime, play day ambient sounds here
                int numSounds = 3;
                int thisRandomSound = rand() % numSounds;

                if(thisRandomSound <= 1)
                {
                    //SoundManager *sm = SoundManager::getSingletonPtr();
                    //sm->manageSound(sm->playPositionedSound("Bird1.wav",randPos+m_Player->m_pos,0.9,10000,false));
                }
                else if(thisRandomSound == 2)
                {
                    //SoundManager *sm = SoundManager::getSingletonPtr();
                    //sm->manageSound(sm->playPositionedSound("Bird2.wav",randPos+m_Player->m_pos,0.9,10000,false));
                }
            }
        }
    }

    //Do time of day callbacks here
    if (TimeManager::getSingleton().isEvening())
    {
        this->startEvening();
    }
    else
    {
        if (TimeManager::getSingleton().isMorning())
        {
            this->startMorning();
        }
    }

    // Update physics
    PhysicsManager::getSingletonPtr()->update();

    //Update Global Entities
    EntityManager::getSingletonPtr()->updateGlobalEntities(timeScaler);

    //Update Local Entities
    EntityManager::getSingletonPtr()->updateLocalEntities(timeScaler);

    //Update Quests
    QuestManager::getSingletonPtr()->Update(timeScaler);

    //mInputDevice->capture();

    if(m_playerLight != NULL && m_camera != NULL)
    {
        m_playerLight->setPosition(m_Player->getPosition());
    }

    if(this->m_WorldTerrain != 0 && m_Player != 0)
    {
        Vector3 pos = m_Player->getPosition();
        m_WorldTerrain->Update(pos.x, pos.z, timeScaler);
    }

    //Show a message window if we need to
    if(MessageState::getInstance()->hasMessage() == true)
    {
        pause();
        m_Player->resetMovementEvents();
        pushState(MessageState::getInstance());
    }

    // Update HUD
    m_GUI->injectFrameStarted(evt);

    return true;
}


bool PlayState::frameEnded(const FrameEvent& evt)
{
    if (PauseState::isQuitRequested())
    {
        popState();
        return false;
    }
    if (ConsoleState::isQuitRequested())
    {
        popState();
        return false;
    }

    //Update HUD
    if(this->m_Player != 0 && m_showHUD == true)
    {
        updateHUD();
    }

    if(MessageState::getInstance()->didJustReturn())
    {
        MessageState::getInstance()->clearDidReturn();
        this->returnedFromMessageBox(MessageState::getInstance()->getReturnedValue(), MessageState::getInstance()->getMessageType());
    }

    PythonManager::getSingleton().Update();

    return true;
}


void PlayState::mouseMoved(const OIS::MouseEvent &e)
{
    if(m_Player != NULL && m_camera != NULL)
    {
        float mouse_speed_multiplier = 0.13;

        // yaws the player head node according to mouse relative movement on x axis (left and right)
        // camera is attached to the player head node
        m_Player->m_PlayerHeadNode->yaw(Degree(-e.state.X.rel * mouse_speed_multiplier));

        // pitches the camera according to the mouse relative movement.
        m_camera->pitch(Degree(-e.state.Y.rel * mouse_speed_multiplier));

        // clamp pitch to prevent flipping
        Ogre::Degree pitchAngle;
        Ogre::Real pitchAngleSign;

        // Angle of rotation around the X-axis.
        pitchAngle = (2 * Ogre::Degree(Ogre::Math::ACos(m_camera->getOrientation().w).valueDegrees()));

        // Just to determine the sign of the angle we pick up above, the
        // value itself does not interest us.
        pitchAngleSign = m_camera->getOrientation().x;

        // Limit the pitch between -90 degress and +90 degrees
        if (pitchAngle > Ogre::Degree(90.0f))
        {
            if (pitchAngleSign > 0)
            {
                // Set orientation to 90 degrees on X-axis.
                m_camera->setOrientation(Ogre::Quaternion(Ogre::Math::Sqrt(0.5f), Ogre::Math::Sqrt(0.5f), 0, 0));
            }
            else
            {
                if (pitchAngleSign < 0)
                {
                    // Sets orientation to -90 degrees on X-axis.
                    m_camera->setOrientation(Ogre::Quaternion(Ogre::Math::Sqrt(0.5f), -Ogre::Math::Sqrt(0.5f), 0, 0));
                }
            }
        }
    }
}


void PlayState::mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
    if(m_Player != NULL)
    {
        m_Player->startAttackEvent();
    }
}

void PlayState::mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
    if(m_Player != NULL)
    {
        m_Player->endAttackEvent();
    }
}

//Gui Event Functions
void PlayState::showHUD()
{
    m_showHUD = true;

    if (!m_hud)
    {
        m_hud = new GuiHUD(m_GUI);
        m_hud->setStatsVisible(true);
        m_hud->setup();
    }
    else
    {
        m_hud->show();
    }

    if(m_Player->m_equippedWeaponIcon != "")
    {
        switchHUDEquippedWeaponIcon(m_Player->m_equippedWeaponIcon);
    }

/*
        mGUISystem->setDefaultMouseCursor((CEGUI::utf8*)"TaharezLook", (CEGUI::utf8*)"MouseArrow");
        mGUISystem->setDefaultFont((CEGUI::utf8*)"Tahoma-12");
        mEditorGuiSheet = CEGUI::WindowManager::getSingleton().createWindow((CEGUI::utf8*)"DefaultWindow", (CEGUI::utf8*)"PlaySheet");  
        mGUISystem->setGUISheet(mEditorGuiSheet);

        float menuScale = 0.1f;
        CEGUI::Window* status_bg = CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/StaticImage", (CEGUI::utf8*)"HUD_Status_bg");
        mEditorGuiSheet->addChildWindow(status_bg);

        //status_bg->setMaximumSize(CEGUI::Size(10.0f, 10.0f));
//      status_bg->setPosition(CEGUI::UVector2(CEGUI::UDim(0.015f,0),CEGUI::UDim(0.89f,0)));    //simg->setPosition(CEGUI::Point(0.29f, 0.19f));
//      status_bg->setSize(CEGUI::UVector2(CEGUI::UDim((2.0f * menuScale),0), CEGUI::UDim(1.0f * menuScale,0)));
//      status_bg->setFrameEnabled(false);
//      status_bg->setBackgroundEnabled(false);
        status_bg->setProperty((CEGUI::utf8*)"Image", (CEGUI::utf8*)"set:GUI1 image:HUDStatus");
        //status_bg->setImage((CEGUI::utf8*)"GUI1", (CEGUI::utf8*)"HUDStatus");
        status_bg->setVisible(setVisible);

        CEGUI::Window* healthBar = CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/StaticImage", (CEGUI::utf8*)"HUD_HealthBar");
        CEGUI::Window* magicBar = CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/StaticImage", (CEGUI::utf8*)"HUD_MagicBar");
        CEGUI::Window* fatigueBar = CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/StaticImage", (CEGUI::utf8*)"HUD_FatigueBar");
        CEGUI::Window* dayOfMonth = CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/StaticText", (CEGUI::utf8*)"HUD_Debug");
        CEGUI::Window* info = CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/StaticText", (CEGUI::utf8*)"HUD_Info");

        status_bg->addChildWindow(healthBar);
        status_bg->addChildWindow(magicBar);
        status_bg->addChildWindow(fatigueBar);

        mEditorGuiSheet->addChildWindow(dayOfMonth);
        mEditorGuiSheet->addChildWindow(info);

        CEGUI::Window* weapon_icon = CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/StaticImage", (CEGUI::utf8*)"HUD_weaponIcon");
        mEditorGuiSheet->addChildWindow(weapon_icon);

//      weapon_icon->setMaximumSize(CEGUI::Size(10.0f, 10.0f));
        weapon_icon->setPosition(CEGUI::UVector2(CEGUI::UDim(0.21f,0), CEGUI::UDim(0.885f,0))); //simg->setPosition(CEGUI::Point(0.29f, 0.19f));
//      weapon_icon->setSize(CEGUI::Size(0.7f * menuScale, (0.7f * menuScale) * 1.33));
//      weapon_icon->setFrameEnabled(false);
//      weapon_icon->setBackgroundEnabled(false);

        if(m_Player->m_equippedWeaponIcon != "")
        {
            //weapon_icon->setImage((CEGUI::utf8*)"InventoryIcons", (CEGUI::utf8*)m_Player->m_equippedWeaponIcon.c_str());
            weapon_icon->setVisible(setVisible);
        }
        else
        {
            weapon_icon->setVisible(false);
        }

        float statusBarScale = 0.3f;
//      healthBar->setMaximumSize(CEGUI::Size(10.0f, 10.0f));
//      healthBar->setPosition(CEGUI::UVector2(0.075f, 0.09f)); //simg->setPosition(CEGUI::Point(0.29f, 0.19f));
//      healthBar->setSize(CEGUI::UVector2(2.625f * statusBarScale, 0.5f * statusBarScale));
//      healthBar->setFrameEnabled(false);
//      healthBar->setBackgroundEnabled(false);
        healthBar->setProperty((CEGUI::utf8*)"Image", (CEGUI::utf8*)"set:GUI1 image:HUDHealthBar");
        //healthBar->setImage((CEGUI::utf8*)"GUI1", (CEGUI::utf8*)"HUDHealthBar");

//      magicBar->setMaximumSize(CEGUI::Size(10.0f, 10.0f));
//      magicBar->setPosition(CEGUI::UVector2(0.075f, 0.34f));  //simg->setPosition(CEGUI::Point(0.29f, 0.19f));
//      magicBar->setSize(CEGUI::UVector2(2.625f * statusBarScale, 0.5f * statusBarScale));
//      magicBar->setFrameEnabled(false);
//      magicBar->setBackgroundEnabled(false);
        magicBar->setProperty((CEGUI::utf8*)"Image", (CEGUI::utf8*)"set:GUI1 image:HUDMagicBar");
        //magicBar->setImage((CEGUI::utf8*)"GUI1", (CEGUI::utf8*)"HUDMagicBar");

//      fatigueBar->setMaximumSize(CEGUI::Size(10.0f, 10.0f));
//      fatigueBar->setPosition(CEGUI::UVector2(0.075f, 0.56f));    //simg->setPosition(CEGUI::Point(0.29f, 0.19f));
//      fatigueBar->setSize(CEGUI::UVector2(2.625f * statusBarScale, 0.5f * statusBarScale));
//      fatigueBar->setFrameEnabled(false);
//      fatigueBar->setBackgroundEnabled(false);
        fatigueBar->setProperty((CEGUI::utf8*)"Image", (CEGUI::utf8*)"set:GUI1 image:HUDFatigueBar");
        //fatigueBar->setImage((CEGUI::utf8*)"GUI1", (CEGUI::utf8*)"HUDFatigueBar");

//      info->setMaximumSize(CEGUI::Size(10.0f, 10.0f));
//      info->setPosition(CEGUI::Point(0.37f, 0.878f)); //simg->setPosition(CEGUI::Point(0.29f, 0.19f));
//      info->setSize(CEGUI::Size(0.63, 0.105));
//      info->setBackgroundEnabled(false);
//      info->setFrameEnabled(false);
        info->setText((CEGUI::utf8*)"");
        info->setVerticalAlignment(CEGUI::VerticalAlignment::VA_TOP);
        //info->setVerticalFormatting(CEGUI::StaticText::TopAligned);
//      info->setTextColours(CEGUI::colour(1.0 ,215.0 / 255.0, 56.0 / 255.0));

//      dayOfMonth->setMaximumSize(CEGUI::Size(10.0f, 10.0f));
//      dayOfMonth->setPosition(CEGUI::UVector2(0.005f, 0.000f));   //simg->setPosition(CEGUI::Point(0.29f, 0.19f));
//      dayOfMonth->setSize(CEGUI::UVector2(0.7f, 0.25f));
//      dayOfMonth->setBackgroundEnabled(false);
//      dayOfMonth->setFrameEnabled(false);
        dayOfMonth->setText((CEGUI::utf8*)"HI");
        dayOfMonth->setVerticalAlignment(CEGUI::VerticalAlignment::VA_TOP);
//      dayOfMonth->setTextColours(CEGUI::colour(1.0 ,215.0 / 255.0, 56.0 / 255.0));
    }*/
}

void PlayState::updateHUD()
{
    float healthScaler = m_Player->getHealth() / m_Player->getTotalHealth();
    float magicScaler = m_Player->getMagicka() / m_Player->getTotalMagicka();
    float fatigueScaler = m_Player->getFatigue() / m_Player->getTotalFatigue();

    if (healthScaler  < 0.0)    healthScaler    = 0.0;
    if (magicScaler   < 0.0)    magicScaler     = 0.0;
    if (fatigueScaler < 0.0)    fatigueScaler   = 0.0;
    if (healthScaler  > 1.0)    healthScaler    = 1.0;
    if (magicScaler   > 1.0)    magicScaler     = 1.0;
    if (fatigueScaler > 1.0)    fatigueScaler   = 1.0;

    char terrX[15];
    char terrY[15];
    sprintf(terrX, "%d", m_WorldTerrain->currentZoneX);
    sprintf(terrY, "%d", m_WorldTerrain->currentZoneY);

    String theDate;
    TimeManager::getSingleton().getDateStr(theDate);

    Location * thisLoc = GameWorld::getSingletonPtr()->getLocation(m_WorldTerrain->currentZoneX,m_WorldTerrain->currentZoneY);
    if(thisLoc != 0)
    {
        theDate += thisLoc->m_Name;
    }
    theDate += "\n";
    theDate += terrX;
    theDate += ",";
    theDate += terrY;

    m_hud->setDebug(theDate);
    m_hud->setInfo(worldInfo);

    static String currFps = "Current FPS: ";
    static String avgFps = "Average FPS: ";
    static String bestFps = "Best FPS: ";
    static String worstFps = "Worst FPS: ";
    static String tris = "Triangle Count: ";
    static String batches = "Batch Count: ";
    const RenderTarget::FrameStats& stats = m_root->getAutoCreatedWindow()->getStatistics();
    String statsInfo;
    statsInfo = currFps + StringConverter::toString(stats.lastFPS) + "\n";
    statsInfo += avgFps + StringConverter::toString(stats.avgFPS) + "\n";
    statsInfo += bestFps + StringConverter::toString(stats.bestFPS) + " " + StringConverter::toString(stats.bestFrameTime) + " ms\n";
    statsInfo += worstFps + StringConverter::toString(stats.worstFPS) + " " + StringConverter::toString(stats.worstFrameTime)+ " ms\n";
    statsInfo += tris + StringConverter::toString(stats.triangleCount) + "\n";
    statsInfo += batches + StringConverter::toString(stats.batchCount) + "\n";
    m_hud->setStats(statsInfo);

    m_hud->setHealth(healthScaler);
    m_hud->setMagica(magicScaler);
    m_hud->setFatigue(fatigueScaler);

    m_hud->update();
}

void PlayState::hideHUD()
{
    m_showHUD = false;

    m_hud->hide();
}

void PlayState::switchHUDEquippedWeaponIcon(string newIcon)
{
    m_hud->switchWeapon(newIcon);
}

void PlayState::startMorning()
{
    if(isOutdoor)
    {
        playOutdoorMusic();
    }
}

void PlayState::startNoon()
{

}

void PlayState::startEvening()
{
    if(isOutdoor)
    {
        playOutdoorMusic();
    }
}

void PlayState::startMidnight()
{

}

void PlayState::playOutdoorMusic()
{
    // TODO: make music configurable, disable for now -- Archwyrm
    return;
    int twentyfour_hour = TimeManager::getSingletonPtr()->get24FormatHour();

    if(twentyfour_hour < 6 || twentyfour_hour >= 18) //If it's nighttime
    {
        SoundManager::getSingletonPtr()->playStreamingOutdoorAmbient("");
        SoundManager::getSingletonPtr()->playStreamingMusic("Daggerfall_'Night'.ogg");
    }
    else //If it's daytime
    {
        SoundManager::getSingletonPtr()->playStreamingOutdoorAmbient("grassland3.ogg");
        SoundManager::getSingletonPtr()->playStreamingMusic("RobertHood_DF-TheVirtuous.ogg");
    }
}

void PlayState::addStringToHUDInfo(string newString)
{
    //Add the new string to the top
    worldInfo = newString + "\n" + worldInfo;
}

void PlayState::returnedFromMessageBox(int returnedVal, string mesType)
{
    MessageState * lastState = MessageState::getInstance();
    
    //Call up a function in a script
    if(lastState->getDoScriptFile())
    {
        //Calling up a message box pauses the last running script, unpause it here
        if(PythonManager::getSingletonPtr()->isWaiting() == true)
        {
            PythonManager::getSingletonPtr()->resumeThread();
        }
    }
}
