#ifndef INCLUDED_MouseMap_h
#define INCLUDED_MouseMap_h
////////////////////////////////////////////////////////////////////////////////
#include <vector>
#include "Point.h"

// 
// 
// 

    template <class T>
    class 
MouseMap 
    {
    ////////// inner types
    
            protected: class
        PositionInfo
            {
            public: Point mStart;
            public: Point mEnd;
            public: T mAction;
            };


    ////////// class data 
        protected: std::vector<PositionInfo> mPositions;


    ////////// member data 

    ////////// construction

    ////////// methods -- data access
    
            public: void
        Add( T action, const Point& start, const Point& end )
            {
// std::cerr << "\n\n" << "MouseMap.Add()    start = " << start.ToString() << "     end = " << end.ToString();
            PositionInfo new_position;
            new_position.mStart = start;
            new_position.mEnd = end;
            new_position.mAction = action;

            mPositions.push_back( new_position );
            }


    ////////// methods
    
            public: void
        FindAction( const Point& point, 
                    bool& action_set, T& action ) const // modified
            {
// std::cerr << "\n\n" << "FindAction()";
// std::cerr << "\n\t" << "point = " << point.ToString();
            action_set = false;

            for ( typename std::vector<PositionInfo>::const_iterator p = mPositions.begin();  p != mPositions.end();  p++ )
                {
// std::cerr << "\n\t" << "considering position  " << p->mStart.ToString() << "  to  " << p->mEnd.ToString();

                if ( p->mStart.Y <= point.Y  &&  point.Y < p->mEnd.Y  && 
                     p->mStart.X <= point.X  &&  point.X < p->mEnd.X )
                    {
// std::cerr << "\n\t\t" << "found action = " << p->mAction;
                    action_set = true;
                    action = p->mAction;
                    return;
                    }
                }
            }


    ////////// methods -- display

    };


////////////////////////////////////////////////////////////////////////////////
#endif // #ifndef INCLUDED_MouseMap_h
