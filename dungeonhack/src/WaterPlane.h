#ifndef WaterPlane_H
#define WaterPlane_H

#include <OgreSceneManager.h>
using namespace Ogre;

class ReflectionTextureListener : public RenderTargetListener
{
public:
    SceneManager * m_scene_manager;
    Camera * m_camera;
    MovablePlane * m_reflection_plane;
    MovablePlane * m_reflection_clip_plane;
    SceneNode * m_plane_node;
    SceneNode * m_sky_node;
    bool m_visible;

    ReflectionTextureListener();
    void setVisible(bool visible);
    void init(SceneManager * scene_manager, Camera * the_camera, MovablePlane * the_reflection_plane, SceneNode * plane_node);
    void preRenderTargetUpdate(const RenderTargetEvent& evt);
    void postRenderTargetUpdate(const RenderTargetEvent& evt);
};

class WaterPlane
{
public:
    WaterPlane();
    ~WaterPlane();
    void Init(SceneManager * scene_manager, Camera * the_camera);
    void Update(Vector3 position);
    void SetHeight(float height);
    float GetHeight();
    MovablePlane * getReflectionPlane();
    MovablePlane * getClipPlane();
    void setActive(bool visible);

private:
    bool m_visible;
    bool InitReflection(Camera * the_camera, MovablePlane * the_reflection_plane);
    ReflectionTextureListener m_reflection_listener;
    SceneManager * m_scene_manager;
    Entity* m_water_plane_entity;
    SceneNode* m_water_plane_node;
    Material * m_water_material;
    RenderTarget * rttTex;
    float m_water_height;
};

#endif
