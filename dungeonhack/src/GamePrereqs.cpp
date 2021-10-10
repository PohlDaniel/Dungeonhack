#include "DungeonHack.h"
#include <algorithm>
#include <string>

// Defining itoa for gcc
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
    /**
     * C++ version char* style "itoa":
     */
    char* itoa( int value, char* result, int base );
    std::string itoa( int value, int base );

    char* itoa( int value, char* result, int base )
    {
        // check that the base is valid
        if ( base < 2 || base > 16 )
        {
            *result = 0;
            return result;
        }
        char* out = result;
        int quotient = value;
        do
        {
            *out = "0123456789abcdef"[ std::abs( quotient % base ) ];
            ++out;
            quotient /= base;
        }
        while ( quotient );
        // Only apply negative sign for base 10
        if ( value < 0 && base == 10 )
            * out++ = '-';
        std::reverse( result, out );
        *out = 0;
        return result;
    }

    /**
     * C++ version std::string style "itoa":
     */

    std::string itoa( int value, int base )
    {
        enum
        {
            kMaxDigits = 35
        };
        std::string buf;
        buf.reserve( kMaxDigits ); // Pre-allocate enough space.
        // check that the base if valid
        if ( base < 2 || base > 16 )
            return buf;
        int quotient = value;
        // Translating number to string with base:
        do
        {
            buf += "0123456789abcdef"[ std::abs( quotient % base ) ];
            quotient /= base;
        }
        while ( quotient );
        // Append the negative sign for base 10
        if ( value < 0 && base == 10 )
            buf += '-';
        std::reverse( buf.begin(), buf.end() );
        return buf;

    }
#endif
