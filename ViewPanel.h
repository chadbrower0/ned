#ifndef INCLUDED_ViewPanel_h
#define INCLUDED_ViewPanel_h
////////////////////////////////////////////////////////////////////////////////
#include <curses.h>
#include <fstream>
#include <vector>
#include <Shared/String.h>
#include "WindowPanel.h"
#include "Point.h"
#include "CharData.h"
#include "Settings.h"

//
// panel which is a view of some underlying "data window".
//     data moves from text string -> yx grid -> ncurses window.
//
// Character display/sizes are different for InputLine versus InputArea (e.g. newline).
// And tab display size varies by x position.
//

    class
ViewPanel
    {
    ////////// member data
        protected: const std::vector<CharData>* mText;  // not owner
        protected: WindowPanel mWindow;
        protected: Point mWindowStart;
        protected: long mWindowStartCharIndex;
        public: chtype mBackgroundChar;
        public: bool mClearRedundantly;

        public: String mTabDisplayText;       // may be "" for default tab display
        public: String mNewlineDisplayText;   // may be "" for default newline display


    ////////// construction

            public:
        ViewPanel( const std::vector<CharData>& text )  // const std::string& text )
            :
            mText ( & text )
            {
            mBackgroundChar = ' ';
            mClearRedundantly = false;
            }

            public:
        ViewPanel( )
            :
            mText ( NULL )
            {
            mBackgroundChar = ' ';
            mClearRedundantly = false;
            }

            public:
        ~ViewPanel( )
            {
            }

            public: void
        SetText( const std::vector<CharData>& text ) {  mText = & text;  }

            public: void
        CreatePanel( int y_size, int x_size, int y, int x )
            {
            mWindow.Create( y_size, x_size, y, x );
            mWindowStart = Point ( 0, 0 );
            mWindowStartCharIndex = 0;
            }



    ////////// methods -- data access

            public: WINDOW* // caller does not own returned window
        Window( ) {  return mWindow.Window();  }

            public: const WINDOW* // caller does not own returned window
        Window( ) const {  return mWindow.Window();  }

            public: PANEL* // caller does not own returned panel
        Panel( ) {  return mWindow.Panel();  }

            public: const Point&
        GetWindowStart( ) const {  return mWindowStart;  }

            public: long
        GetWindowStartCharIndex( ) const {  return mWindowStartCharIndex;  }

            public: bool
        InWindow( int y, int x ) const
            {
            int y_size, x_size;
            getmaxyx( mWindow.Window(), y_size, x_size );

            int screen_y = y - mWindowStart.Y;
            int screen_x = x - mWindowStart.X;

            return ( 0 <= screen_y  &&  screen_y < y_size  &&
                     0 <= screen_x  &&  screen_x < x_size );
            }



    ////////// methods -- updating grid -> ncurses window

            public: void
        MoveWindow( Point window_start, long window_start_char_index )  // char_index must match start of display line window_start.Y
            // updates grid -> ncurses window.  does not update screen.
            { PSEUDOSTACK_START
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "MoveWindow()", Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            mWindowStart = window_start;
            mWindowStartCharIndex = window_start_char_index;
            UpdateWholeWindow();  // window_start_char_index );

            PSEUDOSTACK_END
            }


            public: void
        UpdateWholeWindow( )
            // Assumes that mWindowStartCharIndex and mWindowStart did not change.
            // updates text -> ncurses window.  does not update screen.
            { PSEUDOSTACK_START
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "UpdateWholeWindow()", Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            long char_index = mWindowStartCharIndex;

            int y_size, x_size;
            getmaxyx( mWindow.Window(), y_size, x_size );

            if ( mClearRedundantly ) wclear( mWindow.Window() );

            // for each grid line... copy data to window
            int y, window_y;
            long text_i;
            for ( window_y = 0,  y = mWindowStart.Y,  text_i = char_index;  window_y < y_size  &&  text_i < (long) mText->size();  ++window_y, ++y )
                {
                text_i = UpdateTextLineToWindow( y, text_i );
                }

            // clear rest of window
            for (  ;  window_y < y_size;  window_y++ )
                for ( int window_x = 0;  window_x < x_size;  window_x++ )
                    ClearCharToWindow( window_y, window_x );

            PSEUDOSTACK_END
            }


            public: long  // returns text index of start of next display line
        UpdateTextLineToWindow( int y, long char_index )  // char_index must match start of grid line y
            { PSEUDOSTACK_START
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "UpdateTextLineToWindow( y=" + y + ", char_index=" + char_index + " ) " , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
// std::cerr << "\n UpdateTextLineToWindow() y=" << y << "  char_index=" << char_index;

            int y_size, x_size;
            getmaxyx( mWindow.Window(), y_size, x_size );

//             int window_y = y - mWindowStart.Y;

            // for each text character on this display line... (folds may encompass many text lines)
            long text_i = char_index;
            int x = 0;
            int window_end_x = mWindowStart.X + x_size;
            for (  ;  text_i < mText->size();  ++text_i )
                {
                CharData c = (*mText)[text_i];
// if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "c = " + c.GetCharacter() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
                // fold start 
                // Always displayed on one line.
                // Only the outermost fold is displayed.
                if ( c.mFoldState == CharData::FOLD_START  &&  c.mFoldLevel == 1 )  
                    { 
                    x = UpdateTextCharToWindow( y, x, c );
                    // advance text to end of outermost fold (no display for folded text)
                    for ( ++text_i;  text_i < mText->size();  ++text_i )
                        {
                        CharData c2 = (*mText)[text_i];
                        if ( c2.mFoldState == CharData::FOLD_END  &&  c2.mFoldLevel == 1 ) 
                            {
                            // display closing bracket
                            if ( mWindowStart.X <= x  &&  x < window_end_x )
                                x = UpdateTextCharToWindow( y, x, c2 );
                            break;
                            }
                        }
                    }
                else if ( c.mFoldState == CharData::FOLD_END ) throw Exception ( (String) "Unexpected FOLD_END without FOLD_START", __FILE__, __FUNCTION__, __LINE__ ); 
                else if ( c.mFoldLevel > 0 )
{
// for ( long ei = 0;  ei < mText->size();  ++ei )
//  {
//  CharData ec = (*mText)[ei];
//  std::cerr << "\n index=" << ei
//  << " \t mCharacter=" << ec.GetCharacter()
//  << " \t mFoldLevel=" << ec.mFoldLevel
//  << " \t mFoldState=" << CharData::FoldStateToString(ec.mFoldState);
//  }
throw Exception ( (String) "Unexpected mFoldLevel=" + c.mFoldLevel + " without FOLD_START at index=" + text_i , __FILE__, __FUNCTION__, __LINE__ ); 
}
                // tab -- always displayed on one line
                else if ( c.GetCharacter() == '\t' )
                    {
                    x = UpdateTextCharToWindow( y, x, c );
                    }
                // newline -- done with line
                else if ( c.GetCharacter() == '\n' )
                    {
                    if ( mNewlineDisplayText == "" )
                        {
                        x = UpdateTextCharToWindow( y, x, c );
                        ++y;
                        ++text_i;
                        break;
                        }
                    else
                        {
                        CharData newline_char = c;
                        for ( int n = 0;  n < mNewlineDisplayText.length();  ++n )
                            {
                            newline_char.SetCharacter( mNewlineDisplayText.at(n) );
                            x = UpdateTextCharToWindow( y, x, newline_char );
                            }
                        }
                    }
                // normal character
                else
                    {
                    x = UpdateTextCharToWindow( y, x, c );
                    }
                }

            // clear remainder of screen line
            for (  ;  x < window_end_x;  ++x )
                ClearCharToWindow( y-mWindowStart.Y, x-mWindowStart.X );

            return text_i;

            PSEUDOSTACK_END
            }


            public: int  // returns x location of character end
        UpdateTextCharToWindow( int y, int x, const CharData& c )  // must match each other
            { PSEUDOSTACK_START
// if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "UpdateTextCharToWindow(" + c.GetCharacter() + ")" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            int y_size, x_size;
            getmaxyx( mWindow.Window(), y_size, x_size );

            int window_y = y - mWindowStart.Y;
            int window_x = x - mWindowStart.X;
            int window_end_x = mWindowStart.X + x_size;

                // fold start -- Only the outermost fold is displayed.
                if ( c.mFoldState == CharData::FOLD_START  &&  c.mFoldLevel == 1 )
                    {
                    // display outer bracket
                    if ( mWindowStart.X <= x  &&  x < window_end_x )
                        UpdateCharToWindow( window_y, window_x, c );
                    ++x;  ++window_x;
                    // display fold marker
                    CharData fold_char = c;
                    fold_char.SetCharacter( '.' );
                    fold_char.mAttributes = fold_char.mAttributes | A_UNDERLINE;
                    for ( int i = 0;  i < Settings::FOLD_VISIBLE_LENGTH;  ++i, ++x, ++window_x )
                        if ( mWindowStart.X <= x  &&  x < window_end_x )
                            UpdateCharToWindow( window_y, window_x, fold_char );
                    return x;
                    }
                else if ( c.mFoldState == CharData::FOLD_END  &&  c.mFoldLevel == 1 )
                    {
                    // display closing bracket
                    if ( mWindowStart.X <= x  &&  x < window_end_x )
                        UpdateCharToWindow( window_y, window_x, c );
                    ++x;  ++window_x;
                    return x;
                    }
                else if ( c.mFoldLevel > 0 )
                    {
                    return x;
                    }
                // tab -- size must match next tab stop
                else if ( c.GetCharacter() == '\t' )
                    {

if ( mTabDisplayText == "" )
    {
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "mTabDisplayText=''", Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
                    CharData tab_char = c;
                    tab_char.SetCharacter( ' ' );
                    for ( int i = 0;  i == 0  ||  x % Settings::Get().tab_size != 0;  ++i, ++x, ++window_x )
                        {
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t tab_char='" + tab_char.GetCharacter() + "'" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
                        if ( mWindowStart.X <= x  &&  x < window_end_x )
                            UpdateCharToWindow( window_y, window_x, tab_char );
                        }
                   return x;
    }
else
    {
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "mTabDisplayText='" + mTabDisplayText + "'" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
    CharData tab_char = c;
    for ( int t = 0;  t < mTabDisplayText.length();  ++t, ++x, ++window_x )
        {
        tab_char.SetCharacter( mTabDisplayText.at(t) );
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t tab_char='" + tab_char.GetCharacter() + "'" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
        if ( mWindowStart.X <= x  &&  x < window_end_x )
            UpdateCharToWindow( window_y, window_x, tab_char );
        }
    return x;
    }


                   }
                // newline
                else if ( c.GetCharacter() == '\n' )
                    {

if ( mNewlineDisplayText == "" )
    {
                    CharData newline_char = c;
                    newline_char.SetCharacter( ' ' );
                    if ( mWindowStart.X <= x  &&  x < window_end_x )
                        UpdateCharToWindow( window_y, window_x, newline_char );
                    ++x;  ++window_x;
                    for (  ;  x < window_end_x;  ++x, ++window_x )
                        if ( mWindowStart.X <= x  &&  x < window_end_x )
                            ClearCharToWindow( window_y, window_x );
                    return x;
    }
else
    {
    CharData newline_char = c;
    for ( int n = 0;  n < mNewlineDisplayText.length();  ++n, ++x, ++window_x )
        {
        newline_char.SetCharacter( mNewlineDisplayText.at(n) );
        if ( mWindowStart.X <= x  &&  x < window_end_x )
            UpdateCharToWindow( window_y, window_x, newline_char );
        }
    return x;
    }

                    }
                // normal character
                else 
                    {  
                    if ( mWindowStart.X <= x  &&  x < window_end_x )
                        UpdateCharToWindow( window_y, window_x, c );
                    ++x;  ++window_x;
                    return x;
                    }

            PSEUDOSTACK_END
            }


            public: int  
        TextCharToDisplayLength( long text_i, 
int x ) const
            {
            return TextCharToDisplayLength( (*mText)[text_i], x );
            }
 


            public: int  
        TextCharToDisplayLength( CharData c, 
int x ) const
            {  PSEUDOSTACK_START
                // fold -- Only the outermost fold is displayed.
                if ( c.mFoldState == CharData::FOLD_START  &&  c.mFoldLevel == 1 )  
                    { 
                    // display starting bracket
                    // display fold marker
                    return 1 + Settings::FOLD_VISIBLE_LENGTH;
                    }
                else if ( c.mFoldState == CharData::FOLD_END  &&  c.mFoldLevel == 1 )
                    {
                    // display closing bracket
                    return 1;
                    }
                else if ( c.mFoldLevel > 0 ) 
                    {
                    return 0;
                    }
                // tab -- size must match next tab stop
                else if ( c.GetCharacter() == '\t' ) 
                    {  
if ( mTabDisplayText == "" ) 
{
for ( int tab_display_size = 1;  ;  ++tab_display_size )
    if ( ( x + tab_display_size ) % Settings::Get().tab_size == 0 ) 
        return tab_display_size;
throw Exception ( (String) "could not set tab_display_size" , __FILE__, __FUNCTION__, __LINE__ ); 
}
else return mTabDisplayText.length();
                    }  
                // newline 
                else if ( c.GetCharacter() == '\n' )   
                    {

if ( mNewlineDisplayText == "" )
                    return 1;
else
    return mNewlineDisplayText.length();

                    }
                // normal character
                else 
                    {  
                    return 1;
                    }

            PSEUDOSTACK_END
            }


            protected: void
       	UpdateCharToWindow( int window_y, int window_x, const CharData& char_data )
            {  PSEUDOSTACK_START

            chtype formatted_char = char_data.ToFormattedChar();
            if ( mvwinch( mWindow.Window(), window_y, window_x ) != formatted_char )
                mvwaddch( mWindow.Window(), window_y, window_x, formatted_char );

            PSEUDOSTACK_END
            }

            protected: void
        ClearCharToWindow( int window_y, int window_x )
            {  PSEUDOSTACK_START

            if ( mvwinch( mWindow.Window(), window_y, window_x ) != mBackgroundChar  ||  mClearRedundantly )
                mvwaddch( mWindow.Window(), window_y, window_x, mBackgroundChar );

            PSEUDOSTACK_END
            }


    };



////////////////////////////////////////////////////////////////////////////////
#endif 

