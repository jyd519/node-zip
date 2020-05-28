#ifndef ZIP_WRITER_API_H
#define ZIP_WRITER_API_H

#pragma once

#include <napi.h>

#include "zip_writer.h"

namespace api {

class ZipWriterAPI : public Napi::ObjectWrap<ZipWriterAPI> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  static Napi::Object NewInstance(Napi::Env env, Napi::Value arg);

  ZipWriterAPI(const Napi::CallbackInfo& info);

 private:
  static Napi::FunctionReference constructor;
  Napi::Value addDir(const Napi::CallbackInfo& info);
  Napi::Value addFile(const Napi::CallbackInfo& info);
  Napi::Value addBuffer(const Napi::CallbackInfo& info);
  Napi::Value close(const Napi::CallbackInfo& info);
  std::unique_ptr<ziputil::ZipWriter> writer_;
};

Napi::Object InitWriter(Napi::Env env, Napi::Object exports);

}  // namespace api

#endif /* ifndef ZIP_WRITER_API_H */
