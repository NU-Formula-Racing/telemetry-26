#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace can {

enum class CanBaudRate : uint8_t { BAUD_125K, BAUD_250K, BAUD_500K, BAUD_1M };
enum class CanIdType : uint8_t { STANDARD, EXTENDED };
enum class CanEndianess : uint8_t { LITTLE, BIG };  // TODO

}  // namespace can