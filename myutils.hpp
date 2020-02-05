#pragma once

#include <unistd.h>


class SyncEvent {
private:
    bool signaled = false;
public:;
    void Set();
    void Wait();
    void Clear();
};

void SyncEvent::Set() {
    signaled = true;
}

void SyncEvent::Wait() {
    while (!signaled) {
        usleep(100e3);
    }
}

void SyncEvent::Clear() {
    signaled = false;
}