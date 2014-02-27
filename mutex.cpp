#include "mutex.h"
#include <string>
#include <vector>

namespace FrogLies {

    Mutex::Mutex() {
        mutex = CreateMutex( NULL, FALSE, NULL );
    }

    Mutex::~Mutex() {
        CloseHandle( mutex );
    }

    void Mutex::Lock() {
        WaitForSingleObject( mutex, INFINITE );
    }

    void Mutex::Unlock() {
        ReleaseMutex( mutex );
    }

    void Mutex::Wait() {
        WaitForSingleObject( mutex, INFINITE );
        ReleaseMutex( mutex );
    }
}
