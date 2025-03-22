#pragma once

#include <cstdint>
#include <type_traits>

using stage_mask_t = uint8_t;

enum class pipeline_stage : stage_mask_t {
  FETCH         = 0b00001,
  DECODE        = 0b00010,
  EXECUTE       = 0b00100,
  MEMORY_ACCESS = 0b01000,
  WRITE_BACK    = 0b10000
};


constexpr pipeline_stage operator|(pipeline_stage lhs, pipeline_stage rhs) {
  return static_cast<pipeline_stage>(static_cast<stage_mask_t>(lhs) 
                                     | static_cast<stage_mask_t>(rhs));
}

constexpr pipeline_stage operator&(pipeline_stage lhs, pipeline_stage rhs) {
  return static_cast<pipeline_stage>(static_cast<stage_mask_t>(lhs) 
                                     & static_cast<stage_mask_t>(rhs));
}

constexpr stage_mask_t operator|(stage_mask_t lhs, pipeline_stage rhs) {
  return lhs | static_cast<stage_mask_t>(rhs);
}

constexpr stage_mask_t operator&(stage_mask_t lhs, pipeline_stage rhs) {
  return lhs & static_cast<stage_mask_t>(rhs);
}

constexpr stage_mask_t to_mask(pipeline_stage stage) {
  return static_cast<stage_mask_t>(stage);
}

constexpr bool is_stage_in_mask(stage_mask_t mask, pipeline_stage stage) {
  return mask & to_mask(stage);
}
