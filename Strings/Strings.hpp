#ifndef MUD_STRINGS_STRINGS_HPP
#define MUD_STRINGS_STRINGS_HPP

#include<string>
#include<string.h>
#include<stdio.h>
#include <type_traits>
#include <cxxabi.h>


namespace GlobalMUD{
    std::string BufferToString( const char* buffer, size_t len );
    std::string StringToUpper( std::string str );
    std::string StringToLower(std::string in);
    std::string StringFromUInt(unsigned int i);
    unsigned int HashString(std::string v);

    template<class... Args>
    std::string StringFormat( std::string format, Args... args );
    namespace{
        //Use this to split up a template list and convert std::string into const char*
        template<class T>
        T Helper(T value){
            return value;
        }

        const char* Helper(std::string value){
            return value.c_str();
        }

        //If T has the cast for std::string overloaded, set Has to 1.
        //Otherwise, set Has to 0.
        template<class T>
        struct HasStringCast
        {
            template<class U, std::string (U::*)()> struct Tester  {};
            template<class U> static char Test(Tester <U, &U::operator std::string>*);
            template<class U> static int Test(...);

            static const int Has = sizeof(Test<T>(nullptr)) == sizeof( char );
        };

        //If T has the cast for const char* overloaded, set Has to 1.
        //Otherwise, set Has to 0.
        template<class T>
        struct HasCStringCast
        {
            template<class U, const char* (U::*)()> struct Tester  {};
            template<class U> static char Test(Tester <U, &U::operator const char*>*);

            template<class U> static int Test(...);
            static const int Has = sizeof(Test<T>(nullptr)) == sizeof( char );
        };

        //If the object doesn't have the cast for std::string overloaded, but does have
        //the cast for const char* overloaded, return a string containing the casted value.
        //Call the overload explicitly to avoid ambiguity errors.
        template<class TMap>
        std::string CastString(TMap& m, std::integral_constant<int, 3>)
        {
            std::string ret = m.operator const char*();
            return ret;
        }

        //If the object doesn't have the cast for std::string or the cast for const char*
        //overloaded, don't return anything of significance.
        template<class TMap>
        std::string CastString(TMap& m, std::integral_constant<int, 2>)
        {
            return "";
        }

        //If the object has the cast for std::string overloaded, then a string containing
        //the casted value. Call the overload explicitly to avoid ambiguity errors.
        template<class TMap>
        std::string CastString(TMap& m, std::integral_constant<int, 1>)
        {
            return m.operator std::string();
        }

        //If the object doesn't have an overload for a cast to std::string, check if it has
        //a cast for const char*.
        template<class TMap>
        std::string CastString(TMap& m, std::integral_constant<int, 0>)
        {
            return CastString(m,
                std::integral_constant<int, HasCStringCast<TMap>::Has?3:2>());
        }

        //Check if the object has an overload for a cast to std::string.
        template<class TMap>
        std::string CastString(TMap& m)
        {
            return CastString(m,
                std::integral_constant<int, HasStringCast<TMap>::Has?1:0>());
        }

        template<class T, class U>
        struct is_same {
            enum { value = 0 };
        };

        template<class T>
        struct is_same<T, T> {
            enum { value = 1 };
        };

        //This is the default ToString method for any types that aren't accounted for
        //in the explicit specializations that follow.
        template<class T>
        std::string ToString( T value ){
            std::string ret = "";

            if( is_same<const char*, decltype(value)>::value ){
                ret = value;
            }
            else{
                ret = CastString( value );
            }
            return ret;
        }

        //A list of specializations for the template that will spit out std::string versions of various types.
        template<>          std::string ToString<bool>                  ( bool value )                  { return StringFormat( "%s",  value?"True" : "False" ); }
        template<>          std::string ToString<char>                  ( char value )                  { return StringFormat( "%c",  value ); }
        template<>          std::string ToString<unsigned char>         ( unsigned char value )         { return StringFormat( "%hhu",value ); }
        template<>          std::string ToString<short>                 ( short value )                 { return StringFormat( "%hd", value ); }
        template<>          std::string ToString<unsigned short>        ( unsigned short value )        { return StringFormat( "%hu", value ); }
        template<>          std::string ToString<int>                   ( int value )                   { return StringFormat( "%d",  value ); }
        template<>          std::string ToString<unsigned int>          ( unsigned int value )          { return StringFormat( "%u",  value ); }
        template<>          std::string ToString<long int>              ( long int value )              { return StringFormat( "%ld", value ); }
        template<>          std::string ToString<unsigned long int>     ( unsigned long int value )     { return StringFormat( "%lu", value ); }
        template<>          std::string ToString<long long int>         ( long long int value )         { return StringFormat( "%lld",value ); }
        template<>          std::string ToString<unsigned long long int>( unsigned long long int value ){ return StringFormat( "%llu",value ); }
        template<>          std::string ToString<float>                 ( float value )                 { return StringFormat( "%f",  value ); }
        template<>          std::string ToString<double>                ( double value )                { return StringFormat( "%f",  value ); }
        template<>          std::string ToString<long double>           ( long double value )           { return StringFormat( "%Lf", value ); }
        template<>          std::string ToString<const char*>           ( const char* value )           { return StringFormat( "%s",  value ); }
        template<>          std::string ToString<std::string>           ( std::string value )           { return StringFormat( "%s",  value ); }

        //Recursively concatenate a list of objects with operator+.
        //This version of Concatenate is used to terminate the recursion once the argument list has been exhausted.
        template<class Str1>
        std::string Concatenate( Str1 string1 ){
            return string1;
        }

        //This version of Concatenate will recurse until it only has one argument left,
        //at which point it will call the non-variadic version of the function to terminate the recursion.
        template<class Str1, class... Args>
        std::string Concatenate( Str1 string1, Args... args ){
            return string1 + Concatenate( args... );
        }
    }

    //Return a string formatted with printf parameters.
    template<class... Args>
    std::string StringFormat( std::string format, Args... args ){

        size_t len = snprintf( nullptr, 0, format.c_str(), Helper(args)...);
        std::string ret;
        ret.resize(len);
        ret.reserve(len+1);
        snprintf( &ret[0], len+1, format.c_str(), Helper(args)...);
        return ret;
    }

    //Print a string formatted with printf parameters.
    template<class... Args>
    size_t PrintFormat( std::string format, Args... args ){
        std::string toPrint = StringFormat( format, args... );
        fwrite( toPrint.c_str(), 1, toPrint.length(), stdout );
        return toPrint.length();
    }

    //Concatenate a series of values of any type into a single string.
    template<class... Args>
    std::string ToString( Args... args ){
        return Concatenate( ToString(args)... );
    }

    //Print a series of values of any type.
    template<class... Args>
    size_t Print( Args... args ){
        std::string toPrint = ToString( args... );
        fwrite( toPrint.c_str(), 1, toPrint.length(), stdout );
        return toPrint.length();
    }

    //Hash a string for use in a switch statement.
    namespace{static const int wrapPoint = ((2 << (sizeof(unsigned int)*4)) - 1);}
    constexpr unsigned int HashString(const char* v, unsigned int inNum = 27487){
        return (
                    (
                        v[1] != '\0' ?
                            ( v[0] + HashString( v+1, (v[0]*v[0]*inNum) % wrapPoint ) )
                        :
                            v[0]
                    ) * inNum
                )
                % wrapPoint;
    }

    void strupr( char *str );
    std::string StringRepeat(std::string str, int amt);

}

#endif
