
#include "TSMutex.h"

#include <QMutex>

struct TSMutexPrivate {
    TSMutexPrivate()
    {
    }

    QMutex _spin;
};

TSMutex::TSMutex()
    : _p(new TSMutexPrivate())
{
}

TSMutex::TSMutex(const TSMutex & /*r*/)
{
    _p = new TSMutexPrivate();
}

TSMutex &TSMutex::operator=(const TSMutex &r)
{
    if (this != &r) {
        delete _p;
        _p = new TSMutexPrivate();
    }

    return *this;
}

TSMutex::~TSMutex()
{
    delete _p;
}

bool TSMutex::TryAcquire()
{
    return _p->_spin.try_lock();
}

void TSMutex::Acquire()
{
    _p->_spin.lock();
}

void TSMutex::Release()
{
    _p->_spin.unlock();
}

TSMutex::Lock::Lock(TSMutex &lock)
    : _lock(lock)
{
    _lock.Acquire();
}

TSMutex::Lock::~Lock()
{
    _lock.Release();
}
