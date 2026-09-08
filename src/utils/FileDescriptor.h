#pragma once

#include <unistd.h>

class FileDescriptor {
  int fd_;

 public:
  explicit FileDescriptor(int fd) : fd_(fd) {}

  FileDescriptor(const FileDescriptor&) = delete;
  FileDescriptor& operator=(const FileDescriptor&) = delete;

  FileDescriptor(FileDescriptor&& other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
  }

  static FileDescriptor make_temp() {
    char filename[] = "/tmp/tlang.XXXXXX";
    int tmp_fd = mkstemp(filename);

    if (tmp_fd == -1) {
      throw std::runtime_error(
          fmt::format("Could not open file: {}.", strerror(errno)));
    }

    unlink(filename);

    return FileDescriptor(tmp_fd);
  }

  FileDescriptor& operator=(FileDescriptor&& other) noexcept {
    fd_ = other.fd_;
    other.fd_ = -1;

    return *this;
  }

  int get() const { return fd_; }

  ~FileDescriptor() {
    if (fd_ != -1) {
      close(fd_);
    }
  }
};
