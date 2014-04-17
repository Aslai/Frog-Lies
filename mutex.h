#ifndef __FROGLIES_MUTEX_H__
#define __FROGLIES_MUTEX_H__
#include <windows.h>

namespace FrogLies {
    class Mutex {
        HANDLE mutex;

    public:
        Mutex();
        ~Mutex();
        void Lock();
        void Unlock();
        void Wait();
    };
}

#endif
