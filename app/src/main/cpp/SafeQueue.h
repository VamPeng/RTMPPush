//
// Created by Yuhui Peng on 2023/3/6.
//

#ifndef VPUSHER_SAFEQUEUE_H
#define VPUSHER_SAFEQUEUE_H

#include "queue"
#include "pthread.h"

using namespace std;

template<typename T>

class SafeQueue {
    typedef void (*ReleaseCallback)(T &);

public:
    SafeQueue() {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
    }

    ~SafeQueue() {
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&mutex);
    }

    void push(T new_value) {
        pthread_mutex_lock(&mutex);
        if (work) {
            q.push(new_value);
            pthread_cond_signal(&cond);
        } else {
            releaseCallback(new_value);
        }
        pthread_mutex_unlock(&mutex);
    }

    int get(T &value) {
        int ret = 0;
        pthread_mutex_lock(&mutex);
        while (work && q.empty()) {
            pthread_cond_wait(&cond, &mutex);
        }
        if (!q.empty()) {
            value = q.front();
            q.pop();
            ret = 1;
        }
        pthread_mutex_unlock(&mutex);
        return ret;
    }

    void setWork(int work) {
        pthread_mutex_lock(&mutex);
        this->work = work;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    void clear() {
        pthread_mutex_lock(&mutex);
        while (!q.empty()) {
            T value = q.front();
            releaseCallback(value);
            q.pop();
        }
        pthread_mutex_unlock(&mutex);
    }

    void setReleaseCallback(ReleaseCallback r){
        releaseCallback = r;
    }

private:
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int work = 0;
    queue<T> q;
    ReleaseCallback releaseCallback;
};


#endif //VPUSHER_SAFEQUEUE_H
