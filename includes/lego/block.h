#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <istream>

#include "consts.h"
#include "visualizer/visualizer.h"

struct block_t {
  uint8_t version;
  uint8_t width;    // 4 bits    8 bits
  uint8_t height;   // 4 bits    8 bits
  uint8_t dx;       // 4 bits    8 bits
  uint8_t dy;       // 4 bits    8 bits
  uint8_t color;    // 8 bits    8 bits
  
  block_t(const std::array<uint32_t, 6> data) {
    version = data[0];
    width = data[1] & 0x0F;
    height = data[2] & 0x0F;
    dx = data[3] & 0x0F;
    dy = data[4] & 0x0F;
    color = data[5] & 0xFF;
  }

  std::vector<uint8_t> decode() {
    switch (version) {
      case 1: return decode_v1();
      case 2: return decode_v2();
      default: throw std::runtime_error("Manual Unsupported version");
    }
  }

private:
  std::vector<uint8_t> decode_v2() {
    std::vector<uint8_t> res = {width, height, dx, dy, color};
    return res;
  }

  std::vector<uint8_t> decode_v1() {
    std::vector<uint8_t> res = {
      uint8_t(width << 4 | height), 
      uint8_t(dx << 4 | dy), 
      color};
    return res;
  }
};

struct blocks_t {
  std::vector<block_t> blocks;

  void load(std::istream&& input) {
    std::array<uint32_t, 6> row;
    size_t count = 1;

    uint32_t version;
    input >> version;
    
    if (version != 1 && version != 2) {
      throw std::runtime_error("Manual Unsupported version");
    }
    row[0] = version;

    for (uint32_t n; input >> n;) {
      row[count++] = n;

      if (count == 6) {
        blocks.emplace_back(row);
        count = 1;
        row[0] = version;
      }
    }

    if (count != 1) {
      throw std::runtime_error("Manual file is corrupted");
    }
  }

  template<typename C>
  void store(C& container) {
    size_t count = 0;

    for (auto& block : blocks) {
      for (const auto& d : block.decode()) {
        container->set(count++, d);
      }
    }
  }
};

struct map_blocks_t : visualizer::i_data_t {
  enum { n_blocks = 64 };

  std::array<block_color_cur_t, n_blocks> map;

  map_blocks_t() {
    for (size_t i = 0; i < n_blocks; ++i)
      map[i] = static_cast<block_color_cur_t>(0);
  }

  void set(size_t pos, uint32_t val) {
    for (; pos < n_blocks; ++pos) {
      uint32_t mask = static_cast<uint32_t>(map[pos]);
      map[pos] = static_cast<block_color_cur_t>(val | mask);
      val >>= 8;
    }
  }

  std::vector<std::string> to_data() const {
    std::vector<std::string> res;
    res.reserve(n_blocks);

    for (size_t i = 0; i < n_blocks; ++i)
      res.emplace_back(std::to_string(map[i]));

    return res;
  }
};
