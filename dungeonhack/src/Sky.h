#ifndef Sky_H
#define Sky_H

#include <Ogre.h>
#include "GameManager.h"

using namespace Ogre;


class GameSun;
class GameMoon;

class Sky : public Ogre::RenderQueueListener
{
public:
    Sky();
    void Init(int iX, int iY, int terrain_tile_size, float center_offset, SceneManager * scene_manager, Camera * camera = 0, MovablePlane * clip_plane = 0);
    void Update(Vector3 position, double time_of_day);
    void SetFogThickness(float thickness) { fog_thickness = thickness; }
    void SetCloudMaterialName(const String & name);

    virtual void renderQueueStarted(Ogre::uint8 queue_id, const Ogre::String& invocation, bool& skipThisQueue);
    virtual void renderQueueEnded(Ogre::uint8 queue_id, const Ogre::String& invocation, bool& repeatThisQueue);

    SceneManager * m_scene_manager;
    Camera * m_camera;
    MovablePlane * m_clip_plane;

    Light * m_sunLight;
    Image * fogColorImage;
    Image * sunColorImage;
    ColourValue m_ambientLightColour;

    SceneNode * m_SunHolder;
    SceneNode * m_MoonHolder;
    SceneNode * m_SunNode;
    SceneNode * m_MoonNode;
    SceneNode * m_cloud_node;

    GameSun * theSun;
    GameMoon * moon1;

    Entity* mCloudDomeEntity;
    Entity* mNightDomeEntity;
    Entity* mSkyDomeEntity;
    Entity* mFogRingEntity;
    SceneNode* mSkyNode;

    Material* skyMat;
    Material * m_cloudMaterial;

    float cloud_scaler;
    float fog_thickness;
    int dayPos;
};

#endif
