#pragma once
#include <iostream>

#include "usbd_cdc_if.h"

// ansi color codes
#define RESET "\x1B[0m"
#define BOLD "\x1B[1m"
#define UNDERLINE "\x1B[4m"
#define REVERSED "\x1B[7m"
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define WHITE "\x1B[37m"

#define VERBOSITY(v) Debug::getInstance().setVerbosity(v);

#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)
#define DEBUG_PREFIX(sys, style) style "[" sys "][" __FILE__ ":" STRINGIFY(__LINE__) "] " RESET

#define ERROR(sys, ...) Debug::getInstance().error(DEBUG_PREFIX(sys, RED), __VA_ARGS__);

#define WARNING(sys, ...) Debug::getInstance().print(DEBUG_PREFIX(sys, YELLOW), __VA_ARGS__);

#define DEBUG_OUT(sys, style, ...) \
  Debug::getInstance().debug_out(DEBUG_PREFIX(sys, style), __VA_ARGS__);

#define PRINT(...) Debug::getInstance().print(__VA_ARGS__);

enum class Verbosity : uint8_t { SILENT, VERBOSE };

class Debug {
 private:
  Debug() = default;
  ~Debug() = default;
  Verbosity verbosity = Verbosity::SILENT;

 public:
  Debug(const Debug&) = delete;
  Debug& operator=(const Debug&) = delete;

  static Debug& getInstance() {
    static Debug d;
    return d;
  }

  void setVerbosity(Verbosity v) { verbosity = v; }

  static void append(std::string& out, std::string_view s) { out += s; }

  template <typename... Args>
  void error(Args&&... args) {
    std::string msg;
    (append(msg, std::string_view{std::forward<Args>(args)}), ...);
    const std::string result = "ERROR: " + msg;
    CDC_Transmit_FS((uint8_t*)result.data(), static_cast<uint16_t>(result.size()));
  }

  template <typename... Args>
  void debug_out(Args&&... args) {
    if (verbosity == Verbosity::VERBOSE) {
      std::string msg;
      (append(msg, std::string_view{std::forward<Args>(args)}), ...);
      const std::string result = "DEBUG: " + msg;
      CDC_Transmit_FS((uint8_t*)result.data(), static_cast<uint16_t>(result.size()));
    }
  }

  template <typename... Args>
  void print(Args&&... args) {
    std::string msg;
    (append(msg, std::string_view{std::forward<Args>(args)}), ...);
    CDC_Transmit_FS((uint8_t*)msg.data(), static_cast<uint16_t>(msg.size()));
  }
};