#include "DungeonHack.h"
#include "GameLight.h"
#include "GameManager.h"


GameLight::GameLight()
{
    m_displayRepresentation = new VisualRepresentation("GameLight");

    flickerAlpha = 1.0;
    flickerUpdateTime = 0;

    m_doFlicker = false;
}

void GameLight::Update(float MoveFactor)
{
    if(m_doFlicker)
    {
        if(m_Light)
        {
            flickerUpdateTime += 0.3 * MoveFactor;

            if(flickerUpdateTime >= 1.0)
            {
                flickerUpdateTime = 0;
                flickerAlpha = ((rand()% 40) / 200.0) + 0.80;
            }

            m_LightColor = m_NewLightFlickerColor;
            m_NewLightFlickerColor = (m_StartLightColor * flickerAlpha);
            float flickerTimeScaler = (flickerUpdateTime / 1.0);
            float originalColour = 1.0 - flickerTimeScaler;

            m_Light->setDiffuseColour((m_LightColor * originalColour) + (m_NewLightFlickerColor * flickerTimeScaler));

        }

    //IsLightVisible();
    }
    }

bool GameLight::IsLightVisible()
{
    return false;
}

void GameLight::InitializeLight(Ogre::Light * newLight, Vector3 Position, SceneNode * rootNode)
{
    if(m_displayRepresentation != NULL)
    {
        m_displayRepresentation->m_displaySceneNode = rootNode->createChildSceneNode();
        if(m_displayRepresentation->m_displaySceneNode != NULL)
        {
            m_Light = newLight;
            m_LightColor = newLight->getDiffuseColour();
            m_NewLightFlickerColor = m_LightColor;
            m_StartLightColor = newLight->getDiffuseColour();

            this->m_displayRepresentation->m_displaySceneNode->attachObject(newLight);
        }
        setPosition(Position);
    }
}

void GameLight::setLightColour(Ogre::ColourValue newColour)
{
    if(this->m_displayRepresentation != 0 && m_Light != 0)
    {
        m_LightColor = newColour;
        m_StartLightColor = newColour;

        m_Light->setDiffuseColour(m_LightColor);
        m_Light->setSpecularColour(m_LightColor);
    }
}

void GameLight::setDoFlicker(bool flicker)
{
    m_doFlicker = flicker;
}

void GameLight::setFlickerSpeed(float newFlickerSpeed)
{
    flickerSpeed = newFlickerSpeed;
}
