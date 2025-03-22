#pragma once

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <ncurses.h>

using reg_t = uint32_t;

struct hex {
  reg_t value;
  explicit hex(reg_t v) : value(v) {}

  friend std::ostream& operator<<(std::ostream& os, const hex& h) {
    os << "0x"
       << std::left << std::hex << std::setw(8) << std::setfill(' ')
       << h.value;
    return os;
  }
};

struct dec {
  reg_t value;
  explicit dec(reg_t v) : value(v) {}

  friend std::ostream& operator<<(std::ostream& os, const dec& d) {
    os << std::left <<  std::dec << std::setw(10) << std::setfill(' ') 
       << d.value;
    return os;
  }
};

enum { n_regs = 5 };
enum { stack_size = 32 };

enum block_color_cur_t : uint8_t {
  BLACK = 7,
  BLUE = 1,
  GREEN = 2,
  CYAN = 3,
  RED = 4,
  MAGENTA = 5,
  YELLOW = 6,
  WHITE = 0
};

const uint8_t blocks_color_t[8] = {
  COLOR_WHITE,
  COLOR_BLUE,
  COLOR_GREEN,
  COLOR_CYAN,
  COLOR_RED,
  COLOR_MAGENTA,
  COLOR_YELLOW,
  COLOR_BLACK
};

#define color_pair(color) COLOR_PAIR(color % 8 + 1)
