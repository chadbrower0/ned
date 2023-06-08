#ifndef INCLUDED_WindowPanel_h
#define INCLUDED_WindowPanel_h
//////////////////////////////////////////////////////////////////////
#include "WindowOwner.h"
#include "PanelOwner.h"

// 
// container class for a panel & its window
// 

	class
WindowPanel
	{
	////////// member data
		public: WindowOwner mWindow;
		public: PanelOwner mPanel;



	////////// construction

			public: 
		WindowPanel( )
			{
			}

			public: 
		WindowPanel( int y_size, int x_size, int y, int x )
			{
			Create( y_size, x_size, y, x );
			}

			public: void
		Clear( )
			{
			mPanel.Clear();
			mWindow.Clear();
			}



	////////// methods -- data access

			public: WINDOW* // caller does not own returned WINDOW
		Window( )
			{
			return mWindow.Get();
			}

			public: const WINDOW* // caller does not own returned WINDOW
		Window( ) const
			{
			return mWindow.Get();
			}

			public: PANEL* // caller does not own returned PANEL
		Panel( ) 
			{
			return mPanel.Get();
			}



	////////// methods

			public: void
		Create( int y_size, int x_size, int y, int x ) 
			{
                        mPanel.Clear();
			mWindow = newwin( y_size, x_size, y, x );
			mPanel = new_panel( mWindow.Get() );
			}

	};




//////////////////////////////////////////////////////////////////////
#endif // #ifndef INCLUDED_WindowPanel_h
