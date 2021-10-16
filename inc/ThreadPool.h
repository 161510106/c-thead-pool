#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

typedef struct _Runnable{
    void (*run)(void* data);
    void* data;
}Runnable;

typedef struct _ThreadPool ThreadPool;




void ThreadPool_shutdown(ThreadPool* pool);

void ThreadPool_submit(ThreadPool* pool, Runnable runnable);


ThreadPool* ThreadPool_create(int corePoolSize, int maximumPoolSize, long keepAliveTime);

void ThreadPool_destory(ThreadPool* pool);


#endif // _THREADPOOL_H_