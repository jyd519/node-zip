#include "zip_writer_api.h"

#include "async_op.h"
#include "napi.h"
#include "zip_common.h"

namespace api {

using namespace ziputil;

class CreateZipAsync : public Napi::AsyncWorker {
 public:
  CreateZipAsync(Napi::Env env, std::string filename, std::string password)
      : Napi::AsyncWorker(env),
        deferred(Napi::Promise::Deferred::New(env)),
        filename_(std::move(filename)),
        password_(std::move(password)) {}
  ~CreateZipAsync() {}

  void Execute() override {
    w_ = std::make_unique<ZipWriter>();
    try {
      w_->create(filename_, password_);
    } catch (const std::exception& e) {
      SetError(e.what());
    }
  }

  // Executed when the async work is complete
  // this function will be run inside the main event loop
  // so it is safe to use JS engine data again
  void OnOK() override {
    Napi::HandleScope scope(Env());
    auto exp = Napi::External<ZipWriter>::New(Env(), this->w_.release());
    auto wrapper = ZipWriterAPI::NewInstance(Env(), {exp});
    deferred.Resolve(wrapper);
  }

  void OnError(Napi::Error const& error) override {
    deferred.Reject(error.Value());
  }

  Napi::Promise::Deferred deferred;
  std::unique_ptr<ZipWriter> w_;

 private:
  std::string filename_;
  std::string password_;
};

Napi::Value CreateZip(const Napi::CallbackInfo& info) {
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

  auto* wk = new CreateZipAsync(info.Env(), info[0].ToString(), password);
  wk->Queue();
  return wk->deferred.Promise();
}

// ZipWriterAPI
//

Napi::Object ZipWriterAPI::Init(Napi::Env env, Napi::Object exports, AddonData* addon_data) {
  Napi::HandleScope scope(env);

  exports.Set(Napi::String::New(env, "create"),
              Napi::Function::New(env, CreateZip));

  Napi::Function func =
      DefineClass(env, "ZipWriter",
                  {ZipWriterAPI::InstanceMethod("addDir", &ZipWriterAPI::addDir),
                   ZipWriterAPI::InstanceMethod("addFile", &ZipWriterAPI::addFile),
                   ZipWriterAPI::InstanceMethod("addBuffer", &ZipWriterAPI::addBuffer),
                   ZipWriterAPI::InstanceMethod("close", &ZipWriterAPI::close)}, nullptr);

  addon_data->ctor_writer = Napi::Persistent(func);
  return exports;
}

Napi::Object ZipWriterAPI::NewInstance(Napi::Env env, Napi::Value arg) {
  Napi::EscapableHandleScope scope(env);
  auto addon_data = env.GetInstanceData<AddonData>();
  auto obj = addon_data->ctor_writer.New({arg});
  return scope.Escape(napi_value(obj)).ToObject();
}

ZipWriterAPI::ZipWriterAPI(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<ZipWriterAPI>(info) {
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

  writer_.reset(info[0].As<Napi::External<ZipWriter>>().Data());
}

Napi::Value ZipWriterAPI::addDir(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  std::string dir = info[0].ToString();
  std::string root;
  bool recursive = true;
  if (info.Length() > 1 && info[1].IsString()) {
    root = info[1].ToString();
  }
  if (info.Length() > 2) {
    recursive = info[2].ToBoolean();
  }
  return MakePromise(env, [&, dir = std::move(dir), root = std::move(root)]() {
        auto ok = writer_->addDir(dir, root, recursive);
    return ok;
      });
}

Napi::Value ZipWriterAPI::addFile(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  std::string name = info[0].ToString();
  std::string name_in_zip;
  if (info.Length() > 1) {
    name_in_zip = info[1].ToString();
  }

  return MakePromise(env, [&, name = std::move(name), name_in_zip = std::move(name_in_zip)]() {
        auto ok = writer_->addFile(name, name_in_zip);
    return ok;
      });
}

class AddBufferAsync : public Napi::AsyncWorker {
 public:
  AddBufferAsync(Napi::Env env, Napi::Buffer<uint8_t>& data)
      : Napi::AsyncWorker(env),
        ref_(Napi::ObjectReference::New(data, 1)),
        deferred(Napi::Promise::Deferred::New(env)),
        dataPtr(data.Data()),
        dataLength(data.ByteLength()) {}
  ~AddBufferAsync() { ref_.Unref(); }

  void Execute() override {
    FileInfo b{0};
    b.data = dataPtr;
    b.len = dataLength;
    b.comment = comment;
    writer->addBuffer(name, b);
  }

  // Executed when the async work is complete
  // this function will be run inside the main event loop
  // so it is safe to use JS engine data again
  void OnOK() override {
    Napi::HandleScope scope(Env());
    deferred.Resolve(Napi::Value::From(Env(), true));
  }

  void OnError(Napi::Error const& error) override {
    deferred.Reject(error.Value());
  }

  ZipWriter* writer;
  Napi::Promise::Deferred deferred;
  std::string name, comment;

 private:
  Napi::ObjectReference ref_;
  uint8_t* dataPtr;
  size_t dataLength;
};

Napi::Value ZipWriterAPI::addBuffer(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (!info[0].IsString()) {
    Napi::Error::New(info.Env(), "Expected an String")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  if (!info[1].IsBuffer()) {
    Napi::Error::New(info.Env(), "Expected an Buffer")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  auto buf = info[1].As<Napi::Buffer<uint8_t>>();

  auto wk = new AddBufferAsync(env, buf);
  wk->writer = this->writer_.get();
  wk->name = info[0].ToString();
  if (info.Length() > 2) {
    wk->comment = info[2].ToString();
  }
  wk->Queue();
  return wk->deferred.Promise();
}

Napi::Value ZipWriterAPI::close(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  writer_->close();
  return Napi::Boolean::New(env, true);
}

}  // namespace api
