#ifndef XML_CONFIG
#define XML_CONFIG

#include "tinyXML/tinyxml.h"

class xmlConfig
{
public:
    static xmlConfig* Instance();
    void setConfigFile(std::string newFileName);

    //xml config file write functions
    bool saveConfigVariable(std::string varName, std::string value);
    bool saveConfigVariableInteger(std::string varName, int value);
    bool saveConfigVariableFloat(std::string varName, float value);

    //xml config file read functions
    std::string getConfigVariable(std::string varName);
    int getConfigVariableInteger(std::string varName);
    float getConfigVariableFloat(std::string varName);

    static int stringToInt(std::string theString);
    static float stringToFloat(std::string theString);

    bool getErrorState() { return m_haserror; }
    void clearErrorState() { m_haserror = false; }

protected:
    xmlConfig();
    xmlConfig(const xmlConfig&);
    xmlConfig& operator= (const xmlConfig&);
private:
    static xmlConfig* pinstance;
    std::string m_filename;
    bool m_haserror;
};

#endif
