#include <DungeonHack.h>
#include "TimeManager.h"

using namespace Ogre;

TimeManager TimeManager::m_instance;


TimeManager::TimeManager() : m_timeOfDay(0), m_lastHour(0), m_timeFactor(1.0), m_dayScale(0.8)
{
    m_isMorning = false;
    m_isEvening = false;
    m_dayPos = 0;

    //Setup months
    gameMonths[0] = "Morning Star";
    gameMonths[1] = "Sun's Dawn";
    gameMonths[2] = "First Seed";
    gameMonths[3] = "Rain's Hand";
    gameMonths[4] = "Second Seed";
    gameMonths[5] = "Mid Year";
    gameMonths[6] = "Sun's Height";
    gameMonths[7] = "Last Seed";
    gameMonths[8] = "Heartfire";
    gameMonths[9] = "Frost Fall";
    gameMonths[10] = "Sun's Dusk";
    gameMonths[11] = "Evening Star";
}


TimeManager::~TimeManager()
{
}


void TimeManager::setTimeFactor(float factor)
{
    if (factor >= 0.0)
    {
        m_timeFactor = factor;
        ControllerManager::getSingleton().setTimeFactor(m_timeFactor);
        //ParticleSystemManager::getSingleton().setTimeFactor(m_timeFactor);
    }
}


double TimeManager::update(Real moveFactor)
{
    //86400 time units in a day
    //7200 in an hour
    //120 in a minute
    //2 in a second

    m_timeOfDay += m_timeFactor * moveFactor * m_dayScale;
    m_timeOfDay  = (int)m_timeOfDay % LENGTH_OF_DAY;

    m_isMorning = false;
    m_isEvening = false;
    int twentyfour_hour = get24FormatHour();
    if (twentyfour_hour <= 6 || twentyfour_hour >= 18) //If it's nighttime
    {
        if (m_lastHour >= 6 || m_lastHour <= 18)
        {
            if (m_dayPos != 1)
            {
                m_isEvening = true;
                m_dayPos = 1;
            }
        }
    }
    else
    {
        if (m_lastHour <= 6 || m_lastHour >= 18)
        {
            if (m_dayPos != 2)
            {
                m_isMorning = true;
                m_dayPos = 2;
            }
        }
    }
    m_lastHour = twentyfour_hour;

    return m_timeOfDay;
}


long TimeManager::getYear()
{
    return (long)(m_timeOfDay + (LENGTH_OF_DAY / 2)) / LENGTH_OF_YEAR;
};

int TimeManager::getMonth()
{
    return ((long)(m_timeOfDay + (LENGTH_OF_DAY / 2))% LENGTH_OF_YEAR) / LENGTH_OF_MONTH;
};

int TimeManager::getDay()
{
    return (((long)(m_timeOfDay + (LENGTH_OF_DAY / 2)) % LENGTH_OF_MONTH) / LENGTH_OF_DAY) + 1;
};

int TimeManager::getHour()
{
    int thisHour = ((long)(m_timeOfDay + (LENGTH_OF_DAY / 2)) % LENGTH_OF_DAY / LENGTH_OF_HOUR);

    if(thisHour < 1)
    {
        thisHour = 12;
    }
    if(thisHour > 12)
    {
        thisHour -= 12;
    }
    return thisHour;
};

int TimeManager::get24FormatHour()
{
    int thisHour = ((long)(m_timeOfDay + (LENGTH_OF_DAY / 2)) % LENGTH_OF_DAY / LENGTH_OF_HOUR);
    return thisHour;
};

int TimeManager::getMinute()
{
    int thisHour = ((long)(m_timeOfDay + (LENGTH_OF_DAY / 2)) % LENGTH_OF_HOUR / LENGTH_OF_MINUTE);
    return thisHour;
}

String TimeManager::getAMorPM()
{
    int thisHour = ((long)(m_timeOfDay + (LENGTH_OF_DAY / 2)) % LENGTH_OF_DAY / LENGTH_OF_HOUR);
    if(thisHour < 12)
    {
        return String("AM");
    }
    return String("PM");
}

void TimeManager::getDateStr(String& theDate)
{
    char date[5];
    char hour[5];
    char minute[5];

    sprintf(date, "%d", TimeManager::getSingletonPtr()->getDay());    //%G for float, %d for int
    sprintf(hour, "%d", TimeManager::getSingletonPtr()->getHour());
    sprintf(minute, "%02d", TimeManager::getSingletonPtr()->getMinute());

    theDate += "Day ";
    theDate += date;
    theDate += " of ";
    theDate += gameMonths[ getMonth() ];
    theDate += ", ";
    theDate += hour;
    theDate += ":";
    theDate += minute;
    theDate += " ";
    theDate += getAMorPM();
    theDate += "\n";
}
