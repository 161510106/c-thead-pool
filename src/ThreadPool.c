#include "ThreadPool.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>


enum stat_code {
    TP_OK                   = 0,
};

typedef struct _ThreadNode{
    int id;
    pthread_t thread;
    int isThreadEnd;
    long lastTaskFinishTime;
    ThreadPool* pool;
}ThreadNode;


typedef struct _ThreadPool{
    int mCorePoolSize;
    int mMaximumPoolSize;
    long mKeepAliveTime;
    ListQueue* mJobs;
    ThreadNode* threads;

    int workingPoolSize;
    int alivePoolSize;

    pthread_mutex_t thoopSizeMutex;
};

typedef struct _QueueItem{
    void* data;
    void* next;
}QueueItem;

typedef struct _ListQueue{
    pthread_mutex_t rwmutex;
    QueueItem* front;
    QueueItem* rear;
    int len;
    sem_t count;
}ListQueue;

QueueItem* QueueItem_create(void *element) {
    QueueItem* item = (QueueItem*)malloc(sizeof(QueueItem));
    if (item == NULL) {
        return NULL;
    }
    memset(item, 0, sizeof(QueueItem));
    item->data = element;
    return item;
}

void QueueItem_release(QueueItem *item, void (*cb) (void*)) {
    if (cb != NULL) {
        cb(item->data);
    }
    free(item);
}

ListQueue* ListQueue_create() {
    ListQueue* queue = (ListQueue*) malloc(sizeof(ListQueue));
    memset(queue, sizeof(ListQueue));
    pthread_mutex_init(&(queue->rwmutex), NULL);
    sem_init(&queue->count, 0, 0);
    return queue;
}

void* ListQueue_poll(ListQueue *queue) {
    pthread_mutex_lock(&queue->rwmutex);
    QueueItem* item = NULL;
    void *element = NULL;
    if (queue->len == 0) {
        goto end;
    } else if (queue->len == 1) {
        item = queue->front;
        queue->front = NULL;
        queue->rear = NULL;
        queue->len = 0;
    } else {
        item = queue->front;
        queue->front = queue->front->next;
        queue->len--;
    }
    if (item != NULL) {
        void *element = item->data;
        QueueItem_release(item, NULL);
    }
    end:
    pthread_mutex_unlock(&queue->rwmutex);
    return element;
}

enum stat_code ListQueue_enqueue(ListQueue *queue, void *element) {
    pthread_mutex_lock(&queue->rwmutex);
    QueueItem* item = QueueItem_create(element);
    if (queue->len == 0) {
        queue->rear = item;
        queue->front = item;
    } else {
        queue->rear->next = item;
        queue->rear = item;
    }
    queue->len++;
    sem_post(&queue->count);
    pthread_mutex_unlock(&queue->rwmutex);
    return TP_OK;

}

void ListQueue_destroy(ListQueue *queue, void (*cb) (void*)) {
	while(queue->len){
        if (cb != NULL) {
            cb(ListQueue_poll(queue));
        }
	}

    free(queue);
}



void ThreadPool_shutdown(ThreadPool* pool) {

}

Runnable* Runnable_dump(Runnable* runnable) {
    Runnable* tmpRunnable = (Runnable*) malloc(sizeof(Runnable));
    memset(tmpRunnable, 0, sizeof(Runnable));
    *tmpRunnable = *runnable;
    return tmpRunnable;
}

void ThreadPool_addThread() {

}

void ThreadPool_destoryThread() {

}

void ThreadPool_submit(ThreadPool* pool, Runnable* runnable) {
    Runnable* tmpRunnable = Runnable_dump(runnable);
    if (pool->workingPoolSize >= pool->alivePoolSize && pool->workingPoolSize < pool->mMaximumPoolSize) {
        ThreadPool_addThread();
    }
    ListQueue_enqueue(pool->mJobs, (void*)tmpRunnable);

}

long currentTime(){
    return 0;
}

void ThreadPool_do(ThreadNode* threadNode) {
    ThreadPool* pool = threadNode->pool;
    pthread_mutex_lock(&(pool->thoopSizeMutex));
    pool->alivePoolSize++;
    pthread_mutex_unlock(&(pool->thoopSizeMutex));
    while(1) {
        /*当队列中没有任务时阻塞*/
        sem_wait(&pool->mJobs->count);
        pthread_mutex_lock(&(pool->thoopSizeMutex));
        pool->workingPoolSize++;
        pthread_mutex_unlock(&(pool->thoopSizeMutex));
        Runnable* runnable = (Runnable*)ListQueue_poll(pool->mJobs);
        if (runnable != NULL) {
            runnable->run(runnable->data);
            //threadNode->lastTaskFinishTime = ;
        } else {
            if (currentTime() - threadNode->lastTaskFinishTime >= pool->mKeepAliveTime &&
                pool->alivePoolSize > pool->mCorePoolSize) {
                threadNode->isThreadEnd = 1;
            }
        }

        /*超过keepAliveTime，同时活动的线程超过核心线程，退出 */
        pthread_mutex_lock(&(pool->thoopSizeMutex));
        pool->workingPoolSize--;
        pthread_mutex_unlock(&(pool->thoopSizeMutex));
        if (threadNode->isThreadEnd == 1) {
            break;
        }

    }
    pthread_mutex_lock(&(pool->thoopSizeMutex));
    pool->alivePoolSize--;
    pthread_mutex_unlock(&(pool->thoopSizeMutex));

}

void ThreadNode_init(ThreadPool* pool, int id, ThreadNode* threadNode) {
        threadNode->id = id;
        pthread_create(&threadNode->thread, NULL, ThreadPool_do, (void*)threadNode);
        pthread_detach(threadNode->thread);
}

void ThreadPool_init(ThreadPool* pool) {
    pthread_mutex_init(&(pool->thoopSizeMutex), NULL);
    for (int i=0; i< pool->mCorePoolSize; i++) {
        pool->threads[i].id = i;
        pthread_create(&pool->threads[i].thread, NULL, ThreadPool_do, (void*)pool);
        pthread_detach(pool->threads[i].thread);
    }

}


ThreadPool* ThreadPool_create(int corePoolSize, int maximumPoolSize, long keepAliveTime) {
    if (corePoolSize < maximumPoolSize) {
        return NULL;
    }
    ThreadPool* pool = (ThreadPool*) malloc(sizeof(ThreadPool));
    memset(pool, 0, sizeof(ThreadPool));
    pool->mCorePoolSize = corePoolSize;
    pool->mMaximumPoolSize = maximumPoolSize;
    pool->mKeepAliveTime = keepAliveTime;
    pool->mJobs = ListQueue_create();
    pool->threads = (ThreadNode*) malloc(sizeof(ThreadNode) * maximumPoolSize);
    memset(pool->threads, 0, sizeof(ThreadNode) * maximumPoolSize);
    return pool;
}

void ThreadPool_destory(ThreadPool* pool) {
    
}


