#ifndef HEAPTIMER_H
#define HEAPTIMER_H

#include <iostream>
#include <vector>
#include <cassert>
#include <unordered_map>
#include <chrono>
#include <functional>

using std::cout;
using std::endl;
using std::vector;
using std::unordered_map;
using std::ostream;
using std::function;

using TimeoutCallback = function<void()>;
using Clock = std::chrono::high_resolution_clock;
using MilliSec = std::chrono::milliseconds; // millisecond
using TimePoint = Clock::time_point;

struct TimerNode {
    int fd; // file descriptor
    TimePoint expires;
    TimeoutCallback tcb; // timeout callback
    bool operator<(const TimerNode& tn) {
        return expires < tn.expires;
    }
};

/**
 * The heap is implemented using an array
 * e.g. root index is n, indexes of its left and right children is 2*n+1 and 2*n+2 respectively
*/
class HeapTimer {
public:
    HeapTimer();
    virtual ~HeapTimer();
    void Add(int fd, size_t expires, const TimeoutCallback& tcb);
    void Update(int fd, size_t newExpires);
    TimerNode Top();
    void Pop();
    void Print();
private:
    void SiftUp_(size_t idx); // param: index of heap_
    bool SiftDown_(size_t idx); // param: index, maxIdx: the last compared index
    void SwapTimerNode_(size_t i, size_t j); // param: i and j are indexes
    void Delete_(size_t idx);
private:
    vector<TimerNode> heap_;
    unordered_map<int, size_t> lookup_; // <fd, index>
};
#endif