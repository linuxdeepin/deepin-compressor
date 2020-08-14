#ifndef TSMUTEX_H
#define TSMUTEX_H

struct TSMutexPrivate;

class TSMutex
{
public:
    TSMutex();
    TSMutex(const TSMutex &r);
    ~TSMutex();

    TSMutex &operator=(const TSMutex &r);

public:
    class Lock
    {
    public:
        Lock(TSMutex &lock);
        ~Lock();

    private:
        TSMutex &_lock;
    };

public:
    bool TryAcquire();
    void Acquire();
    void Release();

private:
    TSMutexPrivate *_p;
};

#endif // TSMutex_H
