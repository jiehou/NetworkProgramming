#include <iostream>
#include <vector>
#include <cassert>
#include <unordered_map>

using std::cout;
using std::endl;
using std::vector;
using std::unordered_map;
using std::ostream;

struct TimerNode {
    int id; // file descriptor
    size_t expires;
    bool operator<(const TimerNode& tn) {
        return expires < tn.expires;
    }
};

ostream& operator<<(ostream& os, const TimerNode& tn) {
    os << "id: " << tn.id << ", expires: " << tn.expires;
    return os;
}

class HeapTimer {
public:
    HeapTimer();
    virtual ~HeapTimer();
    void Add(int id, size_t expires);
    void Update(int id, size_t timeout);
    TimerNode Top();
    void Pop();
    void Print();
private:
    void SiftUp_(size_t idx); // param: index
    bool SiftDown_(size_t idx, size_t maxIdx); // param: idx,  
    void SwapTimerNode_(size_t i, size_t j); // param: i and j are indexes
    void Delete_(size_t idx);
private:
    vector<TimerNode> heap_;
    unordered_map<int, size_t> lookup_; // <id, index>
};

HeapTimer::HeapTimer() { heap_.reserve(64); }

HeapTimer::~HeapTimer() { }

void HeapTimer::SwapTimerNode_(size_t i, size_t j) {
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    if(i != j) {
        std::swap(heap_[i], heap_[j]);
        lookup_[heap_[i].id] = i;
        lookup_[heap_[j].id] = j;
    }
}

void HeapTimer::SiftUp_(size_t idx) {
    assert(idx >= 0 && idx < heap_.size());
    //cout << "[D] SiftUp_ idx: " << idx << endl; 
    int j = (idx - 1) / 2; // parent index of i: (i-1)/2
    //cout << "[D] SiftUp_ j: " << j << endl;
    while(j >= 0) {
        if(heap_[j] < heap_[idx]) break;
        SwapTimerNode_(idx, j);
        idx = j;
        j = (idx - 1) >> 1;
    }
}

bool HeapTimer::SiftDown_(size_t idx, size_t maxIdx) {
    assert(idx >= 0 && idx < heap_.size());
    assert(maxIdx >= 0 && maxIdx <= heap_.size()); // maxIdx to be compared
    size_t i = idx;
    size_t j = (i << 1) + 1; // index of left child
    while(j < maxIdx) {
        if(j+1 < maxIdx && heap_[j+1] < heap_[j]) j++;
        if(heap_[i] < heap_[j]) break; // dont need to be swapped
        SwapTimerNode_(i, j);
        i = j;
        j = (i << 1) + 1;
    }
    return (i > idx);
}

void HeapTimer::Delete_(size_t idx) {
    assert(!heap_.empty() && idx >= 0 && idx < heap_.size());
    size_t i = idx;
    size_t n = heap_.size() - 1;
    if(i < n) {
        SwapTimerNode_(i, n);
        if(!SiftDown_(i, n)) {
            SiftUp_(i);
        }
    }
    // delete the element
    lookup_.erase(heap_.back().id);
    heap_.pop_back();
}

void HeapTimer::Add(int id, size_t expires) {
    assert(id >= 0);
    size_t i;
    if(lookup_.count(id) == 0) {
        i = heap_.size();
        lookup_[id] = i;
        heap_.push_back({id, expires});
        SiftUp_(i);
    }
    else {
        i = lookup_[id];
        heap_[i].expires = expires;
        if(!SiftDown_(i, heap_.size())) SiftUp_(i);
    }
}

void HeapTimer::Update(int id, size_t expires) {
    assert(!heap_.empty());
    if(lookup_.count(id) > 0) {
        size_t i = lookup_[id];
        heap_[i].expires = expires;
        if(!SiftDown_(i, heap_.size())) SiftUp_(i);
    }    
}

TimerNode HeapTimer::Top() {
    assert(!heap_.empty());
    return heap_[0];
}

void HeapTimer::Pop() {
    if(heap_.size() > 0) {
        Delete_(0);
    }
}

void HeapTimer::Print() {
    for(const auto& e : heap_) cout << e << ", idx: " << lookup_[e.id] << endl;     
}

int main() {
    /*
    // swap TimerNode
    TimerNode n1, n2;
    n1.id = 1; 
    n1.expires = 100;
    n2.id = 2;
    n2.expires = 200;
    cout << n1 << endl;
    cout << "[I] swap(n1, n2)\n";
    std::swap(n1, n2);
    cout << n1 << endl;
    */
    HeapTimer timer;
    timer.Add(1000, 100);
    timer.Add(1001, 75);
    timer.Add(1002, 95);
    cout << "[I] top: " << timer.Top() << endl;
    timer.Print();
    cout << "[I] add another node (1003, 60)\n";
    timer.Add(1003, 60);
    cout << "[I] top: " << timer.Top() << endl;
    timer.Print();
    cout << "[I] update the existing node (1003, 90)\n";
    timer.Update(1003, 90);
    cout << "[I] top: " << timer.Top() << endl;
    timer.Print();
    return 0;
} 