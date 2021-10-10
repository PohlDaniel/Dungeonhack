#include "DungeonHack.h"
#include "TerrainGenerator.h"
#include "PerlinNoise.h"
#include "TerrainType.h"
#include "GameWorld.h"

void TerrainGenerator::createTileMesh(float * mapArray, int width, int height, String ID,int numLODs,float scale, float heightScale)
{
    MeshPtr msh;
    try
    {
        msh = MeshManager::getSingleton().createManual(ID, "General");
    }
    catch(Exception e)
    {
        return;
    }

    int numFaces = (width) * (height);  //int numFaces = (width - 1) * (height - 1);
    int numVertices = numFaces * 6;

    //num of terrain tiles = (width - 1) * (height - 1)
    //num of terrain faces = terrain tiles * 2
    ///The vertices are stored here, as 3 groups of floats for each vertex
    float * vertices = new float[(8*2*3*numVertices)];

    ///The index buffer, 3 for each face
    unsigned short * faces = new unsigned short[(numFaces) * 6];

    int indexNum = 0;
    int vertNum = 0;

    int timesThrough = 0;

    float terrainTexSize = 0.25;

    int terrainWidth = width;
    int terrainHeight = height;

    int imageHeight = height + 1;
    //int imageWidth = width + 1;

    float minY = 0;
    float maxY = 0;

    Vector3 * VectorLocs = new Vector3 [(terrainWidth * terrainHeight) * 6];
    Vector3 * Vectors = new Vector3 [(terrainWidth * terrainHeight) * 6];
    Vector3 * FinalVects = new Vector3 [((terrainWidth + 1) * (terrainHeight + 1))];

    int curVector = 0;

    //Generate Normals
    for(int x = 0; x < terrainWidth; x++)
    {
        for(int y = 0; y < terrainHeight; y++)
        {

            Vector3 p1 = Vector3(x * scale,mapArray[y * (imageHeight) + x] * heightScale,y * scale);
            Vector3 p2 = Vector3((x + 1) * scale, mapArray[(y+1) * (imageHeight) + (x+1)] * heightScale , (y + 1) * scale);
            Vector3 p3 = Vector3((x + 1) * scale,mapArray[y * (imageHeight) + (x+1)] * heightScale,y * scale);
            Vector3 p4 = Vector3(x * scale,mapArray[(y + 1) * (imageHeight) + x] * heightScale,(y+1) * scale);

            //Face 1

            Vector3 v1 = p2 - p1;
            Vector3 v2 = p3 - p1;

            Vector3 tmp = v1.crossProduct(v2);
            Vector3 tmp2 = tmp;

            Vector3 result = tmp / tmp2.normalise();

            //v1
            VectorLocs[curVector] = p1;
            Vectors[curVector++] = result;

            //v2
            VectorLocs[curVector] = p2;
            Vectors[curVector++] = result;

            //v3
            VectorLocs[curVector] = p3;
            Vectors[curVector++] = result;


            //Face 2

            v1 = p4 - p1;
            v2 = p2 - p1;

            tmp = v1.crossProduct(v2);
            tmp2 = tmp;

            result = tmp / tmp2.normalise();

            //v1
            VectorLocs[curVector] = p1;
            Vectors[curVector++] = result;

            //v2
            VectorLocs[curVector] = p2;
            Vectors[curVector++] = result;

            //v3
            VectorLocs[curVector] = p4;
            Vectors[curVector++] = result;

        }
    }

    //Smooth Normals
    for(int x = 0; x <= terrainWidth; x++)
    {
        for(int y = 0; y <= terrainHeight; y++)
        {
            FinalVects[y * (imageHeight) + x] = Vector3(0,0,0);
        }
    }

    for(int x = 0; x <= terrainWidth; x++)
    {
        for(int y = 0; y <= terrainHeight; y++)
        {
            int numFound = 0;

            Vector3 tempVect(0,0,0);
            Vector3 thisVect;

            FinalVects[y * (imageHeight) + x] = tempVect;

            thisVect.x = x * scale;
            thisVect.z = y * scale;
            thisVect.y = mapArray[y * (imageHeight) + x] * heightScale;

            Vector3 testVect;

            int startX = x - 1;
            int endX = x + 1;

            int startY = y - 1;
            int endY = y + 1;

            if(startX < 0)
                startX = 0;

            if(startY < 0)
                startY = 0;

            if(endX > terrainWidth)
                endX = terrainWidth;

            if(endY > terrainHeight)
                endY = terrainHeight;

            for(int nx = startX; nx < endX; nx++)
            {
                for(int ny = startY; ny < endY; ny++)
                {
                    for(int i = 0; i < 6; i++)
                    {
                        if(thisVect == VectorLocs[nx * ((imageHeight - 1) * 6) + (ny * 6) + i])
                        {
                            numFound++;
                            tempVect += Vectors[nx * ((imageHeight - 1) * 6) + (ny * 6) + i];
                        }
                    }
                }
            }

            if(numFound > 1)
                tempVect /= numFound;

            tempVect.normalise();

            FinalVects[y * (imageHeight) + x] = tempVect;
        }
    }

    //Create Mesh
    int curVertex = 0;
    for(int x = 0; x < terrainWidth; x++)
    {
        for(int y = 0; y < terrainHeight; y++)
        {
            timesThrough++;
            //Face 1

            //v1
            float curHeight = mapArray[y * (imageHeight) + x] * heightScale;

            faces[indexNum] = indexNum;
            ++indexNum;
            vertices[vertNum++] = x * scale;
            vertices[vertNum++] = curHeight;    //Height
            vertices[vertNum++] = y * scale;

            Vector3 myVect = FinalVects[y * (imageHeight) + x];

            if(myVect == Vector3(0,0,0))
            {
                myVect = Vectors[curVertex];
            }

            vertices[vertNum++] = myVect.x;
            vertices[vertNum++] = myVect.y;
            vertices[vertNum++] = myVect.z;

            vertices[vertNum++] = (float)((x) / terrainTexSize);
            vertices[vertNum++] = (float)((y) / terrainTexSize);

            if(curHeight < minY)
                minY = curHeight;

            if(curHeight > maxY)
                maxY = curHeight;

            curVertex++;


            //v3
            curHeight = mapArray[(y+1) * (imageHeight) + (x+1)] * heightScale;

            faces[indexNum] = indexNum;
            ++indexNum;
            vertices[vertNum++] = (x + 1) * scale;
            vertices[vertNum++] = mapArray[(y+1) * (imageHeight) + (x+1)] * heightScale;    //Height
            vertices[vertNum++] = (y + 1) * scale;

            myVect = FinalVects[(y+1) * (imageHeight) + (x+1)];

            if(myVect == Vector3(0,0,0))
            {
                myVect = Vectors[curVertex];
            }

            vertices[vertNum++] = myVect.x;
            vertices[vertNum++] = myVect.y;
            vertices[vertNum++] = myVect.z;

            vertices[vertNum++] = (float)((x + 1) / terrainTexSize);
            vertices[vertNum++] = (float)((y + 1) / terrainTexSize);

            if(curHeight < minY)
                minY = curHeight;

            if(curHeight > maxY)
                maxY = curHeight;

            curVertex++;

            //v2
            curHeight = mapArray[y * (imageHeight) + (x+1)] * heightScale;

            faces[indexNum] = indexNum;
            ++indexNum;
            vertices[vertNum++] = (x + 1) * scale;
            vertices[vertNum++] = mapArray[y * (imageHeight) + (x+1)] * heightScale;    //Height
            vertices[vertNum++] = y * scale;

            myVect = FinalVects[y * (imageHeight) + (x+1)];

            if(myVect == Vector3(0,0,0))
            {
                myVect = Vectors[curVertex];
            }

            vertices[vertNum++] = myVect.x;
            vertices[vertNum++] = myVect.y;
            vertices[vertNum++] = myVect.z;

            vertices[vertNum++] = (float)((x + 1) / terrainTexSize);
            vertices[vertNum++] = (float)(y / terrainTexSize);

            if(curHeight < minY)
                minY = curHeight;

            if(curHeight > maxY)
                maxY = curHeight;

            curVertex++;

            //Face 2
            //v1
            curHeight = mapArray[y * (imageHeight) + x] * heightScale;

            faces[indexNum] = indexNum;
            ++indexNum;
            vertices[vertNum++] = x * scale;
            vertices[vertNum++] = mapArray[y * (imageHeight) + x] * heightScale;    //Height
            vertices[vertNum++] = y * scale;

            myVect = FinalVects[y * (imageHeight) + x];

            if(myVect == Vector3(0,0,0))
            {
                myVect = Vectors[curVertex];
            }

            vertices[vertNum++] = myVect.x;
            vertices[vertNum++] = myVect.y;
            vertices[vertNum++] = myVect.z;

            vertices[vertNum++] = (float)(x / terrainTexSize);
            vertices[vertNum++] = (float)(y / terrainTexSize);

            if(curHeight < minY)
                minY = curHeight;

            if(curHeight > maxY)
                maxY = curHeight;

            curVertex++;

            //v3
            curHeight = mapArray[(y+1) * (imageHeight) + x] * heightScale;

            faces[indexNum] = indexNum;
            ++indexNum;
            vertices[vertNum++] = (x) * scale;
            vertices[vertNum++] = mapArray[(y+1) * (imageHeight) + x] * heightScale;    //Height
            vertices[vertNum++] = (y + 1) * scale;

            myVect = FinalVects[(y+1) * (imageHeight) + x];

            if(myVect == Vector3(0,0,0))
            {
                myVect = Vectors[curVertex];
            }

            vertices[vertNum++] = myVect.x;
            vertices[vertNum++] = myVect.y;
            vertices[vertNum++] = myVect.z;

            vertices[vertNum++] = (float)((x) / terrainTexSize);
            vertices[vertNum++] = (float)((y + 1) / terrainTexSize);

            if(curHeight < minY)
                minY = curHeight;

            if(curHeight > maxY)
                maxY = curHeight;

            curVertex++;

            //v2
            curHeight = mapArray[(y+1) * (imageHeight) + (x+1)] * heightScale;

            faces[indexNum] = indexNum;
            ++indexNum;
            vertices[vertNum++] = (x + 1) * scale;
            vertices[vertNum++] = mapArray[(y+1) * (imageHeight) + (x+1)] * heightScale;    //Height
            vertices[vertNum++] = (y + 1) * scale;

            myVect = FinalVects[(y+1) * (imageHeight) + (x+1)];

            if(myVect == Vector3(0,0,0))
            {
                myVect = Vectors[curVertex];
            }

            vertices[vertNum++] = myVect.x;
            vertices[vertNum++] = myVect.y;
            vertices[vertNum++] = myVect.z;

            vertices[vertNum++] = (float)((x + 1) / terrainTexSize);
            vertices[vertNum++] = (float)((y + 1) / terrainTexSize);

            if(curHeight < minY)
                minY = curHeight;

            if(curHeight > maxY)
                maxY = curHeight;

            curVertex++;

        }
    }

    delete[] Vectors;
    delete[] VectorLocs;
    delete[] FinalVects;

    SubMesh* sub = msh->createSubMesh();
    sub->vertexData = new VertexData();
    sub->useSharedVertices = false;
    sub->vertexData->vertexStart = 0;
    sub->vertexData->vertexCount = vertNum / 8;

    /// Create declaration (memory format) of vertex data
    VertexDeclaration* decl = sub->vertexData->vertexDeclaration;
    size_t offset = 0;
    decl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
    offset += VertexElement::getTypeSize(VET_FLOAT3);
    decl->addElement(0, offset, VET_FLOAT3, VES_NORMAL);
    offset += VertexElement::getTypeSize(VET_FLOAT3);
    decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES);
    offset += VertexElement::getTypeSize(VET_FLOAT2);

    /// Allocate vertex buffer of the requested number of vertices (vertexCount) 
    /// and bytes per vertex (offset)
    HardwareVertexBufferSharedPtr vbuf = 
    HardwareBufferManager::getSingleton().createVertexBuffer(
        offset, sub->vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);

    vbuf->writeData(0, vbuf->getSizeInBytes(), vertices, true);

    /// Set vertex buffer binding so buffer 0 is bound to our vertex buffer
    VertexBufferBinding* bind = sub->vertexData->vertexBufferBinding; 
    bind->setBinding(0, vbuf);

    /// Allocate index buffer of the requested number of vertices (ibufCount) 
    HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().
    createIndexBuffer(
        HardwareIndexBuffer::IT_16BIT, 
            numFaces * 6, 
            HardwareBuffer::HBU_STATIC_WRITE_ONLY);

    /// Upload the index data to the card
    ibuf->writeData(0, ibuf->getSizeInBytes(), faces, true);

    sub->indexData->indexBuffer = ibuf;
    sub->indexData->indexStart = 0;
    sub->indexData->indexCount = numFaces * 6;

    msh->_setBounds(AxisAlignedBox(0,minY,0,width * scale,maxY,height * scale));
    msh->_setBoundingSphereRadius((width * scale) * (Math::PI / 180));

    msh->load();
}

String TerrainGenerator::createTerrainTile(float * mapArray, int zoomLevel,int zoneX, int zoneY, int size, int tileSize, Ogre::SceneManager * mSceneMgr)
{
    String thisTileString;

    int zoom = Math::Pow(2,zoomLevel);

    TerrainGenerator::GeneratePerlinZonedTerrain(mapArray ,size+1 ,size+1 ,zoneX ,zoneY ,zoom ,0.2,0.65);

    //flatten land for a Location, if there is one
    checkLocation(mapArray,zoneX,zoneY,size,tileSize);

    char * zoneXString = new char[10];
    itoa(zoneX,zoneXString,10);

    char * zoneYString = new char[10];
    itoa(zoneY,zoneYString,10);

    char * myZoneString = new char[20];

    thisTileString = zoneXString;
    thisTileString += zoneYString;

    strcpy(myZoneString,zoneXString);
    strcat(myZoneString,zoneYString);

    delete[] zoneXString;
    delete[] zoneYString;

    Entity* thisEntity = 0;

    try
    {
        thisEntity = mSceneMgr->getEntity(thisTileString);
    }
    catch(Ogre::Exception e)
    {
        thisEntity = 0;

        TerrainGenerator::createTileMesh(mapArray,size,size,thisTileString,2,tileSize,70000);   //18000
        thisEntity = mSceneMgr->createEntity(thisTileString,thisTileString);
    }

    SceneNode* thisSceneNode = static_cast<SceneNode*>(mSceneMgr->getRootSceneNode()->createChild(thisTileString));
    thisSceneNode->attachObject(thisEntity);

    delete[] myZoneString;
    return thisTileString;
}

void TerrainGenerator::checkLocation(float * mapArray, int zoneX, int zoneY, int size, int tileSize)
{
    float heightScale = 70000;

    int center_x_pos = size / 2;
    int center_y_pos = size / 2;

    //Check if there is a location for this tile, and load it if we find one
    char zx[128];
    char zy[128];

    sprintf(zx, "%d", zoneX);   //%G for float, %d for int
    sprintf(zy, "%d", zoneY);

    const String zoneXS = zx;
    const String zoneYS = zy;
    String myZoneString = zoneXS + "," + zoneYS;

    Location * thisLoc = 0;
    thisLoc = GameWorld::getSingletonPtr()->gameLocations[myZoneString];

    if(thisLoc != 0)
    {
        float midHeight = mapArray[center_x_pos * (size+1) + center_y_pos]; // * heightScale;
        thisLoc->m_LocHeight = midHeight * heightScale;

        int flattenStart = size * thisLoc->m_radius;
        int flattenEnd = size * thisLoc->m_radius;

        flattenStart = (size / 2) - (flattenStart / 2.0);
        flattenEnd = (size / 2) + (flattenEnd / 2.0);

        //Clamp values, avoid tearing at edges
        if(flattenStart == 0)
            flattenStart = 1;

        int smoothSize = 3;
        int smoothStart = flattenStart - smoothSize;
        int smoothEnd = flattenEnd + smoothSize;

        //Clamp values, avoid tearing at edges
        if(smoothStart <= 0)
            smoothStart = 1;
        if(smoothEnd >= 64)
            smoothEnd = 64;

        int numLoops = smoothSize;

        //We found a Location! Flatten the ground here now if it wants to
        if(thisLoc->m_flattenGround == true)
        {
            for(int i = 0; i <= numLoops; i++)
            {
                //Do Tops/Bottoms of box
                int theStart = smoothStart + i;
                int theEnd = smoothEnd - i;

                for(int x = theStart+1; x < theEnd; x++)
                {
                    int y = theStart;
                    int by = theEnd;

                    //float thisFactor = (y - smoothStart) / (flattenStart - smoothStart);
                    float theFlattenFactor = (float)(i) / (float)numLoops;
                    float originalFactor = 1.0 - theFlattenFactor;

                    mapArray[x * (size+1) + y] = (mapArray[x * (size+1) + y] * originalFactor) + ((midHeight + ((rand() % 1000) / 20000000.0)) * theFlattenFactor);
                    mapArray[x * (size+1) + by] = (mapArray[x * (size+1) + by] * originalFactor) + ((midHeight + ((rand() % 1000) / 20000000.0)) * theFlattenFactor);
                }

                //Do sides of box
                for(int y = theStart; y <= theEnd; y++)
                {
                    int x = theStart;
                    int bx = theEnd;

                    //float thisFactor = (y - smoothStart) / (flattenStart - smoothStart);
                    float theFlattenFactor = (float)(i) / (float)numLoops;
                    float originalFactor = 1.0 - theFlattenFactor;

                    mapArray[x * (size+1) + y] = (mapArray[x * (size+1) + y] * originalFactor) + ((midHeight + ((rand() % 1000) / 20000000.0)) * theFlattenFactor);
                    mapArray[bx * (size+1) + y] = (mapArray[bx * (size+1) + y] * originalFactor) + ((midHeight + ((rand() % 1000) / 20000000.0)) * theFlattenFactor);
                }
            }

            for(int x = flattenStart; x <= flattenEnd; x++)
            {
                for(int y = flattenStart; y <= flattenEnd; y++)
                {
                    float xFactor = 1;
                    float yFactor = 1;

                    float theFlattenFactor = (xFactor + yFactor) / 2.0;
                    float originalFactor = 1.0 - theFlattenFactor;

                    mapArray[x * (size+1) + y] = (mapArray[x * (size+1) + y] * originalFactor) + ((midHeight + ((rand() % 1000) / 20000000.0)) * theFlattenFactor);
                }
            }
            //done flattening ground
        }
    }

}

void TerrainGenerator::GeneratePerlinZonedTerrain(float * mapArray, int width, int height, int zoneX, int zoneY, float Scale, float min, float max)
{
    perlinNoise newNoise;

    int xOffset = (zoneX * width) + Scale - (zoneX * 1);
    int yOffset = (zoneY * height) + Scale - (zoneY * 1);

    for(int x = 0 + xOffset; x < width + xOffset; x++)
    {
        for(int y = 0 + yOffset; y < height + yOffset; y++)
        {
            float curColValue = newPerlinNoise::turbulence(mapArray,x,y,16 * Scale,width,height,2,0.9) / 256.0;
            float curColValue2 = newPerlinNoise::turbulence(mapArray,x,y,64 * Scale,width,height,2,0.9) / 256.0; 

            float scaler = 1.0 / (0.9 - 0.425);

            curColValue = (curColValue / 10);
            curColValue2 = (curColValue2 - 0.425) * scaler;

            if(curColValue < 0)
                curColValue = 0;    //Clamp to 0

            if(curColValue2 < 0)
                curColValue2 = 0;   //Clamp to 0 and 1
            if(curColValue2 > 1)
                curColValue2 = 1;

            curColValue2 *= 1.1;
            curColValue += (curColValue2 * curColValue2);

            curColValue *= 1.8;
            //curColValue /= 0.5;

            if(curColValue <= 0.2)
            {

            }
            else if(curColValue <= 0.5)
            {
                //curColValue = (curColValue + 0.35) / 2.0; //for smooth valleys
            }
            else if(curColValue <= 0.58)
            {
                
            }
            else if(curColValue <= 0.60)
            {
                
            }
            else
            {
                
            }

            mapArray[(y - yOffset) * (height) + (x - xOffset)] = curColValue;
        }
    }
}

void TerrainGenerator::GeneratePerlinTerrain(float * mapArray, int width, int height)
{
    perlinNoise newNoise;
    for(int x = 0; x < width; x++)
    {
        for(int y = 0; y < height; y++)
        {
            mapArray[y * (height) + x] = (rand() % 256) / 256.0;
        }
    }

    int zoneX = 0;
    int zoneY = 0;

    float Scale = 16;
    int xOffset = (zoneX * width) + Scale;
    int yOffset = (zoneY * height) + Scale;

    for(int x = 0 + xOffset; x < width + xOffset; x++)
    {
        for(int y = 0 + yOffset; y < height + yOffset; y++)
        {
            mapArray[(y - yOffset) * (height) + (x - xOffset)] = newPerlinNoise::turbulence(mapArray,x,y,8 * Scale,width,height,2,0.9) / 256.0;
        }
    }
}

void TerrainGenerator::GenerateTerrain(float * mapArray, float x, float y, float width, float height, float c1, float c2, float c3, float c4, int terrainWidth, int terrainHeight, int rowPitch, int seedValue, int recursiveLevel)
{
    int nextRecLevel = recursiveLevel + 1;

    float Edge1, Edge2, Edge3, Edge4, Middle;
    float newWidth = width / 2;
    float newHeight = height / 2;

    float midDist = y - (terrainHeight / 2.0);
    Math::Abs(midDist);

    midDist = 1 - (midDist / (terrainHeight / 2.0));


    if (width > 2 || height > 2)
    {
        Middle = (c1 + c2 + c3 + c4) / 4.0;

        float max = (newWidth + newHeight) / (terrainWidth + terrainHeight) * 5.0;

        float result = (((rand() % 256) /256.0) - 0.5f) * (max);

        Middle += result * (midDist); //(newHeight / terrainHeight);

        Edge1 = (c1 + c2) / 2;  //Calculate the edges by averaging the two corners of each edge.
        Edge2 = (c2 + c3) / 2;
        Edge3 = (c3 + c4) / 2;
        Edge4 = (c4 + c1) / 2;
        
        //Make sure that the midpoint doesn't accidentally "randomly displaced" past the boundaries!
        if (Middle < 0)
        {
            Middle = 0;
        }
        else if (Middle > 1.0f)
        {
            Middle = 1.0f;
        }
        
        //Do the operation over again for each of the four new grids.           
        GenerateTerrain(mapArray, x, y, newWidth, newHeight, c1, Edge1, Middle, Edge4, terrainWidth, terrainHeight, rowPitch,  seedValue, nextRecLevel);
        GenerateTerrain(mapArray, x + newWidth, y, newWidth, newHeight, Edge1, c2, Edge2, Middle,  terrainWidth, terrainHeight, rowPitch, seedValue, nextRecLevel);
        GenerateTerrain(mapArray, x + newWidth, y + newHeight, newWidth, newHeight, Middle, Edge2, c3, Edge3,  terrainWidth, terrainHeight, rowPitch, seedValue, nextRecLevel);
        GenerateTerrain(mapArray, x, y + newHeight, newWidth, newHeight, Edge4, Middle, Edge3, c4,  terrainWidth, terrainHeight, rowPitch, seedValue, nextRecLevel);
    }
    else    //This is the "base case," where each grid piece is less than the size of a pixel.
    {
        //The four corners of the grid piece will be averaged and drawn as a single pixel.
        float c = (c1 + c2 + c3 + c4) / 4.0;
        
        //(int)x, (int)y
        mapArray[(int)y * (rowPitch) + (int)x] = c;
    }
}
