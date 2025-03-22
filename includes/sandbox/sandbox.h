#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <unordered_set>

#include "consts.h"
#include "parser.h"
#include "statement.h"
#include "pipeline/pipeline.h"
#include "instructions/instructions.h"
#include "visualizer/visualizer.h"

struct sandbox_t : visualizer::i_data_t {
  using instruction_t = instruction::match::variant_instructions;
  using statement_t = instruction::statement_t;

  sandbox_t() 
    : pipeline(std::make_shared<pipeline::pipeline_t>(stmt)) {}

  void load(std::istream&& ss, const std::unordered_set<reg_t>& bps) {
    std::string line;
    size_t line_number = 0;
    uint64_t pc = 0;

    while (std::getline(ss, line)) {
      ++line_number;

      line = parser::prepare_input(line);
      if (line.empty()) continue;

      bool has_breakpoint = bps.find(line_number) != bps.end();
      if (has_breakpoint)
        breakpoints.insert(pc);
      
      std::string prefix = has_breakpoint ? "B " : "  ";

      auto it = line.cbegin();
      if (auto el = instruction::parse(it, line.cend()); el) {
        std::visit([this, line, &pc, prefix](auto&& el) {
          if constexpr (std::is_same_v<std::decay_t<decltype(el)>, instruction::lbl_t>) {
            if (stmt->labels.find(el.name()) != stmt->labels.end())
              errors.push_back("Label " + el.name() + " already defined");
            stmt->labels[el.name()] = pc;
            code.push_back(prefix + el.name() + ":");
          } else {
            code.push_back(prefix + "\t" + line);
            instructions.push_back(el);
            pc += 4;
          }
        }, *el);
      } else {
        errors.push_back("Error on line " + std::to_string(line_number));
      }
    }
   
    instructions.push_back(instruction::nop_t{});
  }


  bool stepi() {
    bool end = stmt->pc >= instructions.size() * 4;
    if (end && pipeline->empty()) return true;

    ++cycle;

    if (!pipeline->execute_stages()) return false;
    pipeline->advance();
    if (!end && pipeline->add_instr(instructions[stmt->pc / 4]))
      stmt->pc += 4;

    return pipeline->empty() && stmt->pc >= instructions.size() * 4;
  }

  bool stepi_until_breakpoint() {
    stepi();

    while (breakpoints.find(stmt->pc) == breakpoints.end() && !stepi());

    return pipeline->empty() && stmt->pc >= instructions.size() * 4;
  }

  std::vector<std::string> to_data() const { 
    std::vector<std::string> res = code;
    res.emplace_back("Cycle : " + std::to_string(cycle));
    res.emplace_back("PC : " + std::to_string(stmt->pc));

    return res; 

  }
  uint64_t pc() const { return stmt->pc; }

  void print() {
    stmt->print();
    std::cout << "Cycle: " << cycle << std::endl;
  }

  std::vector<std::string> code;
  std::vector<instruction_t> instructions;
  std::vector<std::string> errors;
  std::shared_ptr<statement_t> stmt = std::make_shared<statement_t>();
  std::shared_ptr<pipeline::pipeline_t> pipeline;
  std::unordered_set<uint32_t> breakpoints;

  uint32_t cycle = 0;
};
