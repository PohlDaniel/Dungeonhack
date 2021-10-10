/*
    Game configuration placeholder
*/

#ifndef _GAME_CONFIG_H
#define _GAME_CONFIG_H

#include <map>
#include <string>

using namespace std;


#define VARIANT_UNDEF   0
#define VARIANT_INTEGER 1
#define VARIANT_FLOAT   2
#define VARIANT_STRING  3


/**
    Placeholder for data of variable type, used to store
    the configuration elements in a generic way.

    We assume that the data has been written in the good format,
    which means that if the reader use another, there's a coding fault.
*/
class Variant
{
public:
    Variant() : m_type(VARIANT_UNDEF) {}
    Variant(int i) : m_type(VARIANT_INTEGER), m_integer(i) {}
    Variant(float f) : m_type(VARIANT_FLOAT), m_float(f) {}
    Variant(string s) : m_type(VARIANT_STRING), m_string(s) {}
    ~Variant() {};

    int getType() { return m_type; }

    int getInteger() { assert(m_type == VARIANT_INTEGER); return m_integer; }

    float getFloat() { assert(m_type == VARIANT_FLOAT); return m_float; }

    string getString() { assert(m_type == VARIANT_STRING); return m_string; }

protected:
    int m_type;
    int m_integer;
    float m_float;
    string m_string;
};


/**
    Centralized configuration data

    The items come from:
     - the main config.xml file, which is the place for persistent configuration data,
     - in game volatile data, either set at startup in GameConfig.cpp or later
*/
class GameConfiguration
{
public:
    GameConfiguration();
    ~GameConfiguration();

    int getInteger(string key) { return m_data[key].getInteger(); }
    float getFloat(string key) { return m_data[key].getFloat(); }
    string getString(string key) { return m_data[key].getString(); }

    void setInteger(string key, int value) { m_data[key] = Variant(value); }
    void setFloat(string key, float value) { m_data[key] = Variant(value); }
    void setString(string key, string value) { m_data[key] = Variant(value); }

    bool load();
    bool save();

protected:
    map<string, Variant> m_data;
    map<string, int> m_expected;

    static GameConfiguration* m_instance;

    void initTypes();
};

#endif // _GAME_CONFIG_H
