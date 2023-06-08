#ifndef INCLUDED_LineNumBar_h
#define INCLUDED_LineNumBar_h
////////////////////////////////////////////////////////////////////////////////
#include "KeyListener.h"
#include "WindowPanel.h"
#include "InputLine.h"
#include "Settings.h"
#include "MouseMap.h"

// 
// a bar across the bottom of the screen, for displaying a line number, 
// and jumping to another line.
// 
// panel-based.
// 

    class 
LineNumBar 
    {
    ////////// inner types
        protected: enum tAction {  EDIT_GOTO, GOTO,  CANCEL  };


    ////////// member data 
        public: static const int UNKNOWN = -1;
    
        public: WindowPanel mBar;
        public: InputLine mInputNumber;

        protected: MouseMap<tAction> mMouseToAction;


    ////////// construction

            public:
        LineNumBar( )
            {
            }

            public: virtual 
        ~LineNumBar( ) 
            {
            }



    ////////// methods -- data access



    ////////// methods

            public: void
        CreatePanel( bool resizeOnly=false )
            {
            // find screen size
            int y_size, x_size;
            getmaxyx( stdscr, y_size, x_size );

            std::string current_label = "Current line ";
            std::string goto_label = "(ENTER) Go to line ";
            std::string cancel_label = "(ESC) Cancel";
            int input_size = x_size - 1 - current_label.length() - 10
                                    - 1 - goto_label.length() 
                                    - 4 - cancel_label.length() - 1;

            // make bar
            Point bar_start ( y_size - 1, 0 );
            mBar.Create( 1, x_size, bar_start.Y, bar_start.X );
            wattron( mBar.Window(), A_REVERSE );
            mvwhline( mBar.Window(), 0, 0, ' ', x_size );

            // make fields
            // current
            Point field_start ( 0, 0 );
            Print( " ", field_start );
            Print( current_label, field_start );
            mvwprintw( mBar.Window(), field_start.Y, field_start.X,  "%-10d", 0 );
            getyx( mBar.Window(), field_start.Y, field_start.X );
            // goto label
            Print( " ", field_start );
            mMouseToAction.Add(  GOTO,  field_start,  Point( field_start.Y + 1,  field_start.X + goto_label.length() ) );
            Print( goto_label, field_start );
            // goto input
            mMouseToAction.Add(  EDIT_GOTO,  field_start,  Point( field_start.Y + 1,  field_start.X + input_size ) );
            if ( resizeOnly ){
                mInputNumber.ResizeWindow( input_size,  bar_start.Y + field_start.Y,  bar_start.X + field_start.X );
                }
            else {
                mInputNumber.CreatePanel( input_size, 
                                          bar_start.Y + field_start.Y,  bar_start.X + field_start.X, 
                                          A_UNDERLINE, "" );
                }
            field_start.X += input_size;
            // cancel label
            Print( "    ", field_start );
            mMouseToAction.Add(  CANCEL,  field_start,  Point( field_start.Y + 1,  field_start.X + cancel_label.length() ) );
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
        SetInputNumber( int line_number )
            {
            mInputNumber.SetText(  TextFunctions::ToString( line_number )  );
            }


            public: int // returns line number, or UNKNOWN if cancelled
        Run( int current_line, KeyListener& resizeListener )
            {
            mInputNumber.MoveLineEnd();

            // display current line
            mvwprintw( mBar.Window(), 0, 0, " Current line %-10d", current_line );

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
                    bool number_set; 
                    int line_number;
                    HandleMouseEvent( mouse_event, number_set, line_number );
                    if ( number_set ) return line_number;
                    }
                else if ( i == KEY_RESIZE ) {  int result = Cancel();  resizeListener.onKey(i);  return result;  }
                else if ( i == -1 ) {  }   // Ignore noise from window switching
                else if ( Settings::Get().KeyHasFunction( i, Settings::MENU ) )
                    {
                    return Cancel();
                    }
                else if ( c == '\n' ) 
                    {
                    Hide();
                    update_panels();
                    doupdate();
                    return atoi( mInputNumber.GetText().c_str() );
                    }
                else if ( Settings::Get().KeyHasFunction( i, Settings::BACKWARD ) )  mInputNumber.MoveLeft();
                else if ( Settings::Get().KeyHasFunction( i, Settings::FORWARD ) )   mInputNumber.MoveRight();
                else if ( Settings::Get().KeyHasFunction( i, Settings::HOME ) )      mInputNumber.MoveLineStart();
                else if ( Settings::Get().KeyHasFunction( i, Settings::END ) )       mInputNumber.MoveLineEnd();
                else if ( Settings::Get().KeyHasFunction( i, Settings::DELETE ) )    mInputNumber.Delete();
                else if ( Settings::Get().KeyHasFunction( i, Settings::BACKSPACE ) ) mInputNumber.Backspace();
                else if ( isdigit( c ) )                     
                    {
                    mInputNumber.Insert( std::string ( 1, c ) );
                    }
                }
            }


            protected: void
        HandleMouseEvent( const MEVENT& mouse_event, 
                          bool& number_set, int& line_number )  // modified
            {
            number_set = false;
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

                    if ( action == EDIT_GOTO )
                        {
                        mInputNumber.HandleMouseEvent( mouse_event );
                        }
                    else if ( action == GOTO )
                        {
                        Hide();
                        update_panels();
                        doupdate();
                        number_set = true;
                        line_number = atoi( mInputNumber.GetText().c_str() );
                        }
                    else if ( action == CANCEL ) 
                        {
                        number_set = true;
                        line_number = Cancel();
                        }
                    }
                }
            else
                {
                number_set = true;
                line_number = Cancel();
                }
            }


            protected: int
        Cancel( ) 
            {
            Hide();
            update_panels();
            doupdate();
            return UNKNOWN;
            }


            protected: void
        Hide( )
            {
            mInputNumber.Hide();
            hide_panel( mBar.Panel() );
            }

            protected: void
        Show( )
            {
            show_panel( mBar.Panel() );
            mInputNumber.Show();
            }

    };



////////////////////////////////////////////////////////////////////////////////
#endif // #ifndef INCLUDED_LineNumBar_h
