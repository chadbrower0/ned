#ifndef INCLUDED_InputLine_H
#define INCLUDED_InputLine_H
////////////////////////////////////////////////////////////////////////////////
#include <Shared/TextFunctions.h>
#include <Shared/String.h>
#include "CharData.h"
#include "ViewPanel.h"

// 
// a simple single text line editor, for forms input.
//     no selecting
//     no copying
//     no undo
//     no block indent nor comment
// 
// panel-based implementation.
// uses ViewPanel for left/right scrolling.
// 
// use by instantiating (takes control), then read InputLine.mText, then destroy.
// 

    class 
InputLine 
    {
    ////////// inner types

		public: class
	Position
		{
		public: long mTextIndex;
		public: int mDisplayX;

			public:
		Position( long text_index = -1, int display_x = -1 )
			{
			mTextIndex = text_index;
			mDisplayX = display_x;
			}

			public: bool
		operator== ( const Position& other ) const
			{
			return ( mTextIndex == other.mTextIndex  &&  mDisplayX == other.mDisplayX );
			}

			public: bool
		operator!= ( const Position& other ) const
			{
			return ( mTextIndex != other.mTextIndex  ||  mDisplayX != other.mDisplayX );
			}

			public: std::string
		ToString( ) const
			{
			return TextFunctions::ToString( mTextIndex ) + ":" + TextFunctions::ToString( mDisplayX );
			}
		};

            public: const Position& // returns position
        Increment( Position& position ) const // modified
            {
            if ( position.mTextIndex >= TextSize() ) throw Exception ( (String) "incrementing position " + position.ToString() + " greater than text size " + TextFunctions::ToString( TextSize() ), __FILE__, __FUNCTION__, __LINE__ ); 
//            position.mDisplayX += DisplaySize( position );
position.mDisplayX += mView.TextCharToDisplayLength( position.mTextIndex, position.mDisplayX );
            position.mTextIndex++;

            return position;
            }

            public: const Position& // returns position
        Decrement( Position& position ) const // modified
            {
            if ( position.mTextIndex <= 0 ) throw Exception ( (String) "decrementing position " + position.ToString() + " less than or equal to 0.", __FILE__, __FUNCTION__, __LINE__ );

//             position.mDisplayX -= DisplaySize( mText.at( position.mTextIndex - 1 ) );
// position.mDisplayX -= mView.TextCharToDisplayLength( position.mTextIndex - 1 );
//             position.mTextIndex--;
// cannot know how long position.mTextIndex-1 will be, unless we know its x coordinate -- paradox
Position new_position ( 0, 0 );
while ( new_position.mTextIndex+1 < position.mTextIndex )
    Increment( new_position );
position = new_position;

            return position;
            }



    ////////// member data 
//         protected: std::string mText;
protected: std::vector<CharData> mText;
        protected: ViewPanel mView;
        protected: int mAttributes;

        protected: Position mCursor;


    ////////// construction

            public:
        InputLine( )
            {
mView.SetText( mText );
mView.mTabDisplayText = "\\t";
mView.mNewlineDisplayText = "\\n";
            }

            public: virtual 
        ~InputLine( ) {  }

            public: void
        CreatePanel( int x_size, int y, int x, int attributes, const std::string& start_text )
            {
            mView.CreatePanel( 1, x_size, y, x );
            mView.mBackgroundChar = ( ' ' | attributes );

            mAttributes = attributes;
//             mText = start_text;
// mText.clear();
// for ( long i = 0;  i < start_text.length();  ++i )
//     {
//     mText.push_back( CharData ( start_text.at(i), mAttributes ) );
// if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "InputLine::CreatePanel  character=" + mText.back().GetCharacter() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
//     }
SetText( start_text );

            mCursor.mTextIndex = 0;
            mCursor.mDisplayX = 0;

            FormatLine();
            wmove( mView.Window(), 0, mCursor.mDisplayX - mView.GetWindowStart().X );
            }



    ////////// methods -- data access
/*
            public: const std::string&
        GetText( ) const
            {
            return mText;
            }
*/

            public: std::string
        GetText( ) const
            {
            std::string text;
            for ( long t = 0;  t < mText.size();  ++t )
                text += mText.at(t).GetCharacter();
            return text;
            }

            public: long
        TextSize( ) const {  return mText.size();  }

            public: void
        SetText( const std::string& new_text )
            {
//             mText = new_text;
mText.resize( 0 );
for ( long c = 0;  c < new_text.length();  ++c )
    {
    mText.push_back( CharData ( new_text.at(c), 0 ) );
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "InputLine::SetText  character=" + new_text.at(c) + "=" + mText.back().GetCharacter() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
    }

            mCursor.mTextIndex = 0;
            mCursor.mDisplayX = 0;

            FormatLine();
            }



    ////////// methods -- display

            public: void
        Hide( ) 
            {
            hide_panel( mView.Panel() );
            }

            public: void
        Show( ) 
            {
            show_panel( mView.Panel() );
            }

            protected: void
        FormatLine( )
            // updates yx data and view.  does not refresh screen.
            {
/*
            mView.ClearLine( 0 );
            // for each text char... set display 
            int x = 0;
            for ( int i = 0;  i < mText.size();  i++ )
                {
                char c = mText.at(i);
                std::string char_display = CharToDisplay( c );
                for ( int d = 0;  d < char_display.size();  d++ )
                    {
                    mView.SetChar( 0, x, char_display.at(d), mAttributes );
                    x++;
                    }
                }
*/
//             mView.UpdateGridToWindow();
// mView.UpdateWholeWindow( mCursor.mTextIndex );
mView.UpdateWholeWindow();
            }

/*
            protected: int
        DisplaySize( Position p ) const
            {
            return DisplaySize( mText.at( p.mTextIndex ) );
            }

            protected: int
        DisplaySize( char c ) const
            {
            return CharToDisplay(c).size();
            }

            protected: std::string
        CharToDisplay( char c ) const
            {
            if ( c == '\n' ) return "\\n";
            else if ( c == '\t' ) return "\\t";
            else return std::string ( "" ) + c;
            }
*/



    ////////// methods -- navigation


            public: void
        MoveRight( ) 
            {
            if ( mCursor.mTextIndex >= mText.size() ) return;
            Increment( mCursor );
            NavigationViewUpdate();
            }

            public: void
        MoveLeft( ) 
            {
            if ( mCursor.mTextIndex <= 0 ) return;
            Decrement( mCursor );
            NavigationViewUpdate();
            }

            public: void
        MoveLineEnd( )
            {
            if ( mCursor.mTextIndex >= mText.size() ) return;

            while ( mCursor.mTextIndex < mText.size() )
                {
                Increment( mCursor );
                }

            NavigationViewUpdate();
            }

            public: void
        MoveLineStart( )
            {
            if ( mCursor.mTextIndex <= 0 ) return;

            while ( mCursor.mTextIndex > 0 )
                {
                Decrement( mCursor );
                }

            NavigationViewUpdate();
            }

            public: void
        MoveNextWord( )
            {
            // move to end of current word
            while ( mCursor.mTextIndex < mText.size()  &&  IsAlpha( mText.at(mCursor.mTextIndex) ) )
                {
		Increment( mCursor );
                }
            // move to start of next word
            while ( mCursor.mTextIndex < mText.size()  &&  ! IsAlpha( mText.at(mCursor.mTextIndex) ) )
                {
		Increment( mCursor );
                }
            NavigationViewUpdate();
            }

            protected: void
        NavigationViewUpdate( )
            {
            EnsureCursorOnScreen();
            wmove( mView.Window(), 0, mCursor.mDisplayX - mView.GetWindowStart().X );
            update_panels();
            doupdate();
            }

            protected: bool // returns whether view was moved
        EnsureCursorOnScreen( )
            // scrolls left/right to keep cursor within middle 50% of display area.
            // may update view.  does not move cursor.  does not refresh window.
            {
            Point size = GetScreenSize();
            Point view_start = mView.GetWindowStart();

            // adjust view x
            while ( view_start.X > 0  &&  mCursor.mDisplayX < view_start.X + (size.X/4) )
                view_start.X -= 1;
            while ( mCursor.mDisplayX >= view_start.X + ((size.X*3)/4) )
                view_start.X += 1;

            // move view?
            if ( view_start != mView.GetWindowStart() )
                {
                mView.MoveWindow( view_start, 0 );  // mCursor.mTextIndex
                return true;
                }
            else
                return false;
            }

            protected: Point
        GetScreenSize( ) const
            {
            int size_y, size_x;
            getmaxyx( mView.Window(), size_y, size_x );
            return Point ( size_y, size_x );
            }

            protected: bool
        IsAlpha( const CharData& c ) const
            {
//             return ( ! c.IsBlank()  &&  IsAlpha( c.GetCharacter() ) );
return IsAlpha( c.GetCharacter() );
            }

            protected: bool
        IsAlpha( char c ) const
            {
            return ( ( 'a' <= c  &&  c <= 'z' )  ||  ( 'A' <= c  &&  c <= 'Z' ) );
            }


            public: void
        HandleMouseEvent( const MEVENT& mouse_event )
            {
            if ( wenclose( mView.Window(), mouse_event.y, mouse_event.x ) )
                {
                // translate mouse_event coordinates to window-relative coordinates
                Point window_start;
                getbegyx( mView.Window(), window_start.Y, window_start.X );
                Point mouse_relative_window = Point (  mouse_event.y - window_start.Y ,  mouse_event.x - window_start.X );

                if ( mouse_event.bstate & BUTTON1_CLICKED )
                    {
                    // move backward to position
                    if ( mouse_relative_window.X < mCursor.mDisplayX )
                        {
                        while ( mCursor.mTextIndex > 0  &&  mouse_relative_window.X < mCursor.mDisplayX )
                            MoveLeft();
                        }
                    // move forward to position
                    else if ( mouse_relative_window.X > mCursor.mDisplayX )
                        {
                        while ( mCursor.mTextIndex < mText.size()  &&  mouse_relative_window.X > mCursor.mDisplayX )
                            MoveRight();
                        }
                    }
                }
            }


    ////////// methods -- editing

            public: void
        Insert( const std::string& insert_text )
            {
            // for each inserted character... 
            for ( long t = 0;  t < insert_text.size();  t++ )
                {
//                // convert to display text
//                char c = insert_text.at(t);
//                std::string display_char = CharToDisplay( c );
CharData char_data ( insert_text.at(t), 0 );

                // insert
//                 mText.insert( mCursor.mTextIndex, std::string ( 1, c ) );
mText.insert( mText.begin() + mCursor.mTextIndex, char_data );
//                for ( int d = 0, x = mCursor.mDisplayX;  d < display_char.size();  d++, x++ )
//                    {
//                    mView.InsertChar( 0, x, display_char.at(d), mAttributes );
//                    }

                Increment( mCursor );
                }

            FormatLine();
            EnsureCursorOnScreen();
            wmove( mView.Window(), 0, mCursor.mDisplayX - mView.GetWindowStart().X );
            update_panels();
            doupdate();
            }

            public: void
        Delete( )
            {
            if ( mCursor.mTextIndex >= mText.size() ) return;

//            char c = mText.at( mCursor.mTextIndex );
//            std::string display_char = CharToDisplay( c );
//            for ( int d = 0;  d < display_char.size();  d++ )
//                {
//                mView.DeleteChar( 0, mCursor.mDisplayX );
//                }
//            mText.erase( mCursor.mTextIndex, 1 );
mText.erase( mText.begin() + mCursor.mTextIndex );

            FormatLine();
            EnsureCursorOnScreen();
            wmove( mView.Window(), 0, mCursor.mDisplayX - mView.GetWindowStart().X );
            update_panels();
            doupdate();
            }

            public: void
        Backspace( )
            {
            if ( mCursor.mTextIndex <= 0 ) return;
            Decrement( mCursor );
            Delete();
            }



    };



////////////////////////////////////////////////////////////////////////////////
#endif // #ifndef INCLUDED_InputLine_H
