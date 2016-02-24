#ifndef INCLUDED_MenuBar_h
#define INCLUDED_MenuBar_h
////////////////////////////////////////////////////////////////////////////////
#include <vector>
#include <set>
#include <Shared/Owner.h>
#include "WindowPanel.h"
#include "MessageBar.h"
#include "Settings.h"
#include "Point.h"

// 
// a menubar, with drop-down menus.
// panel-based.
// 
// want to return just a menu path (versus using callbacks), for simplicity.
// 
// want to support key shortcut menu navigation.
// also want to have labels for program shortcuts.
// 
// in the future, want to make cascading/hierarchical menus.
// 
// 

    class 
MenuBar 
    {
    ////////// inner types
    
        public: const static int UNKNOWN = -1;

            public: class
        Menu
            {
            public: std::string mName;
            public: int mShortcutIndex;  // may be UNKNOWN
            public: std::string mShortcutDescription;  // may be "" if undefined

            public: WindowPanel mPanel;

            public: int mMaxLabelLength;
            public: int mCurrentItem;
            public: bool mActive;
            public: std::vector< Owner<Menu> > mChildren;
            public: Menu* mParent;  // not owner

                public: 
            Menu( const std::string& menu_name, int shortcut_index, const std::string& shortcut_description )
                {
                mName = menu_name;
                mShortcutIndex = shortcut_index;
                mShortcutDescription = shortcut_description;

                mMaxLabelLength = mName.length();
                mCurrentItem = 0;
                mActive = false;
                }


                public: void
            AddItem( Menu* new_submenu )  // takes ownership
                // does not apply to top-level menu (menubar)
                {
                Menu& submenu = *new_submenu;
                mChildren.push_back( new_submenu );
                submenu.mParent = this;

                int line_length = submenu.mName.length();
                if ( submenu.mShortcutDescription != "" ) 
                    line_length += submenu.mShortcutDescription.length() + 4;
                if ( line_length  > mMaxLabelLength ) mMaxLabelLength = line_length;
                }

                public: int
            Depth( ) const {  return ( mParent == NULL )?  0  :  mParent->Depth()+1;  }

                public: void
            CreatePanel( int y, int x,
                         bool char_border )
                // does not apply to top-level menu (menubar)
                {
                // remake panel
                int menu_size_y = mChildren.size() + 2;
                int menu_size_x = mMaxLabelLength + 4;
                mPanel.Create( menu_size_y, menu_size_x, y, x );
                hide_panel( mPanel.Panel() );

                // draw menu border
                if ( char_border )
                    wborder( mPanel.Window(),  '|', '|',  ' ', '_',  '|', '|', '|', '|' );
                else
                    box( mPanel.Window(), 0, 0 );

                RepaintLabels();  //  tree_node );
                update_panels();
                }

                public: void
            RepaintLabels( )  // const TreeNode<Menu>& tree_node )
                {
                // for each item....
                for ( int i = 0;  i < mChildren.size();  ++i )  // tree_node.GetNumChildren();  i++ )
                    {
                    const Menu& child_menu = *mChildren.at(i);  // tree_node.GetChild(i).GetData();
                    if ( i == mCurrentItem ) wattron( mPanel.Window(), A_REVERSE );

                    // print label and shortcut
                    std::string shortcut;
                    if ( child_menu.mShortcutDescription != "" )
                        shortcut = "  (" + child_menu.mShortcutDescription + ")";

                    std::string spaces ( mMaxLabelLength - child_menu.mName.length() - shortcut.length(), ' ' );

                    mvwprintw( mPanel.Window(), i+1, 1, " %s%s%s ", 
                               child_menu.mName.c_str(), spaces.c_str(), shortcut.c_str() );

                    // underline shortcut index
                    if ( child_menu.mShortcutIndex != UNKNOWN )
                        mvwaddch( mPanel.Window(), i+1, child_menu.mShortcutIndex + 2, 
                                  child_menu.mName.at( child_menu.mShortcutIndex ) | A_UNDERLINE );

                    if ( i == mCurrentItem ) wattroff( mPanel.Window(), A_REVERSE );
                    }

                // move cursor to item
                wmove( mPanel.Window(), mCurrentItem+1, 1 );
                }

                public: void
            ShowMenu( )
                {
                show_panel( mPanel.Panel() );
                update_panels();
                doupdate();
                mActive = true;
                }

                public: void
            HideMenu( )
                {
                hide_panel( mPanel.Panel() );
                update_panels();
                doupdate();
                mActive = false;
                }
            };




    ////////// member data
        protected: bool mCharBorder;

        protected: WindowPanel mPanel;
        public: std::string mLabel;

        protected: Owner<Menu> mMenus;


    ////////// construction

            public:
        MenuBar( const std::string& label = "" )
            {
            mCharBorder = Settings::Get().menu_border_char;
            mLabel = label;

            mMenus = new Menu ( "root", UNKNOWN, "" );
            mMenus->mCurrentItem = 0;
            mMenus->mActive = false;
            }



    ////////// methods -- data access

            public: const WINDOW*
        GetWindow( ) const {  return mPanel.Window();  }


            public: void
        AddMenu( const std::vector<std::string>& menu_path,
                 const std::string& shortcut_description )
            // menu/item names may have underscores to indicate shortcut keys.
            {
            // for each sub/menu... add
            Menu* menu_node = & mMenus.MaybeGet();
            for ( int m = 0;  m < menu_path.size();  m++ )
                {
                std::string submenu_shortcut_description =  (m == menu_path.size()-1)?  shortcut_description  :  "";
                menu_node = & AddMenu( *menu_node, menu_path[m], submenu_shortcut_description );
                }
            }

            protected: Menu&
        AddMenu( Menu& parent,
                 const std::string& menu_name,
                 const std::string& shortcut_description )
            {
            // parse menu name
            int menu_shortcut_index;
            std::string menu_parsed_name;
            ParseLabel( menu_name, menu_parsed_name, menu_shortcut_index );

            // find or create menu
            Menu* menu_node = & FindMenuNode( parent, menu_parsed_name );  // menu is not owner
            if ( menu_node == NULL  ||  menu_name == "" )
                {
                // create menu
                Owner<Menu> new_child = new Menu ( menu_parsed_name, menu_shortcut_index, shortcut_description );
                menu_node = & new_child.MaybeGet();
                parent.AddItem( new_child.GiveUp() );
                }
            return *menu_node;
            }

            protected: Menu&  // may return NULL
        FindMenuNode( Menu& parent, const std::string& menu_name )
            {
            for ( int c = 0;  c < parent.mChildren.size();  ++c )
                {
                if ( parent.mChildren.at(c)->mName == menu_name )
                    return *parent.mChildren.at(c);
                }
            return *(Menu*) NULL;
            }


            protected: void
        ParseLabel( const std::string& input_label,
                    std::string& label,   // sets label
                    int& shortcut_index ) // sets shortcut_index.  may be UNKNOWN if unused.
            // Takes an input label in format "_Label" where "_" indicates
            // that the next character is the shortcut character.
            {
            label = "";
            shortcut_index = UNKNOWN;

            // for each character in input_label... find shortcut letter
            for ( int i = 0, c = 0;  i < input_label.size();  i++ )
                {
                if ( input_label.at(i) == '_' )
                    {
                    shortcut_index = c;
                    }
                else
                    {
                    label += input_label.at(i);
                    c++;
                    }
                }
            }




    ////////// methods

            public: void
        CreatePanel( )
            {
            // find screen size
            int max_y, max_x;
            getmaxyx( stdscr, max_y, max_x );

            mPanel.Create( 1, max_x, 0, 0 );

            // for each submenu... create submenu panel
            if ( mMenus != NULL )
                {
                int x = 1 + mLabel.length() + 4;
                for ( int c = 0;  c < mMenus->mChildren.size();  ++c )
                    {
                    Menu& child = *mMenus->mChildren.at(c);
                    CreateSubmenuPanel( 1, x, child );
                    x += 4 + child.mName.length();
                    }
                }

            RepaintBar();
            update_panels();
            doupdate();
            }

            protected: void
        CreateSubmenuPanel( int y, int x, Menu& submenu )
            {
            submenu.CreatePanel( y, x, mCharBorder );
            // for each child... recurse
            for ( int c = 0;  c < submenu.mChildren.size();  ++c )
                {
                CreateSubmenuPanel( y + c + 1,
                                    x + submenu.mMaxLabelLength + 2,
                                    *submenu.mChildren.at(c) );
                }
            }


            protected: void
        RepaintBar( )
            {
// std::cerr << "\n\n" << "RepaintBar()";

            // find window size
            int max_y, max_x;
            getmaxyx( mPanel.Window(), max_y, max_x );

            // print blank bar
            wattron( mPanel.Window(), A_REVERSE );
            mvwhline( mPanel.Window(), 0, 0, ' ', max_x );

            // print bar info
            int x = 1;
            mvwprintw( mPanel.Window(), 0, x, "%s", mLabel.c_str() );
            x += mLabel.length() + 6;

            // print menu names
            int x_cursor = x;
            Menu& menu = *mMenus;
            for ( int m = 0;  m < mMenus->mChildren.size();  ++m )
                {
                if ( menu.mActive  &&  m == menu.mCurrentItem ) wattroff( mPanel.Window(), A_REVERSE );
                Menu& child_menu = *mMenus->mChildren.at(m);
                wmove( mPanel.Window(), 0, x );
// std::cerr << "\n\t" << child_menu.mName << "    x = " << x;
                for ( int c = 0;  c < child_menu.mName.size();  c++ )
                    {
                    if ( c == child_menu.mShortcutIndex ) wattron( mPanel.Window(), A_UNDERLINE );
                    waddch( mPanel.Window(), child_menu.mName.at(c) );
                    if ( c == child_menu.mShortcutIndex ) wattroff( mPanel.Window(), A_UNDERLINE );
                    }
                if ( m == menu.mCurrentItem ) wattron( mPanel.Window(), A_REVERSE );

                if ( m == menu.mCurrentItem ) x_cursor = x-1;
                x += child_menu.mName.length() + 4;
                }

            // move cursor to item
            wmove( mPanel.Window(), 0, x_cursor );
            }



            protected: bool
        ShortcutMatch( char c, const std::string& label, int shortcut_index ) const
            {
            if ( label == "" ) return false;
            if ( shortcut_index == UNKNOWN ) return false;
            return ( tolower( label.at( shortcut_index ) ) == tolower( c ) );
            }


            public: void
        RunMenu( const MEVENT& initial_mouse_event, // may be NULL
                 MenuPath& menu_path, bool& menu_path_set )  // modified
            {
            if ( mMenus->mChildren.size() == 0 ) throw Exception ( "RunMenu() with empty menubar", __FILE__, __FUNCTION__, __LINE__ );

            menu_path_set = false;

            mMenus->mActive = true;
            RepaintBar();
            update_panels();
            doupdate();

            // initial mouse event
            if ( & initial_mouse_event != NULL )
                HandleMenubarMouseEvent( initial_mouse_event, menu_path, menu_path_set );

            // user input loop
            while ( ! menu_path_set )
                {
                // get event
                int i = getch();
                // try to handle event
                HandleEvent( i, menu_path, menu_path_set );
                    // caller needs to handle unhandled mouse event?
                    // No -- just cancel menus.
                }

            mMenus->mActive = false;
            RepaintBar();
            }


            protected: void
        HandleEvent( int i, 
                     MenuPath& menu_path, bool& menu_path_set )  // modified
            {
            char c = (char) i;
            Menu& menu_node = ActiveMenu( *mMenus );
// std::cerr << "\n\n" << "ActiveMenu() = " << menu_node.GetData().mName;  // std::cerr.flush();

            if ( i == KEY_MOUSE )
                {
                // get mouse event data
                MEVENT mouse_event;
                int result = getmouse( & mouse_event );
                if ( result == ERR ) throw Exception ( "i = KEY_MOUSE  and  getmouse() = ERR", __FILE__, __FUNCTION__, __LINE__ );
                if ( result != OK ) throw Exception ( "i = KEY_MOUSE and getmouse() = " + TextFunctions::ToString( result ) + " unhandled", __FILE__, __FUNCTION__, __LINE__ );

                HandleMenubarMouseEvent( mouse_event, menu_path, menu_path_set );
                }
            else if ( Settings::Get().KeyHasFunction( i, Settings::BACKWARD ) )
                {
                if ( mMenus == & menu_node ) 
                    ActivateSubmenuItem( menu_node, menu_node.mCurrentItem - 1, false, false, menu_path, menu_path_set );
                else                                     
                    menu_node.HideMenu(); 
                }
            else if ( Settings::Get().KeyHasFunction( i, Settings::FORWARD ) )
                {
                if ( mMenus == & menu_node ) 
                    ActivateSubmenuItem( menu_node, menu_node.mCurrentItem + 1, false, false, menu_path, menu_path_set );
                else                                     
                    ActivateSubmenuItem( menu_node, menu_node.mCurrentItem, true, false, menu_path, menu_path_set );
                }
            else if ( Settings::Get().KeyHasFunction( i, Settings::PREV_LINE ) )
                {
                if ( menu_node.Depth() == 1  &&  menu_node.mCurrentItem == 0 ) 
                    menu_node.HideMenu();
                else if ( mMenus == & menu_node )                               
                    {  }
                else                                                                        
                    ActivateSubmenuItem( menu_node, menu_node.mCurrentItem - 1, false, false, menu_path, menu_path_set );
                }
            else if ( Settings::Get().KeyHasFunction( i, Settings::NEXT_LINE ) )
                {
                if ( mMenus == & menu_node ) 
                    ActivateSubmenuItem( menu_node, menu_node.mCurrentItem, true, false, menu_path, menu_path_set );
                else                                     
                    ActivateSubmenuItem( menu_node, menu_node.mCurrentItem + 1, false, false, menu_path, menu_path_set );
                }
            else if ( c == '\n' )
                {
                if ( mMenus == & menu_node ) 
                    ActivateSubmenuItem( menu_node, menu_node.mCurrentItem, true, true, menu_path, menu_path_set );
                else                                     
                    ActivateSubmenuItem( menu_node, menu_node.mCurrentItem, true, true, menu_path, menu_path_set );
                }
            else if ( Settings::Get().KeyHasFunction( i, Settings::MENU ) )
                {
                menu_path.clear();
                menu_path_set = true;
                // close menus
                for ( Menu* m = & menu_node;  mMenus != m;  m = m->mParent )
                    m->HideMenu();
                }
            else
                {
                // find menu item matching shortcut key... activate menu item
                for ( int m = 0;  m < menu_node.mChildren.size();  m++ )
                    {
                    Menu& child_menu = *menu_node.mChildren.at(m);
                    if ( ShortcutMatch( c, child_menu.mName, child_menu.mShortcutIndex ) )
                        {
                        ActivateSubmenuItem( menu_node, m, true, true, menu_path, menu_path_set );
                        break;
                        }
                    }
                }
            }


            protected: Menu&
        ActiveMenu( Menu& menu_node )
            {
            // if there's an active child... recurse
            for ( int c = 0;  c < menu_node.mChildren.size();  c++ )
                if ( menu_node.mChildren.at(c)->mActive )
                    return ActiveMenu( *menu_node.mChildren.at(c) );

            // if no active child... this is the active menu
            return menu_node;
            }


            protected: void
        HandleMenubarMouseEvent( const MEVENT& mouse_event, 
                                 MenuPath& menu_path, bool& menu_path_set )  // modified
            {
// std::cerr << "\n\n" << "HandleMenubarMouseEvent() ";  // std::cerr.flush();

            bool event_handled = false;

            if ( wenclose( mPanel.Window(), mouse_event.y, mouse_event.x ) )
                {
// std::cerr << "\n\t" << "wenclose menubar panel";  // std::cerr.flush();

                // translate mouse_event coordinates to window-relative coordinates
                Point window_start;
                getbegyx( mPanel.Window(), window_start.Y, window_start.X );
                Point mouse_relative_window = Point (  mouse_event.y - window_start.Y ,  mouse_event.x - window_start.X );
// std::cerr << "\n\t" << "window_start = " << window_start.ToString();  // std::cerr.flush();
// std::cerr << "\n\t" << "mouse_relative_window = " << mouse_relative_window.ToString();  // std::cerr.flush();

                if ( mouse_event.bstate & BUTTON1_CLICKED )
                    {
// std::cerr << "\n\t" << "BUTTON1_CLICKED";  // std::cerr.flush();
                    // find corresponding menubar item
                    int x = 1 + mLabel.length() + 6;
                    for ( int c = 0;  c < mMenus->mChildren.size();  ++c )
                        {
                        Menu& child = *mMenus->mChildren.at(c);
// std::cerr << "\n\t\t" << child.GetData().mName << "    x = " << x;

                        if ( x <= mouse_relative_window.X  &&  mouse_relative_window.X < x + child.mName.length() )
                            {
                            ActivateSubmenuItem( *mMenus, c, true, true, menu_path, menu_path_set );
                            break;
                            }
                        x += child.mName.length() + 4;
                        }
                    }
                event_handled = true;
                }
            else 
                {
                // for each submenu... maybe handle mouse_event
                for ( int m = 0;  m < mMenus->mChildren.size()  &&  ! event_handled;  ++m )
                    HandleSubmenuMouseEvent( *mMenus->mChildren.at(m), mouse_event, event_handled, menu_path, menu_path_set );
                }

            if ( ! event_handled ) 
                {
                menu_path.clear();
                menu_path_set = true;
                // close menus
                for ( Menu* m = & ActiveMenu( *mMenus );  mMenus != m;  m = m->mParent )
                    m->HideMenu();
                }
            }


            protected: void
        HandleSubmenuMouseEvent( Menu& menu_node, const MEVENT& mouse_event, 
                                 bool& event_handled,  // modified
                                 MenuPath& menu_path, bool& menu_path_set )  // modified
            {

            // recurse to allow submenus to handle event
            for ( int c = 0;  c < menu_node.mChildren.size()  &&  ! event_handled;  ++c )
                HandleSubmenuMouseEvent( *menu_node.mChildren.at(c), mouse_event, event_handled, menu_path, menu_path_set );
            if ( event_handled ) return;

            if ( menu_node.mActive  &&  
                 wenclose( menu_node.mPanel.Window(), mouse_event.y, mouse_event.x ) )
                {
                // translate mouse_event coordinates to window-relative coordinates
                Point window_start;
                getbegyx( menu_node.mPanel.Window(), window_start.Y, window_start.X );
                Point mouse_relative_window = Point (  mouse_event.y - window_start.Y ,  mouse_event.x - window_start.X );

                if ( mouse_event.bstate & BUTTON1_CLICKED )
                    {
                    int i = mouse_relative_window.Y - 1;
                    if ( 2 <= mouse_relative_window.X  &&  mouse_relative_window.X < menu_node.mMaxLabelLength + 2  &&  
                         0 <= i  &&  i < menu_node.mChildren.size() )
                        {
                        ActivateSubmenuItem( menu_node, i, true, true, menu_path, menu_path_set );
                        }
                    }
                event_handled = true;
                }
            }


            protected: void
        HideActiveSubmenus( Menu& menu )
            {
            for ( int m = 0;  m < menu.mChildren.size();  ++ m )
                HideActiveSubmenus( *menu.mChildren.at(m) );
            if ( menu.mActive ) menu.HideMenu();
            }

            protected: void
        ActivateSubmenuItem( Menu& menu, int item_index,
                             bool expand_child_menu,
                             bool choose_if_no_child,
                             MenuPath& menu_path, bool& menu_path_set )  // modified
            {
            if ( item_index < 0 ) return;
            if ( menu.mChildren.size() <= item_index ) return;

            // set selected menu item
            menu.mCurrentItem = item_index;
if ( mMenus == & menu ) RepaintBar();
else                    menu.RepaintLabels();

            // close descendants of menu
            for ( int m = 0;  m < menu.mChildren.size();  ++m )
                HideActiveSubmenus( *menu.mChildren.at(m) );

            // item is submenu
            if ( menu.mChildren.at( menu.mCurrentItem )->mChildren.size() > 0  &&  expand_child_menu )
                {
//                 if ( mMenus == & menu ) RepaintBar();
//                 else                    menu.RepaintLabels();
                update_panels();
                doupdate();

                menu.mChildren.at( item_index )->ShowMenu();
                }
            // item is final selection
            else if ( menu.mChildren.at( menu.mCurrentItem )->mChildren.size() == 0  &&  choose_if_no_child )
                {
                // set menu_path
                menu_path_set = true;
                menu_path.clear();
                for ( const Menu* m = & menu.mChildren.at( menu.mCurrentItem ).MaybeGet();  mMenus != m;  m = m->mParent )
                    menu_path.insert( menu_path.begin(), m->mName );

                // close menus
                for ( Menu* m = & menu;  mMenus != m;  m = m->mParent )
                    m->HideMenu();
                }
            // change selected item
            else
                {
//                 if ( mMenus == & menu ) RepaintBar();
//                 else                    menu.RepaintLabels();
                update_panels();
                doupdate();
                }

            }


    };



////////////////////////////////////////////////////////////////////////////////
#endif // #ifndef INCLUDED_MenuBar_h
