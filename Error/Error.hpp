#ifdef ERROR
#undef ERROR
#endif
#ifndef MUD_ERROR_ERROR_HPP
#define MUD_ERROR_ERROR_HPP

#include <string>
#include <map>

#define FAIL(msg) {GlobalMUD::ERROR::Fail( __LINE__, __FILE__, __func__, msg );}
#define TEST(msg) {GlobalMUD::ERROR::Test(msg);}
#define ASSERT(expr) {if(!(expr)){GlobalMUD::ERROR::Fail( __LINE__, __FILE__, __func__, "Assertion Failure" ); return false;};}
#define TRACE printf("TRACE: %d %s\n", __LINE__, __FILE__ )

namespace GlobalMUD{
    //typedef const unsigned int Error;
    enum class Error {
        None = 0,
        OutOfMemory,
        ParseFailure,
        CryptoFailure,
        ConnectionFailure,
        NegotiationFailure,
        InvalidScheme,
        InvalidHost,
        ConnectionRefused,
        ImportantOperation,
        BindFailure,
        PartialMessage,
        NotConnected,
        ListenFailure,
        NoData,
        SocketFailure,
        FileNotFound,
        MountFailure,
        MountExists,
        Timeout,
        EndOfLine,
        EndOfFile,
        OutOfBounds,
        InvalidSize,
        PartialData,
        Unsupported,
        LuaError,

        NotAnError
    };

    namespace ERROR{
        extern std::string ErrorStrings[];
        std::string ToString(Error e);
        void Fail( int line, const char *file, const char *func, const char *msg );
        void Test( const char *msg );

    }
}

#endif // MUD__GLOBAL_ERROR_HPP
