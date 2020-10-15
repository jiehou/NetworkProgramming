#include "heaptimer.h"

HeapTimer::HeapTimer() {
    heap_.reserve(64); // default 64 timer nodes are allocated
}

HeapTimer::~HeapTimer() {}

void HeapTimer::SwapTimerNode_(size_t i, size_t j) {
    assert(i >= 0 && i < heap_.size()); // i
    assert(j >= 0 && j < heap_.size()); // j
    if(i != j) {
        std::swap(heap_[i], heap_[j]);
        lookup_[heap_[i].fd] = i;
        lookup_[heap_[j].fd] = j;
    }
}

void HeapTimer::SiftUp_(size_t idx) {
    assert(idx >= 0 && idx < heap_.size());
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
bool HeapTimer::SiftDown_(size_t idx) {
    assert(idx >= 0 && idx < heap_.size());
    size_t i = idx;
    size_t j = 2 * i + 1;
    size_t maxIdx = heap_.size() - 1;
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
    size_t n = heap_.size() - 1; // maximum index
    if(idx < n) {
        SwapTimerNode_(idx, n); // swap the last node with the node that will be deleted
        // delete last node
        lookup_.erase(heap_.back().fd);
        heap_.pop_back();
        if(!SiftDown_(idx)) SiftUp_(idx);
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
        if(!SiftDown_(idx)) SiftUp_(idx);
    }
}

void HeapTimer::Update(int fd, size_t newExpires) {
    assert(fd >= 0);
    if(heap_.empty()) return;
    if(lookup_.count(fd) > 0) {
        size_t idx = lookup_[fd];
        heap_[idx].expires = Clock::now() + MilliSec(newExpires);
        if(!SiftDown_(idx)) SiftUp_(idx);
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