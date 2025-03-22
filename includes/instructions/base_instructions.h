#pragma once

#include <array>
#include <tuple>
#include <variant>

#include "parser.h"
#include "pipeline/pipeline_stage.h"
#include "statement.h"

namespace instruction {
using namespace parser;

using element_t = parser::element_t;
using statement_t = statement::statement_t;
using iterator = parser::iterator;


template<typename Derived, size_t OperandCount>
struct base_instruction_t {
  using address_t = int;
  using operands_t = std::array<element_t, OperandCount>;

  operands_t operands;

  base_instruction_t() = default;
  explicit base_instruction_t(const operands_t& ops) 
    : operands(ops) {}

  static auto validate(iterator& it, iterator end) 
    -> std::tuple<bool, Derived> {
    operands_t ops;
    auto res = Derived::parse(it, end, ops);
    return std::make_tuple(res, Derived(ops));
  }

  bool execute_stage(pipeline_stage stage, statement_t& s) {
    auto* derived = static_cast<Derived*>(this);
    switch (stage) {
      case pipeline_stage::FETCH:
        derived->execute_fetch_impl(s, operands); 
        break;
      case pipeline_stage::DECODE:        
        derived->execute_decode_impl(s, operands); 
        break;
      case pipeline_stage::EXECUTE:
        derived->execute_execute_impl(s, operands); 
        break;
      case pipeline_stage::MEMORY_ACCESS:
        derived->execute_memory_access_impl(s, operands); 
        break;
      case pipeline_stage::WRITE_BACK:
        derived->execute_write_back_impl(s, operands);
        break;
    }
    return Derived::is_stage_applicable(stage);
  }

  static bool is_stage_applicable(pipeline_stage stage) {
    return static_cast<stage_mask_t>(stage)
      & Derived::applicable_stages;
  }

  static address_t resolve_operand(const statement_t& s, 
                                   const element_t& op) {
    return std::visit(
      [&](const auto& arg) -> address_t {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, parser::register_t>)
          return s.regs.x[arg.id];
        if constexpr (std::is_same_v<T, parser::immediate_t>)
          return arg.value;
        return 0;
    }, op);
  }

  static address_t resolve_label(const statement_t& s, 
                                 const element_t& op) {
    if (auto lbl = std::get_if<parser::label_t>(&op)) {
      auto it = s.labels.find(lbl->name);
      if (it != s.labels.end()) return it->second;
    }
    return 0;
  }

  bool branch_taken() const { return false; }

  template<typename Ops>
  static void execute_fetch_impl(statement_t&, Ops) {}
    
  template<typename Ops>
  static void execute_decode_impl(statement_t&, Ops) {}
    
  template<typename Ops>
  static void execute_execute_impl(statement_t&, Ops) {}
    
  template<typename Ops>
  static void execute_memory_access_impl(statement_t&, Ops) {}
    
  template<typename Ops>
  static void execute_write_back_impl(statement_t&, Ops) {}
};
}
