#ifndef INCLUDED_Point_h
#define INCLUDED_Point_h
////////////////////////////////////////////////////////////////////////////////
#include <Shared/TextFunctions.h>

// 
// a pair of coordinates: y,x
// 

	class 
Point 
	{
	////////// member data 
		public: static const int Unknown;

		public: int Y;
		public: int X;


	////////// construction

			public:
		Point( int y = Unknown, int x = Unknown )
			{
			Y = y;
			X = x;
			}

	////////// methods

			public: bool
		IsUnknown( ) const
			{
			return ( Y == Unknown  ||  X == Unknown );
			}

			public: std::string
		ToString( ) const
			{
			std::string y_string = ( Y == Unknown )?  "?"  :  TextFunctions::ToString( Y );
			std::string x_string = ( X == Unknown )?  "?"  :  TextFunctions::ToString( X );
			return y_string + "," + x_string;
			}

			public: bool
		operator== ( const Point& other ) const
			{
			return ( X == other.X  &&  Y == other.Y );
			}

			public: bool
		operator!= ( const Point& other ) const
			{
			return ( X != other.X  ||  Y != other.Y );
			}

			public: const Point&
		operator= ( const Point& model )
			{
			Y = model.Y;
			X = model.X;
			return *this;
			}

	};
////////// static data
	const int Point::Unknown = -1;



////////////////////////////////////////////////////////////////////////////////
#endif // #ifndef INCLUDED_Point_h
