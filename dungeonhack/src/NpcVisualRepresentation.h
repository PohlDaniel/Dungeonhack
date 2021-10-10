#ifndef NpcVisualRepresentation_H
#define NpcVisualRepresentation_H

#include <Ogre.h>
#include "GameObject.h"

class NpcVisualRepresentation : public VisualRepresentation
{
public:
    NpcVisualRepresentation(); // { m_displayMesh = NULL; m_displaySceneNode = NULL; m_displayEntity = NULL; };
    ~NpcVisualRepresentation();

    void attachEntityToSkeleton(Entity * newMesh, int position);
    void detachEntityFromSkeleton(int position);
    Entity * getEntityFromPosition(int position);

    Entity * attachPositions[9];
};

#endif
