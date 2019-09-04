//
// Created by llm on 19-8-29.
//

#ifndef NATIVEPLAYER_SAFE_QUEUE_H
#define NATIVEPLAYER_SAFE_QUEUE_H

#include <queue>
#include <pthread.h>

using namespace std;

template <typename T>
class SafeQueue {
    typedef void (*ReleaseCallback) (T *);
    typedef void (*SyncHandle)(queue<T> &);

public:
    SafeQueue() {
        pthread_mutex_init(&mutex, 0);
        pthread_cond_init(&cond, 0);
    }

    ~SafeQueue() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    void push(T value) {   //
        pthread_mutex_lock(&mutex);

        if (work) {
            q.push(value);
            pthread_cond_signal(&cond);
        } else {
            if (releaseCallback) {
                releaseCallback(&value);
            }
        }

        pthread_mutex_unlock(&mutex);
    }

    int pop(T &value) {
        int ret = 0;

        pthread_mutex_lock(&mutex);
        while (work && q.empty()) {
            pthread_cond_wait(&cond, &mutex);   //FIXME: 会和clear造成死锁
        }
        if (!q.empty()) {
            value = q.front();
            q.pop();
            ret = 1;
        }

        pthread_mutex_unlock(&mutex);

        return ret;
    }

    int empty() {
        return q.empty();
    }

    int size() {
        return q.size();
    }

    void clear() {
        pthread_mutex_lock(&mutex);

        unsigned int size = q.size();
        for (int i = 0; i < size; i++) {
            T value = q.front();
            if (releaseCallback) {
                releaseCallback(&value);
            }

            q.pop(); //FIXME:是不是应该在releaseCall之前
        }

        pthread_mutex_unlock(&mutex);
    }

    /**
 * 设置队列的工作状态
 * @param work
 */
    void setWork(int work) {
        //先锁起来
        pthread_mutex_lock(&mutex);
        this->work = work;
        pthread_cond_signal(&cond);
        //解锁
        pthread_mutex_unlock(&mutex);
    }

    void setReleaseCallback(ReleaseCallback releaseCallback) {
        this->releaseCallback = releaseCallback;
    }

    void setSyncHandle(SyncHandle syncHandle) {
        this->syncHandle = syncHandle;
    }

    /**
 * 同步操作
 */
    void sync(){

        pthread_mutex_lock(&mutex);

        syncHandle(q);

        pthread_mutex_unlock(&mutex);
    }

private:
    queue<T> q;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    int work;
    ReleaseCallback releaseCallback;

    SyncHandle syncHandle;
};

#endif //NATIVEPLAYER_SAFE_QUEUE_H
