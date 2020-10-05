#ifndef BUFFER_H
#define BUFFER_H
#include <cassert>
#include <vector>
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <string.h>
using std::vector;

class Buffer {
public:
    Buffer(int initSize=1024);
    size_t WriteableBytes() const; // [writePos_, end)
    size_t ReadableBytes() const; // [readPos_, writePos_)
    size_t ReusableBytes() const; // [start, readPos_)
    
    char* GetWriteAddr();
    const char* GetWriteAddr() const;
    char* GetReadAddr();
    const char* GetReadAddr() const;

    std::string ReadAllToStr();
    void Write(const char* str, size_t len); // append a str of size len to buf_
    void Write(const std::string& str);
    void Write(const Buffer& buf);
private:
    void EnsureWriteable_(size_t len);
    void MakeSpace_(size_t len);
    void ClearAll_();
private:
    vector<char> buf_;
    size_t readPos_;
    size_t writePos_;
};
#endif