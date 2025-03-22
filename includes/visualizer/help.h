#pragma once

#include <vector>
#include <memory>
#include "visualizer/visualizer.h"

struct help_t : visualizer::i_data_t {
  static std::shared_ptr<help_t> get() {
    static std::shared_ptr<help_t> instance 
      = std::make_shared<help_t>();
    return instance;
  }

  lines_t to_data() const {
    lines_t lines = {
      std::string("Controls: [q] Quit   [i] Change focus panel") + 
      "   [w] Scroll Up   [x] Scroll Down   [s] stepi   [c] continue"
    };

    return lines;
  }
};
