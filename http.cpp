#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include<stdarg.h>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include <string>
#include <cctype>
#include "http.h"

static void sendAll( SOCKET sock, const char* buff, size_t len, int flags ) {
    printf("%s", buff);
    size_t sent = 0;
    while( sent < len ) {
        sent += send( sock, buff + sent, len - sent, flags );
    }
}

static void senda( SOCKET sock, const char* str ) {
    sendAll( sock, str, strlen( str ), 0 );
}
void sendf(  SOCKET tosend, const char* format, ... ) {
    int a;
    va_list strt;
    va_start( strt, format );
    size_t len = vsnprintf( 0, 0, format, strt );
    char* sendBuf = ( char* ) malloc( len + 1 );
    va_start( strt, format );
    a = vsprintf( sendBuf, format, strt );
    va_end( strt );
    sendAll( tosend, sendBuf, a, 0 );
    free( sendBuf );
}

static unsigned long resolveAddr( const char* addr, int ind = -1 ) {
    hostent* host = gethostbyname( addr );
    if( host == 0 ) {
        printf( "Failed to lookup %s.", addr );
        return 0;
    }
    if( ind > host->h_length ) { return -1; }
    unsigned long     myhost = *( ( unsigned long* ) host->h_addr );
    return myhost;
}

static SOCKET connectTCP( unsigned long ip, short int port ) {
    SOCKET ret;
    ret = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if( ret < 0 ) { return 0; }
    sockaddr_in clientService;
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = ip;
    clientService.sin_port = htons( port );
    int result = connect( ret, ( sockaddr* ) &clientService, sizeof( clientService ) );
    if( result < 0 ) { return 0; }
    return ret;
}

enum {
    TYPE_HTTP = 0,
    TYPE_HTTPS,
    TYPE_INVALID
};

static struct {
    std::string name;
    int port, type;
} protocols[] = {
    { "http", 80, TYPE_HTTP },
    { "", -1, TYPE_INVALID }
};
enum {
    FIND_PROTOCOL = 0,
    READ_PROTOCOL,
    FIND_HOST,
    READ_HOST,
    READ_PORT,
    FIND_GET,
    READ_GET,
    DONE,
    FAIL
};

int HTTP::ParseUrl( int assumehttp ) {
    std::string PROTOCOL = "";
    std::string HOST = "";
    std::string GET = "";
    std::string PORT = "";
    unsigned int pos = 0;
    int status = FIND_PROTOCOL;
    for( unsigned int i = 0; i < url.size(); ++i ) {
        if( status == DONE || status == FAIL ) {
            break;
        }
        switch( status ) {

            case FIND_PROTOCOL:
                if( url[i] <= ' ' ) {
                    break;
                }
                //Intentional fall through


            case READ_PROTOCOL: {
                    if( assumehttp ) {
                        status = READ_HOST;
                        PROTOCOL = "http";
                        goto ASSUMING_HTTP;
                    }
                    if( url[i] == ':' ) {
                        PROTOCOL = url.substr( 0, i - pos );
                        pos = i + 1;
                        status = FIND_HOST;
                    }
                }
                break;


            case FIND_HOST: {
                    if( url[i] != '/' ) {
                        pos = i;
                        status = READ_HOST;
                    }
                }
                break;


            case READ_HOST: {
ASSUMING_HTTP:
                    if( url[i] == '/' ) {
                        HOST = url.substr( pos, i - pos );
                        status = FIND_GET;
                        GET = "/";
                    } else if( url[i] == ':' ) {
                        HOST = url.substr( pos, i - pos );
                        status = READ_PORT;
                        pos = i + 1;
                    } else if( i + 1 == url.size() ) {
                        HOST = url.substr( pos );
                        GET = "/";
                        status = DONE;
                    }

                }
                break;


            case READ_PORT: {
                    if( url[i] == '/' ) {
                        PORT = url.substr( pos, i - pos );
                        status = FIND_GET;
                        GET = "/";
                    } else if( i + 1 == url.size() ) {
                        PORT = url.substr( pos );
                        GET = "/";
                        status = DONE;
                    }
                }
                break;


            case FIND_GET: {
                    if( url[i] != '/' ) {
                        pos = i;
                        GET = "/" + url.substr( pos );
                        status = DONE;
                    } else if( i + 1 == url.size() ) {
                        GET = "/";
                        status = DONE;
                    }
                }
                break;


            default:
                status = FAIL;
                break;
        }
    }
    //printf("PROTOCOL: %s\nHOST: %s\nPORT: %s\nGET: %s\n\n", PROTOCOL.c_str(), HOST.c_str(), PORT.c_str(), GET.c_str() );
    if( PROTOCOL == "" || HOST == "" ) {
        if( !assumehttp ) {
            return ParseUrl( 1 );
        } else {
            return 0;
        }
    }
    for( unsigned int i = 0; i < PROTOCOL.size(); ++i ) {
        PROTOCOL[i] = tolower( PROTOCOL[i] );
    }
    proto = TYPE_INVALID;
    for( int i = 0; protocols[i].type != TYPE_INVALID; ++i ) {
        if( PROTOCOL == protocols[i].name ) {
            port = protocols[i].port;
            proto = protocols[i].type;
        }
    }
    if( proto == TYPE_INVALID ) {
        if( !assumehttp ) {
            return ParseUrl( 1 );
        } else {
            return 0;
        }
    }
    host = HOST;
    if( PORT != "" ) {
        if( sscanf( PORT.c_str(), "%d", &port ) != 1 ) {
            if( !assumehttp ) {
                return ParseUrl( 1 );
            } else {
                return 0;
            }
        }
    }
    path = GET;
    return 1;
}

int HTTP::hasinit = 0;
HTTP::HTTP( std::string URL ) {
    if( !hasinit ) {
        WSADATA globalWSAData;
        WSAStartup( MAKEWORD( 2, 2 ), &globalWSAData );
    }
    url = URL;
    if( ParseUrl() ) {
        printf( "Contacting |%s|\nUsing port %d\nAnd protocol %d\nAnd path |%s|\n\n", host.c_str(), port, proto, path.c_str() );
        ip = resolveAddr( host.c_str() );
    } else {
        printf( "Failed on |%s|\n\n", url.c_str() );
    }
    data = 0;

}

HTTP::~HTTP() {
    if( data ) {
        free( data );
    }
}

int HTTP::Get() {
    return Post( NULL, 0 );
}

int HTTP::processHeaders( char* buffer, size_t len ) {
    char* prev = buffer;
    unsigned int i;
    for( i = 0; i < len; ++i ) {
        if( buffer[i] == '\r' && i < len + 1 && buffer[i + 1] == '\n' ) {
            buffer[i] = 0;
            i++;
            printf("%s\n", prev);
            Headers.push_back( prev );
            prev = buffer + i + 1;
            if( i < len + 2 && buffer[i + 1] == '\r' && buffer[i + 2] == '\n' ) {
                Headers.push_back( "" );
                return i + 3;
            }
        }
    }
    return -1;
    //if( i - len)
}

int HTTP::Post( void* data, size_t datalen ) {
    SOCKET sock = connectTCP( ip, port );
    if( sock <= 0 ) {
        return -1;
    }
    if( data == 0 ) {
        senda( sock, "GET " );
    } else {
        senda( sock, "POST " );
    }
    sendf( sock, "%s HTTP/1.1\r\n", path.c_str() );
    sendf( sock, "Host: %s\r\n", host.c_str() );
    senda( sock, "Connection: close\r\n" );
    if( data != 0 && datalen == 0 && GetHeader( "Content-length" ) == "" ) {
        datalen = strlen( ( const char* )data );
    }
    if( data != 0 || datalen != 0 ) {
        sendf( sock, "Content-Length: %d\r\n", datalen );
    }

    for( unsigned int i = 0; i < Headers.size(); ++i ) {
        sendf( sock, "%s\r\n", Headers[i].c_str() );
    }
    senda( sock, "\r\n" );

    if( data != 0 ) {
        sendAll( sock, ( const char* )data, datalen, 0 );
    }

    Headers.clear();

    int bufsize = 100000000;
    char* buffer = ( char* ) malloc( bufsize );
    int bufpos = 0;

    while( true ) {
        int len = recv( sock, buffer + bufpos, bufsize - bufpos, 0 );
        bufpos += len;
        if( len <= 0 ) {
            break;
        }
        if( bufpos > bufsize - 1000 ) {
            bufsize *= 2;
            buffer = ( char* ) realloc( buffer, bufsize );
        }

    }
    for( int i = 0; i < bufpos; ++i ) {
        putchar( buffer[i] );
    }
    HTTP::dataoffset = processHeaders( buffer, bufpos );
    if( GetHeader("Transfer-Encoding") == "chunked"){
        printf("ayylmao");
        bool foundRet = false;
        size_t len = 0;
        size_t offset = HTTP::dataoffset;
        size_t end = HTTP::dataoffset;
        size_t total_len = 0;
        do{
            len = 0;
            size_t start = offset;
            for( ;offset < bufpos; ++offset ){
                if( buffer[offset] == '\r' ){
                    foundRet = true;
                }
                else if(foundRet && buffer[offset] == '\n'){
                    buffer[offset - 1] = 0;
                    len = atoi(buffer + start);
                    ++offset;
                    break;
                }
                else{
                    foundRet = false;
                }
            }
            if( len + offset >= bufpos ){
                if( offset >= bufpos ){
                    len = 0;
                }
                else{
                    len = bufpos - offset;
                }
            }
            memmove(buffer + end, buffer + offset, len);
            end += len;
            total_len += len;
            offset += len + 2;
        } while( len != 0 );
        bufpos = dataoffset + total_len;
        buffer[bufpos] = 0;
    }

    if( HTTP::data != 0 ) {
        free( HTTP::data );
    }
    if( HTTP::dataoffset >= 0 ) {
        HTTP::data = buffer;
        HTTP::length = bufpos;
    } else {
        HTTP::data = 0;
        HTTP::dataoffset = HTTP::length = 0;
        free( buffer );
    }
    int ret = 0;
    float garbage;
    if( sscanf( GetHeader( 0 ).c_str(), "HTTP/%f %d", &garbage, &ret ) < 2 ) {
        return -1;
    }
    return ret;
}

void HTTP::SetHeader( std::string value ) {
    Headers.push_back( value );
}

const char* HTTP::GetData( size_t& Length ) {
    if( data ) {
        Length = length - dataoffset;
        return ( ( const char* ) data ) + dataoffset;
    }
    Length = 0;
    return 0;
}

static std::string ToLower( std::string in ) {
    for( unsigned int i = 0; i < in.length(); ++i ) {
        in[i] = tolower( in[i] );
    }
    return in;
}

static std::string TrimLeadingWhitespace( std::string in ) {
    unsigned int i;
    for( i = 0; i < in.length(); ++i ) {
        if( ! isspace( in[i] ) ) {
            break;
        }
    }
    return in.substr( i );
}

std::string HTTP::GetHeader( std::string key ) {
    key = ToLower( key );
    for( unsigned int i = 0; i < Headers.size(); ++i ) {
        printf("%s\n", ToLower( Headers[i] ).substr( 0, key.length() ).c_str());
        if( ToLower( Headers[i] ).substr( 0, key.length() ) == key ) {
            printf("ret %s\n", TrimLeadingWhitespace( Headers[i].substr( key.length() + 1 ) ).c_str());
            return TrimLeadingWhitespace( Headers[i].substr( key.length() + 1 ) );
        }
    }
    return "";
}

std::string HTTP::GetHeader( int key ) {
    return Headers[key];
}
