#ifndef Gamestats_H
#define Gamestats_H

class Item;

struct ActorStats
{
    float agility;
    float adj_agility;

    float strength;
    float adj_strength;
    
    float endurance;
    float adj_endurance;

    float speed;
    float adj_speed;
};

struct ActorStatus
{
    float health;
    float agility;
    float magicka;
};

struct EquippedItems
{
    Item * weaponArm;
    Item * offArm;

    Item * body;
    Item * legs;
};

#endif
