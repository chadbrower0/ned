#ifndef INCLUDED_Settings_h
#define INCLUDED_Settings_h
////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <list>
#include <Shared/StdHashMap_Number.h>
#include <Shared/StdHashMap_String.h>
#include <Shared/StdMap.h>
#include <Shared/StdSet.h>
#include <Shared/TextFunctions.h>
#include <Shared/String.h>
#include <Shared/Logger.h>
#include <Shared/Owner.h>
#include "MenuPath.h"

//
// configuration data, singleton
//

    class
Settings
    {

            public: class
        FunctionInfo
            {
            public: std::string mShortcutDescription;
            };


            public: class
        FileTypeInfo
            {
            public: std::vector<std::string> mSuffixes;
            public: std::string mComment;
            public: std::string mIndent;

                public:
            FileTypeInfo( std::string suffix0,
                  std::string suffix1 = "",
                  std::string suffix2 = "",
                  std::string suffix3 = "",
                  std::string suffix4 = "",
                  std::string suffix5 = "",
                  std::string suffix6 = "" )
                {
                if ( suffix0 != "" ) mSuffixes.push_back( suffix0 );
                if ( suffix1 != "" ) mSuffixes.push_back( suffix1 );
                if ( suffix2 != "" ) mSuffixes.push_back( suffix2 );
                if ( suffix3 != "" ) mSuffixes.push_back( suffix3 );
                if ( suffix4 != "" ) mSuffixes.push_back( suffix4 );
                if ( suffix5 != "" ) mSuffixes.push_back( suffix5 );
                if ( suffix6 != "" ) mSuffixes.push_back( suffix6 );
                }

            };



    ////////// member data

        public: static const int FOLD_VISIBLE_LENGTH = 3;

            public: enum
        tFunctionID
            {
            BACKSPACE,
            BACKWARD,
            COMMENT,
            COPY,
            CUT,
            DELETE,
            END,
            EXIT,
            FORWARD,
            GOTO_LINE,
            HOME,
            INDENT,
            KILL,
            MATCH_PAREN,
            MENU,
            NEW_CLASS,
            NEW_MAIN,
            NEW_FUNCTION,
            NEW_THROW,
            NEXT_LINE,
            NEXT_WORD,
            OUTDENT,
            PAGE_DOWN,
            PAGE_UP,
            PASTE,
            MOUSE_MODE,
            PREV_LINE,
            REDO,
            REPLACE,
            SAVE,
            SAVE_AS,
            SEARCH,
            SELECT,
            SHOW_FILENAME,
            UNCOMMENT,
            UNDO,
            UPPER_CASE,
            LOWER_CASE,
            HUMP_CASE,
            UNDERSCORE_CASE,
            FOLD,
            FOLD_ALL,
            FOLD_INSIDE,
            FOLD_TOGGLE,
            UNFOLD,
            UNFOLD_ALL
            };

        public: StdHashMap_Number<tFunctionID,FunctionInfo> mFunctions;

        public: std::list<MenuPath> mMenuSequence;

        public: StdHashMap_Number< int, StdSet<tFunctionID> > mKeyNumberToFunctions;
        public: StdMap< MenuPath, tFunctionID > mMenuToFunction;

        protected: static Owner<Settings> mSingleton;

        public: bool mouse_mode_internal;
        public: bool key_mode_cbreak;
        public: bool menu_border_char;
        public: bool clear_redundantly;
        public: int max_undos;
        public: int tab_size;
        public: bool mShowCursorPosition;
        public: bool mShowCharNumber;

        public: std::vector<FileTypeInfo> mFileTypes;


    ////////// construction

            public: static Settings&
        Get( )
            {
            if ( mSingleton == NULL ) mSingleton = new Settings ( );
            return *mSingleton;
            }


            protected:
        Settings( )
            {  PSEUDOSTACK_START

            mouse_mode_internal = true;
            key_mode_cbreak = true;
            menu_border_char = true;
            clear_redundantly = true;
            max_undos = 10000;
            tab_size = 4;
            mShowCursorPosition = false;
            mShowCharNumber = false;

            FileTypeInfo cpp ( "h", "hpp", "cpp", "c", "m", "mm" "ipp" );
            cpp.mComment = "// ";
            cpp.mIndent = "    ";
            mFileTypes.push_back( cpp );

            FileTypeInfo java ( "java" );
            java.mComment = "// ";
            java.mIndent = "    ";
            mFileTypes.push_back( java );

            FileTypeInfo perl ( "pl", "pm", "perl" );
            perl.mComment = "# ";
            perl.mIndent = "    ";
            mFileTypes.push_back( perl );

            FileTypeInfo make ( "Makefile" );
            make.mComment = "# ";
            make.mIndent = "\t";
            mFileTypes.push_back( make );

            FileTypeInfo text ( "txt" );
            text.mIndent = "\t";  // "    ";
            mFileTypes.push_back( text );

            FileTypeInfo php ( "php", "module", "inc" );
            php.mComment = "// ";
            php.mIndent = "    ";
            mFileTypes.push_back( php );

            FileTypeInfo python ( "py" );
            python.mComment = "# ";
            python.mIndent = "    ";
            mFileTypes.push_back( python );

            FileTypeInfo sql ( "sql" );
            sql.mComment = "-- ";
            sql.mIndent = "    ";
            mFileTypes.push_back( sql );

            FileTypeInfo css ( "css" );
            css.mComment = (String) "/" + "* ";
            css.mIndent = "    ";
            mFileTypes.push_back( css );

            FileTypeInfo js ( "js", "html", "ejs", "jsx" );
            js.mComment = "// ";
            js.mIndent = "    ";
            mFileTypes.push_back( js );

            FileTypeInfo pig ( "pig" );
            pig.mComment = "-- ";
            pig.mIndent = "    ";
            mFileTypes.push_back( pig );


            AddFunctionKey( BACKSPACE, 263 );
            AddFunctionKey( BACKSPACE, 8, "CTRL+h" );
            AddFunctionKey( BACKWARD, 260 );
            AddFunctionKey( COMMENT, 51, "3" );
            AddFunctionKey( COMMENT, 99, "c" );
            AddFunctionKey( COPY, 107, "k" );
            AddFunctionKey( CUT, 11, "CTRL+k" );
            AddFunctionKey( DELETE, 4, "CTRL+d" );
            AddFunctionKey( DELETE, 330 );
            AddFunctionKey( END, 5, "CTRL+e" );
            AddFunctionKey( EXIT, 24, "CTRL+x" );
            AddFunctionKey( FORWARD, 261 );
            AddFunctionKey( GOTO_LINE, 31, "CTRL+_" );
            AddFunctionKey( HOME, 1, "CTRL+a" );
            AddFunctionKey( INDENT, 9, "TAB" );
            AddFunctionKey( INDENT, 46, "." );
            AddFunctionKey( KILL, 3, "CTRL+c" );
            AddFunctionKey( MENU, 27, "ESC" );
            AddFunctionKey( NEXT_LINE, 258 );
            AddFunctionKey( NEXT_WORD, 0, "CTRL+SPACE" );
            AddFunctionKey( OUTDENT, 353, "SHIFT+TAB" );
            AddFunctionKey( OUTDENT, 44, "," );
            AddFunctionKey( PAGE_DOWN, 338 );
            AddFunctionKey( PAGE_UP, 339 );
            AddFunctionKey( PASTE, 21, "CTRL+u" );
            AddFunctionKey( PREV_LINE, 259 );
            AddFunctionKey( FOLD_TOGGLE, 6, "CTRL+f" );
            AddFunctionKey( REDO, 18, "CTRL+r" );
            AddFunctionKey( REPLACE, 18, "CTRL+r" );
            AddFunctionKey( SAVE, 15, "CTRL+o" );
            AddFunctionKey( SEARCH, 23, "CTRL+w" );
            AddFunctionKey( SELECT, 30, "CTRL+6" );
            AddFunctionKey( SELECT, 270, "F6" );
            AddFunctionKey( UNCOMMENT, 35, "SHIFT+3" );
            AddFunctionKey( UNCOMMENT, 118, "v" );
            AddFunctionKey( UNDO, 20, "CTRL+t" );
            AddFunctionKey( UPPER_CASE, 117, "selection+u" );
            AddFunctionKey( LOWER_CASE, 108, "selection+l" );
            AddFunctionKey( HUMP_CASE, 104, "selection+h" );
            AddFunctionKey( UNDERSCORE_CASE, 115, "selection+s" );


            AddMenu( SAVE,    "_File" , "_Save" );
            AddMenu( SAVE_AS, "_File" , "Save _As" );
            AddMenu( EXIT,    "_File" , "E_xit" );
            AddMenu( SHOW_FILENAME,    "_File" , "Show _Filename" );

            AddMenu( BACKWARD  , "_Navigate" , "_Backward" );
            AddMenu( FORWARD   , "_Navigate" , "_Forward" );
            AddMenu( NEXT_WORD , "_Navigate" , "Next _Word" );
            AddMenuSpacer(       "_Navigate" );
            AddMenu( HOME      , "_Navigate" , "Line _Start" );
            AddMenu( END       , "_Navigate" , "Line _End" );
            AddMenuSpacer(       "_Navigate" );
            AddMenu( PREV_LINE , "_Navigate" , "_Previous Line" );
            AddMenu( NEXT_LINE , "_Navigate" , "_Next Line" );
            AddMenuSpacer(       "_Navigate" );
            AddMenu( GOTO_LINE   , "_Navigate" , "Go To _Line" );
            AddMenu( MATCH_PAREN , "_Navigate" , "_Match Parentheses" );
            AddMenu( SEARCH      , "_Navigate" , "_Find/Replace" );
            AddMenuSpacer(         "_Navigate" );

            AddMenu( FOLD        , "F_old" , "_Fold" );
            AddMenu( FOLD_ALL    , "F_old" , "Fold _All" );
            AddMenu( FOLD_INSIDE , "F_old" , "Fold _Inside" );
            AddMenu( FOLD_TOGGLE , "F_old" , "_Toggle Fold" );
            AddMenu( UNFOLD      , "F_old" , "_Unfold" );
            AddMenu( UNFOLD_ALL  , "F_old" , "U_nfold All" );

            AddMenu( SELECT    , "_Edit" , "Start/End _Selection" );
            AddMenuSpacer(       "_Edit" );
            AddMenu( DELETE    , "_Edit" , "_Delete" );
            AddMenu( BACKSPACE , "_Edit" , "_Backspace" );
            AddMenuSpacer(       "_Edit" );
            AddMenu( CUT       , "_Edit" , "Cut" );
            AddMenu( COPY      , "_Edit" , "Cop_y" );
            AddMenu( PASTE     , "_Edit" , "_Paste" );
            AddMenuSpacer(       "_Edit" );
            AddMenu( INDENT    , "_Edit" , "_Indent" );
            AddMenu( OUTDENT   , "_Edit" , "Unind_ent" );
            AddMenuSpacer(       "_Edit" );
            AddMenu( COMMENT   , "_Edit" , "C_omment" );
            AddMenu( UNCOMMENT , "_Edit" , "U_ncomment" );
            AddMenuSpacer(       "_Edit" );
            AddMenu( UNDO      , "_Edit" , "_Undo" );
            AddMenu( REDO      , "_Edit" , "_Redo" );
            AddMenuSpacer(       "_Edit" );
            AddMenu( UPPER_CASE      , "_Edit" , "Upper Case" );
            AddMenu( LOWER_CASE      , "_Edit" , "Lower Case" );
            AddMenu( HUMP_CASE       , "_Edit" , "Hump Case" );
            AddMenu( UNDERSCORE_CASE , "_Edit" , "Underscore Case" );

            AddMenu( PAGE_UP   , "_Prev Page" );
            AddMenu( PAGE_DOWN , "Ne_xt Page" );
            AddMenu( MOUSE_MODE , "_Mouse" );


if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "Settings", Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
if ( Logger::Enabled(Logger::DEBUG) ) Logger::Log( (String) "mFunctions.size()=" + mFunctions.size() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );

            PSEUDOSTACK_END
            }



    ////////// methods -- data access

            public: bool
        KeyHasFunction( int key_number, tFunctionID function_id ) const
            {  PSEUDOSTACK_START

            const StdSet<tFunctionID>& key_functions = mKeyNumberToFunctions.MaybeGet( key_number );

            if ( & key_functions == NULL ) return false;
            return key_functions.Exists( function_id );

            PSEUDOSTACK_END
            }


    ////////// methods

            public: static std::vector<std::string>
        Remove( char dead_char, const std::vector<std::string>& texts )
            {  PSEUDOSTACK_START
            std::vector<std::string> new_texts;
            std::string dead_text = (String) "" + dead_char;
            for ( int i = 0;  i < texts.size();  i++ )
                {
                std::string new_text = texts[i];
                TextFunctions::Replace( dead_text, "", new_text );
                new_texts.push_back( new_text );
                }

            return new_texts;

            PSEUDOSTACK_END
            }



    protected: void
AddFunctionKey( tFunctionID function_id, int key_num, std::string key_description = "" )
    {  PSEUDOSTACK_START
    // map key to functions
    mKeyNumberToFunctions.GetOrInsert( key_num, StdSet<tFunctionID> ( ) )->second.InsertNew( function_id );

    // add key descriptions to function
    if ( key_description != "" )
        {
        // ensure function exists
        FunctionInfo& function_info = mFunctions.GetOrInsert( function_id, FunctionInfo ( ) )->second;

        // add to function shortcut description
        if ( function_info.mShortcutDescription != "" ) function_info.mShortcutDescription += " or ";
        function_info.mShortcutDescription += key_description;
        }

    PSEUDOSTACK_END
    }

    protected: void
AddMenu( tFunctionID function_id, 
         std::string menu_name_0, 
         std::string menu_name_1 = "", 
         std::string menu_name_2 = "" )
    {  PSEUDOSTACK_START
            // ensure function exists
            mFunctions.GetOrInsert( function_id, FunctionInfo ( ) );

            MenuPath menu_path;
            if ( menu_name_0 != "" ) menu_path.push_back( menu_name_0 );
            if ( menu_name_1 != "" ) menu_path.push_back( menu_name_1 );
            if ( menu_name_2 != "" ) menu_path.push_back( menu_name_2 );

            mMenuToFunction.InsertNew( Remove( '_', menu_path ), function_id );
            mMenuSequence.push_back( menu_path );

    PSEUDOSTACK_END
    }

    protected: void
AddMenuSpacer( std::string menu_name_0, 
         std::string menu_name_1 = "", 
         std::string menu_name_2 = "" )
    {  PSEUDOSTACK_START
            MenuPath menu_path;
            if ( menu_name_0 != "" ) menu_path.push_back( menu_name_0 );
            if ( menu_name_1 != "" ) menu_path.push_back( menu_name_1 );
            if ( menu_name_2 != "" ) menu_path.push_back( menu_name_2 );

            menu_path.push_back( "" );
            mMenuSequence.push_back( menu_path );

    PSEUDOSTACK_END
    }



    };


Owner<Settings> Settings::mSingleton = NULL;  // wait until logger is configured



////////////////////////////////////////////////////////////////////////////////
#endif 

