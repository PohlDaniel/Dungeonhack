#ifndef GEOMETRYNODE_H
#define GEOMETRYNODE_H

#include "GameObject.h"
#include "PhysicsBody.h"

using namespace Ogre;

/**
 * Pairs a rigid body and visual elements to make a piece of static scene geometry
 */
class GeometryNode
{
public:
    /**
     * Create a static piece of geometry
     * \param name Name of the piece
     * \param meshName Name of the mesh for display and collision
     * \param position Position of the piece
     * \param orientation Orientation of the piece
     * \param locationNode Parent SceneNode that this will be attached to
     * \param scale Scale of the piece
     */
    GeometryNode(String name, String meshName, Vector3 position,
            Quaternion orientation, SceneNode* locationNode,
            Vector3 scale = Vector3(1,1,1));
    ~GeometryNode();

private:
    PhysicsBody* m_physicsBody;
    VisualRepresentation* m_displayRepresentation;
};

#endif // GEOMETRYNODE_H
