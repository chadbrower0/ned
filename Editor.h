#ifndef INCLUDED_Editor_h
#define INCLUDED_Editor_h
////////////////////////////////////////////////////////////////////////////////
#include <curses.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <Shared/TextFunctions.h>
#include <Shared/String.h>
#include "KeyListener.h"
#include "MenuBar.h"
#include "MessageBar.h"
#include "LineNumBar.h"
#include "SearchBar.h"
#include "FilepathBar.h"
#include "YesNoBar.h"
#include "InputArea.h"
#include "Settings.h"

//
// a class for running a InputArea as a text editor program.
// Uses ncurses.  Must be created/run in ncurses mode.
//
// selection-sensitive keys
//   uses some selection-sensitive keys.  normally these keys just type,
//   but when there's a selection, these keys do different functions.
//   these control keys are mostly for new functions not found in pico.
//     copy, indent, unindent, comment, uncomment
//


    class
Editor :  KeyListener
    {
    ////////// member data
        protected: std::string mFilepath;
        protected: bool mContinueInputLoop;
        protected: time_t LastFileModTime;  // last file open/save time

        protected: InputArea mInputArea;
        protected: std::string mCopiedText;

        protected: LineNumBar mLineNumBar;
        protected: SearchBar mSearchBar;
        protected: MenuBar mMenuBar;

        protected: enum tMouseMode { INTERNAL_MOUSE, EXTERNAL_MOUSE };
        protected: tMouseMode mMouseMode;

    ////////// construction

            public:
        Editor( const std::string& filepath, int initial_line_number = 0 )
            :
            mCopiedText ( "" ),
            mLineNumBar ( ),
            mSearchBar ( mCopiedText ),
            mMenuBar ( )
            // must be created in ncurses mode
            {  PSEUDOSTACK_START
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "Editor()" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            mFilepath = filepath;

            mInputArea.ClearRedundantly( Settings::Get().clear_redundantly );

            // for each known file type...
            for ( int ft = 0;  ft < Settings::Get().mFileTypes.size();  ++ft )
                {
                const Settings::FileTypeInfo& file_type = Settings::Get().mFileTypes[ft];

                if ( SuffixMatch( mFilepath, file_type.mSuffixes ) )
                    {
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t matched suffix in " + TextFunctions::Join( file_type.mSuffixes , "," ) , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
                    // file type settings
                    if ( file_type.mComment != "" ) mInputArea.mCommentString = file_type.mComment;
                    if ( file_type.mIndent != "" ) mInputArea.mIndentString = file_type.mIndent;
                    }
                }
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t comment='" + mInputArea.mCommentString + "'" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "\t indent='" + mInputArea.mIndentString + "'" , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // set up menus
            mMenuBar.mLabel = "(ESC)";
            // for each menu item... add to menu
            for ( std::list<MenuPath>::const_iterator ms = Settings::Get().mMenuSequence.begin();  ms != Settings::Get().mMenuSequence.end();  ms++ )
                {
                std::string shortcut_description = "";
                // menu_path -> function_id
                MenuPath menu_path = Settings::Remove( '_', *ms );
                if ( Settings::Get().mMenuToFunction.Exists( menu_path ) ) 
                    {
                    Settings::tFunctionID function_id = Settings::Get().mMenuToFunction.Get( menu_path )->second;
                    // function_id -> shortcut description
                    shortcut_description = Settings::Get().mFunctions.Get( function_id )->second.mShortcutDescription;
                    }
                // add menu item
                mMenuBar.AddMenu(  *ms,  shortcut_description );
                }

            // create panels
            MessageBar::CreatePanel();
            mLineNumBar.CreatePanel();
            mSearchBar.CreatePanel();
            mMenuBar.CreatePanel();
            mInputArea.CreatePanel();  // mInputArea panel needs to be created last / on top of menubar and message bar -- to show cursor.

            // try to open/create file
            mInputArea.New();
            if ( mFilepath != "" ) 
                {
                std::ifstream in ( mFilepath.c_str() );
                bool file_exists = ( ! in.fail() );
                in.close();
                // if file does not exist... 
                if ( ! file_exists )
                    {
                    // if user wants to create file...
                    YesNoBar::Answer answer = YesNoBar::Ask( // mSettings, 
                                                  "Cannot open filepath \"" + mFilepath + "\"   Create file?", false );
                    if ( answer == YesNoBar::Yes ) 
                        {
                        // try to create file
                        std::ofstream out ( mFilepath.c_str() );
                        file_exists = ( ! out.fail() );
                        out.close();
                        if ( ! file_exists ) 
                            {
                            MessageBar::Display( "Cannot write to filepath \"" + mFilepath + "\"" );
                            return;
                            }
                        }
                    // if user doesn't want to create file... exit
                    else if ( answer == YesNoBar::No  ||  answer == YesNoBar::Cancel )
                        {
                        MessageBar::Display( "Could not open/create filepath \"" + mFilepath + "\"" );
                        return;
                        }
                    }
                // open file
                else
                    {
                    mInputArea.OpenFile( mFilepath );
                    if ( initial_line_number > 0 )
                        {
                        mInputArea.GoToLine( initial_line_number );
                        mLineNumBar.SetInputNumber( initial_line_number );
                        }
                    }
                }
            this->LastFileModTime = time(NULL);

            SetMouseMode(  Settings::Get().mouse_mode_internal ?  INTERNAL_MOUSE  :  EXTERNAL_MOUSE  );

            // input loop
            mContinueInputLoop = true;
            Run();

            PSEUDOSTACK_END
            }


            protected: void
        ToggleMouseMode( )
            {
            if      ( mMouseMode == EXTERNAL_MOUSE ) SetMouseMode( INTERNAL_MOUSE );
            else if ( mMouseMode == INTERNAL_MOUSE ) SetMouseMode( EXTERNAL_MOUSE );
            else throw Exception ( "unhandled mouse mode", __FILE__, __FUNCTION__, __LINE__ );
            }

            protected: void
        SetMouseMode( tMouseMode mouse_mode )
            {
            if ( mouse_mode == EXTERNAL_MOUSE )
                {
                mMouseMode = EXTERNAL_MOUSE;
                mInputArea.mAutoIndent = false;
                EnableMouse( false );
                MessageBar::Display( "External mouse support only." );
                }
            else if ( mouse_mode == INTERNAL_MOUSE )
                {
                mMouseMode = INTERNAL_MOUSE;
                mInputArea.mAutoIndent = true;
                EnableMouse( true );
                MessageBar::Display( "Internal mouse support enabled." );
                }
            else throw Exception ( "unhandled mouse mode", __FILE__, __FUNCTION__, __LINE__ );
            }

            protected: void
        EnableMouse( bool enable )
            {
            if ( enable )
                {
                mmask_t result_mask = mousemask( ALL_MOUSE_EVENTS, NULL );
                if ( result_mask == 0 ) MessageBar::Display( "could not enable mouse events" );
                else                    MessageBar::Display( "mouse events enabled" );
                }
            else
                {
                mousemask( 0, NULL );
                MessageBar::Display( "mouse events disabled" );
                }
            }

            protected: void
        Paste( const std::string& text )
            {
            bool auto_indent_mode = mInputArea.mAutoIndent;
            if ( mMouseMode == EXTERNAL_MOUSE ) mInputArea.mAutoIndent = false;
            mInputArea.Insert( text );
            mInputArea.mAutoIndent = auto_indent_mode;
            }


            protected: bool
        SuffixMatch( const std::string& filepath,
                     const std::vector<std::string>& known_suffixes ) const
            {  PSEUDOSTACK_START
            // parse filepath suffix
            int dot_index = mFilepath.find_last_of( '.' );
            int suffix_index = ( dot_index == std::string::npos )?  0  :  dot_index + 1;
            std::string file_suffix = mFilepath.substr( suffix_index );
            if ( file_suffix == "" ) return false;

            // for each known suffix... does filepath suffix match?
            for ( int s = 0;  s < known_suffixes.size();  s++ )
                {
                if ( file_suffix == known_suffixes[s] ) return true;
                }
            return false;

            PSEUDOSTACK_END
            }


    ////////// methods

            protected: bool
        FileModifiedByOther( )
            {
            struct stat attrib;
            int error = stat( this->mFilepath.c_str(), &attrib );
            if ( error != 0 ) return false;
            time_t file_time = attrib.st_mtime;
            return ( file_time > this->LastFileModTime );
            }


            protected: void
        Run( )
            {  PSEUDOSTACK_START

            // input loop
            while ( mContinueInputLoop )
                {
                // get input
                int i = wgetch( stdscr );

                // mouse event
                if ( i == KEY_MOUSE ) HandleMouseEvent();
                else if ( i == KEY_RESIZE )  ResizeWindow();
                else if ( i == -1 ) {  }  // ignore noise from window switching
                // keyboard shortcut
                else if ( Settings::Get().KeyHasFunction( i, Settings::BACKWARD ) )  mInputArea.MoveLeft();
                else if ( Settings::Get().KeyHasFunction( i, Settings::FORWARD ) )   mInputArea.MoveRight();
                else if ( Settings::Get().KeyHasFunction( i, Settings::PREV_LINE ) ) mInputArea.MoveUp();
                else if ( Settings::Get().KeyHasFunction( i, Settings::NEXT_LINE ) ) mInputArea.MoveDown();
                else if ( Settings::Get().KeyHasFunction( i, Settings::HOME ) )      mInputArea.LineStart();
                else if ( Settings::Get().KeyHasFunction( i, Settings::END ) )       mInputArea.LineEnd();
                else if ( Settings::Get().KeyHasFunction( i, Settings::PAGE_DOWN ) ) mInputArea.PageDown();
                else if ( Settings::Get().KeyHasFunction( i, Settings::PAGE_UP ) )   mInputArea.PageUp();
                else if ( Settings::Get().KeyHasFunction( i, Settings::NEXT_WORD ) ) mInputArea.NextWord();
                else if ( Settings::Get().KeyHasFunction( i, Settings::FOLD_TOGGLE ) ) mInputArea.FoldToggle();
                else if ( Settings::Get().KeyHasFunction( i, Settings::SELECT ) )
                    {
                    if ( mInputArea.HasSelection() )  mInputArea.EndSelection();
                    else                          mInputArea.StartSelection();
                    }
                else if ( Settings::Get().KeyHasFunction( i, Settings::DELETE ) )    mInputArea.Delete();
                else if ( Settings::Get().KeyHasFunction( i, Settings::BACKSPACE ) ) mInputArea.Backspace();
                else if ( Settings::Get().KeyHasFunction( i, Settings::COPY )  &&  mInputArea.HasSelection() )      mCopiedText = mInputArea.Copy(); 
                else if ( Settings::Get().KeyHasFunction( i, Settings::CUT )  &&  mInputArea.HasSelection() )       mCopiedText = mInputArea.Cut();
                else if ( Settings::Get().KeyHasFunction( i, Settings::PASTE ) )     mInputArea.Insert( mCopiedText );
                else if ( Settings::Get().KeyHasFunction( i, Settings::MOUSE_MODE ) )  ToggleMouseMode();
                else if ( Settings::Get().KeyHasFunction( i, Settings::INDENT )  &&  mInputArea.HasSelection() )    mInputArea.Indent();
                else if ( Settings::Get().KeyHasFunction( i, Settings::OUTDENT )  &&  mInputArea.HasSelection() )   mInputArea.Unindent(); 
                else if ( Settings::Get().KeyHasFunction( i, Settings::COMMENT )  &&  mInputArea.HasSelection() )   mInputArea.Comment(); 
                else if ( Settings::Get().KeyHasFunction( i, Settings::UNCOMMENT )  &&  mInputArea.HasSelection() ) mInputArea.Uncomment(); 
                else if ( Settings::Get().KeyHasFunction( i, Settings::UPPER_CASE )  &&  mInputArea.HasSelection() )      mInputArea.ChangeCase( Settings::UPPER_CASE ); 
                else if ( Settings::Get().KeyHasFunction( i, Settings::LOWER_CASE )  &&  mInputArea.HasSelection() )      mInputArea.ChangeCase( Settings::LOWER_CASE ); 
                else if ( Settings::Get().KeyHasFunction( i, Settings::HUMP_CASE )  &&  mInputArea.HasSelection() )       mInputArea.ChangeCase( Settings::HUMP_CASE ); 
                else if ( Settings::Get().KeyHasFunction( i, Settings::UNDERSCORE_CASE )  &&  mInputArea.HasSelection() ) mInputArea.ChangeCase( Settings::UNDERSCORE_CASE ); 
                else if ( Settings::Get().KeyHasFunction( i, Settings::UNDO ) )      mInputArea.Undo();
                else if ( Settings::Get().KeyHasFunction( i, Settings::REDO ) )      mInputArea.Redo();
                else if ( Settings::Get().KeyHasFunction( i, Settings::MENU ) )
                    {
                    MenuPath menu_path;
                    bool choice_set;
                    mMenuBar.RunMenu( *(MEVENT*) NULL, menu_path, choice_set );
                    if ( choice_set ) HandleMenuSelection( menu_path );
                    }
                else if ( Settings::Get().KeyHasFunction( i, Settings::GOTO_LINE ) )
                    {
                    int new_line_num = mLineNumBar.Run( mInputArea.GetLineNumber(), *this );
                    if ( new_line_num != -1 )
                        mInputArea.GoToLine( new_line_num );
                    }
                else if ( Settings::Get().KeyHasFunction( i, Settings::SEARCH ) )        mSearchBar.Run( mInputArea, *this );
                else if ( Settings::Get().KeyHasFunction( i, Settings::MATCH_PAREN ) )   mInputArea.JumpToMatchingParentheses();
                else if ( Settings::Get().KeyHasFunction( i, Settings::NEW_CLASS ) )     mInputArea.Insert( mInputArea.mNewClassString );
                else if ( Settings::Get().KeyHasFunction( i, Settings::NEW_MAIN ) )      mInputArea.Insert( mInputArea.mNewMainString );
                else if ( Settings::Get().KeyHasFunction( i, Settings::NEW_FUNCTION ) )  mInputArea.Insert( mInputArea.mNewFunctionString );
                else if ( Settings::Get().KeyHasFunction( i, Settings::NEW_THROW ) )     mInputArea.Insert( mInputArea.mNewThrowString );
                else if ( Settings::Get().KeyHasFunction( i, Settings::SAVE ) )          SaveFile();
                else if ( Settings::Get().KeyHasFunction( i, Settings::SAVE_AS ) )       SaveFileAs();
                else if ( Settings::Get().KeyHasFunction( i, Settings::EXIT ) )          Exit();
                // TAB
                else if ( i == (int) '\t' )
                    {
                    if ( mInputArea.mIndentString == "\t" ) mInputArea.Insert( "\t" );
                    else                                mInputArea.Insert(  std::string ( mInputArea.CurrentTabSize(), ' ' )  );
                    }
                // content
                else if ( ! mInputArea.HasSelection() )
                    {
                    mInputArea.Insert( std::string ( ) + (char) i );
                    }

// show status message
if ( i != KEY_MOUSE )
{
String message = "";
if ( Settings::Get().mShowCharNumber ) message += (String) "key=" + TextFunctions::ToString( (int) i ) + "    ";
if ( Settings::Get().mShowCursorPosition )
    message += (String) "y=" + mInputArea.GetCursorPosition().mGridPoint.Y +
                        " x=" + mInputArea.GetCursorPosition().mGridPoint.X +
                        " index=" + mInputArea.GetCursorPosition().mTextIndex + "    ";
if ( message != "" ) MessageBar::Display( message );
}

                // check for file modified by another process
                if ( FileModifiedByOther() ) MessageBar::Display( (std::string) "File " + mFilepath + " was modified by another process." );

                }

            PSEUDOSTACK_END
            }


            protected: void
        HandleMouseEvent( )
            {  PSEUDOSTACK_START

            MEVENT mouse_event;
            int result = getmouse( & mouse_event );
            if ( result == ERR ) throw Exception ( "i = KEY_MOUSE  and  getmouse() = ERR", __FILE__, __FUNCTION__, __LINE__ );
            if ( result != OK ) throw Exception ( "i = KEY_MOUSE and getmouse() = " + TextFunctions::ToString( result ) + " unhandled", __FILE__, __FUNCTION__, __LINE__ );

            // show status message
            String message = MouseEventStateToString( mouse_event.bstate ) + "  at " + mouse_event.x + "," + mouse_event.y;
            MessageBar::Display( message );
            if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( message , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
            if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( BitsToString(mouse_event.bstate) , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            // menu panel
            if ( wenclose( mMenuBar.GetWindow(), mouse_event.y, mouse_event.x ) )
                {
                MenuPath menu_path;
                bool menu_path_set;
                mMenuBar.RunMenu( mouse_event, menu_path, menu_path_set );
                if ( menu_path_set ) HandleMenuSelection( menu_path );
                }
            // message panel
            else if ( wenclose( MessageBar::mOut->mPanel.Window(), mouse_event.y, mouse_event.x ) )
                {
                }
            // edit panel
            else if ( wenclose( mInputArea.GetWindow(), mouse_event.y, mouse_event.x ) )
                {
                if ( mouse_event.bstate & BUTTON1_CLICKED )
                    {
                    if ( mInputArea.HasSelection() )  mInputArea.EndSelection();
                    mInputArea.GoToWindowPoint(  mouse_event.y - mInputArea.mPanelStart.Y,  mouse_event.x - mInputArea.mPanelStart.X  );
// SetMouseMode( mMouseMode );
mousemask( 0, NULL );
mousemask( ALL_MOUSE_EVENTS, NULL );
                    }
                else if ( mouse_event.bstate & BUTTON1_PRESSED )
                    {
                    if ( mInputArea.HasSelection() )  mInputArea.EndSelection();
                    mInputArea.GoToWindowPoint(  mouse_event.y - mInputArea.mPanelStart.Y,  mouse_event.x - mInputArea.mPanelStart.X  );
                    mInputArea.StartSelection();
                    }
               else if ( mouse_event.bstate & BUTTON1_RELEASED )
                    {
                    if ( mInputArea.HasSelection() )
                        mInputArea.GoToWindowPoint(  mouse_event.y - mInputArea.mPanelStart.Y,  mouse_event.x - mInputArea.mPanelStart.X  );
// SetMouseMode( mMouseMode );
mousemask( 0, NULL );
mousemask( ALL_MOUSE_EVENTS, NULL );
                    }
                else if ( mouse_event.bstate & BUTTON1_DOUBLE_CLICKED )
                    {
                    mInputArea.GoToWindowPoint(  mouse_event.y - mInputArea.mPanelStart.Y,  mouse_event.x - mInputArea.mPanelStart.X  );
                    mInputArea.SelectCurrentWord();
                    }
                else if ( mouse_event.bstate & BUTTON3_CLICKED )
                    {
                    }
                else if ( mInputArea.HasSelection() )
                    {
                    mInputArea.GoToWindowPoint(  mouse_event.y - mInputArea.mPanelStart.Y,  mouse_event.x - mInputArea.mPanelStart.X  );
// SetMouseMode( mMouseMode );
                    mousemask( 0, NULL );
                    mousemask( ALL_MOUSE_EVENTS, NULL );
                    }
                }

            PSEUDOSTACK_END
            }


            protected: void
        HandleMenuSelection( const MenuPath& choice ) 
            {  PSEUDOSTACK_START

            if ( ! Settings::Get().mMenuToFunction.Exists( choice ) ) 
                {
                MessageBar::Display( choice.ToString() ); 
                return;
                }

            Settings::tFunctionID function = Settings::Get().mMenuToFunction.Get( choice )->second;

            if      ( function == Settings::BACKWARD )  mInputArea.MoveLeft();
            else if ( function == Settings::FORWARD )   mInputArea.MoveRight();
            else if ( function == Settings::PREV_LINE ) mInputArea.MoveUp();
            else if ( function == Settings::NEXT_LINE ) mInputArea.MoveDown();
            else if ( function == Settings::HOME )      mInputArea.LineStart();
            else if ( function == Settings::END )       mInputArea.LineEnd();
            else if ( function == Settings::PAGE_DOWN ) mInputArea.PageDown();
            else if ( function == Settings::PAGE_UP )   mInputArea.PageUp();
            else if ( function == Settings::NEXT_WORD ) mInputArea.NextWord();
            else if ( function == Settings::FOLD ) mInputArea.Fold();
            else if ( function == Settings::FOLD_ALL ) mInputArea.FoldAll();
            else if ( function == Settings::FOLD_INSIDE ) mInputArea.FoldInside();
            else if ( function == Settings::FOLD_TOGGLE ) mInputArea.FoldToggle();
            else if ( function == Settings::UNFOLD ) mInputArea.Unfold();
            else if ( function == Settings::UNFOLD_ALL ) mInputArea.UnfoldAll();
            else if ( function == Settings::SELECT )
                {
                if ( mInputArea.HasSelection() )  mInputArea.EndSelection();
                else                          mInputArea.StartSelection();
                }
            else if ( function == Settings::DELETE )    mInputArea.Delete();
            else if ( function == Settings::BACKSPACE ) mInputArea.Backspace();
            else if ( function == Settings::COPY  &&  mInputArea.HasSelection() )      mCopiedText = mInputArea.Copy(); 
            else if ( function == Settings::CUT  &&  mInputArea.HasSelection() )       mCopiedText = mInputArea.Cut();
            else if ( function == Settings::PASTE )     mInputArea.Insert( mCopiedText );
            else if ( function == Settings::MOUSE_MODE ) ToggleMouseMode();
            else if ( function == Settings::INDENT  &&  mInputArea.HasSelection() )    mInputArea.Indent();
            else if ( function == Settings::OUTDENT  &&  mInputArea.HasSelection() )   mInputArea.Unindent(); 
            else if ( function == Settings::COMMENT  &&  mInputArea.HasSelection() )   mInputArea.Comment(); 
            else if ( function == Settings::UNCOMMENT  &&  mInputArea.HasSelection() ) mInputArea.Uncomment(); 
            else if ( function == Settings::UPPER_CASE  &&  mInputArea.HasSelection() )      mInputArea.ChangeCase( Settings::UPPER_CASE );
            else if ( function == Settings::LOWER_CASE  &&  mInputArea.HasSelection() )      mInputArea.ChangeCase( Settings::LOWER_CASE );
            else if ( function == Settings::HUMP_CASE  &&  mInputArea.HasSelection() )       mInputArea.ChangeCase( Settings::HUMP_CASE );
            else if ( function == Settings::UNDERSCORE_CASE  &&  mInputArea.HasSelection() ) mInputArea.ChangeCase( Settings::UNDERSCORE_CASE );
            else if ( function == Settings::UNDO )      mInputArea.Undo();
            else if ( function == Settings::REDO )      mInputArea.Redo();
            else if ( function == Settings::GOTO_LINE )
                {
                int new_line_num = mLineNumBar.Run( mInputArea.GetLineNumber(), *this );
                if ( new_line_num != -1 )
                    mInputArea.GoToLine( new_line_num );
                }
            else if ( function == Settings::SEARCH )        mSearchBar.Run( mInputArea, *this );
            else if ( function == Settings::MATCH_PAREN )   mInputArea.JumpToMatchingParentheses();
            else if ( function == Settings::NEW_CLASS )     mInputArea.Insert( mInputArea.mNewClassString );
            else if ( function == Settings::NEW_MAIN )      mInputArea.Insert( mInputArea.mNewMainString );
            else if ( function == Settings::NEW_FUNCTION )  mInputArea.Insert( mInputArea.mNewFunctionString );
            else if ( function == Settings::NEW_THROW )     mInputArea.Insert( mInputArea.mNewThrowString );
            else if ( function == Settings::SAVE )          SaveFile();
            else if ( function == Settings::SAVE_AS )       SaveFileAs();
            else if ( function == Settings::EXIT )          Exit();
            else if ( function == Settings::SHOW_FILENAME ) MessageBar::Display( mFilepath );
            else                                            MessageBar::Display( choice.ToString() );

            PSEUDOSTACK_END
            }


            private: void
        onKey( int keyNum ){
            if ( keyNum == KEY_RESIZE )  ResizeWindow();
        }

            private: void
        ResizeWindow(){
            MessageBar::CreatePanel();
            mLineNumBar.CreatePanel( true );  // Line-number & search & input-area save state, so resize rather than recreate
            mSearchBar.CreatePanel( true );
            mMenuBar.ResizeWindow();  // Menu & message bars re-display while resizing
            mInputArea.ResizeWindow();
            MessageBar::Display( "KEY_RESIZE" );
        }

            protected: void
        SaveFileAs( )
            {  PSEUDOSTACK_START

            FilepathBar filepath_bar;  // ( mSettings );
            filepath_bar.CreatePanel( "Save as filepath" );
            std::string new_filepath = filepath_bar.Run( *this, mFilepath );
            if ( new_filepath != "" )
                {
                mFilepath = new_filepath;
                mInputArea.SaveFile( mFilepath );
                this->LastFileModTime = time(NULL);
                MessageBar::Display( "Saved to file \"" + mFilepath + "\"" );
                }

            PSEUDOSTACK_END
            }

            protected: void
        SaveFile( )
            {  PSEUDOSTACK_START

            // if file is unnamed... ask user to name file
            if ( mFilepath == "" )
                {
                FilepathBar filepath_bar;
                filepath_bar.CreatePanel( "Save as filepath" );
                mFilepath = filepath_bar.Run( *this );
                }
            // if file is named... save
            if ( mFilepath != "" )
                {
                mInputArea.SaveFile( mFilepath );
                this->LastFileModTime = time(NULL);
                MessageBar::Display( "Saved to file \"" + mFilepath + "\"" );
                }

            PSEUDOSTACK_END
            }


            protected: void
        Exit( )
            {  PSEUDOSTACK_START

            // if unsaved changes...
            if ( mInputArea.UnsavedChanges() )
                {
                // if user wants to save changes...
                YesNoBar::Answer answer = YesNoBar::Ask( // mSettings,
                                                "You have unsaved changes.  Do you want to save?", true );
                if ( answer == YesNoBar::Yes )
                    {
                    // prompt for filepath
                    FilepathBar filepath_bar;  //  ( mSettings );
                    filepath_bar.CreatePanel( "Save as filepath" );
                    std::string new_filepath = filepath_bar.Run( *this, mFilepath );

                    // if filepath given... save & exit
                    if ( new_filepath != "" )
                        {
                        mFilepath = new_filepath;
                        mInputArea.SaveFile( mFilepath );
                        MessageBar::Display( "Saved to file \"" + mFilepath + "\"" );

                        mContinueInputLoop = false;
                        return;
                        }
                    }
                // if user doesn't want to save changes... exit
                else if ( answer == YesNoBar::No )
                    {
                    mContinueInputLoop = false;
                    return;
                    }
                }

            // no unsaved changes
            else
                {
                mContinueInputLoop = false;
                return;
                }

            PSEUDOSTACK_END
            }



    protected: static String
MouseEventStateToString( mmask_t mouse_event_state )
    {
    String s = (String) "mouse_event_state=" + mouse_event_state;

    if ( mouse_event_state & BUTTON_SHIFT ) s += "SHIFT ";
    if ( mouse_event_state & BUTTON_CTRL )  s += "CONTROL ";
    if ( mouse_event_state & BUTTON_ALT )   s += "ALT ";

    if ( mouse_event_state & BUTTON1_PRESSED )        s +=  "BUTTON1_PRESSED";
// why is button1_released not showing when combined with button_event_all ?
    if ( mouse_event_state & BUTTON1_RELEASED )       s +=  "BUTTON1_RELEASED";
    if ( mouse_event_state & BUTTON1_CLICKED )        s +=  "BUTTON1_CLICKED";
    if ( mouse_event_state & BUTTON1_DOUBLE_CLICKED ) s +=  "BUTTON1_DOUBLE_CLICKED";
    if ( mouse_event_state & BUTTON1_TRIPLE_CLICKED ) s +=  "BUTTON1_TRIPLE_CLICKED";

    if ( mouse_event_state & BUTTON2_PRESSED )        s +=  "BUTTON2_PRESSED";
    if ( mouse_event_state & BUTTON2_RELEASED )       s +=  "BUTTON2_RELEASED";
    if ( mouse_event_state & BUTTON2_CLICKED )        s +=  "BUTTON2_CLICKED";
    if ( mouse_event_state & BUTTON2_DOUBLE_CLICKED ) s +=  "BUTTON2_DOUBLE_CLICKED";
    if ( mouse_event_state & BUTTON2_TRIPLE_CLICKED ) s +=  "BUTTON2_TRIPLE_CLICKED";

    if ( mouse_event_state & BUTTON3_PRESSED )        s +=  "BUTTON3_PRESSED";
    if ( mouse_event_state & BUTTON3_RELEASED )       s +=  "BUTTON3_RELEASED";
    if ( mouse_event_state & BUTTON3_CLICKED )        s +=  "BUTTON3_CLICKED";
    if ( mouse_event_state & BUTTON3_DOUBLE_CLICKED ) s +=  "BUTTON3_DOUBLE_CLICKED";
    if ( mouse_event_state & BUTTON3_TRIPLE_CLICKED ) s +=  "BUTTON3_TRIPLE_CLICKED";

    if ( mouse_event_state & BUTTON4_PRESSED )        s +=  "BUTTON4_PRESSED";
    if ( mouse_event_state & BUTTON4_RELEASED )       s +=  "BUTTON4_RELEASED";
    if ( mouse_event_state & BUTTON4_CLICKED )        s +=  "BUTTON4_CLICKED";
    if ( mouse_event_state & BUTTON4_DOUBLE_CLICKED ) s +=  "BUTTON4_DOUBLE_CLICKED";
    if ( mouse_event_state & BUTTON4_TRIPLE_CLICKED ) s +=  "BUTTON4_TRIPLE_CLICKED";

    if ( mouse_event_state & ALL_MOUSE_EVENTS ) s +=  "  ALL_MOUSE_EVENTS";

    return s;
    }

    protected: static String
BitsToString( mmask_t mouse_event_state )
    {
    String s = "";
    for ( int i = (sizeof(mmask_t) * 8) - 1;  i >= 0;  --i )
        s += (String) "" + ( (mouse_event_state & (1 << i))? "1" : "0" );
    return s;
    }


    };



////////////////////////////////////////////////////////////////////////////////
#endif // #ifndef INCLUDED_Editor_h
