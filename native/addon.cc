#include "addon.h"

#include <napi.h>

#include "zip_reader_api.h"
#include "zip_writer_api.h"

// It creates and initializes an instance of the
// `AddonData` structure and ties its lifecycle to that of the addon instance's
// `exports` object. This means that the data will be available to this instance
// of the addon for as long as the JavaScript engine keeps it alive.
static AddonData* CreateAddonData(Napi::Env env, napi_value /* exports */) {
  AddonData* result = new AddonData();
#if NAPI_VERSION > 5  
  env.SetInstanceData(result);
#else
  auto terminate = [](void* data) { delete reinterpret_cast<AddonData*>(data); };
  napi_add_env_cleanup_hook(env, terminate, result);
#endif
  return result;
}

static Napi::Object Init(Napi::Env env, Napi::Object exports) {
  AddonData* addon_data = CreateAddonData(env, exports);
  api::ZipReaderAPI::Init(env, exports, addon_data);
  api::ZipWriterAPI::Init(env, exports, addon_data);
  return exports;
}

NODE_API_MODULE(nodezip, Init)
