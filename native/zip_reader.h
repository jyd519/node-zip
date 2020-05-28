#ifndef ZIP_READER_H
#define ZIP_READER_H

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "zip_common.h"

namespace ziputil {

struct ZipEntry {
  std::string name;
  std::string linkname;
  int64_t compressed_size;
  int64_t uncompressed_size;
  bool is_encrypted;
  bool is_directory;
  bool is_symlink;
  uint32_t crc;
  std::string comment;
  time_t modified_date; /* last modified date in unix time */
  time_t accessed_date; /* last accessed date in unix time */
  time_t creation_date; /* creation date in unix time */
};

class ZipReader {
 public:
  ZipReader() = default;
  explicit ZipReader(const std::string& filename);
  ~ZipReader();

  ZipReader(const ZipReader&) = delete;
  ZipReader& operator=(const ZipReader&) = delete;

  bool open(const std::string& filename, const std::string& password);
  void close();

  bool is_open() const { return is_open_; }

  bool exists(const std::string& filename);
  void setPassword(std::string password);
  size_t count() const { return entries_.size(); }
  const ZipEntry& item(size_t index) const { return entries_[index]; }
  const std::vector<ZipEntry>& entries() const { return entries_; };

  bool extractTo(const std::string& filename, const std::string& outDir);
  bool extractAs(const std::string& filename, const std::string& newname);
  bool readFile(const std::string& filename, std::string& data);
  size_t extractAll(const std::string& outDir, const std::string& pattern = "");

 private:
  MzReaderHandle reader_;
  bool is_open_ = false;
  std::string password_;
  std::vector<ZipEntry> entries_;
};

}  // namespace ziputil
#endif  // ZIP_READER_H
