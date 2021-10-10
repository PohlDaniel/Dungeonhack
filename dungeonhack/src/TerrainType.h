#ifndef TERRAINTYPE_H
#define TERRAINTYPE_H

#include <string>
using namespace std;
#include "tinyXML/tinyxml.h"

class TerrainFoliageDef;
class FoliageGrowAround;

class FoliageGrowAround
{
public:
    TerrainFoliageDef* m_foliageDef;
    int m_numMin;
    int m_numMax;
    float m_radius;
};

class TerrainFoliageDef
{
public:
    string m_Mesh;
    int m_density;
    float m_baseScale;
    float m_yOffset;
    bool m_doImposter;
    string m_imposter;
    float m_imposterScale;
    vector<TerrainFoliageDef*> m_foliageGrowAroundList;
    int m_Radius;
    int m_GrowAroundRadius;

    string m_colMesh;
    float m_colRadius;
    float m_colHeight;
    bool m_doCol;

    int m_growAroundMin;
    int m_growAroundMax;
    int m_numGrowAroundDefs;
    int m_growAroundTotalDensity;

    float m_imposterViewDistance;
    float m_viewDistance;
};

class TerrainDefType
{
public:
    TerrainDefType(string fileName);

    void loadTerrainType(string fileName);
    int m_totalFoliageWeights;
    int m_numFoliage;
    int m_numToMake;
    string m_baseGroundMaterial;

    vector<TerrainFoliageDef*> m_foliageDefList;
};

#endif
