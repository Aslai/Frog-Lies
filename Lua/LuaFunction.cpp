#include "Lua/Lua.hpp"

extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include<string>


Lua::Function::Function( lua_State *Lua, const char* name ) :
    funcname(name),
    ref(-1),
    L(Lua)
{
    p.L = L;
}
Lua::Function::Function( lua_State *Lua, int reference ) :
    funcname(""),
    ref(reference),
    L(Lua)
{
    p.L = L;
}

Lua::Function::Function( ) :
    funcname(""),
    ref(-1),
    L(0){
    p.L = L;
}
Lua::Function& Lua::Function::operator=(Lua::Function f){
    funcname = f.funcname;
    ref = f.ref;
    L = f.L;
    p.L = L;
    return *this;
}
