#ifndef INCLUDED_YesNoBar_h
#define INCLUDED_YesNoBar_h
////////////////////////////////////////////////////////////////////////////////
#include "WindowPanel.h"
#include "Settings.h"
#include "MouseMap.h"

// 
// a bar across the bottom of the screen, for asking a yes/no question.
// 
// panel-based.
// 

    class 
YesNoBar 
    {
    ////////// inner types
        public: enum Answer { Yes, No, Cancel };


    ////////// member data 
        public: WindowPanel mBar;
        public: bool mAllowCancel;
//         public: const Settings& mSettings;

        protected: MouseMap<Answer> mMouseToAction;



    ////////// construction

            protected:
        YesNoBar( ) // const Settings& settings )
//             :
//             mSettings ( settings ) 
            {
            mAllowCancel = false;
            }

            public: virtual 
        ~YesNoBar( ) 
            {
            }

            public: static Answer
        Ask( // const Settings& settings, 
             const std::string& question, bool allow_cancel )
            {
            YesNoBar bar;  //  ( settings );
            bar.mAllowCancel = allow_cancel;
            bar.CreatePanel( question );
            update_panels();
            doupdate();

            Answer answer = bar.Run();

            bar.mBar.Clear();
            update_panels();
            doupdate();

            return answer;
            }



    ////////// methods -- data access



    ////////// methods

            protected: void
        CreatePanel( const std::string& question )
            {
            // find screen size
            int y_size, x_size;
            getmaxyx( stdscr, y_size, x_size );

            // make bar
            Point bar_start ( y_size - 1, 0 );
            mBar.Create( 1, x_size, bar_start.Y, bar_start.X );
            wattron( mBar.Window(), A_REVERSE );
            mvwhline( mBar.Window(), 0, 0, ' ', x_size );

            std::string yes_label = "(Y) Yes";
            std::string no_label = "(N) No";
            std::string cancel_label = ( mAllowCancel )?  "(ESC) Cancel"  :  "";

            // make fields
            // question
            Point field_start ( 0, 0 );
            Print( "  ", field_start );
            Print( question, field_start );
            // yes
            Print( "    ", field_start );
            mMouseToAction.Add(  Yes,  field_start,  Point( field_start.Y + 1,  field_start.X + yes_label.length() ) );
            Print( yes_label, field_start );
            // no
            Print( "    ", field_start );
            mMouseToAction.Add(  No,  field_start,  Point ( field_start.Y + 1,  field_start.X + no_label.length() ) );
            Print( no_label, field_start );
            // cancel
            Print( "    ", field_start );
            mMouseToAction.Add(  Cancel,  field_start,  Point ( field_start.Y + 1,  field_start.X + cancel_label.length() ) );
            Print( cancel_label, field_start );
            }

            protected: void
        Print( const std::string& text, Point& field_start ) // modifies field_start
            {
            mvwprintw( mBar.Window(), field_start.Y, field_start.X, "%s", text.c_str() );
            getyx( mBar.Window(), field_start.Y, field_start.X );
            }

            protected: Answer
        Run( )
            {
            // take input
            while ( true )
                {
                int i = getch();
                char c = (char) i;

                if ( i == KEY_MOUSE )
                    {
                    // get mouse event data
                    MEVENT mouse_event;
                    int result = getmouse( & mouse_event );
                    if ( result == ERR ) throw Exception ( "i = KEY_MOUSE  and  getmouse() = ERR", __FILE__, __FUNCTION__, __LINE__ );
                    if ( result != OK ) throw Exception ( "i = KEY_MOUSE and getmouse() = " + TextFunctions::ToString( result ) + " unhandled", __FILE__, __FUNCTION__, __LINE__ );
                    // handle mouse event
                    bool handled;
                    Answer answer;
                    HandleMouseEvent( mouse_event, handled, answer );
                    if ( handled ) return answer;
                    }
                else if ( Settings::Get().KeyHasFunction( i, Settings::MENU )  &&  mAllowCancel ) 
                    {
                    return Cancel;
                    }
                else if ( c == 'y'  ||  c == 'Y' )
                    {
                    return Yes;
                    }
                else if ( c == 'n'  ||  c == 'N' )
                    {
                    return No;
                    }
                }
            }


            protected: void
        HandleMouseEvent( const MEVENT& mouse_event, 
                          bool& handled, Answer& answer )  // modified 
            {
            // match against bar text?
            // or store map of position to function?
            //     + works even if there's a text input field that happens to match the button/label/name

// std::cerr << "\n\n" << "HandleMouseEvent()";

            handled = false;

            // get mouse event
            if ( wenclose( mBar.Window(), mouse_event.y, mouse_event.x ) )
                {
                // translate mouse_event coordinates to window-relative coordinates
                Point window_start;
                getbegyx( mBar.Window(), window_start.Y, window_start.X );
                Point mouse_relative_window = Point (  mouse_event.y - window_start.Y ,  mouse_event.x - window_start.X );

// std::cerr << "\n\t" << "mouse_relative_window = " << mouse_relative_window.ToString();

                if ( mouse_event.bstate & BUTTON1_CLICKED )
                    {
                    // find option position matching mouse_event...
                    mMouseToAction.FindAction( mouse_relative_window, handled, answer );
                    }
                }
            else if ( mAllowCancel )
                {
                handled = true;
                answer = Cancel;
                }
            }



    };



////////////////////////////////////////////////////////////////////////////////
#endif // #ifndef INCLUDED_YesNoBar_h
