#ifndef GameLight_H
#define GameLight_H

#include "GameObject.h"

class GameLight : public VisibleEntity
{
public:
    GameLight();

    void InitializeLight(Ogre::Light * newLight, Vector3 Position, SceneNode * rootNode);
    virtual void Update(float MoveFactor);
    bool IsLightVisible();
    void setDoFlicker(bool flicker);
    void setFlickerSpeed(float flickerSpeed);
    void setLightColour(Ogre::ColourValue newColour);

    Light * m_Light;
    ColourValue m_StartLightColor;
    ColourValue m_NewLightFlickerColor;
    ColourValue m_LightColor;

protected:
    bool wasVisible;
    float UpdateFrequency;
    float lastUpdate;

    bool m_doFlicker;

    float m_range;
    float m_constant;
    float m_linear;
    float m_quadratic;

    float flickerAlpha;
    float flickerUpdateTime;
    float flickerSpeed;
};

#endif // GameLight_H
