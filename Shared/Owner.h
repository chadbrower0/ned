#ifndef INCLUDED_Shared_Owner_h
#define INCLUDED_Shared_Owner_h
////////////////////////////////////////////////////////////////////////////////
#include "Exception.h"
#include "Defines.h"

// 
// pointer wrapper.
// 
// destructor is not virtual, for less RAM usage -- 
//     do not sub-class to change destructor/cleanup function.
//     ok to sub-class to override operators, e.g. copy semantics
// 

    template<class T>
    class 
Owner 
    {
    ////////// member data 
        protected: T* mData;


    ////////// construction

            public:
        Owner( T* new_data = NULL ) // takes ownership of new_data
            {
            mData = new_data;
            }
            
            public: 
        Owner( const Owner<T>& old_owner ) // takes ownership of old_owner's data
            // constructor for supporting STL containers.
            {
            mData = ( (Owner<T>&) old_owner ).GiveUp();
            }

            public: 
        ~Owner( ) 
            {
            Delete( mData );
            }

            public: void
        Delete( ) {  Delete( mData );  }

            protected: void
        Delete( T*& data )
            {
            if ( data != NULL )
                {
                delete data;
                data = NULL;
                }
            }


    ////////// methods -- data access

            public: void
        Set( T* new_data )
            {
            try 
                {
                if ( mData != NULL  &&  new_data == mData ) {  new_data = NULL;  throw Exception ( "Owner.Set() -- new_data == mData", __FILE__, __FUNCTION__, __LINE__ );  }
                Delete( mData );
                mData = new_data;
                new_data = NULL;
                }
            catch ( Exception& e )
                {
                Delete( new_data );
                throw e;
                }
            }

            public: inline T&  // may return NULL
        MaybeGet( ) {  return *mData;  }
            
            public: inline const T&  // may return NULL
        MaybeGet( ) const {  return *mData;  }
            
            public: inline T* // caller takes ownership of data
        GiveUp( ) 
            {
            T* data = mData;
            mData = NULL;
            return data;
            }
            
            public: inline bool
        IsNull( ) const {  return ( mData == NULL );  }

            public: inline bool
        operator== ( const T* other ) const {  return ( mData == other );  }
                    
            public: inline bool
        operator!= ( const T* other ) const {  return ( mData != other );  }

                    

    ////////// methods -- operators
    
            public: inline T& // may return NULL
        operator* ( )
            // may throw exception if de-referencing NULL
            {
            DEBUG_ONLY(   if ( mData == NULL ) throw Exception ( "Owner -- operator* called on null pointer", __FILE__, __FUNCTION__, __LINE__ );   )
            return *mData;
            }
            
            public: inline const T&  // may return NULL
        operator* ( ) const
            // may throw exception if de-referencing NULL
            {
            DEBUG_ONLY(   if ( mData == NULL ) throw Exception ( "Owner -- operator* called on null pointer", __FILE__, __FUNCTION__, __LINE__ );   )
            return *mData;
            }
            
            
            
            public: inline T*  // may return NULL
        operator-> ( ) 
            // may throw exception if de-referencing NULL
            {
            DEBUG_ONLY(   if ( mData == NULL ) throw Exception ( "Owner -- operator-> called on null pointer", __FILE__, __FUNCTION__, __LINE__ );   )
            return mData;
            }
            
            public: inline const T*  // may return NULL
        operator-> ( ) const
            // may throw exception if de-referencing NULL
            {
            DEBUG_ONLY(   if ( mData == NULL ) throw Exception ( "Owner -- operator-> called on null pointer", __FILE__, __FUNCTION__, __LINE__ );   )
            return mData;
            }
            
            
            
            public: inline const Owner<T>&
        operator= ( T* new_data ) // takes ownership of new_data
            {
            Set( new_data );
            return *this;
            }
            
            public: inline const Owner<T>&
        operator= ( const Owner<T>& old_owner ) // takes ownership of old_owner's data
            // for supporting STL containers
            {
            Set(  ( (Owner<T>&) old_owner ).GiveUp()  );
            return *this;
            }


    };

////////////////////////////////////////////////////////////////////////////////
#endif
