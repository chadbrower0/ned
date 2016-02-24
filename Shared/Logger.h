#ifndef INCLUDED_Shared_Logger_h
#define INCLUDED_Shared_Logger_h
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <sstream>
#include "Exception.h"
#include "Owner.h"
#include "Defines.h"

// 
// Logger interface / base class default implementation
//     Default implementation is not thread-safe
// Also, a singleton instance.
// 
// Usage:   
//     if ( Logger::Enabled(Logger::WARN) ) 
//         Logger::Log( "some details", Logger::WARN, __FILE__, __FUNCTION__, __LINE__ ); 
// 

    class 
Logger 
    {
    ////////// member data -- interface
        public: enum Level {  DEBUG = 1,  WARN = 3,  ERROR = 4,  NONE = 9  };
        public: Level mMinimumLevel;
        public: bool mAutoFlush;

    ////////// construction -- interface

            public:
        Logger( Level level ) {  mMinimumLevel = level;  mAutoFlush = false;  }
            
            public: virtual
        ~Logger( ) {  }


    ////////// methods -- interface

            public: virtual bool
        EnabledImplementation( Level level ) const 
            {  
            return ( level >= mMinimumLevel );  
            }

            public: virtual void
        LogImplementation(  const std::string& message,  Level level,  const char* file,  const char* function,  long line  )
            {
            if ( ! EnabledImplementation( level ) ) throw Exception ( (String) "Logger user must check Enabled() before calling Log()", __FILE__, __FUNCTION__, __LINE__ );
            std::cout << "\n" << message;
            if ( mAutoFlush ) std::cout.flush();
            }


    ////////// member data -- singleton
        public: static Owner<Logger> mInstance;  // singleton instance.  may be NULL.  has to be set.

    ////////// methods -- singleton -- provided for more convenient calls to logger

            public: static bool
        Enabled( Level level )
            {  PSEUDOSTACK_START

            return mInstance->EnabledImplementation( level );

            PSEUDOSTACK_END
            }

            public: static void
        Log(  const std::string& message,  Level level,  const char* file,  const char* function,  long line  )
            {  PSEUDOSTACK_START

            if ( ! Enabled( level ) ) return;
            mInstance->LogImplementation( message, level, file, function, line );

            PSEUDOSTACK_END
            }

            public: static Level
        ToLevel( const std::string& text )
            // to help setting log level from config file parameter or command line argument
            {
            if      ( text == "DEBUG" )       return DEBUG;
            else if ( text == "WARN" )        return WARN;
            else if ( text == "ERROR" )       return ERROR;
            else if ( text == "NONE" )        return NONE;
            else 
                throw Exception ( (String) "unhandled log level name = \"" + text + "\"" , __FILE__, __FUNCTION__, __LINE__ );
            }

    };


// member data -- singleton
    Owner<Logger> Logger::mInstance = new Logger ( Logger::WARN );


////////////////////////////////////////////////////////////////////////////////
#endif



