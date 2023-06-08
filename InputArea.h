#ifndef INCLUDED_InputArea_h
#define INCLUDED_InputArea_h
//////////////////////////////////////////////////////////////////////
#include <fstream>
#include <string.h>
#include "MessageBar.h"
#include "Point.h"
#include "ViewPanel.h"
#include "CharData.h"

//
// contains the following data layers
//     text -- the data as a string.  almost the same as the data in the file.
//     yx grid -- the data arranged in a rectangular grid, as it will be displayed.
//                Not explicitly stored;  just derived on demand.
//     window -- the portion of the grid which can be shown on the screen.
//
// cursor is theoretically between characters, before the character at its index,
// and after the preceding character.
//
// best to publish from text to yx in lines/blocks?
//     + easier than synchronizing piece by piece.
//     - less efficient (but not by much?)
//     - requires knowing when to publish
//
// undo
//     implement undo-indent a single change?
//         - if unindent had no effect, then indent causes uneven changes.
//     store unindent change as a series of deletes/inserts?
//         + works
//         + can be applied as one undo action, for bandwith & usability
//



    class
InputArea
    {
    ////////// inner types -- position

            public: class
        Position
            {
            public: long mTextIndex;
            public: Point mGridPoint;

                public:
            Position( long text_index = 0, const Point& display_point = Point ( 0, 0 ) )
                {
                mTextIndex = text_index;
                mGridPoint = display_point;
                }

                public: bool
            operator== ( const Position& other ) const
                {
                return ( mTextIndex == other.mTextIndex  &&  mGridPoint == other.mGridPoint );
                }

                public: bool
            operator!= ( const Position& other ) const
                {
                return ( mTextIndex != other.mTextIndex  ||  mGridPoint != other.mGridPoint );
                }

                public: String
            ToString( ) const
                {
                return (String) "" + mTextIndex + ":" + mGridPoint.ToString();
                }
            };


            public: const Position& // returns position
        Increment( Position& position ) const // modified
            {  PSEUDOSTACK_START
// if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "InputArea::Increment()", Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            if ( position.mTextIndex >= TextSize() ) throw Exception ( (String) "incrementing position " + position.ToString() + " greater than text size " + TextSize() , __FILE__, __FUNCTION__, __LINE__ );

            CharData c = mText.at( position.mTextIndex );
            int display_length = mView.TextCharToDisplayLength( c, position.mGridPoint.X );
            if ( c.GetCharacter() == '\n'  &&  display_length > 0 )
                {
                position.mGridPoint.Y++;
                position.mGridPoint.X = 0;
                }
            else
                {
                position.mGridPoint.X += display_length;
                }
            ++ position.mTextIndex;
            return position;

            PSEUDOSTACK_END
            }


            public: Position
        FindPosition( long target_index ) const
            // re-positioning from start of text
            // a bit slow
            {  PSEUDOSTACK_START
// if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "InputArea::FindPosition()  target_index=" + target_index , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
            if ( target_index < 0 ) throw Exception ( (String) "target_index=" + target_index , __FILE__, __FUNCTION__, __LINE__ );
            if ( mText.size() < target_index ) throw Exception ( (String) "mText.size=" + mText.size() + "  <=  target_index=" + target_index , __FILE__, __FUNCTION__, __LINE__ );

            Position p ( 0, Point(0,0) );
            long last_newline_position = -1;

            // for each character...
            for ( long i = 0;  i < target_index;  ++i )
                {
                const CharData& c = mText.at(i);
                // if fold... scan to fold end
                if ( c.mFoldLevel > 0 ) {  }
                // if newline... ++Y, last_newline_position = current_index
                else if ( c.GetCharacter() == '\n' )
                    {
                    ++ p.mGridPoint.Y;
                    last_newline_position = i;
                    }
                }
// if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t last_newline_position=" + last_newline_position + "   p=" + p.ToString() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // re-position X from last_newline_position
            p.mTextIndex = last_newline_position + 1;
            while ( p.mTextIndex < target_index )
                Increment( p );
// if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t p=" + p.ToString() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            return p;

            PSEUDOSTACK_END
            }


/*
            protected: long
        GridPointToLineStart( Point point ) const
            // Scans from start of all text to find the text_index that matches point.Y
            {
            Position position ( 0, Point ( 0, 0 ) );
            long prev_text_index = position.mTextIndex;
            // for each text index... if text index's point is above point's yx coord... take previous text index.
            while ( position.mTextIndex < mText.size() )
                {
                if ( position.mGridPoint.Y == point.Y  &&  position.mGridPoint.X == point.X )
                    return position.mTextIndex;
                else if ( position.mGridPoint.Y >= point.Y  &&  position.mGridPoint.X >= point.X )
                    return prev_text_index;

                prev_text_index = position.mTextIndex;
                Increment( position );
                }
            return position.mTextIndex;
            }
*/
            protected: long
        GridPointToLineStart( Point point ) const
            // Scans from start of all text to find the text_index that matches point.Y start of line
            {
            for ( Position p ( 0, Point(0,0) );  p.mTextIndex < mText.size();  Increment(p) )
                if ( p.mGridPoint.Y >= point.Y )
                    return p.mTextIndex;
            }



    ////////// inner types -- change

            protected: class
        Change
            {
            public: enum Type { tInsert, tDelete, tSeries, tNew, tSave };

            public: const Type mType;
            // default members, used for most change types
            public: std::string mText;   // Do not need change start/end grid coordinates, because folding will have changed grid coordinates.
            public: long mStart;
            public: long mEnd;

                public:
            Change( Type change_type ) :  mType ( change_type ) {  }

                public:
            Change( Type change_type, const std::string& text, long start, long end )
                // copies text from start to end
                :
                mType ( change_type ),
                mText ( text.substr( start, end-start ) ),
                mStart ( start ),
                mEnd ( end )
                {
                }

                public:
            Change( Type change_type, const std::vector<CharData>& text, long start, long end )
                // copies text from start to end
                :
                mType ( change_type ),
                mStart ( start ),
                mEnd ( end )
                {  PSEUDOSTACK_START

                for ( long i = start;  i < end;  ++i )
                    mText += text.at(i).GetCharacter();

                PSEUDOSTACK_END
                }

                public: virtual
            ~Change( ) {  }

                public: String
            ToString( int depth = 0 ) const {  return ToStringImp( depth );  }

                protected: virtual String
            ToStringImp( int depth ) const
                {
                String s = "\n";
                for ( int d = 0;  d < depth+1;  ++d )  s += "\t";
                s += TypeToString( mType ) + "  '" + mText + "'  ";
                s += (String) "" + mStart + "  ";
                s += (String) "" + mEnd + "  ";
                return s;
                }

                public: static String
            TypeToString( Type t )
            	{
            	if ( t == tInsert ) return "Insert";
            	if ( t == tDelete ) return "Delete";
            	if ( t == tSeries ) return "Series";
            	if ( t == tNew ) return "New";
            	if ( t == tSave ) return "Save";
                return (String) "" + t;
            	}

            };


            protected: class
        ChangeSeries :  public Change
            {
            public: std::list< Owner<Change> > mChanges;

                public:
            ChangeSeries( ) : Change ( tSeries ) {  }

                protected: virtual String
            ToStringImp( int depth ) const
                {
                String s = "\n";
                for ( int d = 0;  d < depth+1;  ++d )  s += "\t";
                s += TypeToString( mType );
                s += (String) "" + mStart + "  ";
                s += (String) "" + mEnd + "  ";
                for ( std::list< Owner<Change> >::const_iterator c = mChanges.begin();  c != mChanges.end();  ++c )
                    s += (**c).ToString( depth + 1 );
                return s;
                }

            };


    ////////// member data
        protected: static const long UNKNOWN = -1;
        public: static const int LESS = -1;
        public: static const int MORE = 1;

        protected: std::vector<CharData> mText;
        protected: ViewPanel mView;
        public: Point mPanelStart;
        protected: Point mPanelSize;

        protected: Position mCursor;
        protected: Position mSelectionStart;

        public: std::string mCommentString;
        public: std::string mIndentString;
        public: std::string mNewClassString;
        public: std::string mNewMainString;
        public: std::string mNewFunctionString;
        public: std::string mNewThrowString;

        protected: std::list< Owner<Change> >  mUndoHistory;
        protected: std::list< Owner<Change> >::iterator  mCurrentUndo;  // indicates change that has just been undone.

        public: bool mAutoIndent;


    ////////// construction

            public:
        InputArea( )
            {
            mCommentString = "# ";
            mIndentString = "\t";
            mNewClassString = "(new class)";
            mNewMainString = "(new main)";
            mNewFunctionString = "(new function)";
            mNewThrowString = "(throw)";

            mCurrentUndo = mUndoHistory.end();

            mCursor = Position ( 0, Point ( 0, 0 ) );
            mSelectionStart.mTextIndex = UNKNOWN;
            mView.SetText( mText );

            mAutoIndent = true;
            }

            public: virtual
        ~InputArea( )
            {
            }

            public: void
        ResizeWindow( ){  PSEUDOSTACK_START
            Point viewportStartPoint = mView.GetWindowStart();
            long viewportStartIndex = mView.GetWindowStartCharIndex();
            CreatePanel();  // Recreates mView, moves viewport to (0,0)
            mView.MoveWindow( viewportStartPoint, viewportStartIndex );
            updateScreen();
            PSEUDOSTACK_END  }

            public: void
        CreatePanel( )
            {  PSEUDOSTACK_START
            Point screen_size = GetWindowSize( stdscr );
            mPanelStart = Point ( 1, 0 );
            mPanelSize = Point (  screen_size.Y - 1 - mPanelStart.Y,  screen_size.X - mPanelStart.X  );
            mView.CreatePanel( mPanelSize.Y, mPanelSize.X, mPanelStart.Y, mPanelStart.X );

            PSEUDOSTACK_END
            }

            private: void
        updateScreen( ){  PSEUDOSTACK_START
            UpdateCursorToScreen();
            update_panels();
            doupdate();
            PSEUDOSTACK_END  }


    ////////// methods -- data access

            public: const Position&
        GetCursorPosition( ) const {  return mCursor;  }

            public: const ViewPanel&
        GetViewPanel( ) const {  return mView;  }

            public: WINDOW*
        GetWindow( ) {  return mView.Window();  }

            protected: Point
        GetCursorScreenPosition( ) const
            {
            int screen_y, screen_x;
            getyx( mView.Window(), screen_y, screen_x );
            return Point ( screen_y, screen_x );
            }

            protected: Point
        GetScreenSize( ) const {  return GetWindowSize( mView.Window() );   }

            protected: Point
        GetWindowSize( const WINDOW* window ) const
            {
            int size_y, size_x;
            getmaxyx( window, size_y, size_x );
            return Point ( size_y, size_x );
            }

            public: long
        TextSize( ) const  {  return mText.size();  }

            public: int
        GetLineNumber( ) const
            // convert to one-based line number
            // scans from beginning (because of folds)
            {
            long y = 1;
            for ( long index = 0;  index < mCursor.mTextIndex;  ++index )
                if ( mText.at( index ).GetCharacter() == '\n' )
                    ++y;
            return y;
            }

            public: bool
        UnsavedChanges( ) const
            {  PSEUDOSTACK_START
            // if no previous changes in buffer... either no changes, or out of buffer space
            InputArea& this_ref = (InputArea&) *this;
            if ( mCurrentUndo == this_ref.mUndoHistory.begin() )
                {
                // did we run out of buffer space?  (give or take 1 change.)
                return ( mUndoHistory.size() < Settings::Get().max_undos );
                }

            // if previous change was save/new... no unsaved changes
            std::list< Owner<Change> >::const_iterator prev_change = mCurrentUndo;
            prev_change--;
            return ( (**prev_change).mType != Change::tSave  &&  (**prev_change).mType != Change::tNew );

            PSEUDOSTACK_END
            }

            public: void
        ClearRedundantly( bool clear_redundantly ) {  mView.mClearRedundantly = clear_redundantly;  }



    ////////// methods -- file

            public: void
        New( )
            {  PSEUDOSTACK_START
            if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "InputArea::New()", Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            mText.clear();

            // set cursor
            mCursor.mTextIndex = 0;
            mCursor.mGridPoint = Point ( 0, 0 );
            mSelectionStart.mTextIndex = UNKNOWN;
            // update grid -> window -> screen
            mView.MoveWindow( mCursor.mGridPoint, mCursor.mTextIndex );
            UpdateCursorToScreen();
            update_panels();
            doupdate();

            AddToUndoHistory( new Change ( Change::tNew ) );

            PSEUDOSTACK_END
            }


            public: void
        OpenFile( const std::string& filepath )
            // error if file cannot be read
            {  PSEUDOSTACK_START
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "InputArea::OpenFile()", Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // get text from file
            std::vector<CharData> new_text;  // use temporary, to preserve current text if error occurs during file read
            FileToText( filepath, new_text );
            mText.clear();
            for ( long c = 0;  c < new_text.size();  ++c )
                mText.push_back( new_text[c] );

            // remove trailing EOF marker
            if ( TextSize() > 0  &&  mText.at( TextSize() - 1 ).GetCharacter() == EOF )
                mText.erase( mText.begin() + ( TextSize() - 1 ) );

            // remove windows "\r"
            for ( long l = TextSize() - 1;  l >= 0;  l-- )
                if ( mText.at(l).GetCharacter() == '\r' )
                    mText.erase( mText.begin() + l );

            // format each character
            for ( Position line ( 0, Point ( 0, 0 ) );  line.mTextIndex < TextSize();  line = LineEnd( line ) )
                FormatLine( line.mTextIndex );

            // set cursor
            mCursor.mTextIndex = 0;
            mCursor.mGridPoint = Point ( 0, 0 );
            mSelectionStart.mTextIndex = UNKNOWN;
            // update grid -> window -> screen
            mView.MoveWindow( mCursor.mGridPoint, mCursor.mTextIndex );
            UpdateCursorToScreen();
            update_panels();
            doupdate();

            AddToUndoHistory( new Change ( Change::tNew ) );

            PSEUDOSTACK_END
            }


            public: void
        SaveFile( const std::string& filepath )
            {  PSEUDOSTACK_START

            TextToFile( filepath, mText );
            AddToUndoHistory( new Change ( Change::tSave ) );

            PSEUDOSTACK_END
            }

            protected: void
        FileToText( const std::string& filepath, std::vector<CharData>& text ) const // modifies text
            // error if file cannot be read
            {
            text.clear();
            std::ifstream in ( filepath.c_str() );
            if ( in.fail() ) throw Exception ( "could not open file \"" + filepath + "\"", __FILE__, __FUNCTION__, __LINE__ );
            while ( ! in.eof() )
                {
                char c = in.get();
                if ( in.fail() ) break;
                text.push_back( CharData ( c, 0 ) );
                }
            in.close();
            }

            protected: void
        TextToFile( const std::string& filepath, const std::vector<CharData>& text ) const
            {
            std::ofstream out ( filepath.c_str() );
            if ( out.fail() ) throw Exception ( "could not open file \"" + filepath + "\"", __FILE__, __FUNCTION__, __LINE__ );
            for ( long c = 0;  c < text.size();  ++c )
                {
                out << text[c].GetCharacter();
                if ( out.fail() ) throw Exception ( (String) "could not write to file \"" + filepath + "\" \n\n text[" + c + "]=" + text[c].GetCharacter()  , __FILE__, __FUNCTION__, __LINE__ );
                }
            out.close();
            }



    ////////// methods -- view

            protected: void
        FormatLine( long start_index )
            // bolds comments
            // has to work when start_index = mText.size
            {  PSEUDOSTACK_START
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "InputArea::FormatLine()  start_index=" + start_index , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // create comment_marker = mCommentString without trailing spaces
            std::string comment_marker = mCommentString;
            for ( int comment_index = comment_marker.size() - 1;  comment_index >= 0;  comment_index-- )
                if ( comment_marker.at(comment_index) == ' ' )
                    comment_marker.erase( comment_index, 1 );

            if ( start_index >= TextSize() ) return;

            // for each char in text line... format char
            int attributes = 0;
            long i;
            for ( i = start_index;  i < TextSize();  i++ )
                {
                CharData& c = mText.at(i);
                if ( SubsequenceMatch( i, comment_marker ) )
                    {
                    attributes |= A_BOLD;
                    c.mAttributes = attributes;
                    }
                else if ( c.GetCharacter() == '\n' )
                    {
                    c.mAttributes = attributes;
                    break;
                    }
                else
                    {
                    c.mAttributes = attributes;
                    }
                }

            PSEUDOSTACK_END
            }

            public: int
        CurrentTabSize( ) const
            {
            CharData char_data ( '\t', 0 );
            return mView.TextCharToDisplayLength( char_data,  // mText.at( mCursor.mTextIndex ).GetCharacter(),
                                                  mCursor.mGridPoint.X );
            }

            protected: void
        UpdateCursorToScreen( )
            {
            wmove( mView.Window(), mCursor.mGridPoint.Y - mView.GetWindowStart().Y,
                                   mCursor.mGridPoint.X - mView.GetWindowStart().X );
            }



    ////////// methods -- insert/delete

            protected: void
        DeleteImp( long start, long end, bool add_to_undo_history )
            // moves indexes, moves window, moves cursor indexes.
            // updates text -> grid -> window
            {  PSEUDOSTACK_START
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "DeleteImp()  start=" + start + "  end=" + end , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            if ( start > end ) throw Exception ( (String) "start" + start + "  >  end=" + end , __FILE__, __FUNCTION__, __LINE__ );
            if ( start == end )
                {
                // nothing to do
                mSelectionStart.mTextIndex = UNKNOWN;
                return;
                }

            // expose the fold where delete occurs.
            bool unfolded = ExposeFold( start );
            unfolded |= ExposeFold( end );

            // for each deleted display line from end-1 to start... delete display line
            bool deleted_newline = false;
            for ( long c = start;  c < end;  ++c )
                {
                const CharData& char_data = mText.at(c);
                if ( char_data.GetCharacter() == '\n'  &&  char_data.mFoldLevel == 0 )
                    deleted_newline = true;
                }

            // add change to undo history
            if ( add_to_undo_history )
                AddToUndoHistory( new Change ( Change::tDelete, mText, start, end ) );

            // delete in text
            mText.erase(  mText.begin() + start,  mText.begin() + end  );
            long start_line = LineStartIndex( start );
            FormatLine( start_line );

            // set cursor
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t mCursor=" + mCursor.ToString() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
            mSelectionStart.mTextIndex = UNKNOWN;
            mCursor = FindPosition( start );
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t mCursor=" + mCursor.ToString() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // update grid -> window
            bool moved_view = EnsureCursorOnScreen();
            if ( moved_view ) {  } // if already updated whole window... do nothing
            else if ( deleted_newline  ||  unfolded ) mView.UpdateWholeWindow();   // if deleted newline... following lines need to update
            else mView.UpdateTextLineToWindow( mCursor.mGridPoint.Y, start_line );  // if deleted only within a line... only that line needs to updated

            PSEUDOSTACK_END
            }


            protected: void
        InsertImp( long start, const std::string& insert_text, bool add_to_undo_history )
            // moves indexes, moves window, moves cursor indexes.
            // updates text -> grid -> window
            // assumes no selection.
            // has to work when cursor is at mText.end
            {  PSEUDOSTACK_START
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "Insert()  start=" + start + "  insert_text='" + insert_text + "'" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // expose the fold where insert occurs.
            bool unfolded = ExposeFold( start );

            if ( HasSelection() ) throw Exception ( (String) "internal Insert() called with selection" , __FILE__, __FUNCTION__, __LINE__ );
            if ( insert_text.size() == 0 ) return;

            // insert into text
            long insert_end_index = start;
            bool inserted_newline = false;
            for ( long i = 0;  i < insert_text.size();  ++i, ++insert_end_index )
                {
                CharData char_data ( insert_text.at(i), 0 );
                mText.insert(  mText.begin() + insert_end_index,  char_data );
                if ( char_data.GetCharacter() == '\n' ) inserted_newline = true;
                }

            // for each modified line... format
            for ( long line = LineStartIndex( start );  line != UNKNOWN  &&  line <= insert_end_index;  line = NextLine( line ) )
                {
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t line=" + line , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
                FormatLine( line );
                }

            // set cursor
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t mCursor=" + mCursor.ToString() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
            mCursor = FindPosition( insert_end_index );
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t mCursor=" + mCursor.ToString() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // update text -> grid -> window
            bool moved_view = EnsureCursorOnScreen();
            if ( moved_view ) {  }  // updated whole window already
            else if ( inserted_newline || unfolded ) mView.UpdateWholeWindow();  // if inserted newline... following lines need to update
            else mView.UpdateTextLineToWindow( mCursor.mGridPoint.Y, LineStartIndex( start ) );  // if deleted only within a line... only that line needs update

            // add change to undo history
            if ( add_to_undo_history )
                {
                AddToUndoHistory(  new Change ( Change::tInsert, mText, start, start + insert_text.length() )  );
                }

            PSEUDOSTACK_END
            }



    ////////// methods -- navigation

            public: void
        MoveRight( )
            {
            IncrementCursor();
            NavigationViewUpdate();
            }

            public: void
        MoveLeft( )
            {
            DecrementCursor();
            NavigationViewUpdate();
            }

            public: void
        MoveDown( )
            {
            Position old_cursor = mCursor;

            // advance cursor position
            // go to end of line
            while ( mCursor.mTextIndex < TextSize()  &&  mText.at( mCursor.mTextIndex ).GetCharacter() != '\n' )
                IncrementCursor();
            // go to start of next line
            if ( mCursor.mTextIndex < TextSize()  &&  mText.at( mCursor.mTextIndex ).GetCharacter() == '\n' )
                IncrementCursor();
            // go to same column
            while ( mCursor.mTextIndex < TextSize()  &&
                    mText.at( mCursor.mTextIndex ).GetCharacter() != '\n'  &&
                    mCursor.mGridPoint.X < old_cursor.mGridPoint.X )
                {
                IncrementCursor();
                }

            NavigationViewUpdate();
            }


            public: void
        MoveUp( )
            {
            Position old_cursor = mCursor;

            // go back until start of line
            long new_index = LineStartIndex( mCursor.mTextIndex );

            // go back to end of previous line
            if ( new_index > 0  &&  mText.at( new_index - 1 ).GetCharacter() == '\n' )
                -- new_index;
            long prev_line_end = new_index;

            // go back to start of previous line
            new_index = LineStartIndex( new_index );

            // go forward to same column
            Position new_cursor = FindPosition( new_index );  // scans from beginning
            while ( new_cursor.mGridPoint.X < old_cursor.mGridPoint.X  &&
                    new_cursor.mTextIndex < prev_line_end )
            	Increment( new_cursor );

            // update screen
            mCursor = new_cursor;  // format update requires selection range to be set
            UpdateFormatting( new_cursor, old_cursor );
            NavigationViewUpdate();
            }


            public: void
        PageDown( )
            {
            Point size = GetScreenSize();
            // advance cursor position
            for ( int l = 0;  l < size.Y  &&  mCursor.mTextIndex < TextSize();  l++ )
                {
                // go to end of line
                while ( mCursor.mTextIndex < TextSize()  &&  mText.at( mCursor.mTextIndex ).GetCharacter() != '\n' )
                    IncrementCursor();
                // go to start of next line
                if ( mCursor.mTextIndex < TextSize()  &&  mText.at( mCursor.mTextIndex ).GetCharacter() == '\n' )
                    IncrementCursor();
                }

            NavigationViewUpdate();
            }

            public: void
        PageUp( )
            {
            Point screen_size = GetScreenSize();

            // retreat cursor position
            Position old_cursor = mCursor;
            long new_index = mCursor.mTextIndex;
            for ( int l = 0;  l < screen_size.Y  &&  new_index > 0;  l++ )
                {
                // go back until start of line
                new_index = LineStartIndex( new_index );
                // go back to end of previous line
                if ( new_index > 0  &&  mText.at( new_index - 1 ).GetCharacter() == '\n' )
                    -- new_index;
                }
            // go back until start of line
            new_index = LineStartIndex( new_index );

            // update screen
            mCursor = FindPosition( new_index );  // format update requires selection range to be set
            UpdateFormatting( mCursor, old_cursor );
            NavigationViewUpdate();
            }


            public: void
        LineStart( )
            {
            // retract cursor position until start of line
            Position old_cursor = mCursor;
            long new_index = LineStartIndex( mCursor.mTextIndex );

            // update screen
            mCursor = FindPosition( new_index );
            UpdateFormatting( mCursor, old_cursor );
            NavigationViewUpdate();
            }

            public: void
        LineEnd( )
            {
            // advance cursor position until end of line
            while ( mCursor.mTextIndex < TextSize()  &&
                    ( mText.at( mCursor.mTextIndex ).GetCharacter() != '\n'   ||
                      mText.at( mCursor.mTextIndex ).mFoldLevel > 0 )  )
                {
                IncrementCursor();
                }

            NavigationViewUpdate();
            }



            protected: long    // returns index of character after newline, or 0.
        LineStartIndex( long index ) const
            // Not slow.
            {
            if ( index <= 0 ) return 0;

            for (  ;  0 < index;  --index )
                {
                const CharData& prev_char = mText.at( index - 1 );
                if ( prev_char.GetCharacter() == '\n'  &&  prev_char.mFoldLevel == 0 )
                    break;
                }

            return index;
            }

            protected: long    // returns index of newline, or end of text
        LineEndIndex( long index ) const
            // Not slow.
            {
            for (  ;  index < mText.size();  ++index )
                {
                const CharData& cur_char = mText.at( index );
                if ( cur_char.GetCharacter() == '\n'  &&  cur_char.mFoldLevel == 0 )
                    break;
                }

            return index;
            }


            public: void
        SelectCurrentWord( )
            // jump to start/end of current alpha/underscore string
            {
            if ( HasSelection() ) EndSelection();

            // move cursor back until not alpha
            Position old_cursor = mCursor;
            long new_index = mCursor.mTextIndex;
            while ( new_index > 0  &&  IsWordChar( mText.at(new_index).GetCharacter() )  )
                -- new_index;
            // move cursor forward until alpha
            if ( new_index < TextSize()  &&  ! IsWordChar( mText.at(new_index).GetCharacter() )  )
                ++ new_index;
            // start selection
            mCursor = FindPosition( new_index );
            if ( new_index < TextSize() )
                StartSelection();
            // move cursor forward until not alpha
            while (  mCursor.mTextIndex < TextSize()  &&  IsWordChar( mText.at(mCursor.mTextIndex).GetCharacter() )  )
                IncrementCursor();

            NavigationViewUpdate();
            }


            public: void
        NextWord( )
            // jump to start of next alpha string
            {
            // advance cursor position
            // go to end of current alpha string
            while ( mCursor.mTextIndex < TextSize()  &&  IsWordChar( mText.at( mCursor.mTextIndex ).GetCharacter() ) )
                IncrementCursor();
            // go to start of next alpha string
            while ( mCursor.mTextIndex < TextSize()  &&  ! IsWordChar( mText.at( mCursor.mTextIndex ).GetCharacter() ) )
                IncrementCursor();

            NavigationViewUpdate();
            }

            protected: bool
        IsWordChar( char c ) const {  return ( isalpha(c) || isdigit(c) || c == '_' || c == '{' );  }



            public: void
        GoToLine( int y )  // one-based line number
            // searches inside folds
            {
            y -= 1; // convert to zero-based line number
            if ( mCursor.mGridPoint.Y == y ) return;

            if ( HasSelection() ) EndSelection();

            // Searching based on true line, not displayed line.
            Position old_cursor = mCursor;
            long y_index = 0;
            long current_line = 0;
            for (  ;  y_index < mText.size()  &&  current_line < y;  ++y_index )
                if ( mText.at( y_index ).GetCharacter() == '\n' )
                    ++ current_line;

            bool unfolded = ExposeFold( y_index );

            // update cursor position
            mCursor = FindPosition( y_index );
            UpdateFormatting( mCursor, old_cursor );

            // update text -> window
            bool moved_view = EnsureCursorOnScreen();
            if ( ! moved_view  &&  unfolded ) mView.UpdateWholeWindow();
            NavigationViewUpdate();
            }


            public: void
        GoToWindowPoint( int y, int x )
            // search forward/backward until at point, or preceding end-of-line
            {
            Point target_grid_point (  y + mView.GetWindowStart().Y,  x + mView.GetWindowStart().X  );

            // forward
            if ( CompareGridPoints( target_grid_point, mCursor.mGridPoint ) == MORE )
                {
                while ( mCursor.mTextIndex < TextSize()  &&  CompareGridPoints( target_grid_point, mCursor.mGridPoint ) == MORE )
                    IncrementCursor();

                // go back to preceding end-of-line
                if ( 0 < mCursor.mTextIndex  &&  CompareGridPoints( target_grid_point, mCursor.mGridPoint ) == LESS )
                    DecrementCursor();
                }
            // backward
            else if ( CompareGridPoints( target_grid_point, mCursor.mGridPoint ) == LESS )
                {
                Position old_cursor = mCursor;
                mCursor = Position ( 0, Point(0,0) );
                for ( Position next_pos ( 0, Point(0,0) );  CompareGridPoints( next_pos.mGridPoint, target_grid_point ) == LESS;  Increment(next_pos) )
                    mCursor = next_pos;
                UpdateFormatting( mCursor, old_cursor );
                }

            NavigationViewUpdate();
            }


            protected: static int  // if p1>p2 return MORE.  if p1<p2 return LESS. if p1=p2 return equal.
        CompareGridPoints( const Point& p1, const Point& p2 )
            {
            if ( p1.Y < p2.Y ) return LESS;
            if ( p1.Y > p2.Y ) return MORE;

            if ( p1.X < p2.X ) return LESS;
            if ( p1.X > p2.X ) return MORE;
            return 0;
            }


            public: void
        Find( const std::string& target_text )
            {
            // for each position, from current cursor location to end...
            for ( long i = mCursor.mTextIndex;  i < mText.size();  ++i )
                {
                // if target_text is matched... select matching text
                if ( SubsequenceMatch( i, target_text ) )
                    {
                    // unfold to show match
                    bool unfolded = ExposeFold( i );

                    // advance cursor to p
                    if ( HasSelection() ) EndSelection();
                    mCursor = FindPosition( i );

                    // select matching text
                    StartSelection();
                    for ( long t = 0;  t < target_text.size();  ++t )
                        IncrementCursor();

                    NavigationViewUpdate();
                    return;
                    }
                }
            }

            public: bool
        SubsequenceMatch( long text_index, const std::string& match ) const
            {
            for ( long t = text_index,  m = 0;  m < match.length();  ++t, ++m )
                if ( t >= mText.size()  ||  mText.at(t).GetCharacter() != match.at(m) )
                     return false;

            return true;
            }


            public: void
        JumpToMatchingParentheses( )
            {
            long matching_index = MatchingParentheses( mCursor.mTextIndex );
            if ( matching_index == UNKNOWN ) return;
            MessageBar::Display( (String) "matching_index = " + matching_index );

            Position old_cursor = mCursor;
            mCursor = FindPosition( matching_index );
            UpdateFormatting( old_cursor, mCursor );

            NavigationViewUpdate();
            }


            protected: void
        IncrementCursor( )
            // Steps over folded text.
            // Increments cursor indices, updates highlight, updates grid -> window .
            // Does not update cursor to screen -- caller must call NavigationViewUpdate() .
            {
// if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "InputArea::IncrementCursor()", Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
            // repeat while text is folded...
            bool is_first_increment = true;
            while ( mCursor.mTextIndex < TextSize()  &&  ( is_first_increment  ||  ! FoldIsVisible( mCursor.mTextIndex ) )  )
                {
                // if selecting... extend/shrink selection
                if ( HasSelection() )
                    {
                    bool highlight = ( mSelectionStart.mTextIndex <= mCursor.mTextIndex );
                    CharData& char_data = mText.at( mCursor.mTextIndex );
                    if ( highlight ) char_data.mAttributes |= A_REVERSE;
                    else             char_data.mAttributes &= ~A_REVERSE;
                    mView.UpdateTextCharToWindow( mCursor.mGridPoint.Y, mCursor.mGridPoint.X, char_data );
                    }

                // advance cursor position
                Increment( mCursor );
                is_first_increment = false;
                }
            }


// Efficient way to decrement cursor and update position?
//     Folded line start may be very far from cursor.
//     Compute cursor text-index from beginning of text, then update highlight/formatting in forward direction?

    protected: void
DecrementCursor( )
    // steps over folded text.
    // slow, because it recomputes position from beginning.  not for use in loops.
    // decrement loops should compute new text_index, then FindPosition(text_index), then UpdateFormatting(old_pos, new_pos).
    {
    Position old_pos = mCursor;
    // minimal decrement
    long dec_index = ( mCursor.mTextIndex > 1 )?  mCursor.mTextIndex - 1  :  0;
    // decrement out of fold
    while ( 0 < dec_index  &&  ! FoldIsVisible( dec_index ) )
        -- dec_index;
    Position dec_pos = FindPosition( dec_index );  // scans from beginning
    // update formatting
    mCursor = dec_pos;
    UpdateFormatting( dec_pos, old_pos );
    }

    protected: void
UpdateFormatting( Position start_pos, Position end_pos )
    // requires that selection range is set, from mSelectionStart to mCursor.
    // Steps into folded text.
    // updates highlight, updates grid -> window .
    // Does not update cursor to screen -- caller must call NavigationViewUpdate() .
    {
    if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "InputArea::UpdateFormatting( " + start_pos.ToString() + " , " + end_pos.ToString() + " )", Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

//     if ( start_pos.mTextIndex > end_pos.mTextIndex ) throw Exception ( (String) "start_pos > end_pos" ,  __FILE__, __FUNCTION__, __LINE__ );
    // ensure start_pos before end_pos
    if ( start_pos.mTextIndex > end_pos.mTextIndex )
        {
        Position temp_pos = start_pos;
        start_pos = end_pos;
        end_pos = temp_pos;
        }

    // for each character in from start_pos to end_pos...
    Position p = start_pos;
    while ( p.mTextIndex < TextSize()  &&  p.mTextIndex < end_pos.mTextIndex )
        {

        // if selecting... extend/shrink selection
        if ( HasSelection() )
            {
            // set character formatting
            bool highlight = ( mSelectionStart.mTextIndex <= p.mTextIndex  &&  p.mTextIndex < mCursor.mTextIndex )  ||
                             ( mCursor.mTextIndex <= p.mTextIndex  &&  p.mTextIndex < mSelectionStart.mTextIndex );
            CharData& char_data = mText.at( p.mTextIndex );
            if ( highlight ) char_data.mAttributes |= A_REVERSE;
            else             char_data.mAttributes &= ~A_REVERSE;
            // write character to screen
            if ( FoldIsVisible( p.mTextIndex ) )
                mView.UpdateTextCharToWindow( p.mGridPoint.Y, p.mGridPoint.X, char_data );
            }

        Increment( p );
        }
    }


            protected: void
        NavigationViewUpdate( )
            // may move window in grid.
            // moves cursor in window.
            // updates window -> screen
            {
            EnsureCursorOnScreen(); // may update grid -> window
            UpdateCursorToScreen();

            update_panels();
            // wnoutrefresh( mView.Window() );
            doupdate();
            }


            protected: bool // returns whether updated grid -> window
        EnsureCursorOnScreen( )
            // may update text -> window
            // does not change cursor location.
            // tries to keep cursor within middle 50% of screen.
            {
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "EnsureCursorOnScreen()" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
            Point size = GetScreenSize();
            Point view_start = mView.GetWindowStart();

            // adjust view x
            while ( view_start.X > 0  &&  mCursor.mGridPoint.X < view_start.X + (size.X/4) )
                view_start.X -= 1;
            while ( mCursor.mGridPoint.X >= view_start.X + ((size.X*3)/4) )
                view_start.X += 1;

            // adjust view y
            while ( view_start.Y > 0  &&  mCursor.mGridPoint.Y < view_start.Y + (size.Y/4) )
                view_start.Y -= 1;
            while ( mCursor.mGridPoint.Y >= view_start.Y + ((size.Y*3)/4) )
                view_start.Y += 1;

            // move view?
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t view_start=" + view_start.ToString() + "  mView.GetWindowStart()=" + mView.GetWindowStart().ToString() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
            if ( view_start != mView.GetWindowStart() )
                {
                // text_index may not exactly match the window start point.
                // (e.g. character display size > 1 , window starts after end of line...)
                // So how does ViewPanel UpdateWholeWindow() ?
                // Have to start from beginning of line with matching Y value.
                if ( view_start.Y == mView.GetWindowStart().Y )
                    {
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t line does not change" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
                    mView.MoveWindow( view_start, mView.GetWindowStartCharIndex() );  // index of start of first display line does not change
                    }
                else
                    {
                    long window_start_text_index = GridPointToLineStart( view_start );
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t window_start_text_index=" + window_start_text_index , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
                    mView.MoveWindow( view_start, window_start_text_index );
                    }
                return true;
                }
            else
                return false;
            }




    ////////// methods -- folding

       // Do not include folds in undo history.
       // Too much pain for user to have to undo folds to undo edits.

            protected: bool  // returns unfolded:boolean
        ExposeFold( long index )
            {  PSEUDOSTACK_START

            bool unfolded = false;
            if ( index == mText.size() ) return unfolded;
            if ( index < 0  ||  mText.size() < index ) {  throw Exception ( (String) "index=" + index ,  __FILE__, __FUNCTION__, __LINE__ );  }

            while ( mText.at(index).mFoldLevel > 0 )
                {
                long fold_start = FindFoldStart( index );
                long fold_end = FindFoldEnd( fold_start );
                UnfoldImp( fold_start, fold_end );
                unfolded = true;
                }

            return unfolded;

            PSEUDOSTACK_END
            }


            public: void
        Fold( )
            {  PSEUDOSTACK_START
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "Fold()" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // if selection exists... fold each top-level parentheses pair inside selection
            if ( HasSelection() )
                {
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t selection exists" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
                Position selection_start, selection_end;
                GetSelectionStartEnd( selection_start, selection_end );

                // for each possible fold start inside selection...
                for ( long open_paren = selection_start.mTextIndex;  open_paren < selection_end.mTextIndex;  ++open_paren )
                    {
                    while ( open_paren < selection_end.mTextIndex  &&
                            ( mText.at(open_paren).GetCharacter() != '{'  ||  mText.at(open_paren).mFoldLevel > 0 )  )
                         {
                         ++ open_paren;
                         }
                    // find matching parentheses
                    long close_paren = MatchingParentheses( open_paren );
                    if ( close_paren == UNKNOWN ) break;
                    // fold
                    FoldImp( open_paren, close_paren );
                    // skip possible inner folds
                    open_paren = close_paren;
                    }
                }

            // if no selection exists... fold parentheses pair that encloses cursor
            else
                {
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t no selection" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

                if ( mCursor.mTextIndex >= mText.size() ) return;

                // find first possible fold start at/preceding cursor, with matching fold end at/following cursor
                long open_paren = mCursor.mTextIndex;
                long close_paren = UNKNOWN;
                for (  ;  0 <= open_paren;  --open_paren )
                    if ( mText.at(open_paren).GetCharacter() == '{'  &&  mText.at(open_paren).mFoldLevel == 0 )
                        {
                        close_paren = MatchingParentheses( open_paren );
                        if ( close_paren == UNKNOWN ) break;   // no matching paren can be found after open_paren
                        if ( mCursor.mTextIndex <= close_paren ) break;
                        }
                if ( open_paren == -1 ) return;  // no opening parentheses found
                if ( close_paren == UNKNOWN ) return;  // no matching close parentheses
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t open_paren=" + open_paren + "  close_paren=" + close_paren , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

                // fold
                FoldImp( open_paren, close_paren );
                }

            // update cursor position
            mCursor = FindPosition( mCursor.mTextIndex );
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t mCursor=" + mCursor.ToString() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // update grid -> window
            bool moved_view = EnsureCursorOnScreen();
            if ( ! moved_view ) mView.UpdateWholeWindow();
            NavigationViewUpdate();

            PSEUDOSTACK_END
            }


            public: void
        FoldAll( )
            {
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "FoldAll()" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            if ( HasSelection() ) EndSelection();

            // What if someone inserts/deletes parentheses mixed with existing folds?
            // Not a problem -- unfolding is based on fold meta-data, not based on matching parentheses.

            // for each opening parentheses ...
            for ( long i = 0;  i < mText.size();  ++i )
                if ( mText.at(i).GetCharacter() == '{'  &&  ! IsFoldStart(i) )
                    {
                    long close_paren = MatchingParentheses( i );
                    if ( close_paren == UNKNOWN ) continue;
                    if ( IsFoldEnd( close_paren ) ) continue;
                    // fold
                    FoldImp( i, close_paren );
                    }

            // update cursor position
            mCursor = FindPosition( mCursor.mTextIndex );
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t mCursor=" + mCursor.ToString() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // update grid -> window
            bool moved_view = EnsureCursorOnScreen();
            if ( ! moved_view ) mView.UpdateWholeWindow();
            NavigationViewUpdate();
            }


            public: void
        FoldInside( )
            {
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "FoldInside()" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            if ( mCursor.mTextIndex >= mText.size() ) return;
            if ( HasSelection() ) EndSelection();

            // find brackets surrounding cursor.
            // find first open bracket preceding cursor, with matching close bracket at/following cursor.
            long class_open_paren = mCursor.mTextIndex;
            long class_close_paren = UNKNOWN;
            for (  ;  0 <= class_open_paren;  --class_open_paren )
                if ( mText.at(class_open_paren).GetCharacter() == '{' )
                    {
                    class_close_paren = MatchingParentheses( class_open_paren );
                    if ( class_close_paren == UNKNOWN ) break;   // no matching paren can be found after class_open_paren
                    if ( mCursor.mTextIndex <= class_close_paren ) break;
                    }
//             if ( class_open_paren == -1 ) return;  // no opening parentheses found
//             if ( class_close_paren == UNKNOWN ) return;  // no matching close parentheses
if ( class_open_paren == -1 ) {  class_open_paren = -1;  class_close_paren = mText.size();  }  // if opening parentheses found... fold whole file
if ( class_close_paren == UNKNOWN ) class_close_paren = mText.size();  // if no matching close parentheses... continue to end of file
// std::cerr << "\n class_open_paren=" << class_open_paren << "  class_close_paren=" << class_close_paren;
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t class_open_paren=" + class_open_paren + "  class_close_paren=" + class_close_paren , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // for each possible fold start inside class...
            for ( long open_paren = class_open_paren + 1;  open_paren < class_close_paren - 1;  ++open_paren )
                {
                if ( mText.at(open_paren).GetCharacter() == '{'  &&  mText.at(open_paren).mFoldLevel == 0 )
                    {
                    // find matching parentheses
                    long close_paren = MatchingParentheses( open_paren );
                    if ( close_paren == UNKNOWN ) break;  // no matching parens can be found after missing close paren
                    // fold
// std::cerr << "\n open_paren=" << open_paren << "  close_paren=" << close_paren;
                    FoldImp( open_paren, close_paren );
                    // skip possible inner folds
                    open_paren = close_paren;
                    }
                }

            // update cursor position
            mCursor = FindPosition( mCursor.mTextIndex );
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t mCursor=" + mCursor.ToString() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // update grid -> window
            bool moved_view = EnsureCursorOnScreen();
//             if ( ! moved_view ) mView.UpdateWholeWindow();  // fold preceding block fails when preceding block encloses window start
if ( ! moved_view )
{
// cannot use View.UpdateWholeWindow() because here we violate the assumption that window start index did not change
Point view_start = mView.GetWindowStart();
long window_start_text_index = GridPointToLineStart( view_start );
mView.MoveWindow( view_start, window_start_text_index );
}
            NavigationViewUpdate();
            }




            public: void
        FoldToggle( )
            {
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "FoldToggle()" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            if ( mCursor.mTextIndex >= mText.size() ) return;
            if ( HasSelection() ) EndSelection();

            // if not folded...
            if ( mText.at( mCursor.mTextIndex ).mFoldLevel <= 0 )
                {
                // find first possible fold start at/preceding cursor, with matching fold end at/following cursor
                long open_paren = mCursor.mTextIndex;
                long close_paren = UNKNOWN;
                for (  ;  0 <= open_paren;  --open_paren )
                    if ( mText.at(open_paren).GetCharacter() == '{'  &&  mText.at(open_paren).mFoldLevel <= 0 )
                        {
                        close_paren = MatchingParentheses( open_paren );
                        if ( close_paren == UNKNOWN ) break;   // no matching paren can be found after open_paren
                        if ( mCursor.mTextIndex <= close_paren ) break;
                        }
                if ( open_paren == -1 ) return;  // no opening parentheses found
                if ( close_paren == UNKNOWN ) return;  // no matching close parentheses
                if ( mText.at(close_paren).mFoldLevel > 0 ) return;  // mis-matched fold and parentheses
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t open_paren=" + open_paren + "  close_paren=" + close_paren , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
                // fold
                FoldImp( open_paren, close_paren );
                }

            // if folded...
            else
                {
                // find enclosing fold
                long fold_start = FindFoldStart( mCursor.mTextIndex );
                long fold_end = FindFoldEnd( mCursor.mTextIndex );
                // unfold
                UnfoldImp( fold_start, fold_end );
                }

            // update cursor position
            mCursor = FindPosition( mCursor.mTextIndex );
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t mCursor=" + mCursor.ToString() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // update grid -> window
            bool moved_view = EnsureCursorOnScreen();
            if ( ! moved_view ) mView.UpdateWholeWindow();
            NavigationViewUpdate();
            }



            public: void
        Unfold( )
            {
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "Unfold()" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // if selection exists... unfold each top-level fold inside selection
            if ( HasSelection() )
                {
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t selection exists" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
                Position selection_start, selection_end;
                GetSelectionStartEnd( selection_start, selection_end );

                // for each fold start inside selection ...
                for ( long open_paren = selection_start.mTextIndex;  open_paren < selection_end.mTextIndex;  ++open_paren )
                    if ( IsFoldStart( open_paren ) )
                        {
                        long close_paren = FindFoldEnd( open_paren );
                        // unfold
                        UnfoldImp( open_paren, close_paren );
                        }
                }

            // if no selection exists... unfold only if cursor is on/in a fold
            else
                {
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t no selection" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

                if ( mCursor.mTextIndex >= mText.size() ) return;

                // find enclosing fold
                if ( mText.at( mCursor.mTextIndex ).mFoldLevel <= 0 ) return;
                long fold_start = FindFoldStart( mCursor.mTextIndex );
                long fold_end = FindFoldEnd( mCursor.mTextIndex );
                // unfold
                UnfoldImp( fold_start, fold_end );
                }
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t mCursor=" + mCursor.ToString() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // update cursor position
            mCursor = FindPosition( mCursor.mTextIndex );
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t mCursor=" + mCursor.ToString() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // update grid -> window
            bool moved_view = EnsureCursorOnScreen();
            if ( ! moved_view ) mView.UpdateWholeWindow();
            NavigationViewUpdate();
            }


            public: void
        UnfoldAll( )
            {
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "UnfoldAll()" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            if ( HasSelection() ) EndSelection();

            // for each fold start ...
            for ( long fold_start = 0;  fold_start < mText.size();  ++fold_start )
                if ( IsFoldStart( fold_start ) )
                    {
                    // find fold end
                    long fold_end = FindFoldEnd( fold_start );
                    // unfold
                    UnfoldImp( fold_start, fold_end );
                    }

            // update cursor position
            mCursor = FindPosition( mCursor.mTextIndex );
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t mCursor=" + mCursor.ToString() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // update grid -> window
            bool moved_view = EnsureCursorOnScreen();
            if ( ! moved_view ) mView.UpdateWholeWindow();
            NavigationViewUpdate();
            }



            protected: void
        FoldImp( long open_paren, long close_paren )
            // does not update cursor position
            {
            // set characters fold meta-data
            mText.at( open_paren ).mFoldState = CharData::FOLD_START;
            mText.at( close_paren ).mFoldState = CharData::FOLD_END;
            for ( long i = open_paren;  i <= close_paren;  ++i )
                ++ mText.at(i).mFoldLevel;
            }


            protected: void
        UnfoldImp( long open_paren, long close_paren )
            // does not update cursor position
            {
            // set characters fold meta-data
            for ( long i = open_paren;  i <= close_paren;  ++i )
                {
                if ( mText.at(i).mFoldLevel == 0 ) throw Exception ( (String) "mText.at(" + i + ").mFoldLevel == 0" , __FILE__, __FUNCTION__, __LINE__ );
                -- mText.at(i).mFoldLevel;
                }
            mText.at( open_paren ).mFoldState = IsFoldStart( open_paren )?  CharData::FOLD_START  :  CharData::FOLD_NONE;
            mText.at( close_paren ).mFoldState = IsFoldEnd( close_paren )? CharData::FOLD_END  :  CharData::FOLD_NONE;
            }


            protected: bool
        IsFoldStart( long index ) const
            {
            return ( mText.at(index).mFoldLevel > 0  &&
                       ( index < 1  ||  mText.at(index).mFoldLevel > mText.at(index-1).mFoldLevel )  );
            }

            protected: bool
        IsFoldEnd( long index ) const
            {
            return ( mText.at(index).mFoldLevel > 0  &&
                       ( index >= mText.size() - 1  ||  mText.at(index).mFoldLevel > mText.at(index+1).mFoldLevel )  );
            }


            protected: long
        FindFoldEnd( long fold_start ) const
            {
            int fold_level = mText.at( fold_start ).mFoldLevel;
            if ( fold_level <= 0 ) throw Exception ( (String) "fold_level=" + fold_level , __FILE__, __FUNCTION__, __LINE__ );

            long fold_end = fold_start;
            while ( fold_end + 1 < mText.size()  &&  mText.at( fold_end + 1 ).mFoldLevel >= fold_level )
                ++ fold_end;

            return fold_end;
            }

            protected: long
        FindFoldStart( long fold_end ) const
            {
            int fold_level = mText.at( fold_end ).mFoldLevel;
            if ( fold_level <= 0 ) throw Exception ( (String) "fold_level=" + fold_level , __FILE__, __FUNCTION__, __LINE__ );

            long fold_start = fold_end;
            while ( fold_start - 1 >= 0  &&  mText.at( fold_start - 1 ).mFoldLevel >= fold_level )
                -- fold_start;

            return fold_start;
            }


            protected: bool
        FoldIsVisible( long index ) const
            {
            const CharData& c = mText.at( index );
            if ( c.mFoldLevel == 0 ) return true;
            if ( c.mFoldLevel == 1  &&  IsFoldStart( index ) ) return true;
            if ( c.mFoldLevel == 1  &&  IsFoldEnd( index ) ) return true;
            return false;
            }




    ////////// methods -- editing

            public: bool
        HasSelection( ) const
            {
            return ( mSelectionStart.mTextIndex != UNKNOWN );
            }

            public: void
        StartSelection( )
            // may update grid -> window -> screen
            {
            EndSelection();
            mSelectionStart = mCursor;
            }

            public: void
        EndSelection( )
            // may update grid -> window -> screen
            {
            if ( mSelectionStart.mTextIndex == UNKNOWN ) return;

            // change grid formatting
            Position start, end;
            GetSelectionStartEnd( start, end );
            for ( Position p = start;  p.mTextIndex < end.mTextIndex;  Increment( p ) )
                {
                CharData& char_data = mText.at( p.mTextIndex );
                char_data.mAttributes &= ~A_REVERSE;
                mView.UpdateTextCharToWindow( p.mGridPoint.Y, p.mGridPoint.X, char_data );
                }

            // update grid -> window
//             mView.UpdateGridToWindow();

            // set cursor
            mSelectionStart.mTextIndex = UNKNOWN;

            // update window -> screen
            UpdateCursorToScreen();
            update_panels();
            doupdate();
            }



            public: void
        Indent( bool add_to_undo_history = true )
            // updates text -> grid -> window -> screen
            {  PSEUDOSTACK_START
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "Indent()" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // find lines to indent
            long start, end;
            LineBlockBounds( start, end );
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t start=" + start + "  end=" + end , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
            if ( end == UNKNOWN  ||  end <= 0 ) return;

            if ( HasSelection() ) EndSelection();
            Owner<ChangeSeries> changes = new ChangeSeries ( );

            // for each indented line, in reverse order (to retain text indexes) ...
            for ( long l = LineStartIndex( end - 1 );  l >= start;  l = LineStartIndex( l - 1 ) )
                {
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "l=" + l , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
                // if line is not blank... insert indent
                if ( mText.at(l).GetCharacter() != '\n' )
                    {
                    // change text, update text -> grid -> window
                    InsertImp( l, mIndentString, false );  // moves cursor
                    changes->mChanges.push_back(  new Change ( Change::tInsert, mText, l, l + mIndentString.length() )  );
                    end += mIndentString.length();
                    }

                if ( l == start ) break;
                }

            // select indented lines.
            mCursor = FindPosition( start );
            StartSelection();
            while ( mCursor.mTextIndex < end )
                IncrementCursor();

            // update window -> screen
            UpdateCursorToScreen();
            update_panels();
            doupdate();

            if ( add_to_undo_history ) AddToUndoHistory( changes.GiveUp() );

            PSEUDOSTACK_END
            }


            public: void
        Unindent( bool add_to_undo_history = true )
            // updates text -> grid -> window -> screen
            {
            // find lines to indent
            long start, end;
            LineBlockBounds( start, end );
            if ( end == UNKNOWN  ||  end <= 0 ) return;

            if ( HasSelection() ) EndSelection();
            Owner<ChangeSeries> changes = new ChangeSeries ( );

            // for each unindented line, in reverse order (to preserve text indexes) ...
            for ( long l = LineStartIndex( end - 1 );  l >= start;  l = LineStartIndex( l - 1 )  )
                {
                // if line starts with indent... delete indent
                if ( SubsequenceMatch( l, mIndentString ) )
                    {
                    // change text, update text -> grid -> window
                    changes->mChanges.push_back(  new Change ( Change::tDelete, mText, l, l + mIndentString.length() )  );
                    DeleteImp( l, l + mIndentString.length(), false );
                    end -= mIndentString.length();
                    }

                if ( l == start ) break;
                }

            // select unindented lines.
            mCursor = FindPosition( start );
            StartSelection();
            while ( mCursor.mTextIndex < end )
                IncrementCursor();

            // update window -> screen
            UpdateCursorToScreen();
            update_panels();
            doupdate();

            if ( add_to_undo_history ) AddToUndoHistory( changes.GiveUp() );
            }


            public: void
        Comment( bool add_to_undo_history = true )
            // updates text -> grid -> window -> screen
            {
            // find lines to indent
            long start, end;
            LineBlockBounds( start, end );
            if ( end == UNKNOWN  ||  end <= 0 ) return;

            if ( HasSelection() ) EndSelection();
            Owner<ChangeSeries> changes = new ChangeSeries ( );

            // for each commented line, in reverse order... insert comment marker
            for ( long l = LineStartIndex( end - 1 );  l >= start;  l = LineStartIndex( l - 1 )  )
                {
                // change text, update text -> grid -> window
                InsertImp( l, mCommentString, false );
                changes->mChanges.push_back(  new Change ( Change::tInsert, mText, l,  l + mCommentString.length() )  );
                end += mCommentString.length();

                if ( l == start ) break;
                }

            // select commented lines
            mCursor = FindPosition( start );
            StartSelection();
            while ( mCursor.mTextIndex < end )
                IncrementCursor();

            // update window -> screen
            UpdateCursorToScreen();
            update_panels();
            doupdate();

            if ( add_to_undo_history ) AddToUndoHistory( changes.GiveUp() );
            }


            public: void
        Uncomment( bool add_to_undo_history = true )
            // updates text -> grid -> window -> screen
            {
            // find lines to indent
            long start, end;
            LineBlockBounds( start, end );
            if ( end == UNKNOWN  ||  end <= 0 ) return;

            if ( HasSelection() ) EndSelection();
            Owner<ChangeSeries> changes = new ChangeSeries ( );

            // for each uncommented line, in reverse order...
            for ( long l = LineStartIndex( end - 1 );  l >= start;  l = LineStartIndex( l - 1 ) )
                {
                // if comment string at front of line... delete comment string
                if ( SubsequenceMatch( l, mCommentString ) )
                    {
                    // change text, update text -> grid -> window
                    changes->mChanges.push_back(  new Change ( Change::tDelete, mText, l, l + mCommentString.length() )  );
                    DeleteImp( l, l + mCommentString.length(), false );
                    end -= mCommentString.length();
                    }

                if ( l == start ) break;
                }

            // select indented lines
            mCursor = FindPosition( start );
            StartSelection();
            while ( mCursor.mTextIndex < end )
                IncrementCursor();

            // update window -> screen
            UpdateCursorToScreen();
            update_panels();
            doupdate();

            if ( add_to_undo_history ) AddToUndoHistory( changes.GiveUp() );
            }


    public: void
ChangeCase( Settings::tFunctionID conversion, bool add_to_undo_history = true )
    // updates text -> grid -> window -> screen
    {
    Owner<ChangeSeries> changes = new ChangeSeries ( );

    // copy selected text
    Position start, end;
    GetSelectionStartEnd( start, end );

    // convert selected text
    std::string converted_text;
    // for each character... convert
    for ( long p = start.mTextIndex;  p < end.mTextIndex;  ++p )
        {
        if ( conversion == Settings::LOWER_CASE )
            converted_text += tolower( mText.at(p).GetCharacter() );

        else if ( conversion == Settings::UPPER_CASE )
            converted_text += toupper( mText.at(p).GetCharacter() );

        else if ( conversion == Settings::HUMP_CASE )
            {
            if ( mText.at(p).GetCharacter() == '_' )
                {
                if ( p + 1 < end.mTextIndex )
                    {
                    converted_text += toupper( mText.at(p+1).GetCharacter() );
                    ++p;
                    }
                }
            else if ( p > 0   &&  ! isalpha( mText.at(p-1).GetCharacter() )  )
                converted_text += toupper( mText.at(p).GetCharacter() );
            else
                converted_text += tolower( mText.at(p).GetCharacter() );
            }

        else if ( conversion == Settings::UNDERSCORE_CASE )
            {
            if (  isupper( mText.at(p).GetCharacter() )  &&  p > 0  &&  isalpha( mText.at(p-1).GetCharacter() )  )
                converted_text += "_";
            converted_text += tolower( mText.at(p).GetCharacter() );
            }

        else
            throw Exception ( "unhandled conversion", __FILE__, __FUNCTION__, __LINE__ );
        }

    changes->mChanges.push_back(  new Change ( Change::tDelete, mText, start.mTextIndex, end.mTextIndex )  );
    DeleteImp( start.mTextIndex, end.mTextIndex, false );
    InsertImp( start.mTextIndex, converted_text, false );  // moves mCursor
    changes->mChanges.push_back(  new Change ( Change::tInsert, mText, start.mTextIndex, mCursor.mTextIndex )  );

    // select converted text
    mCursor = FindPosition( start.mTextIndex );
    StartSelection();
    while ( mCursor.mTextIndex < start.mTextIndex + converted_text.length() )
        IncrementCursor();

    // update window -> screen
    UpdateCursorToScreen();
    update_panels();
    doupdate();

    if ( add_to_undo_history ) AddToUndoHistory( changes.GiveUp() );
    }



            public: void
        Delete( )
            {
            // If deleting selected text that is partly folded, then need to set new fold start/end?
            //     (Same for all editing operations.)
            //     Impossible -- folded text is atomic.

            // delete selection
            if ( HasSelection() )
                {
                Position start, end;
                GetSelectionStartEnd( start, end );
                DeleteImp( start.mTextIndex, end.mTextIndex, true );
                }
            // delete single character
            else
                {
                if ( mCursor.mTextIndex >= TextSize() ) return;
                Position end = mCursor;
                Increment( end );
                DeleteImp( mCursor.mTextIndex, end.mTextIndex, true );
                }

            // update window -> screen
            UpdateCursorToScreen();
            update_panels();
            doupdate();
            }


            public: void
        Backspace( )
            {
            // delete selection
            if ( HasSelection() )
                {
                Position start, end;
                GetSelectionStartEnd( start, end );
                DeleteImp( start.mTextIndex, end.mTextIndex, true );
                }
            // delete single character
            else
                {
                if ( mCursor.mTextIndex <= 0 ) return;
                Position end = mCursor;
                mCursor = FindPosition( mCursor.mTextIndex - 1 );
                DeleteImp( mCursor.mTextIndex, end.mTextIndex, true );
                }

            // update window -> screen
            UpdateCursorToScreen();
            update_panels();
            doupdate();
            }


            public: void
        Insert( const std::string& insert_text )
            {
            Position initial_cursor = mCursor;

            // with selection... delete then insert
            if ( HasSelection() )
                {
                Position start, end;
                GetSelectionStartEnd( start, end );
                DeleteImp( start.mTextIndex, end.mTextIndex, true );
                InsertImp( start.mTextIndex, insert_text, true );
                }
            // without selection... insert
            else
                InsertImp( mCursor.mTextIndex, insert_text, true );


            // if inserted only a newline... auto-indent the next line
            if ( insert_text == "\n"  &&  0 < mCursor.mGridPoint.Y  &&  mAutoIndent )
                {
                if ( mCursor.mGridPoint.X != 0 ) throw Exception ( "", __FILE__, __FUNCTION__, __LINE__ );

                // find last line's beginning whitespace
                long prev_line = LineStartIndex( initial_cursor.mTextIndex );
                String whitespace;
                for ( long i = prev_line;  i < TextSize()  &&  ( mText.at(i).GetCharacter() == ' '  ||  mText.at(i).GetCharacter() == '\t' );  ++i )
                    whitespace += mText.at(i).GetCharacter();

                // insert whitespace
                InsertImp( mCursor.mTextIndex, whitespace, true );
                }

            // update window -> screen
            UpdateCursorToScreen();
            update_panels();
            doupdate();
            }


            public: std::string
        Copy( )
            {
            if ( ! HasSelection() ) return "";

            Position start, end;
            GetSelectionStartEnd( start, end );

            std::string copied_text;
            for ( long t = start.mTextIndex;  t < end.mTextIndex;  ++t )
                copied_text += mText.at(t).GetCharacter();
            return copied_text;
            }

            public: std::string
        Cut( )
            {
            if ( ! HasSelection() ) return "";
            std::string copied_text = Copy();
            Delete();
            return copied_text;
            }



    ////////// methods -- undo

            protected: void
        AddToUndoHistory( Change* change ) // takes ownership of change
            {
            Owner<Change> change_owner = change;

            // clear any following history
            mUndoHistory.erase( mCurrentUndo, mUndoHistory.end() );

            // add change to history
            mUndoHistory.push_back( change_owner.GiveUp() );
            mCurrentUndo = mUndoHistory.end();

            // enforce history size constraint
            while ( mUndoHistory.size() > Settings::Get().max_undos )
                mUndoHistory.pop_front();
            }



            public: void
        Undo( )
            {
            // if all changes have been undone... do nothing
            if ( mCurrentUndo == mUndoHistory.begin() ) return;

            // if previous change was a new file/buffer... do nothing
            std::list< Owner<Change> >::iterator u = mCurrentUndo;
            u--;
            if ( (**u).mType == Change::tNew )
                {
                MessageBar::Display( "Cannot undo new file." );
                return;
                }

            // clear selection
            if ( HasSelection() ) EndSelection();

            // go to previous change
            mCurrentUndo--;
            // do opposite of previous change
            Undo( **mCurrentUndo );

            // refresh view
            UpdateCursorToScreen();
            update_panels();
            doupdate();
            }


            protected: void
        Undo( Change& change )
            {
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "Undo()  change=" + change.ToString() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // insert <-> delete
            if ( change.mType == Change::tInsert ) DeleteImp( change.mStart, change.mEnd, false );
            else if ( change.mType == Change::tDelete ) InsertImp( change.mStart, change.mText, false );

            // series -- recurse
            else if ( change.mType == Change::tSeries )
                {
                ChangeSeries& change_series = (ChangeSeries&) **mCurrentUndo;

                // for each change, in reverse order... undo change
                for ( std::list< Owner<Change> >::reverse_iterator c = change_series.mChanges.rbegin();  c != change_series.mChanges.rend();  c++ )
                    Undo( **c );
                }

            // save -- do nothing
            else if ( change.mType == Change::tSave ) MessageBar::Display( "Cannot undo save file." );

// // fold <-> unfold
// else if ( change.mType == Change::tFold ) UnfoldImp( change.mStart, change.mEnd, false );
// else if ( change.mType == Change::tUnfold ) FoldImp( change.mStart, change.mEnd, false );

            else throw Exception ( "unhandled change type in Undo()", __FILE__, __FUNCTION__, __LINE__ );
            }


            public: void
        Redo( )
            {
            // if no currently undone change... do nothing
            if ( mCurrentUndo == mUndoHistory.end() ) return;

            // if previous change was a new file/buffer... impossible
            if ( (**mCurrentUndo).mType == Change::tNew ) throw Exception ( "trying to redo a new file/buffer", __FILE__, __FUNCTION__, __LINE__ );

            // clear selection
            if ( HasSelection() ) EndSelection();

            // do currently undone change
            Redo( **mCurrentUndo );
            // go to next undone change
            mCurrentUndo++;

            // refresh view
            UpdateCursorToScreen();
            update_panels();
            doupdate();
            }


            protected: void
        Redo( Change& change )
            {
            // insert
            if ( change.mType == Change::tInsert )
                {
                InsertImp( change.mStart, change.mText, false );
                }
            // delete
            else if ( change.mType == Change::tDelete )
                {
                DeleteImp( change.mStart, change.mEnd, false );
                }

            // series
            else if ( change.mType == Change::tSeries )
                {
                ChangeSeries& change_series = (ChangeSeries&) **mCurrentUndo;

                // for each change, in forward order... redo change
                for ( std::list< Owner<Change> >::iterator c = change_series.mChanges.begin();  c != change_series.mChanges.end();  c++ )
                    Redo( **c );
                }

            // save -- do nothing
            else if ( change.mType == Change::tSave )
                {
                MessageBar::Display( "Cannot redo save file." );
                }

            else
                throw Exception ( "unhandled change type in Undo()", __FILE__, __FUNCTION__, __LINE__ );
            }



    ////////// methods -- positions

            protected: void
        LineBlockBounds( long& start, long& end ) // modified
            {
            // if no selection... block is current line
            if ( mSelectionStart.mTextIndex == UNKNOWN  ||  mSelectionStart.mTextIndex == mCursor.mTextIndex )
                {
                start = LineStartIndex( mCursor.mTextIndex );
                end = LineEnd( mCursor ).mTextIndex;
                }
            // if selection... extend block to whole first & last line.
            else
                {
                Position selection_start, selection_end;
                GetSelectionStartEnd( selection_start, selection_end );

                start = LineStartIndex( selection_start.mTextIndex );
                end = selection_end.mTextIndex;
                if ( selection_end.mGridPoint.X > 0 )
                    end = LineEnd( selection_end ).mTextIndex;
                }
            }

            protected: void
        GetSelectionStartEnd( Position& start, Position& end ) // modified
            {
            if ( mSelectionStart.mTextIndex < mCursor.mTextIndex )
                {
                start = mSelectionStart;
                end = mCursor;
                }
            else
                {
                start = mCursor;
                end = mSelectionStart;
                }
            }

            protected: long
        NextLine( long index ) const
            // returns index of character after next newline, or UNKNOWN
            {
            for (  ;  index < TextSize();  ++index )
                if ( mText.at(index).GetCharacter() == '\n' )
                    {
                    return index + 1;
                    }

            return UNKNOWN;
            }

            protected: Position
        LineEnd( Position position ) const
            // returns position of character after newline, or end of mText
            {
            for (  ;  position.mTextIndex < TextSize();  Increment( position ) )
                if ( mText.at( position.mTextIndex ).GetCharacter() == '\n' )
                    {
                    Increment( position );
                    return position;
                    }

            return position;
            }


            protected: long   // may return UNKNOWN
        MatchingParentheses( long index ) const
            {
            // Simple parentheses matching.
            // Does not use literal markers (e.g. \"\'), literal marker escapes (e.g. \\).

            // create parentheses pairs
            static std::vector< std::pair<char,char> > parentheses_pairs;
            if ( parentheses_pairs.size() == 0 )
                {
                parentheses_pairs.push_back( std::pair<char,char> ( '{', '}' ) );
                parentheses_pairs.push_back( std::pair<char,char> ( '[', ']' ) );
                parentheses_pairs.push_back( std::pair<char,char> ( '(', ')' ) );
                parentheses_pairs.push_back( std::pair<char,char> ( '<', '>' ) );
                }

            // select matching parentheses
            char current_paren = mText.at( index ).GetCharacter();
            char matching_paren;
            bool open_paren = false;
            bool close_paren = false;
            int p;
            for ( p = 0;  p < parentheses_pairs.size();  p++ )
                {
                if ( current_paren == parentheses_pairs[p].first )
                    {
                    open_paren = true;
                    matching_paren = parentheses_pairs[p].second;
                    }
                else if ( current_paren == parentheses_pairs[p].second )
                    {
                    close_paren = true;
                    matching_paren = parentheses_pairs[p].first;
                    }
                }

            // scan to matching parentheses
            int num_unmatched = 0;
            if ( open_paren )
                {
                for ( long m = index;  m < TextSize();  ++m )
                    {
                    if ( mText.at(m).GetCharacter() == current_paren )
                        ++ num_unmatched;
                    else if ( mText.at(m).GetCharacter() == matching_paren )
                        {
                        -- num_unmatched;
                        if ( num_unmatched == 0 ) return m;
                        }
                    }
                }
            else if ( close_paren )
                {
                for ( long m = index;  m >= 0;  --m )
                    {
                    if ( mText.at(m).GetCharacter() == current_paren )
                        ++ num_unmatched;
                    else if ( mText.at(m).GetCharacter() == matching_paren )
                        {
                        -- num_unmatched;
                        if ( num_unmatched == 0 ) return m;
                        }
                    }
                }

            return UNKNOWN;
            }


    };



//////////////////////////////////////////////////////////////////////
#endif
