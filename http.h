#ifndef __LIBHTTP_H__
#define __LIBHTTP_H__

#include <string>
#include <vector>

class HTTP{
    std::vector<std::string> Headers;
    int status;
    void* data;
    int dataoffset;
    int length;
    std::string url;
    int proto;
    unsigned long long ip;
    unsigned int port;
    std::string host;
    std::string path;
    int ParseUrl(int a = 0);
    static int hasinit;
    int processHeaders( char* buffer, size_t len );
public:
    HTTP(std::string URL);
    ~HTTP();
    int Get();
    int Post( void* data, size_t datalen = 0 );
    void SetHeader( std::string value );
    std::string GetHeader( std::string key );
    std::string GetHeader( int key );
    const char* GetData(size_t& length);
};

#endif
