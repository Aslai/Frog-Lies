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
#include <curl/curl.h>
#include "http.h"


#include "files.h"
#include "whff.h"
#include "bitmap.h"

namespace FrogLies{
    void* FillTemplate( unsigned int& bpos, const char** Template, ... ) {
        va_list args;
        va_start( args, Template );
        unsigned int bsize = 1000;
        char* buffer = (char*) malloc( bsize );
        bpos = 0;

        for( int i = 0; Template[i] != 0; i += 2 ) {
            switch( Template[i][0] ) {
            case 'R':
            case 'r': {
                unsigned int rawlen = strlen( Template[i+1] );
                while( bpos + rawlen >= bsize ) {
                    bsize *= 2;
                    buffer = (char*) realloc( buffer, bsize );
                }
                bpos += sprintf( buffer + bpos, "%s", Template[i+1] );
            }
            break;
            case 'S':
            case 's': {
                char* arg = va_arg( args, char* );
                unsigned int rawlen = strlen( arg );
                while( bpos + rawlen >= bsize ) {
                    bsize *= 2;
                    buffer = (char*) realloc( buffer, bsize );
                }
                bpos += sprintf( buffer + bpos, "%s", arg );
            }
            break;
            case 'D':
            case 'd': {
                void* arg = va_arg( args, void* );
                unsigned int rawlen = va_arg( args, unsigned int );
                while( bpos + rawlen >= bsize ) {
                    bsize *= 2;
                    buffer = (char*) realloc( buffer, bsize );
                }
                memcpy( buffer + bpos, arg, rawlen );
                bpos += rawlen;
            }
            break;
            }
        }
        return buffer;
    }

    int WHFF::HasInit = 0;

    WHFF::WHFF(){
        WHFF("");
    }

    WHFF::WHFF( std::string owner ) {
        if( !HasInit ) {
            HasInit = 1;
            #ifdef USE_CURL
            curl_global_init(CURL_GLOBAL_WIN32);
            #endif
        }
        SetOwner( owner );
    }

    void WHFF::SetOwner( std::string owner ) {
        Owner = owner;
    }

    size_t WHFF::callback( char *ptr, size_t size, size_t nmemb, void *userdata ) {
        printf("HAH %d|%s|", size * nmemb, ptr);
        char* tmp = (char*) malloc( size*nmemb + 1 );
        memcpy( tmp, ptr, size*nmemb );
        tmp[size*nmemb] = 0;
        std::string value = tmp;
        if( value.substr(0, 5) != "Error" )
            value = "http://fiel.tk/?i=" + value;
        WHFF* self = (WHFF*) userdata;
        self->LastUpload = value;
        return size * nmemb;
    }

    std::string WHFF::GetLastUpload() {
        return LastUpload;
    }

    int WHFF::Upload( std::string name, const void* data, size_t datalen, std::string mimetype, std::string password ) {
        for( unsigned int i = 0; i < name.length(); ++i ) {
            if( name[i] == '\"' || name[i] < ' ' )
                name[i] = ' ';
        }

        const char * posttemplate[] = {
            "raw",  "-----------------------------28251299466151\r\n",
            "raw",  "Content-Disposition: form-data; name=\"file\"; filename=\"",
            "str",  "",
            "raw",  "\"\r\n",
            "raw",  "Content-Type: ",
            "str",  "",
            "raw",  "\r\n\r\n",
            "dat",  "",
            "raw",  "\r\n-----------------------------28251299466151\r\n",
            "raw",  "Content-Disposition: form-data; name=\"password\"\r\n\r\n",
            "dat",  "",
            "raw",  "\r\n-----------------------------28251299466151\r\n",
            "raw",  "Content-Disposition: form-data; name=\"owner\"\r\n\r\n",
            "dat",  "",
            "raw",  "\r\n-----------------------------28251299466151--\r\n",
            0, 0
        };
        unsigned int length;
        void *postobject = FillTemplate(   length, posttemplate,
                                           name.c_str(),
                                           mimetype.c_str(),
                                           data, datalen,
                                           password.c_str(), password.length(),
                                           Owner.c_str(), Owner.length() );

        printf("%s\n", (char*) postobject );

        #ifdef USE_CURL
        CURL *curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        const char *theUrl = "http://frogbox.es/whff/upload.php?raw";
        curl_easy_setopt(curl, CURLOPT_URL, theUrl );
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postobject);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, length);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WHFF::callback );
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this );



        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: multipart/form-data; boundary=---------------------------28251299466151" );
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);


        curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        #else
        HTTP http("http://frogbox.es/whff/upload.php?raw");
        http.SetHeader( "Content-Type: multipart/form-data; boundary=---------------------------28251299466151" );

        http.Post( postobject, length );
        size_t length2;
        char* c = (char*)http.GetData(length2);

        callback( c, length2, 1, this );
        #endif

        free( postobject );
        return 1;
    }

    int WHFF::Upload( std::string fname, std::string password ) {
        void* buffer;
        size_t len;
        buffer = read_file_to_buffer( fname, len );
        Upload( basename( fname ), buffer, len, GetMimeFromExt( extension( fname ) ), password );
        return 1;
    }
}

