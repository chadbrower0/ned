#ifndef INCLUDED_Shared_String_h
#define INCLUDED_Shared_String_h
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include "Exception.h"

// 
// Adds convenience functions to std::string.
// Mainly, supports number concatenation, which std::string lacks.
// 

    class 
String : public std::string
    {
    // construction

            public: 
        String( ) : std::string ( ) {  }

            public: 
        String( const char* text ) : std::string ( text ) {  }

            public: 
        String( const std::string& text ) : std::string ( text ) {  }
    


    // methods 
    
            public:  template<class tNumber> const String&
        operator+= ( tNumber number ) 
            {
            std::ostringstream out;
            out << number;
            this->std::string::append( out.str() );
            return *this;
            }
    
            public:  template<class tNumber> String
        operator+ ( tNumber number ) const
            {
            std::ostringstream out;
            out << *this << number;
            return out.str();
            }
    
    };


////////////////////////////////////////////////////////////////////////////////
#endif
