#pragma once

namespace tasks {

class IJob {
 public:
  IJob() = default;
  virtual ~IJob() = default;

  IJob(const IJob&) = delete;
  IJob& operator=(const IJob&) = delete;
  IJob(IJob&&) = delete;
  IJob& operator=(IJob&&) = delete;

  virtual void init() = 0;
  virtual void run() = 0;
};

}  // namespace tasks