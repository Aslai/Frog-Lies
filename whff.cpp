#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <tchar.h>
#include <string>
#include <map>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <Windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <Shlwapi.h>
#include <process.h>
#include "png.h"
#include "http.h"


#include "files.h"
#include "whff.h"
#include "bitmap.h"

namespace FrogLies {
    std::vector<uint8_t> FillTemplate( const char* boundary, const char* Template, ... ) {
        va_list args;
        va_start( args, Template );
        std::vector<uint8_t> buffer;
        bool escaped = false;
        for( int i = 0; Template[i] != 0; i += 1 ) {
            if( Template[i] == '%' ){
                escaped = true;
                continue;
            }
            if( !escaped ){
                buffer.push_back(Template[i]);
            }
            else{
                switch(Template[i]){
                case 'B': {
                    buffer.push_back('-');
                    buffer.push_back('-');

                    for( char* c = (char*)boundary; *c; ++c){
                            buffer.push_back((uint8_t)*c);
                    }
                } break;
                case 's':{
                    char* arg = va_arg( args, char* );
                    for(; *arg; ++arg){
                            buffer.push_back((uint8_t)*arg);
                    }
                } break;
                case 'v':{
                    uint8_t* arg = va_arg( args, uint8_t* );
                    unsigned int rawlen = va_arg( args, unsigned int );
                    for(size_t p = 0; p < rawlen; ++p){
                            buffer.push_back(arg[p]);
                    }
                } break;
                case '%':
                    buffer.push_back((uint8_t)'%');
                    break;
                }
            }
            escaped = false;
        }

        return buffer;
    }

    int WHFF::HasInit = 0;

    WHFF::WHFF() {
        WHFF( "" );
    }

    WHFF::WHFF( std::string owner ) {
        if( !HasInit ) {
            HasInit = 1;
#ifdef USE_CURL
            curl_global_init( CURL_GLOBAL_WIN32 );
#endif
        }
        SetOwner( owner );
        laststatus = 0;
    }

    void WHFF::SetOwner( std::string owner ) {
        Owner = owner;
    }

    size_t WHFF::callback( char *ptr, size_t size, size_t nmemb, void *userdata ) {
        //printf( "HAH %d|%s|", size * nmemb, ptr );
        char* tmp = ( char* ) malloc( size * nmemb + 1 );
        memcpy( tmp, ptr, size * nmemb );
        tmp[size * nmemb] = 0;
        std::string value = tmp;
        free( tmp );
        if( value.substr( 0, 5 ) != "Error" ) {
            value = "http://i.frogbox.es/" + value;
        }
        WHFF* self = ( WHFF* ) userdata;
        self->LastUpload = value;
        return size * nmemb;
    }

    std::string WHFF::GetLastUpload() {
        return LastUpload;
    }

    int WHFF::Upload( std::string name, const void* data, size_t datalen, std::string mimetype, std::string password ) {
        for( unsigned int i = 0; i < name.length(); ++i ) {
            if( name[i] == '\"' || name[i] < ' ' ) {
                name[i] = ' ';
            }
        }

        const char *posttemplate =
            "%B\r\n"
            "Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"
            "Content-Type: %s\r\n"
            "\r\n"
            "%v\r\n"
            "%B\r\n"
            "Content-Disposition: form-data; name=\"password\"\r\n"
            "\r\n"
            "%v\r\n"
            "%B\r\n"
            "Content-Disposition: form-data; name=\"owner\"\r\n"
            "\r\n"
            "%v\r\n"
            "%B--\r\n";
        srand(time(0));
        char delimiter[100] = "-----";
        for( size_t i = 5; i < 99; ++i ){
            delimiter[i] = '0' + (rand() % 10);
        }
        delimiter[99] = 0;

        auto postobject = FillTemplate(    delimiter, posttemplate,
                                           name.c_str(),
                                           mimetype.c_str(),
                                           data, datalen,
                                           password.c_str(), password.length(),
                                           Owner.c_str(), Owner.length());

#ifdef USE_CURL
        CURL *curl = curl_easy_init();
        curl_easy_setopt( curl, CURLOPT_VERBOSE, 1 );
        const char *theUrl = "http://frogbox.es/whff/upload.php?raw";
        curl_easy_setopt( curl, CURLOPT_URL, theUrl );
        curl_easy_setopt( curl, CURLOPT_POSTFIELDS, postobject );
        curl_easy_setopt( curl, CURLOPT_POSTFIELDSIZE, length );
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WHFF::callback );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, this );



        struct curl_slist *headers = NULL;
        headers = curl_slist_append( headers, "Content-Type: multipart/form-data; boundary=---------------------------28251299466151" );
        curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers );


        curl_easy_perform( curl );
        curl_slist_free_all( headers );
        curl_easy_cleanup( curl );
#else
        HTTP http( "http://frogbox.es/whff/upload.php?raw" );
        http.SetHeader( std::string("Content-Type: multipart/form-data; boundary=") + delimiter );

        laststatus = http.Post( &postobject[0], postobject.size() );
        size_t length2;
        char* c = ( char* )http.GetData( length2 );

        callback( c, length2, 1, this );
#endif

        return 1;
    }

    int WHFF::Upload( std::string fname, std::string password ) {
        void* buffer;
        size_t len;
        buffer = read_file_to_buffer( fname, len );
        Upload( basename( fname ), buffer, len, GetMimeFromExt( extension( fname ) ), password );
        free( buffer );
        return 1;
    }

    int WHFF::GetStatus() {
        return laststatus;
    }
}

