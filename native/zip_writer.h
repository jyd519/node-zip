#ifndef ZIP_WRITER_H
#define ZIP_WRITER_H

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "zip_common.h"

namespace ziputil {

bool ZipDir(const std::string& dir, const std::string& zipfile,
            const std::string& password);

struct FileInfo {
    void* data;
    size_t len;
    std::string comment;
};

class ZipWriter {
 public:
  ZipWriter() = default;
  ZipWriter(const std::string& filename, const std::string& password);
  ~ZipWriter();

  ZipWriter(const ZipWriter&) = delete;
  ZipWriter& operator=(const ZipWriter&) = delete;

  bool create(const std::string& filename, const std::string& password);
  bool close();

  bool is_open() const { return is_open_; }

  bool addDir(const std::string& dir, const std::string& rootPath, bool recursive=true);
  bool addFile(const std::string& path, const std::string& newname);
  bool addBuffer(const std::string& name, const FileInfo& buf);
 private:
  bool is_open_ = false;
  std::string password_;
  MzWriterHandle writer_;
};

}  // namespace ziputil

#endif
