#include "DungeonHack.h"
#include "GameConfig.h"

#include "xmlConfig.h"
#ifdef TIXML_USE_STL
    #include <iostream>
    #include <sstream>
    using namespace std;
#else
    #include <stdio.h>
#endif


/**
    Constructor
*/
GameConfiguration::GameConfiguration()
{
    /* Init xmlConfig */
    xmlConfig::Instance()->setConfigFile("config.xml");

    initTypes();

    /*
        default settings
    */

    /* From config.xml */
    setInteger("firstRun",1);
    setInteger("doGrass", 1);
    setInteger("grassWave", 1);
    setInteger("doWaterReflection", 1);
    setInteger("doBloom", 0);
    setString("bloomTechnique", "HDR");
    setString("testTerrainDefinition", "grassland_terrain.xml");
    setString("defaultFireMaterial", "Emmiters/TorchFire");

    setFloat("fogThickness", 0.000022);
    setInteger("gameStartTileX", 14);
    setInteger("gameStartTileY", 15);
    setInteger("gameStartLocInterior", 1);
    setInteger("gameStartLocStartMarker", 1);
    setFloat("intro_camera_position_x", 3900);
    setFloat("intro_camera_position_y", 2220);
    setFloat("intro_camera_position_z", 6800);
    setString("gameDataPath", "../GameData/");
    setFloat("gravity", 192.913);

    setInteger("treePageSize", 2000);
    setInteger("treeModelDistance", 8000);
    setInteger("treeModelFadeDistance", 11000);
    setInteger("treeImposterDistance", 40000);
    setInteger("treeImposterFadeDistance", 45000);

    setInteger("noX11KeyboardGrab", 1);
    setInteger("noX11MouseGrab", 0);

    setInteger("enableMusic", 1);
    setInteger("enableAmbient", 1);

    /* Internal */
    setFloat("speed", 1.0);
    setFloat("dayScale", 0.8);
    setFloat("musicVolume", 1.0); // 0.5;
    setFloat("ambientVolume", 1.0); // 0.3;

    setInteger("enableGuiConfig", 0.0);
}


/**
    List of values and types
*/
void GameConfiguration::initTypes()
{
    /* From config.xml */
    m_expected["firstRun"] = VARIANT_INTEGER;
    m_expected["doGrass"] = VARIANT_INTEGER;
    m_expected["grassWave"] = VARIANT_INTEGER;
    m_expected["doWaterReflection"] = VARIANT_INTEGER;
    m_expected["doBloom"] = VARIANT_INTEGER;
    m_expected["bloomTechnique"] = VARIANT_STRING;
    m_expected["testTerrainDefinition"] = VARIANT_STRING;
    m_expected["defaultFireMaterial"] = VARIANT_STRING;

    m_expected["fogThickness"] = VARIANT_FLOAT;
    m_expected["gameStartTileX"] = VARIANT_INTEGER;
    m_expected["gameStartTileY"] = VARIANT_INTEGER;
    m_expected["gameStartLocInterior"] = VARIANT_INTEGER;
    m_expected["gameStartLocStartMarker"] = VARIANT_INTEGER;
    m_expected["intro_camera_position_x"] = VARIANT_FLOAT;
    m_expected["intro_camera_position_y"] = VARIANT_FLOAT;
    m_expected["intro_camera_position_z"] = VARIANT_FLOAT;
    m_expected["gameDataPath"] = VARIANT_STRING;
    m_expected["gravity"] = VARIANT_FLOAT;

    m_expected["treePageSize"] = VARIANT_INTEGER;
    m_expected["treeModelDistance"] = VARIANT_INTEGER;
    m_expected["treeModelFadeDistance"] = VARIANT_INTEGER;
    m_expected["treeImposterDistance"] = VARIANT_INTEGER;
    m_expected["treeImposterFadeDistance"] = VARIANT_INTEGER;

    m_expected["noX11KeyboardGrab"] = VARIANT_INTEGER;
    m_expected["noX11MouseGrab"] = VARIANT_INTEGER;

    m_expected["enableMusic"] = VARIANT_INTEGER;
    m_expected["enableAmbient"] = VARIANT_INTEGER;

    /* Internal, not saved */
    //m_expected["speed"] = VARIANT_FLOAT;
    //m_expected["dayScale"] = VARIANT_FLOAT;
    //m_expected["musicVolume"] = VARIANT_FLOAT;
    //m_expected["ambientVolume"] = VARIANT_FLOAT;

    m_expected["enableGuiConfig"] = VARIANT_INTEGER;
}


bool GameConfiguration::load()
{
    xmlConfig* config = xmlConfig::Instance();
    int i;
    float f;
    string s;

    std::map<string, int>::iterator cur = m_expected.begin();
    for (; cur != m_expected.end(); cur++)
    {
        std::pair<string, int> p = *cur;
        config->clearErrorState();
        switch (p.second)
        {
            case VARIANT_INTEGER:
                i = config->getConfigVariableInteger(p.first);
                if (!config->getErrorState())
                    setInteger(p.first, i);
                break;

            case VARIANT_FLOAT:
                f = config->getConfigVariableFloat(p.first);
                if (!config->getErrorState())
                    setFloat(p.first, f);
                break;

            case VARIANT_STRING:
                s = config->getConfigVariable(p.first);
                if (!config->getErrorState())
                    setString(p.first, s);
                break;
        }
    }

    delete config;
    return true;
}


bool GameConfiguration::save()
{
    xmlConfig* config = xmlConfig::Instance();
    std::map<string, int>::iterator cur = m_expected.begin();
    bool res = true;

    while (cur != m_expected.end() && res)
    {
        std::pair<string, int> p = *cur;
        switch (p.second)
        {
            case VARIANT_INTEGER:
                res = config->saveConfigVariableInteger(p.first, getInteger(p.first));
                break;

            case VARIANT_FLOAT:
                res = config->saveConfigVariableFloat(p.first, getFloat(p.first));
                break;

            case VARIANT_STRING:
                res = config->saveConfigVariable(p.first, getString(p.first));
                break;
        }
    }
    return res;
}


GameConfiguration::~GameConfiguration()
{
}
