#include "ThreadPool.h"


enum stat_code {
    TP_OK                   = 0,
};


typedef struct _ThreadPool{

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
        cb(item->item);
    }
    free(item);
}

ListQueue* ListQueue_create() {
    ListQueue* queue = (ListQueue*) malloc(sizeof(ListQueue));
    memset(queue, sizeof(ListQueue));
    pthread_mutex_init(&(queue->rwmutex), NULL);
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
        QueueItem_release(item);
    }
    end:
    pthread_mutex_unlock(&queue->rwmutex);
    return element;
}

enum stat_code ListQueue_enqueue(ListQueue *queue, void *element) {
    pthread_mutex_lock(&queue->rwmutex);
    QueueItem* item = QueueItem_create(element)
    if (queue->len == 0) {
        queue->rear = item;
        queue->front = item;
    } else {
        queue->rear->next = item;
        queue->rear = item;
    }
    queue->len++;
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

void ThreadPool_submit(ThreadPool* pool, Runnable runnable) {

}

void ThreadPool_do(ThreadPool* pool) {



}


ThreadPool* ThreadPool_create(int corePoolSize, int maximumPoolSize, long keepAliveTime) {

}

void ThreadPool_destory(ThreadPool* pool) {
    
}


