#include "Lua/Lua.hpp"

extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include<string>


Lua::Script::Script(){
    Title = "";
}

Lua::Script::~Script(){

}

void Lua::Script::LoadFile( std::string fname, std::string title ){
    Lua l;
    l.Load(fname);
    Title = title;
    Data = l.Dump();
}

void Lua::Script::LoadString( std::string script, std::string title ){
    Lua l;
    l.Load(script, title, 0);
    Title = title;
    Data = l.Dump();
}

void Lua::Script::SetTitle( std::string title ){
    Title = title;
}

std::string Lua::Script::GetTitle(){
    return Title;
}

