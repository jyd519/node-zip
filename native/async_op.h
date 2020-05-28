#pragma once

#include <type_traits>
#include <utility>

#include <napi.h>

template <typename Fn>
class AsyncOp : public Napi::AsyncWorker {
 public:
  typedef typename std::result_of<Fn()>::type R;

  AsyncOp(Napi::Env env, Fn f)
      : Napi::AsyncWorker(env),
        deferred(Napi::Promise::Deferred::New(env)),
        fn_(std::forward<Fn>(f)) {}
  ~AsyncOp() {}

  void Execute() override { result_ = std::move(fn_()); }

  // Executed when the async work is complete
  // this function will be run inside the main event loop
  // so it is safe to use JS engine data again
  void OnOK() override {
    Napi::HandleScope scope(Env());
    deferred.Resolve(Napi::Value::From(Env(), result_));
  }

  void OnError(Napi::Error const& error) override {
    deferred.Reject(error.Value());
  }

  Napi::Promise::Deferred deferred;

 private:
  R result_;
  Fn fn_;
};

template <typename Fn>
inline Napi::Promise MakePromise(Napi::Env env, Fn f) {
  AsyncOp<Fn>* wk = new AsyncOp<Fn>(env, std::forward<Fn>(f));
  wk->Queue();
  return wk->deferred.Promise();
}
