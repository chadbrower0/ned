#ifndef INCLUDED_Shared_TextFunctions_h
#define INCLUDED_Shared_TextFunctions_h
////////////////////////////////////////////////////////////////////////////////
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <Shared/Exception.h>

// 
// utility functions for text.
// 

    class 
TextFunctions 
    {

    ////////// methods -- parsing numbers

            public: template <class tNumber> static tNumber
        To( const std::string& text ) {  return To<tNumber>( text.c_str() );  }

            public: template <class tNumber> static tNumber
        To( const char* text )
            {
            std::istringstream in ( text );
            tNumber number;
            in >> number;
            if ( in.fail() ) throw Exception ( "could not parse text = \"" + (std::string) text + "\"", __FILE__, __FUNCTION__, __LINE__ );
            return number;
            }


    ////////// methods -- printing numbers

            public: template <class tNumber> static std::string
        ToString( tNumber number ) 
            {
            std::ostringstream out;
            out << number;
            return out.str();
            }


            public: template <class T>  static std::string
        ToString( const std::vector<T>& numbers, const std::string& delimiter )
            {
            std::string s;
            for ( int n = 0;  n < numbers.size();  n++ )
                {
                if ( n > 0 ) s += delimiter;
                s += ToString( numbers[n] );
                }
            return s;
            }

            public: template <class T>  static std::string
        ToString( const std::set<T>& numbers, const std::string& delimiter )
            {
            std::string s;
            for ( typename std::set<T>::const_iterator n = numbers.begin();  n != numbers.end();  n++ )
                {
                if ( n != numbers.begin() ) s += delimiter;
                s += ToString( *n );
                }
            return s;
            }

                        


    ////////// methods -- cleaning/filtering

            public: static std::vector<std::string>
        Split( const std::string& text, char delimiter )
            {
            std::vector<std::string> tokens;
            Split( text, delimiter, tokens );
            return tokens;
            }

            public: static void
        Split( const std::string& text, char delimiter, 
               std::vector<std::string>& tokens ) // modifies tokens
            {
            Split( text.c_str(), delimiter, tokens );
            }

            public: static void
        Split( const char* text, char delimiter, 
               std::vector<std::string>& tokens ) // modifies tokens
            {
            tokens.resize( 0 );
            if ( text[0] != '\0' ) tokens.push_back( "" );
            
            // for each char... 
            for ( const char* c = text;  *c != '\0';  c++ )
                {
                // if delimiter... add new token
                if ( *c == delimiter )
                    {
                    tokens.push_back( "" );
                    }
                // if not delimiter... add to last token
                else
                    {
                    tokens.back() += *c;
                    }
                }
            }

            
            public: static std::string
        Join( const std::vector<std::string>& tokens, const std::string& delimiter ) 
            {
            std::string s;

            long s_size = 0;
            for ( size_t t = 0;  t < tokens.size();  t++ )
                s_size += tokens[t].size() + delimiter.size();
            s.reserve( s_size );

            for ( size_t t = 0;  t < tokens.size();  t++ )
                {
                if ( t > 0 ) s += delimiter;
                s += tokens[t];
                }
            return s;
            }

            public: static std::string
        Join( const std::set<std::string>& tokens, const std::string& delimiter ) 
            {
            std::string s;

            long s_size = 0;
            for ( std::set<std::string>::const_iterator t = tokens.begin();  t != tokens.end();  t++ )
                s_size += t->length() + delimiter.size();
            s.reserve( s_size );

            for ( std::set<std::string>::const_iterator t = tokens.begin();  t != tokens.end();  t++ )
                {
                if ( t != tokens.begin() ) s += delimiter;
                s += *t;
                }
            return s;
            }

            
            public: static std::string
        Trim( const std::string& trim_chars, const std::string& text )
            {
            // for each start point...
            size_t start;
            for ( start = 0;  start < text.size();  start++ )
                // if garbage not found... set start
                if ( trim_chars.find( text.at(start) ) == std::string::npos )  
                    break;

            // for each end point...
            size_t end;
            for ( end = text.size();  end > 0;  end-- )
                // if garbage not found... set end
                if ( trim_chars.find( text.at(end-1) ) == std::string::npos )
                    break;

            return text.substr( start, end - start );            
            }

            public: static void
        Lowercase( std::string& text )
            {
            for ( size_t c = 0;  c < text.size();  c++ ) 
                text[c] = tolower( text[c] );
            }

            public: static void
        ReplaceSpan( const char* old_chars, int min_span_length, 
                     const std::string& new_string, std::string& text ) // modifies text
            {
            for ( size_t i = 0;  i < text.size();  )
                {
                int span_length = strspn( text.c_str() + i, old_chars );
                if ( span_length >= min_span_length )
                    {
                    text.replace( i, span_length, new_string );
                    i += new_string.size();
                    }
                else
                    i++;
                }
            }
            
            public: static void
        ReplaceChars( const char* old_chars, char new_char, std::string& text ) //modifies text
            {
            for ( size_t i = 0;  i < text.size();  i++ )
                if ( strchr( old_chars, text[i] ) != NULL )
                    text[i] = new_char;
            }
                    
            public: static void
        Replace( const std::string& old_string, const std::string& new_string, std::string& text ) // modifies text
            {
            for ( size_t i = 0;  i < text.size();  )
                {
                if ( 0 == strncmp( text.c_str() + i, old_string.c_str(), old_string.size() ) )
                    {
                    text.replace( i, old_string.size(), new_string );
                    i += new_string.size();
                    }
                else
                    i++;
                }
            }



    ////////// methods -- files

            public: static std::string
        FileToString( const std::string& filepath ) 
            // error if filepath cannot be read.
            // does not include trailing EOF character.
            {
            std::string text;
            bool success;
            FileToString( filepath, text, success );
            if ( ! success ) throw Exception ( "TextFunctions::FileToString() could not read \"" + filepath + "\"", __FILE__, __FUNCTION__, __LINE__ );
            return text;
            }

            public: static void
        FileToString( const std::string& filepath, std::string& text, bool& success )  // modifies text and success
            // does not include trailing EOF character.
            {
            success = true;

            std::ifstream in ( filepath.c_str() );
            if ( in.fail() ) { success = false;  return; }
            
            text = "";
            while ( ! in.eof() )
                {
                char c = in.get();
                if ( in.fail() ) break;
                if ( c == EOF ) break;
                text += c;
                }
            }


    };


////////////////////////////////////////////////////////////////////////////////
#endif
