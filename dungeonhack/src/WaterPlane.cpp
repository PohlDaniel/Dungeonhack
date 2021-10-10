#include "DungeonHack.h"
#include "WaterPlane.h"
#include "GameManager.h"
#include "GameConfig.h"


ReflectionTextureListener::ReflectionTextureListener()
{
    m_camera = 0;
    m_reflection_plane = 0;
    m_plane_node = 0;
    m_sky_node = 0;
    m_visible = true;
}

void ReflectionTextureListener::setVisible(bool visible)
{
    m_visible = visible;
    if(m_plane_node != 0)
        m_plane_node->setVisible(m_visible);
}

void ReflectionTextureListener::init(SceneManager * scene_manager, Camera * the_camera, MovablePlane * the_reflection_plane, SceneNode * plane_node)
{
    m_camera = the_camera;
    m_reflection_plane = the_reflection_plane;
    m_plane_node = plane_node;

    m_reflection_clip_plane = new Ogre::MovablePlane("WaterPlane_clip-plane");
    m_reflection_clip_plane->normal = Ogre::Vector3::UNIT_Y;
    m_reflection_clip_plane->d = 0;

    SceneNode * clip_node = m_plane_node->createChildSceneNode();
    clip_node->attachObject(m_reflection_clip_plane);
    clip_node->translate(Vector3(0,-2,0));

    m_sky_node = (SceneNode*)scene_manager->getRootSceneNode()->getChild("SkyNode");
}

void ReflectionTextureListener::preRenderTargetUpdate(const RenderTargetEvent& evt)
{
    if(m_camera != 0)
    {
        m_camera->enableCustomNearClipPlane(m_reflection_clip_plane);
        m_camera->enableReflection(m_reflection_plane);
    }

    if(m_plane_node != 0 && m_visible != false)
        m_plane_node->setVisible(false);

    if(m_sky_node != 0)
    {
        Vector3 pos = m_camera->getDerivedPosition();
        m_sky_node->setPosition(pos);
    }

}
void ReflectionTextureListener::postRenderTargetUpdate(const RenderTargetEvent& evt)
{
    if(m_camera != 0)
    {
        m_camera->disableCustomNearClipPlane();
        m_camera->disableReflection();
    }

    if(m_plane_node != 0 && m_visible != false)
        m_plane_node->setVisible(true);

    if(m_sky_node != 0)
    {
        Vector3 pos = m_camera->getDerivedPosition();
        m_sky_node->setPosition(pos);
    }
}

WaterPlane::WaterPlane()
{

}

void WaterPlane::Init(Ogre::SceneManager *scene_manager, Camera * the_camera)
{
    m_scene_manager = scene_manager;
    m_water_height = 1700;
    m_visible = true;
    rttTex = 0;

    MovablePlane *plane = new Ogre::MovablePlane("WaterPlane_plane");
    plane->normal = Ogre::Vector3::UNIT_Y;
    plane->d = 0;

    Ogre::MeshManager::getSingleton().createPlane("WaterPlane_mesh",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, *plane,
      200000, 200000, 1, 1, true, 1, 256, 256, Ogre::Vector3::UNIT_Z);

    m_water_plane_entity = scene_manager->createEntity("WaterPlane_entity", "WaterPlane_mesh");
    m_water_plane_entity->setMaterialName("Terrain/Water");
    m_water_plane_entity->setCastShadows(false);

    m_water_plane_node = m_scene_manager->getRootSceneNode()->createChildSceneNode();
    m_water_plane_node->attachObject(m_water_plane_entity);
    m_water_plane_node->attachObject(plane);

    if(InitReflection(the_camera, plane))
    {
        m_water_plane_entity->setMaterialName("Water/Reflection");
    }
    else
    {
        std::cerr << "WaterPlane: Reflection disabled, probably due to "
            << "unsupported shader." << std::endl;
    }
}

bool WaterPlane::InitReflection(Camera * the_camera, MovablePlane * the_reflection_plane)
{
    GameConfiguration* config = GameManager::getSingletonPtr()->getConfig();
    if(!config->getInteger("doWaterReflection"))
        return false;

    // Check prerequisites first
    const RenderSystemCapabilities* caps = Root::getSingleton().getRenderSystem()->getCapabilities();
    if (!caps->hasCapability(RSC_VERTEX_PROGRAM) || !(caps->hasCapability(RSC_FRAGMENT_PROGRAM)))
    {
        return false;
    }
    else
    {
        if (!GpuProgramManager::getSingleton().isSyntaxSupported("arbfp1") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("fp40") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("ps_2_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("ps_1_4")
            )
        {
            return false;
        }
    }

    TexturePtr mTexture;

    m_reflection_listener.init(m_scene_manager, the_camera, the_reflection_plane, m_water_plane_node);

    mTexture = TextureManager::getSingleton().createManual( "Reflection", 
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 
        256, 256, 0, PF_R8G8B8, TU_RENDERTARGET );
    rttTex = mTexture->getBuffer()->getRenderTarget();
    {
        Viewport *v = rttTex->addViewport( the_camera );
        MaterialPtr mat = MaterialManager::getSingleton().getByName("Water/Reflection");
        mat->getTechnique(0)->getPass(0)->getTextureUnitState(1)->setTextureName("Reflection");
        v->setOverlaysEnabled(false);
        rttTex->addListener(&m_reflection_listener);
    }

    return true;
}

WaterPlane::~WaterPlane()
{
    if (rttTex)
    {
        rttTex->removeListener(&m_reflection_listener);
    }
}

void WaterPlane::Update(Ogre::Vector3 position)
{
    m_water_plane_node->setPosition(position.x, m_water_height, position.z);
}

void WaterPlane::setActive(bool visible)
{
    m_visible = visible;
    m_water_plane_node->setVisible(m_visible);

    if(m_water_plane_entity != 0)
        m_water_plane_entity->setVisible(visible);

    if(rttTex != 0)
        rttTex->setActive(visible);
}

void WaterPlane::SetHeight(float height) { m_water_height = height; }
float WaterPlane::GetHeight() { return m_water_height; }

MovablePlane * WaterPlane::getReflectionPlane()
{
    return this->m_reflection_listener.m_reflection_plane;
}

MovablePlane * WaterPlane::getClipPlane()
{
    return this->m_reflection_listener.m_reflection_clip_plane;
}
