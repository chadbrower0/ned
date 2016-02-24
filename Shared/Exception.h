#ifndef INCLUDED_Shared_Exception_h
#define INCLUDED_Shared_Exception_h
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <sstream>
#include <exception>
#include <iostream>
#include <stdio.h>
#include <Shared/Defines.h>

//
// exception, for throwing and re-throwing
//

    class
Exception :
    public std::exception
    {
    ////////// member data
        public: std::string Message;


    ////////// construction

            public:
        Exception( const std::string& message,
                   const std::string& file, const std::string& function, int line )
            {
            Message = "Shared::Exception";
            Add( message, file, function, line );
            }

            public:
        Exception(  const std::string& message, const std::exception& e,
                    const std::string& file, const std::string& function, int line )
            {
            Message = (std::string) e.what();
            Add( message, file, function, line );
            }

            public: virtual
        ~Exception() throw ( )
            {
            }


    ////////// methods

            public: void
        Add( const std::string& message,
             const std::string& file, const std::string& function, int line )
            {
            if ( message != "" )
                Message += "\n  " + message;
            Message += "\n  in file " + file + "  in function " + function + "  at line  " + ToString( line );
            }

            public: const std::string&
        GetMessage() const {  return Message;  }

            public: const char*
        what( ) const throw ( ) {  return Message.c_str();  }

            protected: static std::string
        ToString( int line )
            {
            std::ostringstream out;
            out << line;
            return out.str();
            }

    };

////////////////////////////////////////////////////////////////////////////////
#endif 
