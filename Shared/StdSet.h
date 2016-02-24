#ifndef INCLUDED_Shared_StdSet_h
#define INCLUDED_Shared_StdSet_h
////////////////////////////////////////////////////////////////////////////////
#include <set>
#include "Exception.h"

// 
// adds convenience functions to std::set
// 
// 

    template <class T> 
    class 
StdSet :
    public std::set<T>
    {
    ////////// member data 


    ////////// construction
    
            public: virtual
        ~StdSet( )
            {
            }


    ////////// methods -- data access 
    
            public: bool
        Exists( const T& key ) const
            // more convenient syntax than std::set::find()
            {
            typename std::set<T>::const_iterator i = std::set<T>::find( key );
            return ( i != std::set<T>::end() );
            }

            public: typename std::set<T>::iterator
        Get( const T& key ) 
            // throws exception if key not found
            {
            typename std::set<T>::iterator i = std::set<T>::find( key );
            if ( i == std::set<T>::end() ) throw Exception ( "StdSet::Get() -- key not found", __FILE__, __FUNCTION__, __LINE__ );
            return i;
            }
            
            public: typename std::set<T>::const_iterator
        Get( const T& key ) const
            // throws exception if key not found
            {
            typename std::set<T>::const_iterator i = std::set<T>::find( key );
            if ( i == std::set<T>::end() ) throw Exception ( "StdSet::Get() -- key not found", __FILE__, __FUNCTION__, __LINE__ );
            return i;
            }
        
        // for optional gets, use std::set::find()



            public: typename std::set<T>::iterator
        GetOrInsert( const T& key )
            // more convenient syntax than std::set::insert()?
            {
            typename std::pair< typename std::set<T>::iterator, bool > result;
            result = insert( key );
            return result.first;
            }

            public: typename std::set<T>::iterator
        InsertNew( const T& key )
            // throws exception if key already exists
            {
            typename std::pair< typename std::set<T>::iterator, bool > result;
            result = std::set<T>::insert( key );
            if ( ! result.second ) throw Exception ( "StdSet::InsertNew() -- key already exists", __FILE__, __FUNCTION__, __LINE__ );
            return result.first;
            }



            public: void
        Union( const std::set<T>& other ) 
            // convenience function.  not particularly fast.  
            {
            for ( typename std::set<T>::const_iterator o = other.begin();  o != other.std::template set<T>::end();  o++ )
                insert( *o );
            }

    };


////////////////////////////////////////////////////////////////////////////////
#endif
