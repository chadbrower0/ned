#ifndef INCLUDED_MenuPath_h
#define INCLUDED_MenuPath_h
////////////////////////////////////////////////////////////////////////////////
#include <vector>
#include <string>

// 
// a menu path -- sequence of menu names
// 

    class
MenuPath : 
    public std::vector<std::string>
    {
        public: 
    MenuPath( const std::string& name0 = "", const std::string& name1 = "", const std::string& name2 = "" )
        {
        if ( name0 != "" ) push_back( name0 );
        if ( name1 != "" ) push_back( name1 );
        if ( name2 != "" ) push_back( name2 );
        }

        public: 
    MenuPath( const std::vector<std::string>& names ) 
        :
        std::vector<std::string> ( names )
        {
        }

        public: bool
    operator== ( const MenuPath& other ) const
        {
        if ( size() != other.size() ) return false;
        for ( int i = 0;  i < size();  i++ )
            if ( (*this)[i] != other[i] ) 
                return false;
        return true;
        }

        public: bool
    operator< ( const MenuPath& other ) const
        {
        for ( const_iterator i = begin(), i2 = other.begin();  ;  i++, i2++ )
            {
            if ( i == end()  &&  i2 == other.end() ) return false;
            if ( i == end() ) return true;
            if ( i2 == other.end() ) return false;

            if ( *i < *i2 ) return true;
            if ( *i > *i2 ) return false;
            }
        return false;
        }

        public: std::string
    ToString( ) const
        {
        std::string s;
        for ( int i = 0;  i < size();  i++ )
            {
            if ( i > 0 ) s += ",";
            s += (*this)[i];
            }
        return s;
        }
    };



////////////////////////////////////////////////////////////////////////////////
#endif 
