//
// Created by alex2772 on 08.12.17.
//

#include "Mutex.h"

Mutex::Mutex() {
    _m = xSemaphoreCreateRecursiveMutex();
}

Mutex::~Mutex() {
    vSemaphoreDelete(_m);
}

void Mutex::lock() {
    xSemaphoreTake(_m, portMAX_DELAY);
}
bool Mutex::tryLock(long delay) {
    return xSemaphoreTake(_m, delay / portTICK_PERIOD_MS) == pdPASS;
}
void Mutex::unlock() {
    xSemaphoreGive(_m);
}

Mutex::L::L(const Mutex& m):
    _p(m)
{
    xSemaphoreTake(_p._m, portMAX_DELAY);
}
Mutex::L::~L()
{
    xSemaphoreGive(_p._m);
}