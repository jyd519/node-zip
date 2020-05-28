#ifndef ZIP_COMMON_H
#define ZIP_COMMON_H

#pragma once
#include <memory>
#include <string>

#include <mz.h>
#include <mz_os.h>
#include <mz_strm.h>
#include <mz_zip.h>
#include <mz_zip_rw.h>

namespace ziputil {
class ZipException : public std::exception {
 public:
  ZipException(int code, const std::string& message);
  virtual const char* what() const noexcept { return message_.c_str(); };

 private:
  int code_ = -1;
  std::string message_;
};

class MzReaderHandle {
 public:
  MzReaderHandle() : reader(nullptr) { mz_zip_reader_create(&reader); };
  ~MzReaderHandle() {
    if (reader) {
      mz_zip_reader_delete(&reader);
    }
  }

  MzReaderHandle(const MzReaderHandle&) = delete;
  MzReaderHandle& operator=(const MzReaderHandle&) = delete;

  MzReaderHandle(MzReaderHandle&& other) noexcept
      : reader{std::exchange(other.reader, nullptr)} {}

  MzReaderHandle& operator=(MzReaderHandle&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    if (reader) {
      mz_zip_reader_delete(&reader);
    }

    reader = std::exchange(other.reader, nullptr);
    return *this;
  }

  operator void*() { return reader; }

 private:
  void* reader;
};

class MzWriterHandle {
 public:
  MzWriterHandle() : writer(nullptr) { mz_zip_writer_create(&writer); }

  ~MzWriterHandle() {
    if (writer) {
      mz_zip_writer_delete(&writer);
    }
  }

  MzWriterHandle(const MzWriterHandle&) = delete;
  MzWriterHandle& operator=(const MzWriterHandle&) = delete;

  MzWriterHandle(MzWriterHandle&& other)
      : writer{std::exchange(other.writer, nullptr)} {}

  MzWriterHandle& operator=(MzWriterHandle&& other) {
    if (this == &other) {
      return *this;
    }
    if (writer) {
      mz_zip_writer_delete(&writer);
    }
    writer = std::exchange(other.writer, nullptr);
    return *this;
  }

  operator void*() { return writer; }

 private:
  void* writer;
};

}  // namespace ziputil
#endif /* ifndef ZIP_COMMON_H */
