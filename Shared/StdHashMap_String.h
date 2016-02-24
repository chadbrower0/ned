#ifndef INCLUDED_Shared_StdHashMap_String_h
#define INCLUDED_Shared_StdHashMap_String_h
////////////////////////////////////////////////////////////////////////////////
#include <ext/hash_map>
#include "Exception.h"

// 
// adds convenience functions to std::hash_map
// 
// usage: 
// 
//     StdHashMap_String< bool >  map;
// 


    class
StdHashMap_String_HashFunc : public  __gnu_cxx::hash<std::string>
    {
    protected: __gnu_cxx::hash<const char*> mHasher;

        public: size_t
    operator() ( const std::string& key ) const 
        {  
        return mHasher( key.c_str() );  
        }
    };


//     class
// StdHashMap_String_Equal : public std::equal_to<std::string>
//     {
//     public: bool
//     operator() ( const std::string& first, const std::string& second ) const {  return ( first == second );  }
//     };



    template< class tValue > 
    class 
StdHashMap_String :
    public __gnu_cxx::hash_map< std::string, tValue, StdHashMap_String_HashFunc >  // , StdHashMap_String_Equal<std::string> >
    {
    ////////// inner types
//         protected: typedef __gnu_cxx::hash_map< std::string, tValue, StdHashMap_String_HashFunc, StdHashMap_String_Equal<std::string> >  base_class;
protected: typedef __gnu_cxx::hash_map< std::string, tValue, StdHashMap_String_HashFunc >  base_class;


    ////////// member data 


    ////////// construction
    
            public: virtual
        ~StdHashMap_String( )
            {
            }


    ////////// methods -- data access 
    
            public: bool
        Exists( const std::string& key ) const
            // more convenient syntax than __gnu_cxx::hash_map::find()
            {
            typename base_class::const_iterator i = base_class::find( key );
            return ( i != base_class::end() );
            }



            public: typename base_class::iterator
        Get( const std::string& key ) 
            // throws exception if key not found
            {
            typename base_class::iterator i = base_class::find( key );
            if ( i == base_class::end() ) throw Exception ( "", __FILE__, __FUNCTION__, __LINE__ );
            return i;
            }
            
            public: typename base_class::const_iterator
        Get( const std::string& key ) const
            // throws exception if key not found
            {
            typename base_class::const_iterator i = base_class::find( key );
            if ( i == base_class::end() ) throw Exception ( "", __FILE__, __FUNCTION__, __LINE__ );
            return i;
            }
        
        // for optional gets, use __gnu_cxx::hash_map::find()

            public: tValue&  // may return NULL
        MaybeGet( const std::string& key )
            {
            typename base_class::iterator i = base_class::find( key );
            if ( i == base_class::end() ) return *(tValue*) NULL;
            return i->second;
            }

            public: const tValue&  // may return NULL
        MaybeGet( const std::string& key ) const
            {
            typename base_class::const_iterator i = base_class::find( key );
            if ( i == base_class::end() ) return *(tValue*) NULL;
            return i->second;
            }


        // for get/insert returning value, can use hash_map::operator[]

            public: typename base_class::iterator
        GetOrInsert( const std::string& key, const tValue& default_value )
            // more convenient syntax than __gnu_cxx::hash_map::insert()
            {
            typename std::pair<std::string,tValue> entry ( key, default_value );
            typename std::pair< typename base_class::iterator, bool > result;
            result = insert( entry );
            return result.first;
            }

            public: typename base_class::iterator
        InsertNew( const std::string& key, const tValue& value )
            // throws exception if key already exists
            {
            typename std::pair<std::string,tValue> entry ( key, value );
            typename std::pair< typename base_class::iterator, bool > result;
            result = insert( entry );
            if ( ! result.second ) throw Exception ( "", __FILE__, __FUNCTION__, __LINE__ );
            return result.first;
            }

    };


////////////////////////////////////////////////////////////////////////////////
#endif
