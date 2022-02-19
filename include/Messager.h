#pragma once
#include "win32Obj.h"
#include <iostream>

struct msgMetaInf {
    LONG nThreadWaiting = 0, nWriter = 0, nReader = 0;// for spin lock
    size_t nSize = 0;
    void newThreadAwaits() {
        InterlockedIncrement(&this->nThreadWaiting);// this->nThreadWaiting++;
    }
    void threadDoneWaiting(win32Event& pNotifyEvent) {
        InterlockedDecrement(&this->nThreadWaiting);// this->nThreadWaiting--;
        if(ReadAcquire(&this->nThreadWaiting) == 0) {
            pNotifyEvent.reset();// reset if no more threads are waiting
        }
    }
    void resetCount() {
        this->nThreadWaiting = 0;
    }
    void writeLock() {
        while(InterlockedCompareExchange(&this->nWriter, 1, 0) != 0);// somebody is writing, so wait
        while(InterlockedCompareExchange(&this->nReader, 1, 0) != 0);// somebody is reading, so wait
    }
    void writeUnlock() {
        WriteRelease(&this->nWriter, 0);
    }
    void readLock() {
        while(InterlockedCompareExchange(&this->nWriter, 1, 0) != 0);// somebody is writing, so wait
        InterlockedIncrement(&this->nReader);
    }
    void readUnlock() {
        //while(InterlockedCompareExchange(&this->nWriter, 1, 0) != 0);// somebody is writing, so wait
        InterlockedDecrement(&this->nReader);
    }
};

class msgChannel {
public:
    //constructer
    msgChannel(LPCWSTR name) {
        WCHAR buffer[MAX_PATH]{ 0 };
        auto len = wcsnlen(name, MAX_PATH - 3);
        wcscpy_s(buffer, name);
        wcscpy_s(buffer + len, MAX_PATH - len, L"_SM");
        this->msgMem.reg(buffer);
        wcscpy_s(buffer + len, MAX_PATH - len, L"_BM");
        this->bufferMutex.reg(buffer);
        wcscpy_s(buffer + len, MAX_PATH - len, L"_ME");
        this->msgEvent.reg(buffer);
        wcscpy_s(buffer + len, MAX_PATH - len, L"_VE");
        this->validityEvent.reg(buffer);
    }
    bool create(DWORD size) {
        if(this->bGood)
            return false;
        // note: manual reset is necessary
        this->bGood = this->msgEvent.create(TRUE) && this->validityEvent.create(TRUE) && this->bufferMutex.create() && this->msgMem.create(size);
        if(this->bGood) {
            // set base pointers
            this->pBase = this->msgMem.map();
            this->inf = (msgMetaInf*)this->pBase;
            this->inf->resetCount();
            this->pTrueBase = (char*)this->pBase + sizeof(msgMetaInf);
            // set sizes
            this->nSMSize = this->msgMem.getSize();
            this->nBufSize = this->nSMSize - sizeof(msgMetaInf);
        }
        this->bOwner = true;
        return this->bGood;
    }
    bool open() {
        if(this->bGood)
            return false;
        this->bGood = this->msgEvent.open() && this->validityEvent.open() && this->bufferMutex.open() && this->msgMem.open();
        if(this->bGood) {
            // set base pointers
            this->pBase = this->msgMem.map();
            this->inf = (msgMetaInf*)this->pBase;
            this->pTrueBase = (char*)this->pBase + sizeof(msgMetaInf);
            // set sizes
            this->nSMSize = this->msgMem.getSize();
            this->nBufSize = this->nSMSize - sizeof(msgMetaInf);
        }
        return this->bGood;
    }
    void send(const void* buffer, size_t size) {
        if(!this->bGood) {
            throw win32ObjExInvalidObject();
        }
        this->bufferMutex.lock();// start writing to buffer
        this->inf->nSize = size;
        memcpy(this->pTrueBase, buffer, size);
        this->msgEvent.set();// a new message awaits
        this->bufferMutex.unlock();// end writing to buffer

    }
    size_t recv(void* buffer, size_t size) {
        if(!this->bGood) {
            throw win32ObjExInvalidObject();
        }
        this->inf->newThreadAwaits();// increase thread counter
        auto waitResult = waitFor(INFINITE, this->msgEvent, this->validityEvent);
        size_t recv_size = min(min(this->nBufSize, size), this->inf->nSize);
        switch(waitResult) {
        case win32WaitResult::WaitObject_0:
            // msg recved
            memcpy(buffer, this->pTrueBase, recv_size);
            this->inf->threadDoneWaiting(this->msgEvent);
            break;
        case win32WaitResult::WaitObject_1:
            return 0;
            break;
        default:
            throw win32ObjExWaitError(waitResult);
            break;
        }
        return recv_size;
    }
    template<typename T, size_t size>
    size_t recv(T(&buffer)[size]) {
        return this->recv(buffer, size * sizeof(T));
    }
    void close() {
        if(this->bOwner && this->bGood)
            this->validityEvent.set();// signal that owner thread is exiting
    }
    //destructer
    ~msgChannel() {
        this->close();
    }

private:
    win32Mutex bufferMutex;
    win32Event msgEvent, validityEvent;
    win32SharedMemory msgMem;
    DWORD64 nSMSize = 0, nBufSize = 0;
    void* pBase = nullptr, * pTrueBase = nullptr;
    msgMetaInf* inf = nullptr;
    bool bGood = false, bOwner = false;// indicate whether current thread is owner
};

