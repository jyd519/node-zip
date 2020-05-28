#include <napi.h>

#include "zip_reader_api.h"
#include "zip_writer_api.h"

static Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports = api::InitReader(env, exports);
  exports = api::InitWriter(env, exports);
  return exports;
}


NODE_API_MODULE(nodezip, Init)
