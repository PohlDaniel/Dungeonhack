#ifndef fireEntity_H
#define fireEntity_H

#include "GameObject.h"
#include "GameLight.h"

class FireEntity : public PointEntity
{
public:
    FireEntity();
    ~FireEntity();

    void init(Vector3 pos, string flameEmitter, bool isBurning = true, float fuel = -1.0, float size = 1.0, Vector3 lightOffset = Vector3(0,0,0), bool makeLight = true);

    void ignite();
    void douse();

    virtual void Update(float MoveFactor);

    Vector3 m_lightOffset; //Used for torches and other lightsources that use fire to move the lights away from the wall
    bool m_isBurning;
    bool m_makeLight;
    float m_fireSize;
    float m_fireStrength;
    float m_yOffset;
    float m_fuel;
    string m_fireEmitterFile;

    //Fire particle effect
    GameEmitter * fireEmitter;
    MaterialPtr m_fireMaterial; //Use a smart pointer for the material so we don't have to worry about it so much

    //Light created by the fire
    Light * fireLight;
    GameLight * m_fireGameLight;
    GameCorona * m_corona;

    //Light info taken from a pre defined light in blender
    /*colourDiffuse r="1.000000" g="0.534611" b="0.090050"/>
        <colourSpecular r="1.000000" g="0.534611" b="0.090050"/>
        <lightAttenuation range="5000.0" constant="1.000000" linear="0.400000" quadratic="0.000000"/>*/


};

#endif
