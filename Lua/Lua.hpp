#ifndef KLUA_WRAPPER_HPP
#define KLUA_WRAPPER_HPP

extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include<string>
#include <vector>
#include <map>

#define RegisterFunction(a,b) funcreg<decltype(&a),a>(b,a)

class Lua{
    lua_State* L;
    int errorlevel;
    public:
    class Table;
    class Function;
    class Value;
    private:
    struct Stack{
        friend Value;
        friend Lua;
        friend Function;
        lua_State *L;int position;
        Stack();
        int trythrow(int idx);
        int lua_push( std::string s );
        int lua_push( const char* s );
        int lua_push( int s );
        int lua_push( unsigned int s );
        int lua_push( double s );
        int lua_push( void );
        int lua_push( void* s );
        int lua_push( Value s );

        std::string lua_ret( std::string, int pos = -100000 );
        const char* lua_ret( const char*, int pos = -100000 );
        int lua_ret( int , int pos = -100000 );
        double lua_ret( double, int pos = -100000 );
        void* lua_ret( void*, int pos = -100000 );
        Function lua_ret( Function, int pos = -100000 );
        Table lua_ret( Table, int pos = -100000 );
        Value lua_ret( Value, int pos = -100000 );
    };
    friend Stack;

    template <class RetType, class...args>
    struct LuaWrap{
        template<RetType (*fun)(args...)>
        static int luawrap(struct lua_State *Lua ){
            Stack p;
            p.L=Lua;
            p.lua_push( fun(  p.lua_ret( args() )... ) );
            return 1;
        }
    };

    static void report_errors(lua_State *L, int status);
    static void *l_alloc (void *ud, void *ptr, size_t osize, size_t nsize);
    static int panic( lua_State *l );
    struct WriteStruct{
        std::vector<char> buffer;
    };
    static int Writer (lua_State *L, const void* p, size_t sz, void* ud);
    static int Reader( lua_State *L, void *data, size_t *size);

    void Init();
    static bool GetValueToStack( lua_State *L, std::string globalid, int reference = -1 );

public:
    class Function{
        public:
        friend Stack;
        std::string funcname;
        Stack p;
        int ref;
        lua_State *L;
    public:
        Function( lua_State *Lua, const char* name );
        Function( lua_State *Lua, int reference );
        Function( );
        Function& operator=(Function f);

        template<typename... Args> inline void pass(Args...)  const {}

        template<class ReturnType, class... Arguments>
        ReturnType Call(Arguments... a){
            printf("\n%d\n", ref );
            p.position = 1; //Reset the position for calling another function
            if( GetValueToStack( L, funcname, ref ) ){
                if (!lua_isfunction(L, -1)){
                    return ReturnType();
                }
                pass(p.lua_push(a)...);

                 lua_pcall(L, sizeof...(Arguments), 1, 0);
                ReturnType ret = p.lua_ret(ReturnType(), -1);

                lua_pop(L,lua_gettop(L));
                return ret;
            }
            return ReturnType();
        }
        template<class... Arguments>
        void Call(Arguments... a){
            p.position = 1; //Reset the position for calling another function
            if( GetValueToStack( L, funcname, ref ) ){
                if (!lua_isfunction(L, -1)){
                    return;
                }
                pass(p.lua_push(a)...);

                lua_pcall(L, sizeof...(Arguments), 1, 0);

                lua_pop(L,lua_gettop(L));
            }
            return;
        }
    };
    class Value{
        friend Lua;
        friend Stack;
        public:
        std::vector<Value> TableIndex;
        std::map<std::string, Value> TableKeys;
        private:
        std::string StringValue;
        lua_CFunction FunctionValue;
        Function LuaFunc;
        double NumberValue;
        enum class Type{
            Table,
            String,
            Number,
            Function,
            Nil
        }myType;
        friend Table;
    public:
        Value();
        Value(double value);
        Value(std::string value);
        Value(const Value& value);
        Value(Table value);
        Value(Function value);
        bool IsNumber();
        bool IsString();
        bool IsTable();
        bool IsFunction();
        bool IsNil();
        double& GetNumber();
        std::string& GetString();
        Function GetFunction();
        Value& GetTable(std::string key);
        Value& GetTable(unsigned int index);
        Value& operator[](std::string key);
        Value& operator[](unsigned int index);
        Value& operator=(double value);
        Value& operator=(std::string value);
        Value& operator=(const Value value);
        Value& operator=(Table value);
        Value& operator=(Function value);
        operator std::string();
    };
    friend Value;

    class Table{
        std::string globalname;
        Stack p;
        int ref;
        lua_State *L;
        void FillValue( Value& ToFill );
        friend Value;
    public:
        Table( lua_State *Lua, const char* name );
        Table( lua_State *Lua, int reference );
        Table( );
        Table& operator=(Table f);
        size_t Length();

        template<typename... Args> inline void pass(Args...) {}

        template<class ReturnType, class T>
        ReturnType Get(T key){
            if( GetValueToStack( L, globalname, ref ) ){
                if (!lua_istable(L, -1)){
                    return ReturnType();
                }
                //Push our desired key to the stack
                p.lua_push( key );
                lua_gettable(L, -2);
                ReturnType ret = p.lua_ret( ReturnType(), -1 );

                lua_pop(L,lua_gettop(L));
                return ret;
            }
            return ReturnType();
        }

        template<class T, class V>
        bool Set(T key, V value){
            if( GetValueToStack( L, globalname, ref ) ){
                if (!lua_istable(L, -1)){
                    return false;
                }
                //Push our desired key to the stack
                p.lua_push( key );
                p.lua_push( value );
                lua_settable(L, -3);
                return true;
            }
            return false;
        }


    };

    /*class Type{
        union T{
            std::string str,
            Function func,
            Table
        };
    };*/



    class Script{
        friend Lua;
        std::vector<char> Data;
        std::string Title;
    public:
        Script();
        ~Script();
        void LoadFile( std::string fname, std::string title = "" );
        void LoadString( std::string script, std::string title = "" );
        void SetTitle( std::string title );
        std::string GetTitle();
    };
    friend Script;

    template<class T, T f, class ReturnType, class... arg0>
    void funcreg(const char* name, ReturnType (*function)(arg0...)){
        lua_CFunction func = Lua::LuaWrap<ReturnType, arg0...>::template luawrap<f>;
        lua_register(L, name, func);
    }
    template<class T, T f, class ReturnType, class... arg0>
    void funcreg(Value& luaval, ReturnType (*function)(arg0...)){
        lua_CFunction func = Lua::LuaWrap<ReturnType, arg0...>::template luawrap<f>;
        luaval.FunctionValue = func;
        luaval.myType = Value::Type::Function;
    }



    void Load(std::string block, std::string name, int flags);
    void Load(std::vector<char> block, std::string name, int flags);
    void Load( std::string name );
    void Load( Script script );
    Lua( std::string block, std::string name, int flags );
    Lua( std::string fname );
    Lua( std::vector<char> block, std::string name, int flags );
    Lua( Script script );
    Lua();
    ~Lua();
    std::vector<char> Dump() const;
    Function GetFunction( const char* name ) const;
    void Run();

    template <class type>
    void Set( std::string name, type value ){
        Stack p;
        p.L=L;
        p.lua_push( value );
        lua_setglobal (L, name.c_str() );
    }

    template <class type>
    type Get( std::string name ) const{
        Stack p;
        p.L=L;
        lua_getglobal (L, name.c_str() );
        return (type) p.lua_ret( type(), -1 );
    }
};

#endif
