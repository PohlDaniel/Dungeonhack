#include "DungeonHack.h"
#include "NpcVisualRepresentation.h"

NpcVisualRepresentation::NpcVisualRepresentation()
    : VisualRepresentation("NPC")
{

}

NpcVisualRepresentation::~NpcVisualRepresentation()
{

}

void NpcVisualRepresentation::attachEntityToSkeleton(Entity * newMesh, int position)
{
    assert(newMesh);
    assert(m_displaySceneNode);

    newMesh->setNormaliseNormals(true);
    m_displaySceneNode->attachObject(newMesh);
    m_displayEntity->shareSkeletonInstanceWith(newMesh);
}

void NpcVisualRepresentation::detachEntityFromSkeleton(int position)
{

}

Entity * NpcVisualRepresentation::getEntityFromPosition(int position)
{
    Entity * returnEntity = 0;
    return(returnEntity);
}
