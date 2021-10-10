/*
    Header file for Entity Spawn requests

    FIXME: This is ugly code, need to rewrite after 0.10 !
*/

#ifndef _ENTITY_LOADER
#define _ENTITY_LOADER

#include <string>
using namespace std;
#include <OgreVector3.h>
using namespace Ogre;


/* Entity types */
const int ENTITY_LOAD_WEAPON = 1;

/*
    Entity Request definition
*/
typedef struct _ENTITY_LOADING_REQUEST
{
   int type; // required
   string name; // required

   /* Weapon specific - FIXME for equipment only */
   string inventoryIcon; 
   float weight;
   float worth;
   string Mesh;
   string collisionMesh;
   Vector3 displayScale;
   Vector3 collisionScale;
   float mass;
   Vector3 displayOffset;
   Vector3 collisionOffset;
   Vector3 centerOfGravity;

   /* Weapon specific, second stage */
   int weaponType;
   float damage;
   float reach;
   float curCondition;
   float maxCondition;

   /* FIXME: we can't handle fire entity here, because the created entity is
      immediatly used by the owner (i.e. the script in the annoying case).
      Just pray that it won't crash the Render as the Weapon did :( */

   /* FIXME: there is no spawn of monsters / NPC yet, since it is done by quests.
      But maybe it could be needed later ... */
}
EntityLoadingRequest;


/*
    The loader class
*/
class EntityLoader
{
public:
    EntityLoader() {}
    ~EntityLoader() {}

    void addRequest(EntityLoadingRequest* req);
    void update();

protected:
    std::list<EntityLoadingRequest> queue;
};


#endif // _ENTITY_LOADER
