#ifndef INCLUDED_MessageBar_h
#define INCLUDED_MessageBar_h
////////////////////////////////////////////////////////////////////////////////
#include <curses.h>
#include <string>
#include <Shared/TextFunctions.h>
#include <Shared/Owner.h>
#include "WindowPanel.h"

// 
// panel for displaying messages at the bottom of the screen -- 
// sort of like a standard output for the GUI.
// 
// a singleton.
//	 Need to instantiate in the access functions, 
//	 to allow ncurses initialization before instantiation?
// 

	class 
MessageBar 
	{
	////////// member data
        public: static Owner<MessageBar> mOut;

        public: WindowPanel mPanel;


	////////// construction

			protected:
		MessageBar( )
			{
			}


	////////// methods

			public: static void
		CreatePanel( ) 
			// should be re-called if screen resizes
			{
			if ( mOut.IsNull() ) mOut = new MessageBar ( );
			mOut->CreatePanelImp();
			}

			protected: void
		CreatePanelImp( ) 
			// should be re-called if screen resizes
			{
			// find screen size
			int max_y, max_x;
			getmaxyx( stdscr, max_y, max_x );

			mPanel.Create( 1, max_x, max_y - 1, 0 );

			// paint window
			wmove( mPanel.Window(), max_y - 1, 0 );
			for ( int i = 0;  i < max_x;  i++ )
				waddch( mPanel.Window(), ' ' | A_REVERSE );

			update_panels();
			doupdate();
			}


			public: static void
		Display( const std::string& message )
			{
			if ( mOut.IsNull() ) mOut = new MessageBar ( );
			if ( mOut->mPanel.Window() == NULL ) mOut->CreatePanelImp();

			mOut->DisplayImp( message );
			}

			protected: void
		DisplayImp( const std::string& message )
			{
			// find screen size
			int max_y, max_x;
			getmaxyx( mPanel.Window(), max_y, max_x );

			// repaint message
			wmove( mPanel.Window(), max_y - 1, 0 );
			int i;
			for ( i = 0;  i < max_x  &&  i < message.size();  i++ )
				waddch( mPanel.Window(), message.at(i) | A_REVERSE );
			for (  ;  i < max_x;  i++ )
				waddch( mPanel.Window(), ' ' | A_REVERSE );

			update_panels();
			doupdate();
			}

			public: static void
		Delete( )
			{
			mOut.Delete();
			}

	};

////////// class data
	Owner<MessageBar> MessageBar::mOut;


////////////////////////////////////////////////////////////////////////////////
#endif // #ifndef INCLUDED_MessageBar_h
