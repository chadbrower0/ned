#ifndef INCLUDED_SearchBar_h
#define INCLUDED_SearchBar_h
////////////////////////////////////////////////////////////////////////////////
#include "WindowPanel.h"
#include "InputLine.h"
#include "InputArea.h"
#include "Settings.h"
#include "MouseMap.h"

// 
// a bar across the bottom of the screen, for searching/replacing text, 
// 
// panel-based.
// 

    class 
SearchBar 
    {
    ////////// inner types 
        protected: enum tAction {  EDIT_FIND, FIND,  EDIT_REPLACE, REPLACE,  CANCEL  };


    ////////// member data 
        public: WindowPanel mBar;
        public: InputLine mInputFind;
        public: InputLine mInputReplace;
//         public: const Settings& mSettings;
        public: const std::string& mCopiedText;

        protected: MouseMap<tAction> mMouseToAction;


    ////////// construction

            public:
        SearchBar( // const Settings& settings, 
                   const std::string& copied_text )
            :
//             mSettings ( settings ), 
            mCopiedText ( copied_text )
            {
            }

            public: virtual 
        ~SearchBar( ) 
            {
            }



    ////////// methods -- data access



    ////////// methods

            public: void
        CreatePanel( )
            {
            // find screen size
            int y_size, x_size;
            getmaxyx( stdscr, y_size, x_size );

            std::string find_label = "(^W) Find ";
            std::string replace_label = "(^R) Replace with ";
            std::string cancel_label = "(ESC) Cancel";
            int find_size = ( x_size - 1 - find_label.length() - 4 - replace_label.length() - 4 - cancel_label.length() - 1 ) / 2;
            int replace_size = ( x_size - 1 - find_label.length() - 4 - replace_label.length() - 4 - cancel_label.length() - 1 ) - find_size;

            // make bar
            Point bar_start ( y_size - 1, 0 );
            mBar.Create( 1, x_size, bar_start.Y, bar_start.X );
            wattron( mBar.Window(), A_REVERSE );
            mvwhline( mBar.Window(), 0, 0, ' ', x_size );

            // make fields
            // find label
            Point field_start ( 0, 0 );
            Print( " ", field_start );
            mMouseToAction.Add(  FIND,  field_start,  Point ( field_start.Y + 1,  field_start.X + find_label.length() ) );
            Print( find_label, field_start );
            // find input
            mMouseToAction.Add(  EDIT_FIND,  field_start,  Point( field_start.Y + 1,  field_start.X + find_size ) );
            mInputFind.CreatePanel( find_size, 
                                    bar_start.Y + field_start.Y,  bar_start.X + field_start.X, 
                                    A_UNDERLINE, "" );
            field_start.X += find_size;
            // replace label
            Print( "    ", field_start );
            mMouseToAction.Add(  REPLACE,  field_start,  Point ( field_start.Y + 1,  field_start.X + replace_label.length() ) );
            Print( replace_label, field_start );
            // replace input
            mMouseToAction.Add(  EDIT_REPLACE,  field_start,  Point( field_start.Y + 1, field_start.X + replace_size ) );
            mInputReplace.CreatePanel( replace_size, 
                                       bar_start.Y + field_start.Y,  bar_start.X + field_start.X,
                                       A_UNDERLINE, "" );
            field_start.X += replace_size;
            // cancel label
            Print( "    ", field_start );
            mMouseToAction.Add(  CANCEL,  field_start,  Point ( field_start.Y + 1,  field_start.X + cancel_label.length() ) );
            Print( cancel_label, field_start );

            Hide();
            }

            protected: void
        Print( const std::string& text, Point& field_start ) // modifies field_start
            {
            mvwprintw( mBar.Window(), field_start.Y, field_start.X, "%s", text.c_str() );
            getyx( mBar.Window(), field_start.Y, field_start.X );
            }


            public: void
        Run( InputArea& synch, // modified
             const std::string& find_text = "" )
            {
            if ( find_text != "" ) mInputFind.SetText( find_text );
            mInputFind.MoveLineEnd();
            mInputReplace.MoveLineEnd();

            bool replace = false;
            Show();
            update_panels();
            doupdate();

            // take input
            while ( true )
                {
                int i = getch();
                char c = (char) i;

                // mouse event
                if ( i == KEY_MOUSE )
                    {
                    // get mouse event data
                    MEVENT mouse_event;
                    int result = getmouse( & mouse_event );
                    if ( result == ERR ) throw Exception ( "i = KEY_MOUSE  and  getmouse() = ERR", __FILE__, __FUNCTION__, __LINE__ );
                    if ( result != OK ) throw Exception ( "i = KEY_MOUSE and  getmouse() = " + TextFunctions::ToString( result ) + " unhandled", __FILE__, __FUNCTION__, __LINE__ );
                    // handle mouse event
                    bool cancel;
                    HandleMouseEvent( mouse_event, synch, replace, cancel );
                    if ( cancel ) return;
                    }
else if ( i == -1 ) {  }   // noise from window switching
                // replace input is active
                else if ( replace )
                    {
                    if ( Settings::Get().KeyHasFunction( i, Settings::MENU ) )  
                        {
                        Cancel();  return; 
                        }
                    else if ( Settings::Get().KeyHasFunction( i, Settings::SEARCH ) ) 
                        {
                        // make find input active
                        replace = false;
                        MakeInputActive( mInputFind );
                        }
                    else if ( Settings::Get().KeyHasFunction( i, Settings::REPLACE ) )
                        {
                        if ( synch.HasSelection() ) synch.Insert( mInputReplace.GetText() );
                        synch.Find( mInputFind.GetText() );
                        }
                    else if ( Settings::Get().KeyHasFunction( i, Settings::BACKWARD ) )  
                        {
                        mInputReplace.MoveLeft();
                        }
                    else if ( Settings::Get().KeyHasFunction( i, Settings::FORWARD ) ) 
                        {
                        mInputReplace.MoveRight();
                        }
                    else if ( Settings::Get().KeyHasFunction( i, Settings::HOME ) )
                        {
                        mInputReplace.MoveLineStart();
                        }
                    else if ( Settings::Get().KeyHasFunction( i, Settings::END ) )
                        {
                        mInputReplace.MoveLineEnd();
                        }
                    else if ( Settings::Get().KeyHasFunction( i, Settings::DELETE ) ) 
                        {
                        mInputReplace.Delete();
                        }
                    else if ( Settings::Get().KeyHasFunction( i, Settings::BACKSPACE ) )  
                        {
                        mInputReplace.Backspace();
                        }
                    else if ( Settings::Get().KeyHasFunction( i, Settings::PASTE ) )  
                        {
                        mInputReplace.Insert( mCopiedText );
                        }
                    else
                        {
                        mInputReplace.Insert( std::string ( 1, c ) );
                        }
                    }
                // find input is active
                else 
                    {
                    if ( Settings::Get().KeyHasFunction( i, Settings::MENU ) ) 
                        {
                        Cancel();  return; 
                        }
                    else if ( Settings::Get().KeyHasFunction( i, Settings::SEARCH ) ) 
                        {
                        synch.Find( mInputFind.GetText() );
                        }
                    else if ( Settings::Get().KeyHasFunction( i, Settings::REPLACE ) ) 
                        {
                        replace = true;
                        MakeInputActive( mInputReplace );
                        }
                    else if ( Settings::Get().KeyHasFunction( i, Settings::BACKWARD ) )  
                        {
                        mInputFind.MoveLeft();
                        }
                    else if ( Settings::Get().KeyHasFunction( i, Settings::FORWARD ) ) 
                        {
                        mInputFind.MoveRight();
                        }
                    else if ( Settings::Get().KeyHasFunction( i, Settings::HOME ) ) 
                        {
                        mInputFind.MoveLineStart();
                        }
                    else if ( Settings::Get().KeyHasFunction( i, Settings::END ) ) 
                        {
                        mInputFind.MoveLineEnd();
                        }
                    else if ( Settings::Get().KeyHasFunction( i, Settings::DELETE ) ) 
                        {
                        mInputFind.Delete();
                        }
                    else if ( Settings::Get().KeyHasFunction( i, Settings::BACKSPACE ) )  
                        {
                        mInputFind.Backspace();
                        }
                    else if ( Settings::Get().KeyHasFunction( i, Settings::PASTE ) )  
                        {
                        mInputFind.Insert( mCopiedText );
                        }
                    else
                        {
                        mInputFind.Insert( std::string ( 1, c ) );
                        }
                    }
                }
            }


            protected: void
        HandleMouseEvent( const MEVENT& mouse_event, 
                          InputArea& synch, bool& replace, bool& cancel )  // modified
            {
            cancel = false;

            // get mouse event
            if ( wenclose( mBar.Window(), mouse_event.y, mouse_event.x ) )
                {
                // translate mouse_event coordinates to window-relative coordinates
                Point window_start;
                getbegyx( mBar.Window(), window_start.Y, window_start.X );
                Point mouse_relative_window = Point (  mouse_event.y - window_start.Y ,  mouse_event.x - window_start.X );

                if ( mouse_event.bstate & BUTTON1_CLICKED )
                    {
                    // find option position matching mouse_event...
                    bool action_set;
                    tAction action;
                    mMouseToAction.FindAction( mouse_relative_window, action_set, action );
                    if ( ! action_set ) return;

                    if ( action == EDIT_FIND )    
                        {
                        replace = false;
                        MakeInputActive( mInputFind );
                        mInputFind.HandleMouseEvent( mouse_event );
                        }
                    else if ( action == FIND )         
                        {
                        replace = false;
                        MakeInputActive( mInputFind );
                        synch.Find( mInputFind.GetText() );
                        }
                    else if ( action == EDIT_REPLACE ) 
                        {
                        replace = true;
                        MakeInputActive( mInputReplace );
                        mInputReplace.HandleMouseEvent( mouse_event );
                        }
                    else if ( action == REPLACE ) 
                        {
                        replace = true;
                        MakeInputActive( mInputReplace );
                        if ( synch.HasSelection() ) synch.Insert( mInputReplace.GetText() );
                        synch.Find( mInputFind.GetText() );
                        }
                    else if ( action == CANCEL ) {  cancel = true;  Cancel();  return;  }
                    }
else if ( mouse_event.bstate & BUTTON1_DOUBLE_CLICKED )
    {
    // find option position matching mouse_event...
    bool action_set;
    tAction action;
    mMouseToAction.FindAction( mouse_relative_window, action_set, action );
    if ( ! action_set ) return;

    if ( action == EDIT_FIND )    
        {
        replace = false;
        MakeInputActive( mInputFind );
        mInputFind.HandleMouseEvent( mouse_event );
        }
    else if ( action == EDIT_REPLACE ) 
        {
        replace = true;
        MakeInputActive( mInputReplace );
        mInputReplace.HandleMouseEvent( mouse_event );
        }
    else if ( action == CANCEL ) {  cancel = true;  Cancel();  return;  }
    }
                }
            else {  cancel = true;  Cancel();  return;  }
            }


            protected: void
        Cancel( )
            {
            Hide();
            update_panels();
            doupdate();
            }

            protected: void
        MakeInputActive( InputLine& input )
            {
            input.Hide();
            input.Show();
            update_panels();
            doupdate();
            }

            protected: void
        Hide( )
            {
            mInputFind.Hide();
            mInputReplace.Hide();
            hide_panel( mBar.Panel() );
            }

            protected: void
        Show( )
            {
            show_panel( mBar.Panel() );
            mInputReplace.Show();
            mInputFind.Show();
            }


    };



////////////////////////////////////////////////////////////////////////////////
#endif // #ifndef INCLUDED_SearchBar_h
