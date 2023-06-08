#ifndef INCLUDED_KeyListener_h
#define INCLUDED_KeyListener_h
//////////////////////////////////////////////////////////////////////
#include <curses.h>


// Interface

    class
KeyListener {
    ////////// Construction

        public: virtual
    ~KeyListener( ){  }


    ////////// Methods

        public: virtual void
    onKey( int keyNum ) = 0;

};


//////////////////////////////////////////////////////////////////////
#endif // #ifndef INCLUDED_KeyListener_h
