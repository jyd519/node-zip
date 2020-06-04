#include "zip_reader.h"

#include <stdint.h>
#include <algorithm>

#include "fs_util.h"

namespace ziputil {

using namespace fs_util;

ZipReader::ZipReader(const std::string &filename) { open(filename, ""); }

ZipReader::~ZipReader() { close(); }

void ZipReader::close() {
  is_open_ = false;
  if (mz_zip_reader_is_open(reader_) == MZ_OK) {
    mz_zip_reader_close(reader_);
  }
}

bool ZipReader::open(const std::string &filename, const std::string &password) {
  mz_zip_file *file_info = NULL;
  int32_t err = MZ_OK;
  std::vector<ZipEntry> files;

  close();

  password_ = password;
  mz_zip_reader_set_password(reader_, password_.c_str());
  err = mz_zip_reader_open_file(reader_, filename.c_str());
  if (err != MZ_OK) {
    throw ZipException(err, "opening archive failed");
  }

  err = mz_zip_reader_goto_first_entry(reader_);
  if (err != MZ_OK && err != MZ_END_OF_LIST) {
    throw ZipException(err, "read archive failed");
  }

  /* Enumerate all entries in the archive */
  do {
    err = mz_zip_reader_entry_get_info(reader_, &file_info);
    if (err != MZ_OK) {
      throw ZipException(err, "read entry info failed");
    }

    files.emplace_back(ZipEntry{
        file_info->filename,
        file_info->linkname ? file_info->linkname : "",
        file_info->comment_size,
        file_info->uncompressed_size,
        (file_info->flag & MZ_ZIP_FLAG_ENCRYPTED) == MZ_ZIP_FLAG_ENCRYPTED,
        mz_zip_attrib_is_dir(file_info->external_fa, file_info->version_madeby) == MZ_OK,
        mz_zip_attrib_is_symlink(file_info->external_fa, file_info->version_madeby) == MZ_OK,
        file_info->crc,
        file_info->comment ? file_info->comment : "",
        file_info->modified_date,
        file_info->accessed_date,
        file_info->creation_date,
    });

    err = mz_zip_reader_goto_next_entry(reader_);
    if (err != MZ_OK && err != MZ_END_OF_LIST) {
      throw ZipException(err, "read entry info failed");
    }
  } while (err == MZ_OK);

  if (err != MZ_END_OF_LIST)
    throw ZipException(err, "read entry info failed");

  entries_ = std::move(files);
  is_open_ = true;
  return true;
}

bool ZipReader::exists(const std::string &filename) {
  return std::find_if(std::cbegin(entries_), std::cend(entries_), [&](auto &e) {
           return e.name.size() == filename.size() && mz_zip_path_compare(e.name.c_str(), filename.c_str(), 1) == 0;
         }) != entries_.end();
}

void ZipReader::setPassword(std::string password) {
  password_ = std::move(password);
  mz_zip_reader_set_password(reader_, password_.c_str());
}

bool ZipReader::extractTo(const std::string &filename,
                          const std::string &outDir) {
  return extractAs(filename, fs_util::join(outDir, filename));
}

size_t ZipReader::extractAll(const std::string &outDir,
                             const std::string &pattern) {
  size_t cnt = 0;
  std::for_each(entries_.cbegin(), entries_.cend(), [&](auto &p) {
    if (pattern.empty() ||
        mz_path_compare_wc(p.name.c_str(), pattern.c_str(), 1) == 0) {
      if (extractTo(p.name, outDir)) {
        ++cnt;
      }
    }
  });
  return cnt;
}

bool ZipReader::extractAs(const std::string &filename,
                          const std::string &newname) {
  int err = mz_zip_reader_locate_entry(reader_, filename.c_str(), 0);
  if (err == MZ_END_OF_LIST) {
    return false;
  }
  if (err != MZ_OK) {
    throw ZipException(err, "entry not found");
  }

  err = mz_zip_reader_entry_save_file(reader_, newname.c_str());
  if (err != MZ_OK) {
    throw ZipException(err, "save entry failed");
  }
  return true;
}

bool ZipReader::readFile(const std::string &filename, std::string &data) {
  mz_zip_reader_set_password(reader_, password_.c_str());
  int err = mz_zip_reader_locate_entry(reader_, filename.c_str(), 0);
  if (err == MZ_END_OF_LIST) {
    return false;
  }
  if (err != MZ_OK) {
    throw ZipException(err, "entry not found");
  }

  mz_zip_file *file_info = NULL;
  err = mz_zip_reader_entry_get_info(reader_, &file_info);
  if (err != MZ_OK) {
    throw ZipException(err, "read entry info failed");
  }

  std::vector<char> buf(file_info->uncompressed_size);
  err = mz_zip_reader_entry_save_buffer(reader_, buf.data(), buf.size());
  if (err != MZ_OK) {
    throw ZipException(err, "read entry data failed");
  }

  data = std::string(buf.begin(), buf.end());
  return true;
}

}  // namespace ziputil
