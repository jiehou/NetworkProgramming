#include "heaptimer.h"

HeapTimer::HeapTimer() {
    heap_.reserve(64); // default 64 timer nodes are allocated
}

HeapTimer::~HeapTimer() {}

void HeapTimer::SwapTimerNode_(size_t i, size_t j) {
    size_t n = heap_.size();
    if(n == 0 || i >=n || j >= n) return;
    if(i != j) {
        std::swap(heap_[i], heap_[j]);
        lookup_[heap_[i].fd] = i;
        lookup_[heap_[j].fd] = j;
    }
}

void HeapTimer::SiftUp_(size_t idx) {
    assert(idx >= 0 && idx < heap_.size());
    //cout << "[D] SiftUp_ idx: " << idx << endl;
    int j = (idx - 1) / 2;
    while (j >= 0) {
        if(heap_[j] < heap_[idx]) break;
        SwapTimerNode_(idx, j);
        idx = j; // store OLD j in idx
        j = (idx - 1) / 2; // compute NEW j
    }
}

/**
 * move the node at idx down until the maxIdx is reached
 * @return
 * true: the node at idx is moved down
 * false: the node at idx stays 
*/
bool HeapTimer::SiftDown_(size_t idx, size_t maxIdx) {
    assert(idx >= 0 && idx < maxIdx); 
    size_t i = idx;
    size_t j = 2 * i + 1; 
    while(j < maxIdx) {
        if(j+1 < maxIdx && heap_[j+1] < heap_[j]) j++; // if right child is smaller than left child, select the right one
        if(heap_[i] < heap_[j]) break;
        SwapTimerNode_(i, j);
        i = j; // store OLD j
        j = 2 * i + 1; // compute NEW j
    }   
    return (i > idx);
}

void HeapTimer::Delete_(size_t idx) {
    assert(idx >= 0 && idx < heap_.size());
    if(heap_.empty()) return;
    size_t n = heap_.size()-1; // maximum index
    if(n == 0) { // only one TimerNode exists
        lookup_.erase(heap_.back().fd);
        heap_.pop_back();
    }
    if(idx < n) {
        SwapTimerNode_(idx, n); // swap the last node with the node that will be deleted
        if(!SiftDown_(idx, n)) SiftUp_(idx); // in this case only consider amount of (heap_.size()-1) nodes 
        // delete last node
        lookup_.erase(heap_.back().fd);
        heap_.pop_back();
    }
}

/**
 * trigger the events whose expires are smaller than Clock::now()
 * find the next eveent whose expire is greater than Clock::now()
*/
void HeapTimer::Tick_() {
    if(heap_.empty()) return;
    while(!heap_.empty()) {
        TimerNode node = heap_.front();
        if(std::chrono::duration_cast<MilliSec>(node.expires - Clock::now()).count() > 0) 
            break;
        node.tcb();
        Pop();   
    }
}

void HeapTimer::Add(int fd, size_t expires, const TimeoutCallback& tcb) {
    assert(fd >= 0);
    size_t idx;
    if(lookup_.count(fd) == 0) { // fd does not exist in lookup_
        idx = heap_.size();
        lookup_[fd] = idx;
        heap_.push_back({fd, Clock::now() + MilliSec(expires), tcb});
        SiftUp_(idx);
    }
    else { // fd has already existed in lookup_
        idx = lookup_[fd];
        heap_[idx].expires = Clock::now() + MilliSec(expires);
        heap_[idx].tcb = tcb;
        if(!SiftDown_(idx, heap_.size())) SiftUp_(idx);
    }
}

void HeapTimer::Update(int fd, size_t newExpires) {
    assert(fd >= 0);
    if(heap_.empty()) return;
    if(lookup_.count(fd) > 0) {
        size_t idx = lookup_[fd];
        heap_[idx].expires = Clock::now() + MilliSec(newExpires);
        if(!SiftDown_(idx, heap_.size())) SiftUp_(idx);
    }
}

TimerNode HeapTimer::Top() {
    assert(!heap_.empty());
    return heap_[0];
}

void HeapTimer::Pop() {
    if(heap_.empty()) return;
    Delete_(0);
}

int HeapTimer::GetNextTick() {
    int ret = -1;
    Tick_();
    if(!heap_.empty()) {
        ret = std::chrono::duration_cast<MilliSec>(heap_.front().expires - Clock::now()).count();
        if(ret < 0) ret = 0;
    }
    return ret;
}