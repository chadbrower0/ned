#ifndef INCLUDED_PanelOwner_h
#define INCLUDED_PanelOwner_h
//////////////////////////////////////////////////////////////////////
#include <curses.h>
#include <panel.h>

// 
// container for a PANEL
// 

	class
PanelOwner
	{
	////////// member data
		protected: PANEL* mPanel;



	////////// construction

			public: 
		PanelOwner( PANEL* panel = NULL )  // takes ownership of panel
			{
			mPanel = panel;
			}

			public: 
		~PanelOwner( )
			{
			Clear();
			}



	////////// methods -- internal

			protected: void
		Set( PANEL* panel = NULL ) // takes ownership of panel
			{
			Clear();
			mPanel = panel;
			}

			public: void
		Clear( )
			{
			if ( mPanel != NULL )
				{
				del_panel( mPanel );
				mPanel = NULL;
				}
			}



	////////// methods -- access


			public: PANEL*
		Get( ) 
			{
			return mPanel;
			}

			public: const PanelOwner&
		operator= ( PANEL* panel ) // takes ownership of panel
			{
			Set( panel );
			return *this;
			}

	};





//////////////////////////////////////////////////////////////////////
#endif // #ifndef INCLUDED_PanelOwner_h
