#ifndef __FROGLIES_WHFF_H__
#define __FROGLIES_WHFF_H__


#include "debug.h"

namespace FrogLies {
    class WHFF {
        static int HasInit;
        std::string Owner;
        std::string LastUpload;
        std::string LastPass;
        int laststatus;
        static size_t callback( char *ptr, size_t size, size_t nmemb, void *userdata );
    public:
        WHFF();
        WHFF( std::string owner );
        void SetOwner( std::string owner );
        int Upload( std::string name, const void* data, size_t datalen, std::string mimetype = "application/octet-stream", std::string password = "" );
        int Upload( std::string fname, std::string password = "" );
        std::string GetLastUpload();
        int GetStatus();
    };
}

#endif
