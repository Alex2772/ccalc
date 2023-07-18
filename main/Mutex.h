#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
class Mutex {
private:
    SemaphoreHandle_t _m;
public:
    class L {
    private:
        const Mutex &_p;
    public:
        L(const Mutex& m);
        ~L();

    };
    Mutex();
    ~Mutex();
    void unlock();
    void lock();

    bool tryLock(long delay);
};