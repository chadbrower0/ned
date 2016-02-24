#ifndef INCLUDED_NcursesModeOwner_h
#define INCLUDED_NcursesModeOwner_h
////////////////////////////////////////////////////////////////////////////////

// 
// utility function & data for turning on ncurses window mode.
// 

	class 
NcursesModeOwner 
	{
	////////// class data 
        protected: static Owner<NcursesModeOwner> mInstance;
    

	////////// member data 
    

	////////// construction

            protected:
        NcursesModeOwner( bool cbreak_mode )
            {
            initscr();
            noecho();

            if ( cbreak_mode ) cbreak(); 
            else raw();

            keypad( stdscr, TRUE );
            }

            public:
        ~NcursesModeOwner( )
            {
            // destroy all windows before exiting window mode
            MessageBar::Delete();
            endwin();
            }


	////////// methods

            public: static void
        NcursesMode( bool cbreak_mode )
            {
            mInstance = new NcursesModeOwner ( cbreak_mode );
            }

            public: static void
        NormalMode( )
            {
            mInstance.Delete();
            }

	};
////////// static data
Owner<NcursesModeOwner> NcursesModeOwner::mInstance;


////////////////////////////////////////////////////////////////////////////////
#endif // #ifndef INCLUDED_NcursesModeOwner_h
