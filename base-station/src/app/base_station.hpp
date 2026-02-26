#pragma once

namespace base {

class BaseStation {
 public:
  BaseStation() = default;
  ~BaseStation() = default;

  BaseStation(const BaseStation&) = delete;
  BaseStation& operator=(const BaseStation&) = delete;
  BaseStation(BaseStation&&) = delete;
  BaseStation& operator=(BaseStation&&) = delete;
};

}  // namespace base