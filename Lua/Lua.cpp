#include "Lua/Lua.hpp"

extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include<string>
#include<cstdlib>
#include<cstring>
#include "Error/Error.hpp"


void Lua::report_errors(lua_State *L, int status){
    if ( status!=0 ) {
        printf( "-- %s\n", lua_tostring(L, -1) );
        lua_pop(L, 1); //remove error message
    }
}


void *Lua::l_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
    if (nsize == 0) {
        free(ptr);
        return NULL;
    }
    else
        return osize < nsize ? realloc(ptr, nsize) : ptr;
}


static int PanicState = 0;
int Lua::panic( lua_State *l ){

    throw GlobalMUD::Error::LuaError;
    //This is dead code used for debugging purposes.
    if( PanicState == 0 ){
        PanicState = 1; //Prevent an infinite loop if checkstring panics
        printf("ERROR [%i]: %s\n", lua_status(l), lua_tostring(l, 1) );

        if( lua_status(l) == 2 ){
            lua_getglobal( l, "__LastCalled" );
            printf("LAST CALLED: %s\n", luaL_checkstring( l, -1 ) );
        }
    }
    PanicState = 0;
    throw GlobalMUD::Error::LuaError;
}

int Lua::Writer (lua_State *L, const void* p, size_t sz, void* ud){
    //Used for extracting compiled byte code from a Lua object
    WriteStruct* args = (WriteStruct*) ud;
    size_t pos = args->buffer.size();
    args->buffer.resize( args->buffer.size() + sz );
    memcpy( &(args->buffer[pos]), p, sz );
    return 0;
}


bool Lua::GetValueToStack( lua_State *L, std::string globalname, int reference ){
    //Summon the variable to the top of the stack
    if( globalname != "" ){
        lua_getglobal(L, globalname.c_str());
    }
    else if( reference != -1 ) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, reference);
    }
    else{
        return false;
    }
    return true;
}

void Lua::Init(){
    L = lua_newstate( l_alloc, nullptr );
    luaL_openlibs(L);
    lua_atpanic( L, panic );
}
void Lua::Load(std::string block, std::string name, int flags){
    size_t pos = block.length();
    errorlevel = luaL_loadbuffer( L, block.c_str(), pos, name.c_str() );
}
void Lua::Load(std::vector<char> block, std::string name, int flags){
    size_t pos = block.size();
    errorlevel = luaL_loadbuffer( L, &(block[0]), pos, name.c_str() );
}
void Lua::Load( std::string fname ){
    errorlevel = luaL_loadfile(L, fname.c_str() );
}
void Lua::Load( Lua::Script script ){
    Load( script.Data, script.Title, 0 );
}

Lua::Lua( std::string block, std::string name, int flags ){
    Init();
    Load( block, name, flags );
}
Lua::Lua( std::vector<char> block, std::string name, int flags ){
    Init();
    Load( block, name, flags );
}
Lua::Lua( std::string fname ){
    Init();
    Load( fname );
}
Lua::Lua( Lua::Script script ){
    Init();
    Load( script );
}

Lua::Lua(){
    Init();
}

Lua::~Lua(){
    report_errors(L, errorlevel);
    lua_close(L);
}

std::vector<char> Lua::Dump() const{
    //Retrieve byte code from the Lua object
    WriteStruct args;
    lua_dump( L, Writer, (void*)&args );
    return args.buffer;
}

Lua::Function Lua::GetFunction( const char* name ) const{
    return Function(L, name);
}

void Lua::Run(){
    if (errorlevel == 0)
        errorlevel = lua_pcall(L, 0, LUA_MULTRET, 0);
}
