#include<stdlib.h>
#include<string>

extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "luawrap.h"



int Lua::luapull::trythrow(int idx){
    int argc = lua_gettop(L);
    if( lua_isnil(L, idx) ) return -1;
    //printf("IDX: %d | ARGC: %d\n", idx, argc );
    if( argc + idx < 0 || idx > argc || idx == 0 )
        return -1;
    return 0;
}
int Lua::luapull::lua_push( std::string s ){
    lua_pushstring( L, s.c_str() );
    return 0;
}

int Lua::luapull::lua_push( const char* s ){
    lua_pushstring( L, s );
    return 0;
}

int Lua::luapull::lua_push( int s ){
    lua_pushinteger( L, s );
    return 0;
}

int Lua::luapull::lua_push( double s ){
    lua_pushnumber( L, s );
    return 0;
}

int Lua::luapull::lua_push( void ){
    return 0;
}

int Lua::luapull::lua_push( void* s ){
    lua_pushnumber( L, (long long)s );
    return 0;
}

std::string Lua::luapull::lua_ret( std::string, int pos ){
    if( trythrow(pos) == 0 )
        return luaL_checkstring( L, pos );
    return "";
}

const char* Lua::luapull::lua_ret( const char*, int pos ){
    if( trythrow(pos) == 0 )
        return luaL_checkstring( L, pos );
    return "";
}

int Lua::luapull::lua_ret( int , int pos ){
    if( trythrow(pos) == 0 )
        return luaL_checkinteger( L, pos );
    return 0;
}

double Lua::luapull::lua_ret( double, int pos ){
    if( trythrow(pos) == 0 )
        return luaL_checknumber( L, pos );
    return 0;
}

void* Lua::luapull::lua_ret( void*, int pos ){
    if( trythrow(pos) == 0 )
        return (void*)luaL_checkint( L, pos );
    return 0;
}


void Lua::report_errors(lua_State *L, int status)
{
    if ( status!=0 ) {
        printf( "-- %s\n", lua_tostring(L, -1) );
    lua_pop(L, 1); // remove error message
    }
}


void *Lua::l_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
    (void)ud;  (void)osize;  /* not used */
    if (nsize == 0) {
        free(ptr);
        return NULL;
    }
    else
        return realloc(ptr, nsize);
}


int Lua::panic( lua_State *l ){
    //int n = lua_gettop(l);
    //for( int i = 0; i < n; ++i )
    //printf( "-- %s\n", lua_tostring(l, i+1) );



    printf("ERROR [%i]: %s\n", lua_status(l), lua_tostring(l, 1) );
    if( lua_status(l) == 2 ){
        lua_getglobal( l, "__LastCalled" );
        printf("LAST CALLED: %s\n", luaL_checkstring( l, -1 ) );
    }

    throw(2);
}
Lua::Lua( std::string block, std::string name, int flags ){
    L = lua_newstate( l_alloc, NULL );
    luaL_openlibs(L);
    size_t pos = block.length();
    errorlevel = luaL_loadbuffer( L, block.c_str(), pos, name.c_str() );
    lua_atpanic( L, panic );
}
Lua::~Lua(){
    //report_errors(L, errorlevel);
    //lua_close(L);
}


void Lua::run(){
    //printf("ERRORLEVEL: %i\n\n", errorlevel );
    if (errorlevel == 0)
        errorlevel = lua_pcall(L, 0, LUA_MULTRET, 0);
}


