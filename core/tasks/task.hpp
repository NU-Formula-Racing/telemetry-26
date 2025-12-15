#ifndef __TASK_HPP__
#define __TASK_HPP__

#include <FreeRTOS.h>
#include <etl/vector.h>
#include <task.h>

#include <cstdint>

// #include "FreeRTOSConfig.h"
#include "job.hpp"

namespace tasks {

enum class TaskPriority : UBaseType_t {
  IDLE = tskIDLE_PRIORITY,
  LOW = configMAX_PRIORITIES / 4,
  STANDARD = configMAX_PRIORITIES / 2,
  HIGH = (configMAX_PRIORITIES * 3) / 4,
  CRITICAL = configMAX_PRIORITIES - 1,
};

// stack allocated per task (in words, NOT BYTES)
enum class TaskStackSize : uint32_t {
  VERY_SMALL = 256,
  SMALL = 512,
  MEDIUM = 1024,
  LARGE = 2048,
  VERY_LARGE = 4096,
};

struct TaskConfig {
  const char* name;
  TaskPriority priority;
  uint32_t periodMs;
  IJob& job;
};

// generic wrapper for FreeRtosTasks
// FreeRtosTasks can have potentially different underlying types due to templating
class AbstractTask {
 public:
  AbstractTask() = default;
  virtual ~AbstractTask() = default;

  AbstractTask(const AbstractTask&) = delete;
  AbstractTask& operator=(const AbstractTask&) = delete;
  AbstractTask(AbstractTask&&) = delete;
  AbstractTask& operator=(AbstractTask&&) = delete;

  virtual void start() = 0;
};

// FreeRTOS task implementation
// tasks are created statically with a fixed stack size given at compile time
// currently only periodic tasks are supported
template <TaskStackSize StackSize>
class FreeRtosTask final : public AbstractTask {
 public:
  FreeRtosTask(const TaskConfig& config) : config_(config) {}
  ~FreeRtosTask() override = default;

  // delete copy and move
  FreeRtosTask(const FreeRtosTask&) = delete;
  FreeRtosTask& operator=(const FreeRtosTask&) = delete;
  FreeRtosTask(FreeRtosTask&&) = delete;
  FreeRtosTask& operator=(FreeRtosTask&&) = delete;

  void start() override {
    handle_ = xTaskCreateStatic(FreeRtosTask::entryPoint,  // ptr to function that implements task
                                config_.name,              // task name (for debugging)
                                static_cast<uint32_t>(StackSize),  // stack size in words
                                this,                              // task parameters
                                config_.priority,                  // task priority
                                stackBuffer_,                      // memory for stack
                                &taskBuffer_                       // memory for tcb
    );
  }

 private:
  static void entryPoint(void* params) {
    FreeRtosTask* task = static_cast<FreeRtosTask*>(params);

    task->config_.job.init();
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t periodTicks = pdMS_TO_TICKS(task->config_.periodMs);

    while (true) {
      task->config_.job.run();
      vTaskDelayUntil(&lastWakeTime, periodTicks);
    }
  }

  TaskConfig config_;
  StaticTask_t taskBuffer_;
  StackType_t stackBuffer_[static_cast<uint32_t>(StackSize)];
  TaskHandle_t handle_ = nullptr;
};

// TaskManager class to manage multiple tasks
class TaskManager {
 public:
  bool addTask(AbstractTask* task) {
    if (tasks_.size() < maxTasks_) {
      tasks_.push_back(task);
      return true;
    }
    return false;
  }

  void startAllTasks() {
    for (auto& task : tasks_) {
      task->start();
    }
  }

 private:
  static constexpr size_t maxTasks_ = 20;
  etl::vector<AbstractTask*, maxTasks_> tasks_;
};

}  // namespace tasks

#endif  // __TASK_HPP__