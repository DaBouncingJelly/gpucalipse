#pragma once

#include <string>
#include <cstdint>

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using b8 = i8;
using b32 = i32;
using f32 = float;
using f64 = double;

enum class Status {
    Success = 0,
    Error = 1,
};

enum class Result {
    Success = 0,
    Error = 1,
    
    // Would be nice to have sth like
    // Error(String)
};