#ifndef INCLUDED_FilepathBar_h
#define INCLUDED_FilepathBar_h
////////////////////////////////////////////////////////////////////////////////
#include "KeyListener.h"
#include "WindowPanel.h"
#include "InputLine.h"
#include "Settings.h"
#include "MouseMap.h"

// 
// a bar across the bottom of the screen, for collecting a filepath from user.
// 
// panel-based.
// 

    class 
FilepathBar 
    {
    ////////// inner types
        protected: enum tAction {  EDIT_FILE,  CANCEL  };


    ////////// member data 
        public: WindowPanel mBar;
        public: InputLine mInputFilepath;

        protected: MouseMap<tAction> mMouseToAction;


    ////////// construction

            public:
        FilepathBar( )
            {
            }

            public: virtual 
        ~FilepathBar( ) 
            {
            }



    ////////// methods -- data access



    ////////// methods

            public: void
        CreatePanel( const std::string& label )
            {
            // find screen size
            int y_size, x_size;
            getmaxyx( stdscr, y_size, x_size );

            // make labels
            std::string filepath_label = " (ENTER) " + label + " ";
            std::string cancel_label = "(ESC) Cancel";
            int filepath_size = ( x_size - 1 - filepath_label.length() - 4 - cancel_label.length() - 1 );

            // make bar
            Point bar_start ( y_size - 1, 0 );
            mBar.Create( 1, x_size,  bar_start.Y, bar_start.X );
            wattron( mBar.Window(), A_REVERSE );
            mvwhline( mBar.Window(), 0, 0, ' ', x_size );

            // make fields
            // file label
            Point field_start ( 0, 0 );
            Print( " ", field_start );
            Print( filepath_label, field_start );
            // file input
            mMouseToAction.Add(  EDIT_FILE,  field_start,  Point( field_start.Y + 1,  field_start.X + filepath_size ) );
            mInputFilepath.CreatePanel( filepath_size, 
                                        bar_start.Y + field_start.Y,  bar_start.X + field_start.X, 
                                        A_UNDERLINE, "" );
            field_start.X += filepath_size;
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




            public: std::string // returns "" if cancelled
        Run( KeyListener& resizeListener, const std::string& old_filepath = "" )
            {
            mInputFilepath.SetText( old_filepath );
            mInputFilepath.MoveLineEnd();

            Show();
            update_panels();
            doupdate();

            // take input
            while ( true )
                {
                int i = getch();

                // mouse event
                if ( i == KEY_MOUSE )
                    {
                    // get mouse event data
                    MEVENT mouse_event;
                    int result = getmouse( & mouse_event );
                    if ( result == ERR ) throw Exception ( "i = KEY_MOUSE  and  getmouse() = ERR", __FILE__, __FUNCTION__, __LINE__ );
                    if ( result != OK ) throw Exception ( "i = KEY_MOUSE and  getmouse() = " + TextFunctions::ToString( result ) + " unhandled", __FILE__, __FUNCTION__, __LINE__ );
                    // handle mouse event
                    bool filepath_set;
                    std::string filepath;
                    HandleMouseEvent( mouse_event, filepath_set, filepath );
                    if ( filepath_set ) return filepath;
                    }
                else if ( i == KEY_RESIZE ) {  std::string result = Cancel();  resizeListener.onKey(i);  return result;  }
                else if ( i == -1 ) {  }  // ignore noise from window switching
                // keyboard shortcut
                else if ( Settings::Get().KeyHasFunction( i, Settings::BACKWARD ) )   mInputFilepath.MoveLeft();
                else if ( Settings::Get().KeyHasFunction( i, Settings::FORWARD ) )    mInputFilepath.MoveRight();
                else if ( Settings::Get().KeyHasFunction( i, Settings::HOME ) )       mInputFilepath.MoveLineStart();
                else if ( Settings::Get().KeyHasFunction( i, Settings::END ) )        mInputFilepath.MoveLineEnd();
                else if ( Settings::Get().KeyHasFunction( i, Settings::DELETE ) )     mInputFilepath.Delete();
                else if ( Settings::Get().KeyHasFunction( i, Settings::BACKSPACE ) )  mInputFilepath.Backspace();
                else if ( Settings::Get().KeyHasFunction( i, Settings::MENU ) )   
                    {
                    return Cancel();
                    }
                // ENTER
                else if ( (char) i == '\n' ) 
                    {
                    Hide();
                    update_panels();
                    doupdate();
                    return mInputFilepath.GetText();
                    }
                // content
                else
                    {
                    mInputFilepath.Insert( std::string ( 1, (char) i ) );
                    }
                }
            }


            protected: void
        HandleMouseEvent( const MEVENT& mouse_event, 
                          bool& filepath_set, std::string& filepath )  // modified
            {
            filepath_set = false;
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

                    if ( action == EDIT_FILE )
                        {
                        mInputFilepath.HandleMouseEvent( mouse_event );
                        }
                    else if ( action == CANCEL ) 
                        {
                        filepath_set = true;
                        filepath = Cancel();
                        }
                    }
                }
            else 
                {
                filepath_set = true;
                filepath = Cancel();
                }
            }

            protected: std::string
        Cancel( )
            {
            Hide();
            update_panels();
            doupdate();
            return "";
            }

            protected: void
        Hide( )
            {
            mInputFilepath.Hide();
            hide_panel( mBar.Panel() );
            }

            protected: void
        Show( )
            {
            show_panel( mBar.Panel() );
            mInputFilepath.Show();
            }


    };



////////////////////////////////////////////////////////////////////////////////
#endif // #ifndef INCLUDED_FilepathBar_h
