#pragma once

#include <cstdint>
#include <unordered_map>
#include <array>
#include <vector>
#include <string>
#include <memory>
#include <cstdio>
#include <iomanip>
#include <sstream>

#include "visualizer/visualizer.h"
#include "lego/block.h"
#include "consts.h"

namespace statement {
struct stack_t : visualizer::i_data_t {
  std::array<reg_t, stack_size> stack{};
  size_t sp = 0;

private:
  uint32_t get(size_t offset) const {
    if (offset >= stack_size) 
      throw std::runtime_error("Stack overflow");
    return stack[offset];
  }

  void set(size_t offset, reg_t v) {
    if (offset >= stack_size) 
      throw std::runtime_error("Stack overflow");
    stack[offset] = v;
  }

public:
  reg_t top() const { return get(sp - 1); }
  reg_t pop() { return get(--sp); }
  void push(reg_t v) { set(sp++, v); }
  void swap() { 
    if (sp >= 2) std::swap(stack[sp - 1], stack[sp - 2]);
  }

  lines_t to_data() const {
    lines_t lines;
    lines.reserve(sp + 1);

    for (size_t i = stack_size; i > sp; --i) {
      std::ostringstream oss;
      oss << std::setw(2) << i - 1 << ": ---";
      lines.emplace_back(oss.str());
    }

    for (size_t i = sp; i != 0; --i) {
      std::ostringstream oss;
      oss << std::setw(2) << i - 1 << ": " 
          << hex(stack[i - 1]) << " | " << dec(stack[i - 1]);
      lines.emplace_back(oss.str());
    }
    return lines;
  }
};


template<size_t N>
struct memory_t : visualizer::i_data_t {
  std::array<uint8_t, N> mem;

  uint8_t operator[](int addr) {
    if (addr >= N) return 0;
    return mem[addr];
  }

  void set(reg_t addr, uint8_t v) {
    if (addr >= N) return;
    mem[addr] = v;
  }

  lines_t to_data() const {
    lines_t lines;
    lines.reserve(N / 4 + 1);

    for (size_t i = 0; i < N / 4; ++i) {
      std::ostringstream oss;
      oss << hex(reg_t(i * 4)) << ": ";

      for (size_t j = 0; j < 4; ++j)
        oss << hex(reg_t(mem[i * 4 + j])) << " ";
      lines.emplace_back(oss.str());
    }
    return lines;
  }
};

struct register_t { reg_t x[n_regs] = { 0 }; };

struct statement_t : visualizer::i_data_t {
  uint64_t pc = 0;
  reg_t lr;
  register_t regs;
  stack_t stack;
  std::shared_ptr<memory_t<128>> mem = 
    std::make_shared<memory_t<128>>();
  std::shared_ptr<map_blocks_t> map = 
    std::make_shared<map_blocks_t>();
  std::unordered_map<std::string, reg_t> labels;

  lines_t to_data() const {
    lines_t lines = stack.to_data();
    lines.emplace_back(std::string{});

    for (size_t i = 0; i < n_regs; ++i) {
      std::ostringstream oss;
      oss << "R" << i << ": " << hex(regs.x[i]) 
                      << " | " << dec(regs.x[i]);
      lines.emplace_back(oss.str());
    }
    return lines;
  }

  void print() {
    auto data = map->to_data();
    for (int i = 1; i <= 64; ++i) {
      std::cout << data[i-1];
      if (i % 8 == 0) std::cout << std::endl;
    }
  }
};
}
