#ifndef INCLUDED_WindowOwner_h
#define INCLUDED_WindowOwner_h
//////////////////////////////////////////////////////////////////////
#include <curses.h>



	class
WindowOwner
	{
	////////// member data
		protected: WINDOW* mWindow;



	////////// construction

			public: 
		WindowOwner( WINDOW* window = NULL )  // takes ownership of window
			{
			mWindow = window;
			}

			public: 
		~WindowOwner( )
			{
			Clear();
			}



	////////// methods -- internal

			protected: void
		Set( WINDOW* window = NULL ) // takes ownership of window
			{
			Clear();
			mWindow = window;
			}

			public: void
		Clear( )
			{
			if ( mWindow != NULL )
				{
				delwin( mWindow );
				mWindow = NULL;
				}
			}



	////////// methods -- access

			public: WINDOW*
		Get( ) 
			{
			return mWindow;
			}

			public: const WINDOW*
		Get( ) const
			{
			return mWindow;
			}

			public: const WindowOwner&
		operator= ( WINDOW* window ) // takes ownership of window
			{
			Set( window );
			return *this;
			}

	};





//////////////////////////////////////////////////////////////////////
#endif // #ifndef INCLUDED_WindowOwner_h
