#ifndef INCLUDED_CharData_h
#define INCLUDED_CharData_h
////////////////////////////////////////////////////////////////////////////////
#include <curses.h>
#include <Shared/Exception.h>

//
// A text character and associated formatting / meta-data.
//

    class
CharData
    {
    public: static const char BLANK_CHARACTER = ' ';
    enum FoldState {  FOLD_START, FOLD_END, FOLD_NONE  };   // probably unnecessary

//             protected: bool mBlank;    // not needed, because CharData exists only in contiguous vector[CharData], without blanks (unlike yx grid, which has blanks).
            protected: char mCharacter;
            public: int mAttributes;
            public: FoldState mFoldState;
            public: short unsigned int mFoldLevel;

                public:
            CharData( )
                {
                mCharacter = BLANK_CHARACTER;
                mFoldState = FOLD_NONE;
                mFoldLevel = 0;
                }

                public:
            CharData( char character, int attributes )
                {
                mCharacter = character;
                mAttributes = attributes;
                mFoldState = FOLD_NONE;
                mFoldLevel = 0;
                }

                public: inline char
            GetCharacter( ) const
                {
                return mCharacter;
                }

                public: void
            SetCharacter( char character )
                {
                mCharacter = character;
                }

                public: chtype
            ToFormattedChar( ) const
                {
                return ( mCharacter | mAttributes );
                }

                public: bool
            operator== ( const CharData& other ) const
                {
                return ( mCharacter == other.mCharacter  &&  mAttributes == other.mAttributes );
                }

                public: bool
            operator!= ( const CharData& other ) const {  return ! operator==( other );  }


                public: static std::string
            FoldStateToString( FoldState f )
                {
                if ( f == FOLD_START ) return "FOLD_START";
                else if ( f == FOLD_END ) return "FOLD_END";
                else if ( f == FOLD_NONE ) return "FOLD_NONE";
                else return "(unknown)";
                }

    };



////////////////////////////////////////////////////////////////////////////////
#endif

