#ifndef INCLUDED_Shared_StdMap_h
#define INCLUDED_Shared_StdMap_h
////////////////////////////////////////////////////////////////////////////////
#include <map>
#include "Exception.h"

// 
// adds convenience functions to std::map
// 
// 

    template<class tKey,class tValue> 
    class 
StdMap :
    public std::map<tKey,tValue>
    {
    ////////// member data 


    ////////// construction
    
            public: virtual
        ~StdMap( )
            {
            }


    ////////// methods -- data access 
    
            public: bool
        Exists( const tKey& key ) const
            // more convenient syntax than std::map::find()
            {
            typename std::map<tKey,tValue>::const_iterator i = std::map<tKey,tValue>::find( key );
            return ( i != std::map<tKey,tValue>::end() );
            }



            public: typename std::map<tKey,tValue>::iterator
        Get( const tKey& key ) 
            // throws exception if key not found
            {
            typename std::map<tKey,tValue>::iterator i = std::map<tKey,tValue>::find( key );
            if ( i == std::map<tKey,tValue>::end() ) throw Exception ( "StdMap::Get() -- key not found", __FILE__, __FUNCTION__, __LINE__ );
            return i;
            }
            
            public: typename std::map<tKey,tValue>::const_iterator
        Get( const tKey& key ) const
            // throws exception if key not found
            {
            typename std::map<tKey,tValue>::const_iterator i = std::map<tKey,tValue>::find( key );
            if ( i == std::map<tKey,tValue>::end() ) throw Exception ( "StdMap::Get() -- key not found", __FILE__, __FUNCTION__, __LINE__ );
            return i;
            }

        
        // for optional gets, use std::map::find()

            public: tValue&  // may return NULL
        MaybeGet( const tKey& key ) 
            {
            typename std::map<tKey,tValue>::iterator i = std::map<tKey,tValue>::find( key );
            if ( i == std::map<tKey,tValue>::end() ) return *(tValue*) NULL;
            return i->second;
            }
            
            public: const tValue&  // may return NULL
        MaybeGet( const tKey& key ) const
            {
            typename std::map<tKey,tValue>::const_iterator i = std::map<tKey,tValue>::find( key );
            if ( i == std::map<tKey,tValue>::end() ) return *(tValue*) NULL;
            return i->second;
            }
        


            public: typename std::map<tKey,tValue>::iterator
        GetOrInsert( const tKey& key, const tValue& default_value )
            // more convenient syntax than std::map::insert()
            {
            typename std::pair<tKey,tValue> entry ( key, default_value );
            typename std::pair< typename std::map<tKey,tValue>::iterator, bool > result;
            result = insert( entry );
            return result.first;
            }



            public: typename std::map<tKey,tValue>::iterator
        InsertNew( const tKey& key, const tValue& value )
            // throws exception if key already exists
            {
            typename std::pair<tKey,tValue> entry ( key, value );
            typename std::pair< typename std::map<tKey,tValue>::iterator, bool > result;
            result = std::map<tKey,tValue>::insert( entry );
            if ( ! result.second ) throw Exception ( "StdMap::InsertNew() -- key already exists", __FILE__, __FUNCTION__, __LINE__ );
            return result.first;
            }


    };


////////////////////////////////////////////////////////////////////////////////
#endif
