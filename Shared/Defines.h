#ifndef INCLUDED_Shared_Defines_h
#define INCLUDED_Shared_Defines_h
////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>

// 
// preprocessor macros
// 
// keep these to the absolute minimum.
// 



#undef DEBUG_ONLY
#ifdef DEBUG
    #define DEBUG_ONLY( x ) x
#else
    #define DEBUG_ONLY( x )
#endif


#ifdef PSEUDOSTACK
    #define PSEUDOSTACK_START  try {
    #define PSEUDOSTACK_END    } catch ( std::exception& e ) {  throw Exception ( "", e, __FILE__, __FUNCTION__, __LINE__ );  }
#else
    #define PSEUDOSTACK_START
    #define PSEUDOSTACK_END
#endif





typedef char byte;



////////////////////////////////////////////////////////////////////////////////
#endif

