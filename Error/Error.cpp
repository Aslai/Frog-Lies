#include "Error/Error.hpp"
#include <cstdio>
namespace GlobalMUD{
    namespace ERROR{
        void Fail( int line, const char *file, const char *func, const char *msg ){
            printf( "Failure at\nLine: %d\nFile: %s\nFunc: %s\nMsg:  %s\n", line, file, func, msg );
        }
        void Test( const char *msg ){
            printf( "Testing %s...\n", msg );
        }
        std::string ToString(Error e){
            switch( e ){
                case Error::BindFailure:        return "Failed to bind to specified port (Error::BindFailure)";
                case Error::ConnectionFailure:  return "Unspecified connection failure (Error::ConnectionFailure)";
                case Error::ConnectionRefused:  return "Connection was refused (Error::ConnectionRefused)";
                case Error::CryptoFailure:      return "Unspecified crypto failure (Error::CryptoFailure)";
                case Error::EndOfFile:          return "End of file (Error::EndOfFile)";
                case Error::EndOfLine:          return "End of line (Error::EndOfLine)";
                case Error::FileNotFound:       return "File not found (Error::FileNotFound)";
                case Error::ImportantOperation: return "Important operation in progress (Error::ImportantOperation)";
                case Error::InvalidHost:        return "Unable to lookup the specified host (Error::InvalidHost)";
                case Error::InvalidScheme:      return "Invalid crypto scheme specified (Error::InvalidScheme)";
                case Error::InvalidSize:        return "Invalid size specified (Error::InvalidSize)";
                case Error::ListenFailure:      return "Failed to listen (Error::ListenFailure)";
                case Error::MountExists:        return "Mount already exists (Error::MountExists)";
                case Error::MountFailure:       return "Failed to mount (Error::MountFailure)";
                case Error::NegotiationFailure: return "Failed to negotiate (Error::NegotiationFailure)";
                case Error::NoData:             return "No data (Error::NoData)";
                case Error::None:               return "No error (Error::None)";
                case Error::NotAnError:         return "Not an Error (Error::NotAnError)";
                case Error::NotConnected:       return "No active connection (Error::NotConnected)";
                case Error::OutOfBounds:        return "Out of bounds (Error::OutOfBounds)";
                case Error::OutOfMemory:        return "Out of memory (Error::OutOfMemory)";
                case Error::ParseFailure:       return "Parse failure (Error::ParseFailure)";
                case Error::PartialData:        return "Data stream ended unexpectedly (Error::PartialData)";
                case Error::PartialMessage:     return "Partial message received (Error::PartialMessage)";
                case Error::SocketFailure:      return "Unspecified socket failure (Error::SocketFailure)";
                case Error::Timeout:            return "Timeout period exceeded (Error::Timeout)";
                case Error::Unsupported:        return "Unsupported operation (Error::Unsupported)";
                case Error::LuaError:           return "Unspecified Lua runtime error. (Error::LuaError)";
                default:    break;
            }
            return "Unknown Error";
        }
    }
}
