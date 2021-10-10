#include "xmlConfig.h"

xmlConfig* xmlConfig::pinstance = 0;// initialize pointer
xmlConfig* xmlConfig::Instance () 
{
    if (pinstance == 0)  // is it the first call?
    {  
        pinstance = new xmlConfig; // create sole instance
    }
    return pinstance; // address of sole instance
}
xmlConfig::xmlConfig() 
{ 

}

void xmlConfig::setConfigFile(std::string newFileName)
{
    m_filename = newFileName;
}

bool xmlConfig::saveConfigVariable(std::string varName, std::string value)
{
    TiXmlDocument * xmlDoc = new TiXmlDocument(m_filename.c_str());

    if (!xmlDoc->LoadFile())
    {
        // Fatal error, cannot load
        m_haserror = true;
        return(false);
    }

    TiXmlElement *xVars = 0;
    xVars = xmlDoc->FirstChildElement("config");

    if (!xVars)
    {
        xmlDoc->Clear();
        delete xmlDoc;

        m_haserror = true;
        return(false);
    }

    TiXmlElement *xVariable = 0;
    xVariable = xVars->FirstChildElement(varName.c_str());

    if(!xVariable)
    {
        xVariable = new TiXmlElement(varName.c_str());
        xVariable->SetAttribute("value",value.c_str());
        xVars->InsertEndChild(*xVariable);
    }
    else
    {
        xVariable->SetAttribute("value",value.c_str());
    }

    xmlDoc->SaveFile();
    delete xmlDoc;
    return(true);
}

bool xmlConfig::saveConfigVariableInteger(std::string varName, int value)
{
    TiXmlDocument * xmlDoc = new TiXmlDocument(m_filename.c_str());

    if (!xmlDoc->LoadFile())
    {
        // Fatal error, cannot load
        m_haserror = true;
        return(false);
    }

    TiXmlElement *xVars = 0;
    xVars = xmlDoc->FirstChildElement("config");

    if (!xVars)
    {
        xmlDoc->Clear();
        delete xmlDoc;

        m_haserror = true;
        return(false);
    }

    TiXmlElement *xVariable = 0;
    xVariable = xVars->FirstChildElement(varName.c_str());

    if(!xVariable)
    {
        xVariable = new TiXmlElement(varName.c_str());
        xVariable->SetAttribute("value",value);
        xVars->InsertEndChild(*xVariable);
    }
    else
    {
        xVariable->SetAttribute("value",value);
    }

    xmlDoc->SaveFile();
    delete xmlDoc;
    return(true);
}

bool xmlConfig::saveConfigVariableFloat(std::string varName, float value)
{
    TiXmlDocument * xmlDoc = new TiXmlDocument(m_filename.c_str());

    if (!xmlDoc->LoadFile())
    {
        // Fatal error, cannot load
        m_haserror = true;
        return(false);
    }

    TiXmlElement *xVars = 0;
    xVars = xmlDoc->FirstChildElement("config");

    if (!xVars)
    {
        xmlDoc->Clear();
        delete xmlDoc;

        m_haserror = true;
        return(false);
    }

    TiXmlElement *xVariable = 0;
    xVariable = xVars->FirstChildElement(varName.c_str());

    char theValString[100];
    sprintf(theValString, "%f", value);

    if(!xVariable)
    {
        xVariable = new TiXmlElement(varName.c_str());
        xVariable->SetAttribute("value",theValString);
        xVars->InsertEndChild(*xVariable);
    }
    else
    {
        xVariable->SetAttribute("value",theValString);
    }

    xmlDoc->SaveFile();
    delete xmlDoc;
    return(true);
}

std::string xmlConfig::getConfigVariable(std::string varName)
{
    std::string theValue = "";
    TiXmlDocument * xmlDoc = new TiXmlDocument(m_filename.c_str());

    if (!xmlDoc->LoadFile())
    {
        // Fatal error, cannot load
        m_haserror = true;
        return("");
    }

    TiXmlElement *xVars = 0;
    xVars = xmlDoc->FirstChildElement("config");

    if (!xVars)
    {
        xmlDoc->Clear();
        delete xmlDoc;

        m_haserror = true;
        return("");
    }

    TiXmlElement *xVariable = 0;
    xVariable = xVars->FirstChildElement(varName.c_str());

    if(xVariable != 0)
    {
        theValue = xVariable->Attribute("value");
    }
    else
    {
        m_haserror = true;
        theValue = "";
    }

    delete xmlDoc;

    return(theValue);
}

int xmlConfig::getConfigVariableInteger(std::string varName)
{
    int theValue = -1;
    TiXmlDocument * xmlDoc = new TiXmlDocument(m_filename.c_str());

    if (!xmlDoc->LoadFile())
    {
        // Fatal error, cannot load
        m_haserror = true;
        return(-1);
    }

    TiXmlElement *xVars = 0;
    xVars = xmlDoc->FirstChildElement("config");

    if (!xVars)
    {
        xmlDoc->Clear();
        delete xmlDoc;

        m_haserror = true;
        return(-1);
    }

    TiXmlElement *xVariable = 0;
    xVariable = xVars->FirstChildElement(varName.c_str());

    if (xVariable != 0)
    {
        std::string thisInteger = xVariable->Attribute("value");
        theValue = atoi(thisInteger.c_str());
    }
    else
    {
        m_haserror = true;
        theValue = -1;
    }

    delete xmlDoc;

    return(theValue);
}

float xmlConfig::getConfigVariableFloat(std::string varName)
{
    float theValue = -1;
    TiXmlDocument * xmlDoc = new TiXmlDocument(m_filename.c_str());

    if (!xmlDoc->LoadFile())
    {
        // Fatal error, cannot load
        m_haserror = true;
        return(-1);
    }

    TiXmlElement *xVars = 0;
    xVars = xmlDoc->FirstChildElement("config");

    if (!xVars)
    {
        xmlDoc->Clear();
        delete xmlDoc;

        m_haserror = true;
        return(-1);
    }

    TiXmlElement *xVariable = 0;
    xVariable = xVars->FirstChildElement(varName.c_str());

    if (xVariable != 0)
    {
        std::string theFloat = xVariable->Attribute("value");
        theValue = atof(theFloat.c_str());
    }
    else
    {
        m_haserror = true;
        theValue = -1;
    }

    delete xmlDoc;

    return(theValue);
}

int xmlConfig::stringToInt(std::string theString)
{
    int thisInt;

    thisInt = atoi(theString.c_str());

    return(thisInt);
}

float xmlConfig::stringToFloat(std::string theString)
{
    float thisFloat;

    thisFloat = atof(theString.c_str());

    return(thisFloat);
}
