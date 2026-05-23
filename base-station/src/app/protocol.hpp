#pragma once

namespace base::protocol {

class ProtocolHandler {
 public:
  ProtocolHandler() = default;
  ~ProtocolHandler() = default;

  // delete copy and move
  ProtocolHandler(const ProtocolHandler&) = delete;
  ProtocolHandler& operator=(const ProtocolHandler&) = delete;
  ProtocolHandler(ProtocolHandler&&) = delete;
  ProtocolHandler& operator=(ProtocolHandler&&) = delete;

  // fsm
  // get state

 private:
  // state
};

}  // namespace base::protocol