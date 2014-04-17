#ifndef __FROGLIES_MIME_H__
#define __FROGLIES_MIME_H__


#include "debug.h"

namespace FrogLies {
    std::string GetMimeFromExt( std::string ext );
    void* read_file_to_buffer( std::string fname, size_t& length );
    std::string basename( std::string in );
    std::string extension( std::string in );
}

#endif
