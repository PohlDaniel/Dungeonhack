#include "DungeonHack.h"
#include "GeometryNode.h"
#include "PhysicsBody.h"

GeometryNode::GeometryNode(String name, String meshName, Vector3 position,
        Quaternion orientation, SceneNode* locationNode, Vector3 scale)
{
    m_displayRepresentation = new VisualRepresentation(name, locationNode);
    m_displayRepresentation->loadMesh(meshName, scale);

    // Set these manually since Bullet will never update the MotionState for static bodies
    m_displayRepresentation->m_displaySceneNode->setPosition(position);
    m_displayRepresentation->m_displaySceneNode->setOrientation(orientation);

    m_physicsBody = new PhysicsBody(*m_displayRepresentation, 0, position,
        orientation, ST_TRIMESH);
}

GeometryNode::~GeometryNode()
{
    delete m_physicsBody;
    delete m_displayRepresentation;
}
