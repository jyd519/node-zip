#include "zip_reader_api.h"

#include <algorithm>
#include <exception>
#include <memory>
#include <type_traits>
#include <utility>

#include "fs_util.h"
#include "napi.h"
#include "zip_reader.h"
#include "async_op.h"

namespace api {

using namespace ziputil;

class OpenZipAsync : public Napi::AsyncWorker {
 public:
  OpenZipAsync(Napi::Env env, std::string filename, std::string password)
      : Napi::AsyncWorker(env),
        deferred(Napi::Promise::Deferred::New(env)),
        filename_(std::move(filename)),
        password_(std::move(password)) {}
  ~OpenZipAsync() {}

  void Execute() override {
    reader_ = std::make_unique<ZipReader>();
    try {
      reader_->open(filename_, password_);
    } catch (const std::exception& e) {
      SetError(e.what());
    }
  }

  // Executed when the async work is complete
  // this function will be run inside the main event loop
  // so it is safe to use JS engine data again
  void OnOK() override {
    Napi::HandleScope scope(Env());
    auto exp = Napi::External<ZipReader>::New(Env(), this->reader_.release());
    auto wrapper = ZipReaderAPI::NewInstance(Env(), {exp});
    deferred.Resolve(wrapper);
  }

  void OnError(Napi::Error const& error) override {
    deferred.Reject(error.Value());
  }

  Napi::Promise::Deferred deferred;
  std::unique_ptr<ZipReader> reader_;

 private:
  std::string filename_;
  std::string password_;
};

Napi::Value OpenZip(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string password;
  if (info.Length() > 1) {
    if (!info[1].IsString()) {
      Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
      return env.Null();
    }
    password = info[1].ToString();
  }

  auto* wk = new OpenZipAsync(info.Env(), info[0].ToString(), password);
  wk->Queue();
  return wk->deferred.Promise();
}

//
// ZipReaderAPI
//

Napi::Object ZipReaderAPI::Init(Napi::Env env, Napi::Object exports, AddonData* addon_data) {
  Napi::HandleScope scope(env);

  exports.Set(Napi::String::New(env, "open"),
              Napi::Function::New(env, OpenZip));

  Napi::Function func =
      DefineClass(env, "ZipReader",
                  {InstanceMethod("item", &ZipReaderAPI::item),
                   InstanceMethod("extract", &ZipReaderAPI::extract),
                   InstanceMethod("extract_all", &ZipReaderAPI::extractAll),
                   InstanceMethod("read", &ZipReaderAPI::readFile),
                   InstanceMethod("exists", &ZipReaderAPI::exists),
                   InstanceAccessor("count", &ZipReaderAPI::count, nullptr),
                   InstanceMethod("close", &ZipReaderAPI::close)}, nullptr);

  addon_data->ctor_reader = Napi::Persistent(func);
  return exports;
}

Napi::Value ZipReaderAPI::setPassword(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  std::string password = info[0].ToString();
  reader_->setPassword(std::move(password));
  return env.Undefined();
}

Napi::Value ZipReaderAPI::item(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  int idx = info[0].ToNumber();
  if (idx < 0 || idx >= reader_->count()) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  auto p = reader_->item(idx);
  auto obj = Napi::Object::New(env);
  obj.Set("name", p.name);
  obj.Set("is_encrypted", p.is_encrypted);
  obj.Set("is_directory", p.is_directory);
  obj.Set("is_symlink", p.is_symlink);
  obj.Set("comment", p.comment);
  if (p.is_symlink) {
    obj.Set("linkname", p.linkname);
  }
  obj.Set("uncompressed_size", p.uncompressed_size);
  obj.Set("modified_date", p.modified_date);
  return obj;
}

Napi::Value ZipReaderAPI::readFile(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  std::string name = info[0].ToString();
  auto op = [this, name = std::move(name)]() {
    std::string content;
    const std::lock_guard<std::mutex> lock(this->mu_);
    reader_->readFile(name, content);
    return content;
  };
  return MakePromise(env, op);
}

Napi::Value ZipReaderAPI::exists(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  std::string name = info[0].ToString();
  return Napi::Boolean::From(env, reader_->exists(name));
}

Napi::Value ZipReaderAPI::count(const Napi::CallbackInfo& info) {
  return Napi::Number::From(info.Env(), reader_->count());
}

Napi::Value ZipReaderAPI::ZipReaderAPI::extract(
    const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 2) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string name;
  std::string dst;
  if (info[0].IsNumber()) {
    int idx = info[0].ToNumber();
    if (idx < 0 || idx >= reader_->count()) {
      Napi::RangeError::New(env, "out of range").ThrowAsJavaScriptException();
      return env.Null();
    }
    name = reader_->item(idx).name;
  } else {
    name = info[0].ToString();
  }
  if (info[1].IsString()) {
    dst = info[1].ToString();
  } else if (info[1].IsObject()) {
    auto options = info[1].ToObject();
    std::string targetDir;
    std::string targetName = name;
    if (options.Has("target")) {
      targetDir = options.Get("target").ToString();
    }
    if (options.Has("name")) {
      targetName = options.Get("name").ToString();
    }
    dst = fs_util::join(targetDir, targetName);
  } else {
    Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
  }
  auto op = [this, name = std::move(name), dst = std::move(dst)]() {
    const std::lock_guard<std::mutex> lock(this->mu_);
    return reader_->extractAs(name, dst);
  };
  return MakePromise(env, op);
}

Napi::Value ZipReaderAPI::ZipReaderAPI::extractAll(
    const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  std::string dir = info[0].ToString();
  std::string pattern;
  if (info.Length() > 1) {
    pattern = info[1].ToString();
  }

  auto op = [this, outdir = std::move(dir), pattern = std::move(pattern)]() {
    const std::lock_guard<std::mutex> lock(this->mu_);
    return reader_->extractAll(outdir, pattern);
  };

  return MakePromise(env, op);
}

Napi::Value ZipReaderAPI::close(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  reader_->close();
  return env.Undefined();
}

ZipReaderAPI::ZipReaderAPI(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<ZipReaderAPI>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return;
  }
  if (!info[0].IsExternal()) {
    Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
    return;
  }

  reader_.reset(info[0].As<Napi::External<ZipReader>>().Data());
}

Napi::Object ZipReaderAPI::NewInstance(Napi::Env env, Napi::Value arg) {
  Napi::EscapableHandleScope scope(env);
  auto addon_data = env.GetInstanceData<AddonData>();
  auto obj = addon_data->ctor_reader.New({arg});
  return scope.Escape(napi_value(obj)).ToObject();
}

}  // namespace api
