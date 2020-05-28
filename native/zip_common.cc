#include "zip_common.h"

#include <fmt/core.h>

namespace ziputil {

ZipException::ZipException(int code, const std::string& message) : code_(code) {
  message_ = fmt::format("zip error: {} code: {}", message, code);
}

}  // namespace ziputil
