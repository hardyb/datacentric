/*
 * SpecialDebug.h
 *
 *  Created on: 19 Nov 2012
 *      Author: AndrewHardy
 */

#ifndef SPECIALDEBUG_H_
#define SPECIALDEBUG_H_

//#define SPECIAL_DEBUG 1

class NullStream {
    public:
    NullStream() { }
    template<typename T> NullStream& operator<<(T const&) { return *this; }
};

#ifdef SPECIAL_DEBUG
    #define COUT std::cout
#else
    #define COUT NullStream()
#endif


#endif /* SPECIALDEBUG_H_ */
