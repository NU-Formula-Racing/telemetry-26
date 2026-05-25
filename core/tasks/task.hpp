#pragma once

#include <FreeRTOS.h>
#include <etl/vector.h>
#include <task.h>

#include <cstdint>
#include <variant>

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
  SMALL = 128,
  MEDIUM = 256,
  LARGE = 512,
  XLARGE = 1600,
};

struct TaskConfig {
  const char* name;
  TaskPriority priority;
  uint32_t periodMs;
  IJob& job;
};

// generic wrapper for FreeRtosTasks
class AbstractTask {
 public:
  AbstractTask() = default;
  virtual ~AbstractTask() = default;

  AbstractTask(const AbstractTask&) = delete;
  AbstractTask& operator=(const AbstractTask&) = delete;
  AbstractTask(AbstractTask&&) = default;
  AbstractTask& operator=(AbstractTask&&) = default;

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

  // delete copy
  FreeRtosTask(const FreeRtosTask&) = delete;
  FreeRtosTask& operator=(const FreeRtosTask&) = delete;
  FreeRtosTask(FreeRtosTask&&) = default;
  FreeRtosTask& operator=(FreeRtosTask&&) = default;

  void start() override {
    handle_ = xTaskCreateStatic(FreeRtosTask::entryPoint,  // ptr to function that implements task
                                config_.name,              // task name (for debugging)
                                static_cast<uint32_t>(StackSize),            // stack size in words
                                this,                                        // task parameters
                                static_cast<UBaseType_t>(config_.priority),  // task priority
                                stackBuffer_,                                // memory for stack
                                &taskBuffer_                                 // memory for tcb
    );
  }

 private:
  static void entryPoint(void* params) {
    auto task = static_cast<FreeRtosTask*>(params);

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
using FreeRtosTaskVariant =
    std::variant<FreeRtosTask<TaskStackSize::SMALL>, FreeRtosTask<TaskStackSize::MEDIUM>,
                 FreeRtosTask<TaskStackSize::LARGE>, FreeRtosTask<TaskStackSize::XLARGE>>;
class TaskManager {
 public:
  TaskManager() = default;
  ~TaskManager() = default;

  TaskManager(const TaskManager&) = delete;
  TaskManager& operator=(const TaskManager&) = delete;
  TaskManager(TaskManager&&) = delete;
  TaskManager& operator=(TaskManager&&) = delete;

  template <TaskStackSize StackSize>
  bool addTask(FreeRtosTask<StackSize>&& task) {
    if (tasks_.size() < maxTasks_) {
      tasks_.emplace_back(std::in_place_type<FreeRtosTask<StackSize>>, std::move(task));
      return true;
    }
    return false;
  }

  void startAllTasks() {
    for (auto& task : tasks_) {
      std::visit([](auto& t) { t.start(); }, task);
    }
  }

 private:
  static constexpr size_t maxTasks_ = 10;
  etl::vector<FreeRtosTaskVariant, maxTasks_> tasks_;
};

}  // namespace tasks