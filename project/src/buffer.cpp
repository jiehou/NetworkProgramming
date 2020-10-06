#include "buffer.h"

Buffer::Buffer(int initSize) : buf_(initSize), readPos_(0), writePos_(0) {}

size_t Buffer::ReadableBytes() const {
    return writePos_ - readPos_;
}

size_t Buffer::WriteableBytes() const {
    assert(buf_.size() >= writePos_);
    return buf_.size() - writePos_;
}

size_t Buffer::ReusableBytes() const {
    return readPos_;
}

char* Buffer::GetReadAddr() {
    return &buf_[0] + readPos_;
}

const char* Buffer::GetReadAddr() const {
    return &buf_[0] + readPos_;
}

char* Buffer::GetWriteAddr() {
    return &buf_[0] + writePos_;
}

const char* Buffer::GetWriteAddr() const {
    return &buf_[0] + writePos_;
}

void Buffer::Write(const char* str, size_t len) {
    assert(str && len > 0);
    EnsureWriteable_(len);
    std::copy(str, str+len, GetWriteAddr());
    writePos_ += len; //@NOTE: after appending, writePos_ should be moved to new position
}

void Buffer::Write(const std::string& str) {
    Write(str.data(), str.length());
}

void Buffer::Write(const Buffer& buf) {
    Write(buf.GetReadAddr(), buf.ReadableBytes());
}

std::string Buffer::ReadAllToStr() {
    std::string res(GetReadAddr(), ReadableBytes());
    ClearAll_();
    return res;
}

void Buffer::ClearAll_() {
    bzero(&buf_[0], buf_.size());
    readPos_ = 0;
    writePos_ = 0;
}

void Buffer::MakeSpace_(size_t len) {
    if(WriteableBytes() + ReusableBytes() < len) {
        buf_.resize(writePos_ + len);
    }
    else {
        char* start = &buf_[0];
        size_t readable = ReadableBytes();
        std::copy(start + readPos_, start + writePos_, start);
        readPos_ = 0;
        writePos_ = readPos_ + readable;
    }
}

void Buffer::EnsureWriteable_(size_t len) {
    if(WriteableBytes() < len) MakeSpace_(len);
    assert(WriteableBytes() >= len);
}

ssize_t Buffer::ReadFd(int fd, int* retErrno) {
    char lbuf[65535]; // local buffer
    struct iovec iov[2];
    size_t writeable = WriteableBytes();
    // readv use case
    iov[0].iov_base = GetWriteAddr();
    iov[0].iov_len = writeable;
    iov[1].iov_base = lbuf;
    iov[1].iov_len = sizeof(lbuf);
    // read contents from fd into two vectored buffers
    ssize_t len = readv(fd, iov, 2);
    if(len < 0) *retErrno = errno;
    else if(len < writeable) {
        writePos_ += len;
    }
    else {
        // append contents of lbuf into buf_
        writePos_ = buf_.size();
        Write(lbuf, len - writeable); 
    }
    return len;
}

ssize_t Buffer::WriteFd(int fd, int* retErrno) {
    size_t readable = ReadableBytes();
    ssize_t len = write(fd, GetReadAddr(), readable);
    if(len < 0) {
        *retErrno = errno;
        return len;
    }
    readPos_ += len;
    return len;
}