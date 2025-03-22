#pragma once

#include <memory>
#include <functional>
#include <array>
#include <algorithm>

#include "parser.h"
#include "statement.h" 
#include "pipeline/pipeline_stage.h"
#include "instructions/instructions.h"
#include "visualizer/visualizer.h"

namespace pipeline {
using instruction_t = instruction::match::variant_instructions;
using statement_t = instruction::statement_t;

struct pipeline_t : visualizer::i_data_t {
  static constexpr size_t n_stages = 5;

  std::array<std::shared_ptr<instruction_t>, n_stages> stages;

  pipeline_t(std::shared_ptr<statement_t> state) : state(state) {}

  void advance() {
    std::array<std::shared_ptr<instruction_t>, n_stages> next_stages;

    for (size_t i = n_stages - 1; i < n_stages; --i) {
      if (!stages[i]) continue;

      std::visit([&](auto& instr) {
        stage_mask_t remaining_stages = instr.applicable_stages 
          & static_cast<stage_mask_t>(~((1 << (i + 1)) - 1));

        if (remaining_stages == 0) return;
          
        size_t next_idx = std::countr_zero(remaining_stages);
          
        if (next_idx < n_stages && !next_stages[next_idx])
          next_stages[next_idx] = std::move(stages[i]);
        else 
          next_stages[i] = std::move(stages[i]);
      }, *stages[i]);
    }

    stages = std::move(next_stages);
  }

  bool execute_stages() {
    bool ok = true;
    for (size_t i = 0; i < n_stages; ++i) 
      ok &= execute_stage(i);
    return ok;
  }

  bool add_instr(instruction_t instr) {
    if (stages[0]) return false;
    stages[0] = std::make_shared<instruction_t>(std::move(instr));
    return true;
  }

  bool empty() const {
    return std::all_of(stages.cbegin(), stages.cend(), 
      [](const auto& stage) { return !stage; });
  }

  lines_t to_data() const {
    lines_t lines = {
      "FETCH         |", 
      "DECODE        |", 
      "EXECUTE       |", 
      "MEMORY_ACCESS |", 
      "WRITE_BACK    |" 
    };

    for (size_t i = 0; i < n_stages; ++i) {
      if (!stages[i]) continue;

      std::visit([&](auto& instr) {
        lines[i] += std::string(3 * i, ' ') + instr.mnemonic_id();
      }, *stages[i]);
    }
    return lines;
  }

private:
  std::array<std::function<void()>, n_stages> stage_executors = {
    [&]() { execute_stage_generic(0, pipeline_stage::FETCH); },
    [&]() { execute_stage_generic(1, pipeline_stage::DECODE); },
    [&]() { execute_stage_generic(2, pipeline_stage::EXECUTE); },
    [&]() { execute_stage_generic(3, pipeline_stage::MEMORY_ACCESS); },
    [&]() { execute_stage_generic(4, pipeline_stage::WRITE_BACK); }
  };

  bool execute_stage(size_t stage_idx) {
    if (stage_idx >= n_stages || !stages[stage_idx]) return true;
    
    stage_executors[stage_idx]();

    if (stage_idx != 4) return true;

    bool branch_taken = std::visit([](auto& stage) { 
      return stage.branch_taken(); }, *stages[stage_idx]);

    if (branch_taken) flush_pipeline();
    return !branch_taken;
  }

  void execute_stage_generic(size_t stage_idx, pipeline_stage stage) {
    if (stages[stage_idx])
      std::visit([&](auto& instr) { 
        instr.execute_stage(stage, *state); }, *stages[stage_idx]);
  }

  void flush_pipeline() {
    for (auto& stage : stages) stage = nullptr;
  }

  std::shared_ptr<statement_t> state;
};
};
