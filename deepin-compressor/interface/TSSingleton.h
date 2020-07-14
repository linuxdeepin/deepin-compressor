#ifndef TSSINGLETON_H
#define TSSINGLETON_H

#include "tstypes.h"

#include <stdlib.h>
#include <assert.h>

#define SINGLETON_DECLARE(cls)\
    friend class TSSingleton<cls>;\
    private:\
    static TSMutex & __GetSingletonObjectLock();\
    static volatile cls * & __GetSingletonObjectInstance();\
    static int & __GetSingletonObjectIsDestroyed()

template<class TYPE>
class TSSingleton
{
protected:
    TSSingleton()
    {

    }

    virtual ~TSSingleton()
    {
        TYPE::__GetSingletonObjectInstance() = nullptr;
        TYPE::__GetSingletonObjectIsDestroyed() = 1;
    }

public:
    static TYPE *Instance()
    {
        if (!TYPE::__GetSingletonObjectInstance()) {
            TSMutex::Lock lock(TYPE::__GetSingletonObjectLock());
            if (!TYPE::__GetSingletonObjectInstance()) {
                if (TYPE::__GetSingletonObjectIsDestroyed() == 0) {
                    __CreateSingletonObject();
                } else if (TYPE::__GetSingletonObjectIsDestroyed() == 1) {
                    __CreateSingletonDiedRefrenceObject();
                } else if (TYPE::__GetSingletonObjectIsDestroyed() == 2) {
                    __CreateSingletonObject();
                    new ((void *)(TYPE::__GetSingletonObjectInstance())) TYPE;
                } else {
                    assert(0);
                }
            }
        }

        return (TYPE *)(TYPE::__GetSingletonObjectInstance());
    }
    static void ForceDestroySingletonObject()
    {
        if (TYPE::__GetSingletonObjectInstance()) {
            TSMutex::Lock lock(TYPE::__GetSingletonObjectLock());
            if (TYPE::__GetSingletonObjectInstance()) {
                __DestroySingletonObject();
                TYPE::__GetSingletonObjectIsDestroyed() = 2;
            }
        }
    }
    static bool SingletonObjectIsDestroyed()
    {
        return TYPE::__GetSingletonObjectIsDestroyed() == 1;
    }
    static bool InitializeStaticObjects()
    {
        TYPE::__GetSingletonObjectLock();
        TYPE::__GetSingletonObjectInstance();

        return TYPE::__GetSingletonObjectIsDestroyed();
    }
private:
    static void __CreateSingletonObject()
    {
        static TYPE Obj;
        TYPE::__GetSingletonObjectInstance() = &Obj;
    }
    static void __DestroySingletonObject()
    {
        if (TYPE::__GetSingletonObjectInstance()) {
            TYPE::__GetSingletonObjectInstance()->~TYPE();
        }

    }
    static void __CreateSingletonDiedRefrenceObject()
    {
        __CreateSingletonObject();
        new ((void *)(TYPE::__GetSingletonObjectInstance())) TYPE;
        atexit(__DestroySingletonObject);
    }
};

#define SINGLETON_IMPLEMENT(cls)    TSMutex & cls::__GetSingletonObjectLock()\
    {\
        static TSMutex __SingletonObjectLock;\
        return __SingletonObjectLock;\
    }\
    volatile cls * & cls::__GetSingletonObjectInstance()\
    {\
        static volatile cls * __SingletonObjectInstance = nullptr;\
        return __SingletonObjectInstance;\
    }\
    int & cls::__GetSingletonObjectIsDestroyed()\
    {\
        static int __SingletonObjectIsDestroyed = 0;\
        return __SingletonObjectIsDestroyed;\
    }\
    bool __sInitialize##cls = cls::InitializeStaticObjects();


#endif
