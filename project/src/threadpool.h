#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
#include <cassert>
#include <future>
#include <iostream>

using std::mutex;
using std::condition_variable;
using std::function;
using std::queue;
using std::thread;
using std::unique_lock;
using std::move;
using std::cout;
using std::endl;

class ThreadPool {
public:
    explicit ThreadPool(size_t);
	// non-copyable
    ThreadPool(const ThreadPool&) = delete;
	// moveable
    ThreadPool(ThreadPool&&) = default;
    template<class F>
    void AddTask(F&& f);
    virtual ~ThreadPool();
private:
    vector<thread> workers_;
    queue<std::function<void()>> tasks_;
    std::condition_variable mcv_;
    std::mutex mtx_;
    size_t num_workers_;
    bool stop_;
};

// add new task (std::function<void()>) to the tasks queue of the threadpool
template<class F>
inline void ThreadPool::AddTask(F&& f) {
    {
        std::unique_lock<mutex> mlock(mtx_);
        if(stop_)
            throw std::runtime_error("AddTask error: threadpool has been stopped");
        tasks_.emplace(std::move(f));
    } // CRITICAL REGION
    mcv_.notify_all();
}

inline ThreadPool::ThreadPool(size_t num_workers) : num_workers_(num_workers), stop_(false) {
    for(size_t i = 0; i < num_workers_; ++i) {
        workers_.emplace_back([this]() {
            while (true) {
                std::function<void()> task;
                {   
                    std::unique_lock<mutex> mlock(mtx_);
                    mcv_.wait(mlock, [this]() { return this->stop_ || !this->tasks_.empty(); });
                    if(stop_) return;
                    if(!tasks_.empty()) {
                        //cout << "[I] tasks_.size(): " << tasks_.size() << endl;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    } // check tasks queue
                } // CRITICAL REGION
                task(); // execute task asyncronous
            } // endless loop
        });
    }
}

inline ThreadPool::~ThreadPool() {
    {
        std::unique_lock<mutex> mlock(mtx_);
        stop_ =  true;
    } // CRITICAL REGION
    mcv_.notify_all();
    for(auto& worker : workers_) worker.join();
}

#endif