#ifndef INCLUDED_Shared_LoggerStream_h
#define INCLUDED_Shared_LoggerStream_h
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include "Logger.h"

// 
// specialized logger sub-class
// 

    class 
LoggerStream :  public Logger
    {
    ////////// member data 
        protected: std::ofstream mFileOut;  // may be used, may not
        protected: std::ostream* mStream;   // not owner

    ////////// construction

            public:
        LoggerStream( Logger::Level level,  std::ostream& out_stream ) 
            : 
            Logger ( level ),
            mStream ( & out_stream )
            {
            }

            protected:
        LoggerStream( Logger::Level level )
            : 
            Logger ( level ),
            mStream ( NULL )
            {
            }

            public: virtual
        ~LoggerStream( ) {  if ( mFileOut.is_open() ) mFileOut.close();  }

            public: static LoggerStream*  // caller owns returned object
        CreateFileStream( Logger::Level level, const std::string& filepath, bool append )
            {
            Owner<LoggerStream> logger = new LoggerStream ( level );

            if ( append )
                logger->mFileOut.open( filepath.c_str(), std::ios::app );
            else
                logger->mFileOut.open( filepath.c_str() );

            logger->mStream = & logger->mFileOut;

            return logger.GiveUp();
            }


    ////////// methods

            public: virtual void
        LogImplementation(  const std::string& message,  Level level,  const char* file,  const char* function,  long line  )
            {
            if ( ! EnabledImplementation( level ) ) throw Exception ( (String) "Logger user must check Enabled() before calling Log()", __FILE__, __FUNCTION__, __LINE__ );
            *mStream << "\n" << message;
            if ( mAutoFlush ) mStream->flush();
            }

    }; 


////////////////////////////////////////////////////////////////////////////////
#endif

