#ifndef __LUAWRAP_H__
#define __LUAWRAP_H__

extern "C" {
#include "lua.h"
}

#include<string>
#include<stdio.h>

class Lua {
public:
    lua_State* L;
    int errorlevel;

    struct luapull {
        lua_State *L;
        int trythrow( int idx );
        int lua_push( std::string s );
        int lua_push( const char* s );

        int lua_push( int s );
        int lua_push( double s );
        int lua_push( void );
        int lua_push( void* s );

        std::string lua_ret( std::string, int pos = -1 );
        const char* lua_ret( const char*, int pos = -1 );
        int lua_ret( int , int pos = -1 );
        double lua_ret( double, int pos = -1  );
        void* lua_ret( void*, int pos = -1 );
    };

    //Template metaprogramming skills are weak with this one.
    template <class RetType, RetType( *fun )()> static int luawrap( lua_State *Lua ) {
        luapull p;
        p.L = Lua;
        p.lua_push( fun( ) );
        return 1;
    }
    template <class RetType, class arg0, RetType( *fun )( arg0 )> static int luawrap( lua_State *Lua ) {
        luapull p;
        p.L = Lua;
        p.lua_push( fun( ( arg0 ) p.lua_ret( arg0(), 1 ) ) );
        return 1;
    }
    template <class RetType, class arg0, class arg1, RetType( *fun )( arg0, arg1 )> static int luawrap( lua_State *Lua ) {
        luapull p;
        p.L = Lua;
        p.lua_push( fun( ( arg0 ) p.lua_ret( arg0(), 1 ), ( arg1 ) p.lua_ret( arg1(), 2 ) ) );
        return 1;
    }
    template <class RetType, class arg0, class arg1, class arg2, RetType( *fun )( arg0, arg1, arg2 )> static int luawrap( lua_State *Lua ) {
        luapull p;
        p.L = Lua;
        p.lua_push( fun( ( arg0 ) p.lua_ret( arg0(), 1 ), ( arg1 ) p.lua_ret( arg1(), 2 ), ( arg2 ) p.lua_ret( arg2(), 3 ) ) );
        return 1;
    }
    template <class RetType, class arg0, class arg1, class arg2, class arg3, RetType( *fun )( arg0, arg1, arg2, arg3 )> static int luawrap( lua_State *Lua ) {
        luapull p;
        p.L = Lua;
        p.lua_push( fun( ( arg0 ) p.lua_ret( arg0(), 1 ), ( arg1 ) p.lua_ret( arg1(), 2 ), ( arg2 ) p.lua_ret( arg2(), 3 ), ( arg3 ) p.lua_ret( arg3(), 4 ) ) );
        return 1;
    }
    template <class RetType, class arg0, class arg1, class arg2, class arg3, class arg4, RetType( *fun )( arg0, arg1, arg2, arg3, arg4 )> static int luawrap( lua_State *Lua ) {
        luapull p;
        p.L = Lua;
        p.lua_push( fun( ( arg0 ) p.lua_ret( arg0(), 1 ), ( arg1 ) p.lua_ret( arg1(), 2 ), ( arg2 ) p.lua_ret( arg2(), 3 ), ( arg3 ) p.lua_ret( arg3(), 4 ), ( arg4 ) p.lua_ret( arg4(), 5 ) ) );
        return 1;
    }
    template <class RetType, class arg0, class arg1, class arg2, class arg3, class arg4, class arg5, RetType( *fun )( arg0, arg1, arg2, arg3, arg4, arg5 )> static int luawrap( lua_State *Lua ) {
        luapull p;
        p.L = Lua;
        p.lua_push( fun( ( arg0 ) p.lua_ret( arg0(), 1 ), ( arg1 ) p.lua_ret( arg1(), 2 ), ( arg2 ) p.lua_ret( arg2(), 3 ), ( arg3 ) p.lua_ret( arg3(), 4 ), ( arg4 ) p.lua_ret( arg4(), 5 ), ( arg5 ) p.lua_ret( arg5(), 6 ) ) );
        return 1;
    }
    template <class RetType, class arg0, class arg1, class arg2, class arg3, class arg4, class arg5, class arg6, RetType( *fun )( arg0, arg1, arg2, arg3, arg4, arg5, arg6 )> static int luawrap( lua_State *Lua ) {
        luapull p;
        p.L = Lua;
        p.lua_push( fun( ( arg0 ) p.lua_ret( arg0(), 1 ), ( arg1 ) p.lua_ret( arg1(), 2 ), ( arg2 ) p.lua_ret( arg2(), 3 ), ( arg3 ) p.lua_ret( arg3(), 4 ), ( arg4 ) p.lua_ret( arg4(), 5 ), ( arg5 ) p.lua_ret( arg5(), 6 ), ( arg6 ) p.lua_ret( arg6(), 7 ) ) );
        return 1;
    }


    static void report_errors( lua_State *L, int status );
    static void *l_alloc ( void *ud, void *ptr, size_t osize, size_t nsize );



public:

    template<class ReturnType, ReturnType ( *f )()>
    void funcreg( const char* name ) {
        lua_register( L, name, ( luawrap<ReturnType, f> ) );
    }
    template<class ReturnType, class arg0, ReturnType ( *f )( arg0 )>
    void funcreg( const char* name ) {
        lua_register( L, name, ( luawrap<ReturnType, arg0, f> ) );
    }
    template<class ReturnType, class arg0, class arg1, ReturnType ( *f )( arg0, arg1 )>
    void funcreg( const char* name ) {
        lua_register( L, name, ( luawrap<ReturnType, arg0, arg1, f> ) );
    }
    template<class ReturnType, class arg0, class arg1, class arg2, ReturnType ( *f )( arg0, arg1, arg2 )>
    void funcreg( const char* name ) {
        lua_register( L, name, ( luawrap<ReturnType, arg0, arg1, arg2, f> ) );
    }
    template<class ReturnType, class arg0, class arg1, class arg2, class arg3, ReturnType ( *f )( arg0, arg1, arg2, arg3 )>
    void funcreg( const char* name ) {
        lua_register( L, name, ( luawrap<ReturnType, arg0, arg1, arg2, arg3, f> ) );
    }
    template<class ReturnType, class arg0, class arg1, class arg2, class arg3, class arg4, ReturnType ( *f )( arg0, arg1, arg2, arg3, arg4 )>
    void funcreg( const char* name ) {
        lua_register( L, name, ( luawrap<ReturnType, arg0, arg1, arg2, arg3, arg4, f> ) );
    }
    template<class ReturnType, class arg0, class arg1, class arg2, class arg3, class arg4, class arg5, ReturnType ( *f )( arg0, arg1, arg2, arg3, arg4, arg5 )>
    void funcreg( const char* name ) {
        lua_register( L, name, ( luawrap<ReturnType, arg0, arg1, arg2, arg3, arg4, arg5, f> ) );
    }
    template<class ReturnType, class arg0, class arg1, class arg2, class arg3, class arg4, class arg5, class arg6, ReturnType ( *f )( arg0, arg1, arg2, arg3, arg4, arg5, arg6 )>
    void funcreg( const char* name ) {
        lua_register( L, name, ( luawrap<ReturnType, arg0, arg1, arg2, arg3, arg4, arg5, arg6, f> ) );
    }



    static int panic( lua_State *l );
    Lua( std::string block, std::string name, int flags );
    ~Lua();

    template <class type>
    void set( std::string name, type value ) {
        luapull p;
        p.L = L;
        p.lua_push( value );
        lua_setglobal ( L, name.c_str() );
    }

    template <class type>
    type get( std::string name ) {
        luapull p;
        p.L = L;
        lua_getglobal ( L, name.c_str() );
        return ( type ) p.lua_ret( type() );
    }

    template<class ReturnType, class... Arguments>
    class LuaFunc {
    private:
        std::string funcname;
        lua_State *L;
        luapull p;
    public:
        LuaFunc( lua_State *Lua, const char* name ) :
            funcname( name ),
            L( Lua ) {
            p.L = L;
        }
        LuaFunc( ) :
            funcname( "" ),
            L( 0 ) {
            p.L = L;
        }
        LuaFunc& operator=( LuaFunc f ) {
            funcname = f.funcname;
            L = f.L;
            p.L = L;
            return *this;
        }
        template<typename... Args> inline void pass( Args... ) {}

        ReturnType operator()( Arguments... a ) {
            //ReturnType ret = ReturnType();
            //try{
#ifdef DEBUG
            lua_pushstring( L, funcname.c_str() );
            lua_setglobal( L, "__LastCalled" );
            //printf("lol");
#endif
            lua_getglobal( L, funcname.c_str() ); /* function to be called */
            pass( p.lua_push( a )... );

            lua_pcall( L, sizeof...( Arguments ), 1, 0 );
            //printf("R=%i\n", r);
            //printf("ARGS: %i\n", sizeof...(Arguments));
            ReturnType ret = p.lua_ret( ReturnType() );

            //}
            //catch(...){

            //}
            lua_pop( L, lua_gettop( L ) );
            return ret;
        }
    };

    template<class ReturnType, class... Arguments>
    LuaFunc<ReturnType, Arguments...> getfuncbyname( const char* name ) {
        return LuaFunc<ReturnType, Arguments...>( L, name );
    }
    void run();


};

#endif
