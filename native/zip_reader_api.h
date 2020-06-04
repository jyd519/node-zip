#ifndef ZIP_READER_API_H
#define ZIP_READER_API_H
#pragma once

#include <napi.h>

#include <thread>

#include "zip_reader.h"

namespace api {

class ZipReaderAPI : public Napi::ObjectWrap<ZipReaderAPI> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  static Napi::Object NewInstance(Napi::Env env, Napi::Value arg);

  ZipReaderAPI(const Napi::CallbackInfo& info);

 private:
  static Napi::FunctionReference constructor;
  Napi::Value setPassword(const Napi::CallbackInfo& info);
  Napi::Value item(const Napi::CallbackInfo& info);
  Napi::Value readFile(const Napi::CallbackInfo& info);
  Napi::Value exists(const Napi::CallbackInfo& info);
  Napi::Value count(const Napi::CallbackInfo& info);
  Napi::Value extract(const Napi::CallbackInfo& info);
  Napi::Value extractAll(const Napi::CallbackInfo& info);
  Napi::Value close(const Napi::CallbackInfo& info);
  std::unique_ptr<ziputil::ZipReader> reader_;
  std::mutex mu_;
};

Napi::Object InitReader(Napi::Env env, Napi::Object exports);

}  // namespace api
#endif /* ifndef ZIP_READER_API_H */