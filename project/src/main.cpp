#include "logger.h"
#include "buffer.h"

int main() {
    std::string str = "hello world";
    Buffer buf;
    LOG_INFO("buf readablebyes: {}", buf.ReadableBytes());
    buf.Write(str);
    LOG_INFO("buf readablebyes: {}", buf.ReadableBytes());
    LOG_INFO("buf {}", buf.ReadAllToStr());
    LOG_INFO("buf {}", buf.ReadAllToStr());
    return 0;
}