#ifndef INCLUDED_Shared_StdHashMap_Number_h
#define INCLUDED_Shared_StdHashMap_Number_h
////////////////////////////////////////////////////////////////////////////////
#include <ext/hash_map>
#include "Exception.h"

// 
// adds convenience functions to std::hash_map
// 
// tKey must be cast-able to size_t
// 
// usage: 
// 
//     StdHashMap_Number< int, bool >  map;
// 


    template< class tKey >
    class
StdHashMap_Number_HashFunc : public  __gnu_cxx::hash<tKey>
    {
        public: size_t
    operator() ( const tKey& key ) const {  return (size_t) key;  }
    };


    template< class tKey >
    class
StdHashMap_Number_Equal : public std::equal_to<tKey>
    {
    public: bool
    operator() ( const tKey& first, const tKey& second ) const {  return ( first == second );  }
    };



    template< class tKey, class tValue > 
    class 
StdHashMap_Number :
    public __gnu_cxx::hash_map< tKey, tValue, StdHashMap_Number_HashFunc<tKey>, StdHashMap_Number_Equal<tKey> >
    {
    ////////// inner types
        protected: typedef __gnu_cxx::hash_map< tKey, tValue, StdHashMap_Number_HashFunc<tKey>, StdHashMap_Number_Equal<tKey> >  base_class;


    ////////// member data 


    ////////// construction
    
            public: virtual
        ~StdHashMap_Number( )
            {
            }


    ////////// methods -- data access 
    
            public: bool
        Exists( const tKey& key ) const
            // more convenient syntax than __gnu_cxx::hash_map::find()
            {
            typename base_class::const_iterator i = base_class::find( key );
            return ( i != base_class::end() );
            }



            public: typename base_class::iterator
        Get( const tKey& key ) 
            // throws exception if key not found
            {
            typename base_class::iterator i = base_class::find( key );
            if ( i == base_class::end() ) throw Exception ( "", __FILE__, __FUNCTION__, __LINE__ );
            return i;
            }
            
            public: typename base_class::const_iterator
        Get( const tKey& key ) const
            // throws exception if key not found
            {
            typename base_class::const_iterator i = base_class::find( key );
            if ( i == base_class::end() ) throw Exception ( "", __FILE__, __FUNCTION__, __LINE__ );
            return i;
            }
        
        // for optional gets, use __gnu_cxx::hash_map::find()

            public: tValue&  // may return NULL if key not found
        MaybeGet( const tKey& key ) 
            {
            typename base_class::iterator i = base_class::find( key );
            if ( i == base_class::end() ) return *(tValue*) NULL;
            return i->second;
            }
        
            public: const tValue&  // may return NULL if key not found
        MaybeGet( const tKey& key ) const
            {
            typename base_class::const_iterator i = base_class::find( key );
            if ( i == base_class::end() ) return *(tValue*) NULL;
            return i->second;
            }
        

        // for get/insert returning value, can use hash_map::operator[]

            public: typename base_class::iterator
        GetOrInsert( const tKey& key, const tValue& default_value )
            // more convenient syntax than __gnu_cxx::hash_map::insert()
            {
            typename std::pair<tKey,tValue> entry ( key, default_value );
            typename std::pair< typename base_class::iterator, bool > result;
            result = base_class::insert( entry );
            return result.first;
            }

            public: typename base_class::iterator
        InsertNew( const tKey& key, const tValue& value )
            // throws exception if key already exists
            {
            typename std::pair<tKey,tValue> entry ( key, value );
            typename std::pair< typename base_class::iterator, bool > result;
            result = insert( entry );
            if ( ! result.second ) throw Exception ( "", __FILE__, __FUNCTION__, __LINE__ );
            return result.first;
            }

    };


////////////////////////////////////////////////////////////////////////////////
#endif
