#include "DungeonHack.h"
#include "TerrainType.h"

TerrainDefType::TerrainDefType(string fileName)
{
    loadTerrainType(fileName);
}

void TerrainDefType::loadTerrainType(string fileName)
{
    TiXmlDocument doc( fileName );
    TiXmlElement* tNodeElement = 0;

    m_numToMake = 0;
    m_numFoliage = 0;
    m_totalFoliageWeights = 0;

    if(doc.LoadFile())
    {
        tNodeElement = doc.FirstChildElement();
        tNodeElement = tNodeElement->FirstChildElement("terrain");
        string numToMake = tNodeElement->Attribute("numToMake");
        if(numToMake != "")
        {
            m_numToMake = atoi(numToMake.c_str());  //Find out how many foliage items to generate
        }

        if(tNodeElement->Attribute("baseGroundMaterial"))
        {
            m_baseGroundMaterial = tNodeElement->Attribute("baseGroundMaterial");
        }
        else
        {
            m_baseGroundMaterial = "Terrain/Grass1";
        }

        tNodeElement = tNodeElement->FirstChildElement("foliage");

        while(tNodeElement != NULL) //Loop through foliage definitions
        {
            string mesh = "";
            string colMesh = "";
            int density = 1;
            float scale = 15;
            float yOffset = -15;
            bool doImposter = false;
            float imposterScale = 1;
            int radius = 0;
            string imposterStr = "";
            float viewDistance = -1;
            float imposterViewDistance = -1;

            float colHeight = 200;
            float colRadius = 10;
            bool doCollision;

            if(tNodeElement->Attribute("mesh"))
            {
                mesh = tNodeElement->Attribute("mesh");
            }

            if(tNodeElement->Attribute("density"))
            {
                string densityStr = tNodeElement->Attribute("density");
                density = atoi(densityStr.c_str());
                //MessageBox(0,densityStr.c_str(),"Loaded Foliage Def Density",0);
            }

            if(tNodeElement->Attribute("scale"))
            {
                string scaleStr = tNodeElement->Attribute("scale");
                scale = atof(scaleStr.c_str());
            }

            if(tNodeElement->Attribute("yOffset"))
            {
                string yOffsetStr = tNodeElement->Attribute("yOffset");
                yOffset = atof(yOffsetStr.c_str());
            }

            if(tNodeElement->Attribute("imposter"))
            {
                imposterStr = tNodeElement->Attribute("imposter");
                doImposter = true;
            }

            if(tNodeElement->Attribute("imposterScale"))
            {
                string imposterScaleStr = tNodeElement->Attribute("imposterScale");
                imposterScale = atof(imposterScaleStr.c_str());
            }

            if(tNodeElement->Attribute("radius"))
            {
                string radiusStr = tNodeElement->Attribute("radius");
                radius = atoi(radiusStr.c_str());
            }

            if(tNodeElement->Attribute("colMesh"))
            {
                colMesh = tNodeElement->Attribute("colMesh");
                doCollision = true;
            }

            if(tNodeElement->Attribute("collisionRadius"))
            {
                string radiusStr = tNodeElement->Attribute("collisionRadius");
                colRadius = atof(radiusStr.c_str());
                doCollision = true;
            }

            if(tNodeElement->Attribute("collisionHeight"))
            {
                string radiusStr = tNodeElement->Attribute("collisionHeight");
                colHeight = atof(radiusStr.c_str());
                doCollision = true;
            }

            if(tNodeElement->Attribute("viewDistance"))
            {
                string tempString = tNodeElement->Attribute("viewDistance");
                viewDistance = atof(tempString.c_str());
            }

            if(tNodeElement->Attribute("imposterViewDistance"))
            {
                string tempString = tNodeElement->Attribute("imposterViewDistance");
                imposterViewDistance = atof(tempString.c_str());
            }

            TerrainFoliageDef* newFoliageDef = new TerrainFoliageDef();
            newFoliageDef->m_baseScale = scale;
            newFoliageDef->m_density = density;
            newFoliageDef->m_doImposter = doImposter;
            newFoliageDef->m_imposterScale = imposterScale;
            newFoliageDef->m_Mesh = mesh;
            newFoliageDef->m_yOffset = yOffset;
            newFoliageDef->m_Radius = radius;
            newFoliageDef->m_growAroundTotalDensity = 0;
            newFoliageDef->m_numGrowAroundDefs = 0;
            newFoliageDef->m_growAroundMin = 0;
            newFoliageDef->m_growAroundMax = 0;
            newFoliageDef->m_imposter = imposterStr;
            newFoliageDef->m_colHeight = colHeight;
            newFoliageDef->m_colRadius = colRadius;
            newFoliageDef->m_doCol = doCollision;
            newFoliageDef->m_imposterViewDistance = imposterViewDistance;
            newFoliageDef->m_colMesh = colMesh;
            newFoliageDef->m_viewDistance = viewDistance;

            this->m_foliageDefList.push_back(newFoliageDef);

            m_totalFoliageWeights += density;
            m_numFoliage++;

            //Generate grow around objects.
            TiXmlElement* tGrowAround = 0;
            tGrowAround = tNodeElement->FirstChildElement("generateAround");

            if(tGrowAround != 0)
            {
                if(tGrowAround->Attribute("numMin"))
                {
                    newFoliageDef->m_growAroundMin = atoi(tGrowAround->Attribute("numMin"));
                }
                if(tGrowAround->Attribute("numMax"))
                {
                    newFoliageDef->m_growAroundMax = atoi(tGrowAround->Attribute("numMax"));
                }

                tGrowAround = tGrowAround->FirstChildElement("foliage");
                while(tGrowAround != NULL)  //Loop through foliage definitions. Gee, haven't we done this somewhere?
                {
                    string growAroundMesh = "";
                    string growAroundColMesh = "";
                    int growAroundDensity = 1;
                    float growAroundScale = 15;
                    float growAroundYOffset = -15;
                    bool growAroundDoImposter = false;
                    float growAroundImposterScale = 1;
                    int growAroundRadius = 100;
                    float growAroundFoliageRadius;
                    string growAroundImposter = "";
                    float growAroundImposterViewDistance = -1;
                    float growAroundViewDistance = -1;

                    float growAroundColHeight = 200;
                    float growAroundColRadius = 10;
                    bool growAroundDoCollision = true;

                    if(tGrowAround->Attribute("mesh"))
                    {
                        growAroundMesh = tGrowAround->Attribute("mesh");
                        //MessageBox(0,mesh.c_str(),"Loaded Foliage Def Mesh",0);
                    }

                    if(tGrowAround->Attribute("density"))
                    {
                        string densityStr = tGrowAround->Attribute("density");
                        growAroundDensity = atoi(densityStr.c_str());
                        //MessageBox(0,densityStr.c_str(),"Loaded Foliage Def Density",0);
                    }

                    if(tGrowAround->Attribute("scale"))
                    {
                        string scaleStr = tGrowAround->Attribute("scale");
                        growAroundScale = atof(scaleStr.c_str());
                    }

                    if(tGrowAround->Attribute("yOffset"))
                    {
                        string yOffsetStr = tGrowAround->Attribute("yOffset");
                        growAroundYOffset = atof(yOffsetStr.c_str());
                    }

                    if(tGrowAround->Attribute("imposter"))
                    {
                        growAroundImposter = tGrowAround->Attribute("imposter");
                        growAroundDoImposter = true;
                    }

                    if(tGrowAround->Attribute("imposterScale"))
                    {
                        string imposterScaleStr = tGrowAround->Attribute("imposterScale");
                        growAroundImposterScale = atof(imposterScaleStr.c_str());
                    }

                    if(tGrowAround->Attribute("radius"))
                    {
                        string radiusStr = tGrowAround->Attribute("radius");
                        growAroundRadius = atoi(radiusStr.c_str());
                    }

                    if(tGrowAround->Attribute("radiusAround"))
                    {
                        string radiusStr = tGrowAround->Attribute("radiusAround");
                        growAroundFoliageRadius = atof(radiusStr.c_str());
                    }

                    if(tGrowAround->Attribute("colMesh"))
                    {
                        growAroundColMesh = tGrowAround->Attribute("colMesh");
                        growAroundDoCollision = true;
                    }

                    if(tGrowAround->Attribute("collisionRadius"))
                    {
                        string radiusStr = tGrowAround->Attribute("collisionRadius");
                        growAroundColRadius = atof(radiusStr.c_str());
                        growAroundDoCollision = true;
                    }

                    if(tGrowAround->Attribute("collisionHeight"))
                    {
                        string heightStr = tGrowAround->Attribute("collisionHeight");
                        growAroundColHeight = atof(heightStr.c_str());
                        growAroundDoCollision = true;
                    }

                    if(tGrowAround->Attribute("viewDistance"))
                    {
                        string tempString = tGrowAround->Attribute("viewDistance");
                        growAroundViewDistance = atof(tempString.c_str());
                    }

                    if(tGrowAround->Attribute("imposterViewDistance"))
                    {
                        string tempString = tGrowAround->Attribute("imposterViewDistance");
                        growAroundImposterViewDistance = atof(tempString.c_str());
                    }

                    TerrainFoliageDef* newGrowAroundFoliageDef = new TerrainFoliageDef();
                    newGrowAroundFoliageDef->m_baseScale = growAroundScale;
                    newGrowAroundFoliageDef->m_density = growAroundDensity;
                    newGrowAroundFoliageDef->m_doImposter = growAroundDoImposter;
                    newGrowAroundFoliageDef->m_imposterScale = growAroundImposterScale;
                    newGrowAroundFoliageDef->m_Mesh = growAroundMesh;
                    newGrowAroundFoliageDef->m_yOffset = growAroundYOffset;
                    newGrowAroundFoliageDef->m_Radius = growAroundRadius;
                    newGrowAroundFoliageDef->m_GrowAroundRadius = growAroundFoliageRadius;
                    newGrowAroundFoliageDef->m_imposter = growAroundImposter;
                    newGrowAroundFoliageDef->m_colHeight = growAroundColHeight;
                    newGrowAroundFoliageDef->m_colRadius = growAroundColRadius;
                    newGrowAroundFoliageDef->m_doCol = growAroundDoCollision;
                    newGrowAroundFoliageDef->m_imposterViewDistance = growAroundImposterViewDistance;
                    newGrowAroundFoliageDef->m_colMesh = growAroundColMesh;
                    newGrowAroundFoliageDef->m_viewDistance = growAroundViewDistance;

                    newFoliageDef->m_growAroundTotalDensity += growAroundDensity;
                    newFoliageDef->m_numGrowAroundDefs++;
                    newFoliageDef->m_foliageGrowAroundList.push_back(newGrowAroundFoliageDef);

                    tGrowAround = tGrowAround->NextSiblingElement();
                }
            }

            tNodeElement = tNodeElement->NextSiblingElement();
        }
    }
}
