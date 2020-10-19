#include "logger.h"
#include "buffer.h"
#include "threadpool.h"
#include "server.h"
#include <iostream>
using std::cout;
using std::endl;

void TestWorld();
void PrintHello();

void TestBuffer() {
    std::string str = "hello world";
    Buffer buf;
    LOG_INFO("buf readablebyes: {}", buf.ReadableBytes());
    buf.Write(str);
    LOG_INFO("buf readablebyes: {}", buf.ReadableBytes());
    LOG_INFO("buf {}", buf.ReadAllToStr());
    LOG_INFO("buf {}", buf.ReadAllToStr());
}

void TestWorld() {
    assert(0 > 100);
    cout << "World: " << 100 << endl;
}

void PrintHello() {
    cout << "Hello: " << 10 << endl;
    //assert(0 > 10);
}

void TestThreadPool() {
    ThreadPool tp{1};
    tp.AddTask(move(PrintHello));
    tp.AddTask(move(TestWorld));
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

const int kHalfMinute = 30000;

int main() {
    Server server(2020, kHalfMinute*2, 4); // port, timeout, numThreads
    server.Start();
    return 0;
}