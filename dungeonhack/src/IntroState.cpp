#include "DungeonHack.h"
#include "TimeManager.h"
#include "IntroState.h"
#include "PlayState.h"

#include "GameWorld.h"
#include "xmlConfig/xmlConfig.h"
#include "SoundManager.h"

using namespace Ogre;

IntroState IntroState::mIntroState;

void IntroState::enter()
{
    mExitGame = false;
    m_root = Root::getSingletonPtr();

    m_sceneMgr = m_root->createSceneManager(ST_GENERIC);
    m_sceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_NONE);

    m_camera = m_sceneMgr->createCamera("GameCamera");
    m_camera->setNearClipDistance(10.0);

    m_viewport = m_root->getAutoCreatedWindow()->addViewport(m_camera);
    m_viewport->setBackgroundColour(ColourValue(0.0, 0.0, 0.0));
    m_camera->setFOVy(Radian(Real(0.92)));
    m_camera->setAspectRatio(Real(m_viewport->getActualWidth()) / Real(m_viewport->getActualHeight())); 

    //Read config settings
    xmlConfig * config = xmlConfig::Instance();
    config->setConfigFile("config.xml");
    fog_thickness = config->getConfigVariableFloat("fogThickness");

    Real cam_x = config->getConfigVariableFloat("intro_camera_position_x");
    Real cam_y = config->getConfigVariableFloat("intro_camera_position_y");
    Real cam_z = config->getConfigVariableFloat("intro_camera_position_z");
    m_camera->setDirection(-1,0,0);
    m_camera->setPosition(cam_x,cam_y,cam_z);

    m_sky = 0;
    m_intro_terrain = 0;

    InitLandscape();

    GameManager::getSingletonPtr()->m_Player = 0;

    time = (LENGTH_OF_MONTH * 10) + LENGTH_OF_DAY / 6.0;

    SoundManager::getSingletonPtr()->playStreamingOutdoorAmbient("grassland3.ogg");
    //SoundManager::getSingletonPtr()->playStreamingMusic("RobertHood_DF-TheVirtuous.ogg");
}

void IntroState::exit()
{
    m_sceneMgr->clearScene();
    m_sceneMgr->destroyAllCameras();
    m_root->getAutoCreatedWindow()->removeAllViewports();
}

void IntroState::pause()
{
}

void IntroState::resume()
{
}

void IntroState::keyPressed(const OIS::KeyEvent &e)
{
    if(e.key == OIS::KC_J)
    {
        Vector3 this_position = m_camera->getPosition();
        this_position.y += 100.0;
        m_camera->setPosition(this_position);
    }

    if(e.key == OIS::KC_K)
    {
        Vector3 this_position = m_camera->getPosition();
        this_position.y -= 100.0;
        m_camera->setPosition(this_position);
    }

    if(e.key == OIS::KC_UP)
    {
        Vector3 this_position = m_camera->getPosition();
        this_position.z += 100.0;
        m_camera->setPosition(this_position);
    }

    if(e.key == OIS::KC_DOWN)
    {
        Vector3 this_position = m_camera->getPosition();
        this_position.z -= 100.0;
        m_camera->setPosition(this_position);
    }

    if(e.key == OIS::KC_LEFT)
    {
        Vector3 this_position = m_camera->getPosition();
        this_position.x += 100.0;
        m_camera->setPosition(this_position);
    }

    if(e.key == OIS::KC_RIGHT)
    {
        Vector3 this_position = m_camera->getPosition();
        this_position.x -= 100.0;
        m_camera->setPosition(this_position);
    }

    if (e.key == OIS::KC_ESCAPE)
    {
        mExitGame = true;
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
        fog_thickness -= 0.000005;
        m_sky->SetFogThickness(fog_thickness);
    }
    if(e.key == OIS::KC_B)
    {
        fog_thickness += 0.000005;
        m_sky->SetFogThickness(fog_thickness);
    }

    if(e.key == OIS::KC_S)
    {
        xmlConfig * config = xmlConfig::Instance();
        config->setConfigFile("config.xml");

        Vector3 save_position = m_camera->getPosition();

        String x_pos = Ogre::StringConverter::toString(save_position.x, 10);
        String y_pos = Ogre::StringConverter::toString(save_position.y, 10);
        String z_pos = Ogre::StringConverter::toString(save_position.z, 10);


        config->saveConfigVariable("intro_camera_position_x",x_pos);
        config->saveConfigVariable("intro_camera_position_y",y_pos);
        config->saveConfigVariable("intro_camera_position_z",z_pos);
    }
}

void IntroState::mouseMoved(const OIS::MouseEvent &e)
{
    if(m_camera != 0)
    {
        float mouse_speed_multiplier = 0.13;
        m_camera->yaw(Degree(-e.state.X.rel * mouse_speed_multiplier));
        m_camera->pitch(Degree(-e.state.Y.rel * mouse_speed_multiplier));
    }
}

void IntroState::mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id) { }

void IntroState::mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id) { }

void IntroState::keyReleased(const OIS::KeyEvent &e)
{
    if (e.key == OIS::KC_SPACE)
    {
        changeState(PlayState::getInstance());
    }
}

bool IntroState::frameStarted(const FrameEvent& evt)
{
    Real move_factor = 80.0 * evt.timeSinceLastFrame;

    if(m_sky != 0)
        m_sky->Update(m_camera->getPosition(),(int)time % LENGTH_OF_DAY);

    std::vector<GameEntity*>::iterator it;
    //Update Global Entities
    for (it = GameManager::getSingletonPtr()->mGameEntities.begin(); it != GameManager::getSingletonPtr()->mGameEntities.end(); ++it)
    {
        if(*it)
        {
            GameEntity * thisEntity = *it;
            if(thisEntity != 0)
                thisEntity->Update(move_factor);
        }
    }

    //Update Local Entities
    for (it = GameManager::getSingletonPtr()->mLocalGameEntities.begin(); it != GameManager::getSingletonPtr()->mLocalGameEntities.end(); ++it)
    {
        if(*it)
        {
            GameEntity * thisEntity = *it;
            if(thisEntity != 0)
                thisEntity->Update(move_factor);

            if(thisEntity)
            {
                if(thisEntity->requestDelete)
                {
                    GameManager::getSingletonPtr()->DeleteEntity(thisEntity);
                    //*it = 0;

                    GameManager::getSingletonPtr()->mLocalGameEntities.erase(it);
                    --it;
                }
            }
        }
    }

    if(m_intro_terrain != 0)
        m_intro_terrain->Update(m_camera->getPosition().x,m_camera->getPosition().z, move_factor);

    return true;
}

bool IntroState::frameEnded(const FrameEvent& evt)
{
    if (mExitGame)
        return false;

    return true;
}

void IntroState::InitLandscape()
{
    int x_tile = 14;
    int y_tile = 15;
    int terrain_tile_size = 512;
    float centerOffset = (30 * terrain_tile_size) / 2.0;

    m_sky = new Sky();
    m_sky->Init(x_tile, y_tile, terrain_tile_size, centerOffset, m_sceneMgr);
    m_sky->SetFogThickness(fog_thickness);

    Ogre::Vector3 Offset = Vector3((x_tile + 0.5) * (64 * terrain_tile_size), 8000, (y_tile + 0.5) * (64 * terrain_tile_size));

    m_intro_terrain = new GameTerrain(x_tile, y_tile, terrain_tile_size,this->m_sceneMgr);

    //m_camera->setPosition(Offset);
}
