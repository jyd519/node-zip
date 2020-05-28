#include "zip_writer.h"

#include <assert.h>

#include "zip_common.h"

namespace ziputil {

bool ZipDir(const std::string& dir, const std::string& zipfile,
            const std::string& password) {
  ZipWriter w(zipfile, password);
  if (!w.is_open()) {
    return false;
  }
  return w.addDir(dir, "");
}

ZipWriter::ZipWriter(const std::string& filename, const std::string& password) {
  create(filename, password);
}

bool ZipWriter::create(const std::string& filename,
                       const std::string& password) {
  assert(!is_open_);
  if (is_open_) {
    return false;
  }

  password_ = password;
  mz_zip_writer_set_password(writer_, password_.c_str());
  mz_zip_writer_set_aes(writer_, 1);
  // mz_zip_writer_set_zip_cd(writer_, 1);
  int32_t err = mz_zip_writer_open_file(writer_, filename.c_str(), 0, 0);
  if (err != MZ_OK) {
    return false;
  }
  is_open_ = true;
  return true;
}

bool ZipWriter::close() {
  if (is_open_) {
    is_open_ = false;
    mz_zip_writer_close(writer_);
  }
  return true;
}

bool ZipWriter::addDir(const std::string& dir, const std::string& rootPath,
                       bool recursive) {
  int32_t err = mz_zip_writer_add_path(
      writer_, dir.c_str(), rootPath.empty() ? NULL : rootPath.c_str(),
      rootPath.empty() ? 1 : 0, recursive);
  if (err != MZ_OK) {
    throw ZipException(err, "Error adding path to archive");
  }
  return true;
}

bool ZipWriter::addFile(const std::string& path, const std::string& newname) {
  int32_t err = mz_zip_writer_add_file(
      writer_, path.c_str(), newname.empty() ? nullptr : newname.c_str());
  if (err != MZ_OK) {
    throw ZipException(err, "Error adding path to archive");
  }
  return true;
}

bool ZipWriter::addBuffer(const std::string& name, const FileInfo& buf) {
  mz_zip_file file_info = {0};
  file_info.filename = name.c_str();
  file_info.comment = buf.comment.empty() ? nullptr : buf.comment.c_str();
  file_info.modified_date = time(NULL);
  file_info.version_madeby = MZ_VERSION_MADEBY;
  file_info.compression_method = MZ_COMPRESS_METHOD_DEFLATE;
  file_info.aes_version = 1;
  file_info.flag = MZ_ZIP_FLAG_UTF8;
  int32_t err =
      mz_zip_writer_add_buffer(writer_, buf.data, buf.len, &file_info);
  if (err != MZ_OK) {
    throw ZipException(err, "Error adding data to archive");
  }
  return true;
}

}  // namespace ziputil
