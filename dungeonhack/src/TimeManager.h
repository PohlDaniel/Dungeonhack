#ifndef _TIME_MANAGER_H
#define _TIME_MANAGER_H

#include <OgreString.h>
using namespace Ogre;

#define SUNRISE             64800
#define SUNSET              21600

#define LENGTH_OF_YEAR      31104000
#define LENGTH_OF_MONTH     2592000
#define LENGTH_OF_DAY       86400
#define LENGTH_OF_HOUR      3600
#define LENGTH_OF_MINUTE    60
#define LENGTH_OF_SECOND    1


/**
    Time and Calendar functions
*/
class TimeManager
{
public:
    ~TimeManager();

    static TimeManager& getSingleton() { return m_instance; };
    static TimeManager* getSingletonPtr() { return &m_instance; }

    void setTimeOfDay(double val) { m_timeOfDay = val; }
    double getTimeOfDay() { return m_timeOfDay; }
    double update(Real moveFactor);

    void setTimeFactor(float factor);
    float getTimeFactor() { return m_timeFactor; }
    void setDayScale(float scale) { if (scale > 0) m_dayScale = scale; }
    void modDayScale(float scale) { if (m_dayScale + scale > 0) m_dayScale += scale; }
    float getDayScale() { return m_dayScale; }

    long getYear();
    int getMonth();
    int getDay();
    int getHour();
    int get24FormatHour();
    int getMinute();
    String getAMorPM();
    void getDateStr(String& theDate);

    bool isMorning() { return m_isMorning; }
    bool isEvening() { return m_isEvening; }

protected:
    TimeManager();
    static TimeManager m_instance;

    double m_timeOfDay;
    String gameMonths[12];

    int m_lastHour, m_dayPos;
    bool m_isMorning;
    bool m_isEvening;

    float m_timeFactor; //Factor to scale time by, speeds up and slows down the game
    float m_dayScale;
};


#endif // _TIME_MANAGER_H
