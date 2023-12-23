#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class ThreadSafeQueue {
    std::mutex mutex;
    std::condition_variable cond_var;
    std::queue<T> queue;

public:
    void push(T&& item) {
        {
            std::lock_guard lock(mutex);
            queue.push(item);
        }

        cond_var.notify_one();
    }

    T& front() {
        std::unique_lock lock(mutex);
        cond_var.wait(lock, [&]{ return !queue.empty(); });
        return queue.front();
    }

    void pop() {
        std::lock_guard lock(mutex);
        queue.pop();
    }

    bool try_pop(T& popped_item) {
        std::unique_lock lock(mutex);

        if (queue.empty()) {
            return false; // La file d'attente est vide
        }

        popped_item = std::move(queue.front());
        queue.pop();

        return true;
    }
};



#endif // THREAD_SAFE_QUEUE_H
