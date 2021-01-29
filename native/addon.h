#ifndef ADDON_H
#define ADDON_H

#include <napi.h>

// Holds per-Instance state
typedef struct {
  Napi::FunctionReference ctor_reader;
  Napi::FunctionReference ctor_writer;
} AddonData;

#endif //ADDON_H
